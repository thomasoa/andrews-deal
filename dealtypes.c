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
 * This file defines new Tcl types, to avoid the constant relookup
 * of certain common words and strings (coverting AKJ2 to its
 * binary representation, for example, or suit or hand names to their
 * internal numeric values.)
 *
 * int getSuitNumFromObj(Tcl_Interp *interp, Tcl_Obj *suit)
 * int getHandNumFromObj(Tcl_Interp *interp, Tcl_Obj *hand)
 * int getHoldingNumFromObj(Tcl_Interp *interp, Tcl_Obj *holding)
 * int getCardNumFromObj(Tcl_Interp *interp, Tcl_Obj *card)
 *
 * Tcl_Obj* Tcl_NewHoldingObj(int holding);
 *
 * The underlying objects are *converted* to these new types,
 * which keeps them from being looked up, again and again.
 *
 * See 
 *
 *    http://www.tcl.tk/man/tcl8.6/TclLib/ObjectType.htm
 * 
 * for more information about object types in Tcl.
 */

#include "dealtypes.h"
#include "ctype.h"
#include "string.h"

Tcl_ObjType CardType, HoldingType;

static Tcl_Obj* lengthObjs[14];

void initializeLengths() {
   int i;
   static int initialized=0;
   if (!initialized) {
     for (i=0; i<=13; i++) {
       Tcl_IncrRefCount(lengthObjs[i]=Tcl_NewIntObj(i));
     }
     initialized=1;
   }
}

Tcl_Obj *getLengthObj(int i) {
  return lengthObjs[i];
}

static void
dupDealRepProc(Tcl_Obj *source, Tcl_Obj *dup)
{
  dup->internalRep.longValue=source->internalRep.longValue;
}

extern char cards[];
extern char suits[];
static void updateCardStringRep(Tcl_Obj *card) {
  int cardnum=card->internalRep.longValue;
  int suit=cardnum&3;
  int denom=cardnum>>2;
  char *string=Tcl_Alloc(3);
  string[0]=cards[denom];
  string[1]=suits[suit];
  string[2]=0;
  card->bytes=string;
  card->length=2;
}

static int convertToCardRep(Tcl_Interp *interp,Tcl_Obj *card)
{
  char *string=Tcl_GetString(card);
  card->typePtr=&CardType;
  card->internalRep.longValue=card_num(string);
  return TCL_OK;
}

void initializeCardType() {
  CardType.freeIntRepProc=NULL;
  CardType.name="Card";
  CardType.dupIntRepProc=dupDealRepProc;
  CardType.freeIntRepProc=NULL;
  CardType.updateStringProc=updateCardStringRep;
  CardType.setFromAnyProc=convertToCardRep;
  Tcl_RegisterObjType(&CardType);

}

static int validHoldingNum(int num) {
   int spots;

   if (num<0) { return 0; }
   spots=(num>>13);
   if (spots<0||spots>13) { return 0; }

   if (spots>0) {
        /* Make sure spots are right */
        int codedspots=(1<<spots)-1;
        if ((codedspots&num)!=codedspots) { return 0; }
    }
    return 1; /* true */
}

Tcl_Obj *Tcl_NewHoldingObj(int holding) {
  Tcl_Obj *result;
  if (!validHoldingNum(holding)) {
    return (Tcl_Obj *)0;
  }

  result=Tcl_NewObj();
  result->typePtr=&HoldingType;
  result->internalRep.longValue=holding;
  Tcl_InvalidateStringRep(result);
  return result;
}

/*
 * Exact holdings are encoded as 13 bits, so AJ932 is,
 * in binary,
 *           1001010000011
 *           AKQJT98765432
 *
 * Abstract holdings like AJ8xxx are recorded in the
 * lower 13 bits as AJ8432, but with the top bits
 * representing the number of x's:
 *
 *        x's|AKQJT98765432
 *         11|1001001000111
 *
 * The '11' means there are 3 x's.
 *
 * Note, this means that AKJ4xxx is meaningless - the xxx's
 * cannot all be smaller than the 4. The procedure returns
 * an error in this case.
 *
 * Functions which take exact holdings can also take
 * holdings with x's by simply examining the lower 13 bits:
 *    holding&8191
 * So, for example, when evaluating:
 *
 *   hcp eval AKxxxxxxxxxx
 *
 * the x's are assumed to be the smallest
 * possible, in which case, we are really
 * evaluating:
 *
 *   hcp eval AKJT98765432
 *
 * which turns out to be 8.
 *
 */
static int convertToHoldingRep(Tcl_Interp *interp,Tcl_Obj *holding)
{
  char *string=Tcl_GetString(holding);
  char *cardCursor=cards;
  int hnum=0,spots=0;
  int cardnum=0;
  if (*string == '-' && string[1]==0) {
     holding->internalRep.longValue = 0;
     holding->typePtr = &HoldingType;
     return TCL_OK;
  }
  
  while (*cardCursor && *string) {
    if (*cardCursor==toupper(*string)) {
      hnum |= (1<<(12-cardnum));
      string++;
    }
    cardCursor++; cardnum++;
  }
  
  while (*string && (*string=='x' || *string=='X')) {
    if (hnum & (1<<spots)) {
      Tcl_AddErrorInfo(interp,"too many x's in holding\n");
      return TCL_ERROR;
    }
    hnum |= (1<<spots);
    spots++;
    string++;
  }

  hnum |= (spots<<13);

  if (*string) {
    return TCL_ERROR;
  }

  holding->internalRep.longValue=hnum;
  holding->typePtr=&HoldingType;
  return TCL_OK;
}

