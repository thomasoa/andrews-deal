/*
 * Copyright (C) 1996-2001, Thomas Andrews
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This file allows the user to define holding procedures in general -
 * a step beyond the type allowed by 'defvector' in 'vector.c'
 *
 * The user simply defines a procedure, say:
 *
 * holdingProc solidsuit {A K Q J len} { 
 *     expr {($len>=7)&&$A&&$K&&$Q&&($J||($len>=8))}
 * }
 *
 * or
 *
 * holdingProc AKQpoints {A K Q} { expr {3*$A+2*$K+$Q } }
 *
 *
 * This, in turn, defines a normal Tcl procedure with the
 * same name and signature in the ::SlowHoldingProcedures
 * namespace:
 *
 * proc ::SlowHoldingProcedures::solidsuit {A K Q J len} {
 *     expr {($len>=7)&&$A&&$K&&$Q&&($J||($len>=8))}
 * }
 *
 * Finally, the user calls the procedure like:
 *
 *     AKQpts south
 *     AKQpts hand {AJ54 642 KQ64 95}
 *
 * or
 *
 *     solidsuit south clubs
 *     solidsuit holding AKQ7432
 *
 * This call checks a lookup table.  In the case of 'AKQpoints' above, the
 * lookup table consists of 8 elements.  In the case 'solidsuit,' the
 * table has 10*16 entries.  If the lookup entry is NULL, it
 * calls the SlowHoldingProcedure with the appropriate args.
 *
 * A benefit of using this method is that one holding procedure
 * can call another:
 *
 * holdingProc foo {A K Q len} { ... }
 *
 * holdingProc bar {A K Q J T len} {
 *      set f [foo $A $K $Q $len]
 *      ...
 * }
 *
 * For the curious, it is possible to dump these tables by calling them
 * without arguments:
 *
 *     puts [solidsuit dump]
 *
 * That's rather crude, and I might not support it in the future.
 *
 * By default, holdingProc assumes the type of the return value is an
 * integer, so if you say:
 *
 *    AKQpts south
 *
 * it computes the values for each suit and adds them up.
 *
 * But you can also declare the type.  For example, you might define:
 *
 *     holdingProc -double losers {A K Q J T 9 len} {
 *         ...
 *     }
 *
 * so that you could count partial losers.
 *
 * holdingProc -string ...
 *
 * defines a procedure which, when applied to a hand or multiple suits,
 * aggregates the results by putting them in a list.
 *
 * The 'boolean' type is the last one, and it is somewhat tricky.
 * It treats the return value as a boolean value.  When evaluating
 * one suit, it returns the boolean value, but when evaluating a hand
 * or several suits, it returns a list of the suits for which
 * the function was true.  For example, you might define:
 *
 * holdingProc -boolean biddableSuit {A K Q J T len} { ... }
 *
 * Then you call:
 *
 *     biddableSuit south spades
 *
 * to determine if south has biddable spades.  To get a
 * list of all biddable suits in the south hand, call:
 *
 *     biddableSuit south
 *
 * This would return {spades diamonds} if south had biddable
 * spade and diamond suits.
 *
 */
#include "tcl_incl.h"
#include "keywords.h"
#include "holdings.h"
#include "dealtypes.h"
#include "string.h"

#define LENGTH_ARGUMENT 13
#define STRING_ARGUMENT 14

extern int atoi(const char*);

/*
 * Aggregators are crude ways of encapsulating different operations
 * when "adding" the evaluations accross suits.  The aggregator
 * can add them together as integers or doubles, append them together
 * in a list or whatever.
 */
typedef struct _aggregation {
  int (*fn)(Tcl_Interp *interp,int objc, Tcl_Obj **objv,Tcl_Obj * CONST *suitnames);
} Aggregator;

/*
 * Structure contain a full holdingProc definition and data
 */
typedef struct holdingProcData {
  Tcl_Obj **lookupTable;  /* table of result objects */
  Aggregator *aggregator; /* What to do when processing multiple holdings */
  int tableLength;        /* size of lookupTable */
  int argumentCount;      /* # of args to pass */
  int sigBitCount;	      /* Number of cards from Ace to
				 smallest significant card */
  int fPassLength;		  /* Pass the length? */
  int signature[14];      /* Sequence of variables declared */
  Tcl_Obj *slowNameObj;   /* Name of the "slow" routine defined */
} *HoldingProcedure;