static char *holdingStrings[8192];

char *getStringForHoldingNum(int hnum,int *lenPtr)
{
  int hlength;
  int spots;
  char *source,*result;

  spots=hnum>>13;
  hnum = ((hnum>>spots)<<spots)&8191;
  if ((source=holdingStrings[hnum])==NULL) {
    int i=1<<12;
    int count=0;
    int j;
    source=Tcl_Alloc(14);
    for (j=0; j<13; j++,i=i/2) {
      if (hnum&i) {
	source[count++]=cards[j];
      }
      source[count]=0;
    }
    holdingStrings[hnum]=source;
  }
  hlength=strlen(source);

  result=Tcl_Alloc(spots+hlength+1);
  strcpy(result,source);
  while (spots--) {
    result[hlength++]='x';
  }
  result[hlength]=0;
  if (lenPtr!=NULL) { *lenPtr=hlength; }
  return result;
}

static void updateHoldingStringRep(Tcl_Obj *holding) {
  int hlength;
  long hnum=holding->internalRep.longValue;     
  holding->bytes=getStringForHoldingNum(hnum,&hlength);
  holding->length=hlength;
}

void initializeHoldingType() {
  int i;
  for (i=0; i<8192; i++) { holdingStrings[i]=NULL; }
  HoldingType.freeIntRepProc=NULL;
  HoldingType.name="Holding";
  HoldingType.dupIntRepProc=dupDealRepProc;
  HoldingType.freeIntRepProc=NULL;
  HoldingType.updateStringProc=updateHoldingStringRep;
  HoldingType.setFromAnyProc=convertToHoldingRep;
  Tcl_RegisterObjType(&HoldingType);
}

static int spadeId=KEYWORD_INVALID_ID;
/* static int aceId=KEYWORD_INVALID_ID; */
static int northId=KEYWORD_INVALID_ID;

/**
 * Procedure for walking through all holdings
 */
static int Deal_foreachHolding(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int result,i;

  if (objc!=3) {
    Tcl_WrongNumArgs(interp,1,objv,"<variable> { ... code ... }");
    return TCL_ERROR;
  }

  for (i=0; i<=8191; i++) {
    Tcl_Obj *holding=Tcl_NewHoldingObj(i);
    Tcl_ObjSetVar2(interp,objv[1],NULL,holding,TCL_PARSE_PART1);
    result=Tcl_EvalObj(interp,objv[2]);
    if (result==TCL_BREAK) { break; }
    if (result==TCL_CONTINUE) { continue; }
    if (result!=TCL_OK) { return result; }
  }
  return TCL_OK;
}

static Tcl_Obj *AllSuits[4],*SuitList,*AllHands[4],*HandsList;

static void initializeAllSuits() {
    int i;
#ifdef DEAL_MEMORY
    fprintf(stderr,"getAllSuitObjs called, allocating suit objects\n");
#endif
    for (i=0; i<4; i++) {
      Tcl_IncrRefCount(AllSuits[i]=Keyword_NewObj(spadeId+i));
      Tcl_IncrRefCount(AllHands[i]=Keyword_NewObj(northId+i));
    }

    Tcl_IncrRefCount(SuitList=Tcl_NewListObj(4,AllSuits));
    Tcl_IncrRefCount(HandsList=Tcl_NewListObj(4,AllHands));
}

static int tcl_type_assert(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int base=(int)cd;
  int value;
  if (objc!=2) {
    Tcl_WrongNumArgs(interp,1,objv,"<word>");
    return TCL_ERROR;
  }
  value=Keyword_getIdFromObj(interp,objv[1]);
  if (value<base||value>base+3) {
    Tcl_AppendResult(interp,
		     Tcl_GetString(objv[0]),
		     ": failed. '",
		     Tcl_GetString(objv[1]),
		     "' does not match type.", NULL);
    return TCL_ERROR;
  }
  return TCL_OK;
}

void initializeDealTypes(Tcl_Interp *interp) {
  static int initialized=0;
  Tcl_CreateObjCommand(interp,"foreachHolding",Deal_foreachHolding,NULL,NULL);

  if (!initialized) {
    Keyword_Init(interp);

    initializeCardType();
    initializeHoldingType();

    spadeId=Keyword_addKey("spades");
    Keyword_addKey("hearts");
    Keyword_addKey("diamonds");
    Keyword_addKey("clubs");
    Keyword_addKey("notrump");
    northId=Keyword_addKey("north");
    Keyword_addKey("east");
    Keyword_addKey("south");
    Keyword_addKey("west");
    Keyword_alias("South","south",0);
    Keyword_alias("North","north",0);
    Keyword_alias("East","east",0);
    Keyword_alias("West","west",0);

    Tcl_CreateObjCommand(interp,"assertHandname",
			 tcl_type_assert,(ClientData)northId,NULL);
    Tcl_CreateObjCommand(interp,"assertSuitname",
			 tcl_type_assert,(ClientData)spadeId,NULL);

    initializeAllSuits();
    initialized=1;
  }
}