#if (TCL_MINOR_VERSION==0)
static int Tcl_EvalObjv(Tcl_Interp *interp,int objc,Tcl_Obj **objv,int dummy)
{
  Tcl_Obj *list=Tcl_NewListObj(objc,objv);

  if (list==NULL) { return TCL_ERROR; }

  return Tcl_GlobalEvalObj(interp,list);
}
#endif

/* Replace this with something which caches values 0..13 */
static Tcl_Obj *getIntObj(int val) {
  static Tcl_Obj *cached[8192];
  if (val<8192 && val>=0) {
    if (cached[val]==NULL) {
      Tcl_IncrRefCount(cached[val]=Tcl_NewIntObj(val));
    }
    return cached[val];
  } else { 
    return Tcl_NewIntObj(val);
  }
}

/*
 * Allocate and initialize the memory for a lookup table
 */
Tcl_Obj **allocateLookupTable(HoldingProcedure proc)
{
  int entries=proc->tableLength;
  int i;
  proc->lookupTable=(Tcl_Obj **)Tcl_Alloc(entries*sizeof(Tcl_Obj*));
  for (i=0; i<entries; i++) { proc->lookupTable[i]=NULL; }
  return proc->lookupTable;
}

/*
 * The namespace where slow procedures will be defined
 */
static const char *tclNamespace="SlowHoldingProcedures";

/*
 * Add up result values across several suits as integer values
 */
static int 
integerAggregator(
		  Tcl_Interp *interp,
		  int objc,
		  Tcl_Obj **objv,
		  Tcl_Obj * CONST *suits
		  )
{
  int i;
  int total=0;
  for (i=0; i<objc; i++) {
    int val;
    int retval;
    if (NULL==objv[i]) {
      return TCL_ERROR;
    }
    retval=Tcl_GetIntFromObj(interp,objv[i],&val);
    if (retval!=TCL_OK) {
      Tcl_SetResult(interp,
		    "Error in holdingProc integer aggregator: not an integer\n",
		    TCL_STATIC
		    );
      return retval;
    }
    total += val;
  }
  Tcl_SetObjResult(interp,getIntObj(total));
  return TCL_OK;
}

/*
 * doubleAggregator adds the results up as double values
 */
static int 
doubleAggregator(
		 Tcl_Interp *interp,
		 int objc,
		 Tcl_Obj **objv,
		 Tcl_Obj * CONST *suits
		 )
{
  int i;
  double total=0;
  for (i=0; i<objc; i++) {
    double val;
    int retval;
    if (NULL==objv[i]) {
      return TCL_ERROR;
    }
    retval=Tcl_GetDoubleFromObj(interp,objv[i],&val);
    if (retval!=TCL_OK) {
      Tcl_SetResult(interp,"\nError in double aggregator\n",TCL_STATIC);
      return retval;
    }
    total += val;
  }
  Tcl_SetObjResult(interp,Tcl_NewDoubleObj(total));
  return TCL_OK;
}

/*
 * The string aggregator creates a list of the resulting values
 */
static int 
stringAggregator(
		 Tcl_Interp *interp,
		 int objc,
		 Tcl_Obj **objv,
		 Tcl_Obj * CONST *suits
		 )
{
  int i;
  for (i=0; i<objc; i++) {
    if (NULL==objv[i]) {
      return TCL_ERROR;
    }
  }
  Tcl_SetObjResult(interp,Tcl_NewListObj(objc,objv));
  return TCL_OK;
}

/*
 * A complicated aggregator, booleanAggregator takes into account hte
 * suit names passed in, if any.  When suits are passed in, returns
 * the list of suit names for which the corresponding result was "true."
 */
static int
booleanAggregator(
		  Tcl_Interp *interp,
		  int objc,
		  Tcl_Obj **objv,
		  Tcl_Obj * CONST *suits
		  )
{
  int i;
  /*
   * If no suits were passed in, simply create a list of
   * results.
   */
  if (suits==NULL) {
    return stringAggregator(interp,objc,objv,suits);
  }

  /*
   * Make sure we're ok (why?)
   */
  for (i=0; i<objc; i++) {
    if (NULL==objv[i]) {
      return TCL_ERROR;
    }
  }

  /*
   * With only one value, just return the raw value - e.g.:
   *    foo north spades => 1
   *    foo north hearts => 0
   *    foo north diamonds => 1
   *    foo north clubs => 0
   * But
   *    foo north => "spades diamonds"
   */
  if (objc==1) {
    Tcl_SetObjResult(interp,objv[0]);
  } else {
    /*
     * Create and build the list
     */
    Tcl_Obj *result=Tcl_NewListObj(0,NULL);
    int value;
    for (i=0; i<objc; i++) {
      if (TCL_OK!=Tcl_GetBooleanFromObj(interp,objv[i],&value)) {
	Tcl_SetResult(interp,"Result was not a boolean\n",TCL_STATIC);
	return TCL_ERROR;
      }
      if (value) {
	if (TCL_OK!=Tcl_ListObjAppendElement(interp,result,suits[i])) {
	  return TCL_ERROR;
	}
      }
    }
    Tcl_SetObjResult(interp,result);
  }
  return TCL_OK;
}

/*
 * allocate the memory for a new holding procedure
 */
static HoldingProcedure allocateHoldingProcedure() {
  HoldingProcedure proc=
    (HoldingProcedure)Tcl_Alloc(sizeof(struct holdingProcData));
  proc->lookupTable=0;
  proc->argumentCount=0;
  proc->sigBitCount=0;
  proc->fPassLength=0;
  proc->signature[0]=-1;
  proc->slowNameObj=NULL;
  proc->aggregator=NULL;
  return proc;
}

/**
 * If holdingProc was called as:
 *
 *   holdingProc foo { a k q ... } { ... } 
 *
 * this routine defines a procedure:
 *
 *   proc ::SlowHoldingProc::foo { a k q ... } { ... }
 *
 * This is the "slow" procedure - it should be called at most
 * once per entry in the lookup table.
 *
 * Returns a Tcl_Obj* representing the name of the slow procedure.
 *
 */
static Tcl_Obj *
defSlowHoldingProc(
		   Tcl_Interp *interp,
		   Tcl_Obj *nameObj,
		   Tcl_Obj *argList,
		   Tcl_Obj *code
		   )
{
  int length;
  Tcl_Obj *objv[4];
  int retval;
	
  char *name=Tcl_GetStringFromObj(nameObj,&length);
	
  char *slowName;

  if (name==NULL) {
    return NULL;
  }

  slowName=(char *)Tcl_Alloc(4+length+strlen(tclNamespace));
  if (slowName==NULL) { return NULL; }

  sprintf(slowName,"%s::%s",tclNamespace,name);

  objv[0]=Tcl_NewStringObj("proc",4);
  objv[1]=Tcl_NewStringObj(slowName,strlen(slowName));
  objv[2]=argList;
  objv[3]=code;

  Tcl_Free(slowName);

  if (objv[1]==NULL) { return NULL; }

  Tcl_IncrRefCount(objv[1]);
  retval=Tcl_EvalObjv(interp,4,objv,TCL_EVAL_GLOBAL);

  if (retval!=TCL_OK) { Tcl_DecrRefCount(objv[1]); return NULL; }

  return objv[1];
}

static Aggregator integerAggr={integerAggregator};
static Aggregator doubleAggr={doubleAggregator};
static Aggregator stringAggr={stringAggregator};
static Aggregator booleanAggr={booleanAggregator};


/**
 * Return the index into the HoldingProcedure table given a
 * holding.
 */
static int
encode_holding(
	       HoldingProcedure procedure,
	       int holdingBits
	       )
{
  int realHolding=holdingBits&8191;
  int realbits=counttable[realHolding];
  int spots;
  int significantBits=realHolding >> (13 - procedure->sigBitCount);

  if (procedure->fPassLength) {
    int result;
    spots=realbits-counttable[significantBits];
    result=significantBits | (spots<<(procedure->sigBitCount));
    if (result>=procedure->tableLength) {
      return -1;
    }
    return result;
  } else {
    return significantBits;
  }
}

/*
 *  Simple delete procedure for a holding procedure.
 *  Should clean out lookup table before deleting.
 */