int getDenomNumFromObj(Tcl_Interp *interp, Tcl_Obj *denom) {
  int denomNum;
  if (denom->typePtr!=&KeywordType) {
    int res=Tcl_ConvertToType(interp,denom,&KeywordType);
    if (res!=TCL_OK) { return NOSUIT; }
  }
  denomNum=(int)denom->internalRep.longValue-spadeId;
  if (denomNum>=0 && denomNum<=4) { return denomNum; }
  return -1;
}

int getSuitNumFromObj(Tcl_Interp *interp, Tcl_Obj *suit) {
  int suitNum;
  if (suit->typePtr!=&KeywordType) {
    int res=Tcl_ConvertToType(interp,suit,&KeywordType);
    if (res!=TCL_OK) { return NOSUIT; }
  }
  suitNum=(int)suit->internalRep.longValue-spadeId;
  if (suitNum>=0 && suitNum<=3) { return suitNum; }
  return NOSUIT;
}

int getHandNumFromObj(Tcl_Interp *interp, Tcl_Obj *hand) {
  int handNum;
  if (hand->typePtr!=&KeywordType) {
    int res=Tcl_ConvertToType(interp,hand,&KeywordType);
    if (res!=TCL_OK) { 
      Tcl_AddErrorInfo(interp,"Couldnt covert object to KeywordType\n");
      return NOSEAT;
    }
  }
  handNum=(int)hand->internalRep.longValue-northId;
  if (handNum>=0 && handNum<=3) { return handNum; }
  return NOSEAT;
}

/*
Tcl_Obj* getHandKeywordObj(Tcl_Interp *interp,int num) {
  static Tcl_Obj *hands[4]={0,0,0,0};
  static int initialized=0;
  if (num<0|| num>3 || northId==KEYWORD_INVALID_ID) { return NULL; }
  if (!initialized) {
    int i;
    for (i=0; i<4; i++) {
      Tcl_IncrRefCount(hands[i]=Keyword_NewObj(i+northId));
    }
    initialized=1;
  }
  return hands[num];
}
*/

int getHoldingNumFromObj(Tcl_Interp *interp, Tcl_Obj *holding) {
  if (holding->typePtr!=&HoldingType) {
    int res=Tcl_ConvertToType(interp,holding,&HoldingType);
    if (res!=TCL_OK) { 
      Tcl_AppendResult(interp,
		       "Could not convert ",
		       Tcl_GetString(holding),
		       "to a suit holding",
		       NULL);
      return -1;
    }
  }
  return (int)holding->internalRep.longValue;
}

int getCardNumFromObj(Tcl_Interp *interp, Tcl_Obj *card) {
  if (card->typePtr!=&CardType) {
    int res=Tcl_ConvertToType(interp,card,&CardType);
    if (res!=TCL_OK) {
      return NOCARD;
    }
  }
  return card->internalRep.longValue;
}

int getHandHoldingsFromObjv(Tcl_Interp *interp,Tcl_Obj * CONST *objv,int *retHoldings)
{
  int cards=0,suit;
  for (suit=0; suit<4; suit++) {
    Tcl_Obj *hObj=objv[suit];
    retHoldings[suit]=getHoldingNumFromObj(interp,hObj);
    if (retHoldings[suit]<0) {
      Tcl_AddErrorInfo(interp,Tcl_GetString(hObj));
      Tcl_AddErrorInfo(interp," is an invalid holding\n");
      return TCL_ERROR;
    }
    cards += counttable[8191&retHoldings[suit]];
  }

  if (cards!=13) {
    Tcl_SetResult(interp,"hand does not have 13 cards.\n",TCL_STATIC);
    return TCL_ERROR;
  }
  return TCL_OK;
}

int
getHandHoldingsFromObj(Tcl_Interp *interp, Tcl_Obj *obj, int *retHoldings)
{
  int retval,length;
  Tcl_Obj **objv;
  retval=Tcl_ListObjGetElements(interp,obj,&length,&objv);
  if (retval!=TCL_OK || length!=4) { 
    Tcl_SetResult(interp,
		  "Hand argument was not a list of length 4",
		  TCL_STATIC);
    return TCL_ERROR;
  }

  return getHandHoldingsFromObjv(interp,objv,retHoldings);
}

Tcl_Obj * CONST *getAllSuitObjs() {
  return AllSuits;
}

Tcl_Obj *getSuitList() {
  return SuitList;
}

Tcl_Obj *getSuitKeywordObj(Tcl_Interp *interp,int num) {
  if (num<0||num>3) { return 0; }
  return AllSuits[num];
}

Tcl_Obj *getHandKeywordObj(Tcl_Interp *interp,int num) {
  if (num<0||num>3) { return 0; }
  return AllHands[num];
}