static void deleteHoldingProcedure(ClientData cd)
{
  HoldingProcedure proc=(HoldingProcedure)cd;
  if (proc->lookupTable!=NULL) {
    Tcl_Free((char*)proc->lookupTable);
  }
  Tcl_Free((char*)proc);
}

/*
 * Evaluate a holding procedure for a single holding
 */
static Tcl_Obj *
evalHoldingProcedure(
		     Tcl_Interp *interp,
		     HoldingProcedure procedure,
		     int holdingBits
		     )
{
  int retval;
  Tcl_Obj **lookupTable=procedure->lookupTable;

  /* Determine index in the lookup table */
  int lookupIndex=encode_holding(procedure,holdingBits);
  if (lookupIndex<0) {
    return NULL;
  }

  /* Allocate the table if it hasn't been allocated yet */
  if (lookupTable==NULL) {
    lookupTable=allocateLookupTable(procedure);
    if (lookupTable==NULL) {
      return NULL;
    }
  }

  /*
   * If the entry for this particular value has not already
   * been calculated, calculate it by calling original Tcl code
   */
  if (lookupTable[lookupIndex]==NULL) {
    int i;
	/* Build object list */
    int objc=1+procedure->argumentCount;
    Tcl_Obj **objv=(Tcl_Obj **)
      Tcl_Alloc(objc*sizeof(Tcl_Obj *));

    objv[0]=procedure->slowNameObj; /* slow procedure name */

	/* For every argument in the signature, add an appropriate
	 * argument to the list */
    for (i=1; i<objc; i++) {
      int card;
      switch (card=procedure->signature[i-1]) {

      case LENGTH_ARGUMENT:
	objv[i]=getIntObj(counttable[holdingBits&8191]);
	break;
	
      case STRING_ARGUMENT:
	{
	  
	  int stringSpots=(lookupIndex>>procedure->sigBitCount);
	  int fakeHolding=
	    (lookupIndex<<(13-procedure->sigBitCount))+((1<<stringSpots)-1);
	  
	  objv[i]=Tcl_NewHoldingObj(fakeHolding);
	}
	break;

      default:
	if (card<0 || card>12) {
	  Tcl_Free((char *)objv);
	  Tcl_AddErrorInfo(interp,"Card value out of range");
	  return NULL;
	}
	/*
	 * Set card parameter to zero or 1 depending on whether the
         * card is held in this holding
	 */
	objv[i]=getIntObj(1&(holdingBits>>(12-card)));
	break;
      }
    }

    /**
     * Once done, evaluate this list in a global context
     */
    retval=Tcl_EvalObjv(interp,objc,objv,TCL_EVAL_GLOBAL);
    Tcl_Free((char *)objv);
    if (retval!=TCL_OK) {
      return NULL;
    }

    lookupTable[lookupIndex]=Tcl_GetObjResult(interp);
    Tcl_IncrRefCount(lookupTable[lookupIndex]);
  }
  return lookupTable[lookupIndex];
}

/*
 * Determine a variables meaning by it's first letter(s)
 */
static int lookupArgumentName(Tcl_Obj *varName) {
  int length, rank;
  char *string=Tcl_GetStringFromObj(varName,&length);
  switch (*string) {
  case 'a':
  case 'A':
    return ACE;
  case 'k':
  case 'K':
    return KING;
  case 'Q':
  case 'q':
    return QUEEN;
  case 'J':
  case 'j':
    return JACK;
  case 'T':
  case 't':
    return TEN;
  case 'x':
  case 'X':
    rank=atoi(string+1);
    if (rank<2 || rank>13) {
      return -1;
    }
    return 14-rank;

  case 'l':
  case 'L':
    return LENGTH_ARGUMENT;

  case 's':
  case 'S':
    return STRING_ARGUMENT;
  default:
    return -1;
  }
}

static HoldingProcedure
newHoldingProcedure(
		    Tcl_Interp *interp,
		    Tcl_Obj *slowNameObj,
		    Tcl_Obj *argsList
		    )
{
  HoldingProcedure procedure=allocateHoldingProcedure();
  int argsLength;
  int retval,index;
  int flagArguments=0;
  Tcl_Obj *varName;
  int maxArg=-1;  /* Smallest significant card */

  if (procedure==NULL) { return NULL; }

  /* Determine how many arguments there are */
  retval=Tcl_ListObjLength(interp,argsList,&argsLength);
  if (retval!=TCL_OK) { Tcl_Free((char *)procedure); return NULL; }
  procedure->argumentCount=argsLength;

  /* Create the signature by looking up the arguments in the argument list */
  procedure->sigBitCount=0;
  for (index=0; index<argsLength; index++) {
    int argNum;
	/* Get argument name */
    retval=Tcl_ListObjIndex(interp,argsList,index,&varName);
    if (retval!=TCL_OK) { Tcl_Free((char *)procedure); return NULL; }

	/* Get argument ID based on name */
    argNum=lookupArgumentName(varName);
    if (argNum<0) { Tcl_Free((char *)procedure); return NULL; }

	/* Add argument ID to the signature */
    procedure->signature[index]=argNum;

	/* Make sure each argument type occurs only once (why?) */
    if (flagArguments&(1<<argNum)) {
      Tcl_Free((char *)procedure); return NULL;
    }
    flagArguments |= (1<<argNum);

	/*
	 * If the argument is a length argument or a string argument
	 * then we need to pass a length.
	 */
    if (argNum==LENGTH_ARGUMENT || argNum==STRING_ARGUMENT) {
      procedure->fPassLength=1;
    } else {
      if (argNum>maxArg) { maxArg=argNum; }
    }
  }
  /* Significant bits is one more than maxArg */
  maxArg++;
  procedure->sigBitCount=maxArg;

  /* Determine table length */
  procedure->tableLength=(1<<maxArg);
  if (procedure->fPassLength) {
    procedure->tableLength *= (14-maxArg);
  }

  /* Store the slow function name */
  Tcl_IncrRefCount(procedure->slowNameObj=slowNameObj);

  return procedure;
}

/*
 * Evaluate a set of holdings passed in an array
 * of integers.
 */ 
static int 
evalHoldingNums(
		Tcl_Interp *interp,
		HoldingProcedure procedure,
		int count,
		int *holdings,
		Tcl_Obj * CONST *suits
		)
{
  int i;
  int retval;
  Tcl_Obj **values=
    (Tcl_Obj **)Tcl_Alloc(count*sizeof(Tcl_Obj *));
  if (values==NULL) { return TCL_ERROR; }


  /*
   * Evaluate for each individual holding
   */
  for (i=0; i<count; i++) {
    int holding=holdings[i];
    if (holding<0) { 
      Tcl_Free((char *)values); 
      return TCL_ERROR;
    }

    values[i]=evalHoldingProcedure(interp,procedure,holding);
    if (values[i]==NULL) {
      Tcl_Free((char *)values);
      return TCL_ERROR;
    }

  }

  /*
   * Just return the value if only one.
   */
  if (count==1) {
    Tcl_SetObjResult(interp,values[0]);
    retval=TCL_OK;
  } else {
    /*
     * Call the aggregator on multiple values
     */
    retval=(procedure->aggregator->fn)(interp,count,values,suits);
  }
  Tcl_Free((char *)values);
  return retval;

}

static int 
IDealHoldingProcedure(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  static int subCmdInit=1,
    holdingSubCmdID=-1,
    handSubCmdID=-1,
    dumpSubCmdID=-1;

  int hand,subCmd;
  HoldingProcedure procedure=(HoldingProcedure)cd;

  if (subCmdInit) {
    handSubCmdID=Keyword_addKey("hand");
    holdingSubCmdID=Keyword_addKey("holding");
    dumpSubCmdID=Keyword_addKey("dump");
    subCmdInit=0;
  }

	

  if (objc>7 || objc<=1) {
    goto usage;
  }

  if (NOSEAT!=(hand=getHandNumFromObj(interp,objv[1]))) {
    FINISH(hand);
    if (objc==2) {
      return 
	evalHoldingNums(interp,
			procedure,
			4,globalDeal.hand[hand].suit,
			getAllSuitObjs());
    } else {
      int i,array[4];
      for (i=2; i<objc; i++) {
	int suit=getSuitNumFromObj(interp,objv[i]);

	if (suit==NOSUIT) { 
	  Tcl_AppendResult(interp,
			   "Expected suit name, got ",
			   Tcl_GetString(objv[i]),
			   NULL);
	  goto usage;
	}

	array[i-2]=globalDeal.hand[hand].suit[suit];

      }
      return evalHoldingNums(interp,procedure,objc-2,array,objv+2);
    }
  }

  subCmd=Keyword_getIdFromObj(interp,objv[1]);

  if (subCmd==dumpSubCmdID) {
    /* Dump the entire table of holdings and values */
    int holding;
    Tcl_Obj *newObjv[2];
    Tcl_Obj *pair;
    Tcl_Obj *list=Tcl_NewListObj(0,NULL);

    for (holding=0; holding<procedure->tableLength; holding++) {
      int spots=(holding>>procedure->sigBitCount);
      int fakeHolding=(holding<<(13-procedure->sigBitCount))+((1<<spots)-1);
      newObjv[1]=evalHoldingProcedure(interp,procedure,fakeHolding);
      newObjv[0]=Tcl_NewHoldingObj(fakeHolding);
      if (newObjv[1]==NULL) { 
	Tcl_AddErrorInfo(interp,"evalHoldingProcedure returned NULL"); 
	return TCL_ERROR;
      }
      if (newObjv[0]==NULL) {
	Tcl_AddErrorInfo(interp,"bad holding...\n");
	return TCL_ERROR;
      }
      pair=Tcl_NewListObj(2,newObjv);
      Tcl_ListObjAppendElement(interp,list,pair);
    }
    Tcl_SetObjResult(interp,list);
    return TCL_OK;
  }

  if (subCmd==handSubCmdID) {
    int allHoldingNums[4];
    int subsetHoldings[4];
    int *hnum;
    int countHoldings;
    Tcl_Obj * CONST *allSuits;
    Tcl_Obj *chosenSuits[4];
    Tcl_Obj * CONST *suits;

    if (objc<3 || objc>7) {
      Tcl_WrongNumArgs(interp,2,objv,"hand <hand> [<suit> ...]");
      goto usage;
    }

    if (TCL_OK!=getHandHoldingsFromObj(interp,objv[2],allHoldingNums)) {
      Tcl_SetResult(interp,"Badly formatted hand",TCL_STATIC);
      return TCL_ERROR;
    }
    allSuits = getAllSuitObjs();

    if (objc == 3) {
       hnum = allHoldingNums;
       countHoldings = 4;
       suits = allSuits;
     } else {
       int i;
       countHoldings = 0;
       for (i=3; i<objc; i++) {
	   int suit=getSuitNumFromObj(interp,objv[i]);
	   if (suit==NOSUIT) { 
	       Tcl_AppendResult(interp,
			   "Expected suit name, got ",
			   Tcl_GetString(objv[i]),
			   NULL);
                return TCL_ERROR;
	   }
           chosenSuits[countHoldings] = allSuits[suit];
           subsetHoldings[countHoldings] = allHoldingNums[suit];
           countHoldings++;
       }
       hnum = subsetHoldings;
       suits = chosenSuits;
    }

    return evalHoldingNums(interp,procedure,countHoldings,hnum,suits);
  }

  if (subCmd==holdingSubCmdID) {
    int hnum;

    if (objc!=3) { 
      Tcl_WrongNumArgs(interp,2,objv,"<holding>");
      return TCL_ERROR;
    }

    hnum=getHoldingNumFromObj(interp,objv[2]);
    return evalHoldingNums(interp,procedure,1,&hnum,NULL);
  }

 usage:
  /* error condition */
  {
    char *command;

    command=Tcl_GetString(objv[0]);

    Tcl_AppendResult(interp,command,
		     ": Bad first argument ",Tcl_GetString(objv[1]),
		     "\nUsage:\n", 
		     command," <handname> [<suitname> ...]\n",
		     command," hand <hand> [<suitname> ...]\n",
		     command," holding <holding> [<holding> ...]\n",
		     (char *)NULL);

    return TCL_ERROR;
  }
}

static int 
IDeal_DefHoldingProc(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  static int initKeywords=1,
    doubleFlag=-1,
    integerFlag=-1,
    stringFlag=-1,
    booleanFlag=-1;

  Tcl_Obj *nameObj, *argsList, *code;
  Tcl_Obj *slowNameObj;
  char *name;
  int len;
  HoldingProcedure procedure;
  Aggregator *aggr=&integerAggr;

  if (initKeywords) {
    doubleFlag=Keyword_addKey("-double");
    integerFlag=Keyword_addKey("-integer");
    stringFlag=Keyword_addKey("-string");
    booleanFlag=Keyword_addKey("-boolean");
    initKeywords=0;
  }

  if (objc!=4 && objc!=5) {
    goto usage;
  }
  
  nameObj=objv[1];
  name=Tcl_GetString(nameObj);
  if (*name=='-') {
    int flag=Keyword_getIdFromObj(interp,nameObj);
    if (flag==integerFlag) {
      aggr=&integerAggr;
    } else if (flag==doubleFlag) {
      aggr=&doubleAggr;
    } else if (flag==booleanFlag) {
      aggr=&booleanAggr;
    } else if (flag==stringFlag) {
      aggr=&stringAggr;
    } else {
      goto usage;
    }
    objc--;
    objv++;
  }

  nameObj=objv[1];
  argsList=objv[2];
  code=objv[3];

  slowNameObj=defSlowHoldingProc(interp,nameObj,argsList,code);
  if (slowNameObj==NULL) {
    return TCL_ERROR;
  }

  procedure=newHoldingProcedure(interp,slowNameObj,argsList);
  if (procedure==NULL) {
    return TCL_ERROR;
  }
  
  procedure->aggregator=(Aggregator *)aggr;
  name=Tcl_GetStringFromObj(nameObj,&len);
  Tcl_CreateObjCommand(interp,name,IDealHoldingProcedure,
		       (ClientData)procedure,deleteHoldingProcedure);

  return TCL_OK;
 usage:
  Tcl_SetResult(interp,
		"holdingProc [ -integer|-double|-string ] <name> <args> <code>",
		NULL);
  return TCL_ERROR;
}

/*
 * This routine implements the "holding" command, which implements
 * various subcommands for processing Deal holding objects.
 */
static int IDeal_HoldingCmd(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  static int lengthCmd,
    disjointCmd,
    unionCmd,
    randomCmd,
    listCmd,
    encodeCmd,
    decodeCmd,
    subsetCmd,
    matchesCmd,
    lengthFlag,
    spotFlag,
    initKeywords=1;
  int cmd;

  if (initKeywords) {

    lengthCmd=Keyword_addKey("length");
    disjointCmd=Keyword_addKey("disjoint");
    unionCmd=Keyword_addKey("union");
    randomCmd=Keyword_addKey("random");
    encodeCmd=Keyword_addKey("encode");
    decodeCmd=Keyword_addKey("decode");
    subsetCmd=Keyword_addKey("contains");
    matchesCmd=Keyword_addKey("matches");

    listCmd=Keyword_addKey("list");
    /* Flags for the list subcommand */
    lengthFlag=Keyword_addKey("-length");
    spotFlag=Keyword_addKey("-spot");

    initKeywords=0;
  }

  if (objc==1) {
    Tcl_WrongNumArgs(interp,1,objv,"<subcommand> ...");
    return TCL_ERROR;
  }

  cmd=Keyword_getIdFromObj(interp,objv[1]);

  if (cmd==lengthCmd) {
    int holding;

    if (objc!=3) {
      Tcl_WrongNumArgs(interp,2,objv,"<holding>");
      return TCL_ERROR;
    }

    holding=getHoldingNumFromObj(interp,objv[2]);

    if (holding<0) { 
      return TCL_ERROR;
    }

    Tcl_SetObjResult(interp,getIntObj(counttable[holding&8191]));
    return TCL_OK;
  }

  if (cmd==encodeCmd) {
    int holding;
    if (objc!=3) {
      Tcl_WrongNumArgs(interp,2,objv,"<holding>");
      return TCL_ERROR;
    }
    holding=getHoldingNumFromObj(interp,objv[2]);
    if (holding<0) { return TCL_ERROR; }

    Tcl_SetObjResult(interp,getIntObj(holding));
    return TCL_OK;
  }
	
  if (cmd==decodeCmd) {
    int result,num;
    Tcl_Obj *obj;
    if (objc!=3) {
      Tcl_WrongNumArgs(interp,2,objv,"<integer>");
      return TCL_ERROR;
    }
    result=Tcl_GetIntFromObj(interp,objv[2],&num);
    if (result!=TCL_OK) { 
      return result;
    }

    obj=Tcl_NewHoldingObj(num);
    if (obj==(Tcl_Obj*)0) {
      return TCL_ERROR;
    }

    Tcl_SetObjResult(interp,obj);
    return TCL_OK;
  }

  /*
   * Construct the union of one or more holdings:
   *     holding union AJT3 KT42  =>  AKJT432
   */
  if (cmd==unionCmd) {
    int result=0,holding,i;
    if (objc<3) {
      Tcl_WrongNumArgs(interp,2,objv,"<holding> [<holding> ... ]");
      return TCL_ERROR;
    }
    for (i=2; i<objc; i++) {
      holding=getHoldingNumFromObj(interp,objv[i]);
      if (holding<0||holding>8191) { return TCL_ERROR; }
      result |= holding;
    }
    Tcl_SetObjResult(interp,Tcl_NewHoldingObj(result));
    return TCL_OK;
  }

  /*
   * Check to see whether the first holding passed contains all
   * of the other holdings passed:
   *    holding contains AJT832 J832    => 1  (true)
   *    holding contains AJT832 J 8 3 2 => 1  (true)
   *    holding contains AJT832 K       => 0  (false)
   */
  if (cmd==subsetCmd) {
    int holding,subset,i;
    if (objc<4) {
      Tcl_WrongNumArgs(interp,2,objv,"<holding> <subset> [<subset> ... ]");
      return TCL_ERROR;
    }
    holding=getHoldingNumFromObj(interp,objv[2]);
    if (holding<0 || holding>8191) { return TCL_ERROR; }
    for (i=3;i<objc;i++) {
      subset=getHoldingNumFromObj(interp,objv[i]);
      if (holding<0||holding>8191) { return TCL_ERROR; }
      if ((subset&holding)!=subset) { 
	Tcl_SetObjResult(interp,getIntObj(0));
	return TCL_OK;
      }
    }
    Tcl_SetObjResult(interp,getIntObj(1));
    return TCL_OK;
	
  }

  /*
   * Determine if the list of passed in holdings are disjoint:
   *
   *      holding disjoint AT32 KJ84 Q9  => 1  (true)
   *      holding disjoint AT32 972      => 0  (false)
   */
  if (cmd==disjointCmd) {
    int cumulative=0,i;
    for (i=2; i<objc; i++) {
      int holding=getHoldingNumFromObj(interp,objv[i]);
      if (holding<0||holding>8191) { return TCL_ERROR; }

      if (cumulative&holding) {
	Tcl_SetObjResult(interp,getIntObj(0));
	return TCL_OK;
      }
      cumulative |= holding;
    }
    Tcl_SetObjResult(interp,getIntObj(1));
    return TCL_OK;
  }

  /* 
   * Determines whether a holding pattern matches a holding:
   *      holding matches AKxxx AK843     => 1  (true)
   *      holding matches AKxxx AKJ92     => 0  (false)
   */
  if (cmd==matchesCmd) {
    Tcl_AddErrorInfo(interp,"'holding matches' not implemented yet");
    return TCL_ERROR;
    /*
      The *lowest spot* that is not a spot in the x should be the
      limit
    */
  }
  Tcl_AppendResult(interp,
		   "Invalid subcommand '", Tcl_GetString(objv[1]) ,
		   "' passed to '", Tcl_GetString(objv[0]),
		   "' command.",NULL);
  return TCL_ERROR;
}

int
IDealHolding_Init(Tcl_Interp *interp)
{
  /* Only way to create a namespace? */
  int retval=Tcl_Eval(interp,"namespace eval ::SlowHoldingProcedures {expr 1}");
  if (retval!=TCL_OK) { return retval; }
  Tcl_CreateObjCommand(interp,"::holdingProc",
		       IDeal_DefHoldingProc,(ClientData)&integerAggr,NULL);
  Tcl_CreateObjCommand(interp,"::holding",
	           IDeal_HoldingCmd,NULL,NULL);
  return TCL_OK;
}
