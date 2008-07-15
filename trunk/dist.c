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
 * This set of routines implements the 'shapeclass' and
 * 'shapefunc' methods.
 */

#include <string.h>
#include "deal.h"
#include "dist.h"
#include "dealtypes.h"

#include <stdlib.h>

HandDist dist_table[DIST_COUNT];
static Tcl_Obj *shapeObjs[DIST_COUNT];

static Tcl_Obj *shapeParams=NULL; /* Will contain constant list "s h d c" */

static Tcl_Obj* lengthObjs[14];

static void  InitializeLengths() {
  int i;
  for (i=0; i<=13; i++) {
    Tcl_IncrRefCount(lengthObjs[i]=Tcl_NewIntObj(i));
  }
}

/*
 * The set of distributions for a hand are in 1-1 correspondence
 * with the set of of 3-subsets of the numbers {0,1,...,15}; if
 * our distribution is S-H-D-C, then we choose as our 3-subset
 * S,S+H+1,S+H+D+2.
 *
 * I have chosen to use the "squashed order" for the 3-subsets
 * as the basis for numbering the distributions because it is
 * easy to compute the index of the 3-subset from it's elements.
 *
 * For information about the squashed order, see:
 * 	Anderson, "Combinatorics of Finite Sets", pp 112-119.
 *
 * I'm sure there is an easier way, but my brain is too tired, and this
 * should work well enough.
 */

/*
 * This method, while sound, isn't as quick as just using
 * pointers to pointers.  I've left it in as a reminder
 *
static int distTableIndex(s,h,d)
int s,h,d;
{
  *
  * 
  * int f1=s, f2=s+h+1, f3=s+h+d+2;
  *            (f3 choose 3) + (f2 choose 2) + (f1 choose 1)
  * return f3*(f3-1)*(f3-2)/6+f2*(f2-1)/2+f1; 

  * I've pretabulated the (x choose 2) and (x choose 3) functions
     for even easier lookup; this function is suitable for inlining. 
  static choose2tab[]={0, 1, 3,  6, 10, 15, 21, 28,  36,  45,  55,  66,  78,
							91};
  static choose3tab[]={0, 1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364,
							455};
  return choose3tab[s+h+d]+choose2tab[s+h]+s;
}
*/

void computeDistTable()
{
  int s,h,d;
  int entry=0;
  Tcl_Obj *lengths[4];
  Tcl_Obj *shapeObj;

  for (s=0;s<=13; s++) {
    lengths[0]=lengthObjs[s];
    for (h=0; h<=13-s; h++) {
      lengths[1]=lengthObjs[h];
      for (d=0; d<=13-s-h; d++) {
        int c=13-s-h-d;
        lengths[2]=lengthObjs[d];
        lengths[3]=lengthObjs[c];

	dist_table[entry][SPADES]=s;
	dist_table[entry][HEARTS]=h;
	dist_table[entry][DIAMONDS]=d;
	dist_table[entry][CLUBS]=13-s-h-d;
        shapeObj=Tcl_NewListObj(4,lengths);
        Tcl_IncrRefCount(shapeObjs[entry]=shapeObj);
	entry++;
      }
    }
  }
}

void printDistTable()
{
  int i;
  for (i=0; i<DIST_COUNT; i++) {
    printf("%3d : %2d %2d %2d %2d\n",i,
	   dist_table[i][SPADES],dist_table[i][HEARTS],
	   dist_table[i][DIAMONDS],dist_table[i][CLUBS]);
  }
}


static void deleteDistFunc(ClientData data)
{
  int i;
  DistFunc d=(DistFunc) data;
  
  for (i=0; i<DIST_COUNT; i++) {
    Tcl_Obj *obj=d->array2[i];
    if (obj != (Tcl_Obj*) NULL) {
      Tcl_DecrRefCount(obj);
    }
  }
  Tcl_Free(data);
}

DistFunc newDistFunc(n)
     int n;
{
  int s,h,d;
  int index1=0,index2=0;
  DistFunc func=(DistFunc)Tcl_Alloc(n*sizeof(struct distfunc));
  for (s=0; s<=13; s++) {
    func->array[s] = index1+(func->array1);
    for (h=0; h<=13-s; h++,index1++) {
      func->array1[index1]= index2 + (func->array2);
      for (d=0; d<=13-s-h; d++, index2++) {
	func->array2[index2] = 0;
      }
    }
  }
  return func;
}
    
DistSet newDistSet(n)
     int n;
{
  int s,h,d;
  int index1=0,index2=0;
  DistSet set=(DistSet)Tcl_Alloc(n*sizeof(struct distset));
  for (s=0; s<=13; s++) {
    set->array[s] = index1+(set->array1);
    for (h=0; h<=13-s; h++,index1++) {
      set->array1[index1]= index2 + (set->array2);
      for (d=0; d<=13-s-h; d++, index2++) {
        set->array2[index2] = NULL;
      }
    } 
  } 
  set->shapesList=NULL;
  return set;
}

void deleteDistSet(ClientData cd) {
  DistSet set=(DistSet)cd;
  int i;
  for (i=0; i<DIST_COUNT; i++) {
    if (set->array2[i]!=NULL) {
      Tcl_DecrRefCount(set->array2[i]);
    }
  }
  Tcl_Free((char*)set);
}

int tcl_shapefunc_eval ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  static int handSubCmd=-1,compileSubCmd=-1, evalSubCmd=-1, shapeSubCmd, subCmdInit=1;
  int hand;
  char *result,*rptr;
  int i;
  int s,h,d,c;
  DistFunc set=(DistFunc) cd;

  if (subCmdInit) {
    handSubCmd=Keyword_addKey("hand");
    compileSubCmd=Keyword_addKey("compile");
    evalSubCmd=Keyword_addKey("eval");
    shapeSubCmd=Keyword_addKey("shape");
    subCmdInit=0;
  }

  if (objc==1) {
    Tcl_WrongNumArgs(interp,1,objv,"handname");
    Tcl_AppendResult(interp,"\tor\n",0);
    Tcl_WrongNumArgs(interp,1,objv,"hand <hand>");
    Tcl_AppendResult(interp,"\tor\n",0);
    Tcl_WrongNumArgs(interp,1,objv,"hand eval <s> <h> <d> <c>");
    Tcl_AppendResult(interp,"\tor\n",0);
    Tcl_WrongNumArgs(interp,1,objv,"hand shape {<s> <h> <d> <c>}");
    return TCL_ERROR;
  }

  if (NOSEAT==(hand=getHandNumFromObj(interp,objv[1]))) {

    int subCmd=Keyword_getIdFromObj(interp,objv[1]);

    if (subCmd==compileSubCmd) {
      int len;
      char *command=Tcl_GetString(objv[0]);
      result=(char *)Tcl_Alloc(8*DIST_COUNT+strlen(command)+50);
      sprintf(result,"shapeclass.binary %s {\n",command);
      rptr=result+strlen(result);

      for (i=1;i<=DIST_COUNT; i++) {
	if (i%8==1) {
	  *(rptr++)='\t';
	}
	sprintf(rptr,"%s",Tcl_GetStringFromObj(DFVal(set,i-1),&len));
	rptr += len;
	if (i%8==0) {
	  *(rptr++)='\n';
	} else {
	  *(rptr++)=' ';
	}
      }
      strcpy(rptr,"\n}\n");
      Tcl_SetResult(interp,result,TCL_DYNAMIC);
      return TCL_OK;
    }

    if (subCmd==handSubCmd) {
        
      int retval,suit,holding[4],lengths[4];

      if (objc!=3) {
        Tcl_WrongNumArgs(interp,2,objv,"<hand>");
        return TCL_ERROR;
      }

      retval=getHandHoldingsFromObj(interp,objv[2],holding);
      if (retval!=TCL_OK) { 
	Tcl_SetResult(interp,"Argument was not a list of four holdings",
		      TCL_STATIC);
	return TCL_ERROR;
      }
      for (suit=0; suit<4; suit++) {
	lengths[suit]=(counttable[holding[suit]&8191]);
      }
      s=lengths[0]; h=lengths[1]; d=lengths[2]; c=lengths[3];
    } else  if (subCmd==evalSubCmd) {
      if (objc!=6) {
        Tcl_WrongNumArgs(interp,2,objv,"slen hlen dlen clen");
        return TCL_ERROR;
      }

      Tcl_GetIntFromObj(interp,objv[2],&s);
      Tcl_GetIntFromObj(interp,objv[3],&h);
      Tcl_GetIntFromObj(interp,objv[4],&d);
      Tcl_GetIntFromObj(interp,objv[5],&c);
    } else if (subCmd==shapeSubCmd) {
      int retval;
      Tcl_Obj **objv2=NULL;
      int objc2=0;
      if (objc!=3) {
	Tcl_WrongNumArgs(interp,2,objv,"{slen hlen dlen clen}");
	return TCL_ERROR;
      }
      retval=Tcl_ListObjGetElements(interp,objv[2],&objc2,&objv2);
      if (retval==TCL_ERROR || objc2!=4) {
	Tcl_SetResult(interp,"Shape must be a list of four suit lengths",
		      TCL_STATIC);
	return TCL_ERROR;
      }
      Tcl_GetIntFromObj(interp,objv2[0],&s);
      Tcl_GetIntFromObj(interp,objv2[1],&h);
      Tcl_GetIntFromObj(interp,objv2[2],&d);
      Tcl_GetIntFromObj(interp,objv2[3],&c);

    } else {
      char *command=Tcl_GetString(objv[0]);
      Tcl_AppendResult(interp,Tcl_GetString(objv[1]),
		       " is not a legitimate subcommand for the shapefunc ",
		       command,
		       NULL);
      return TCL_ERROR;
    }
  } else {
    if (objc!=2) {
      Tcl_WrongNumArgs(interp,2,objv,0);
      return TCL_ERROR;
    }
    FINISH(hand);
    s=count_suit(hand,SPADES);
    h=count_suit(hand,HEARTS);
    d=count_suit(hand,DIAMONDS);
    c=count_suit(hand,CLUBS);
  }
  if (s+h+d+c!=13) {
    Tcl_SetResult(interp,"Suit lengths did not add to 13",TCL_STATIC);
    return TCL_ERROR;
  }

  Tcl_SetObjResult(interp,set->array[s][h][d]);
  return TCL_OK;
}

int tcl_shapeclass_eval ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  static int subCmdInit=1,
    compileSubCmd=-1,
    handSubCmd=-1,
    evalSubCmd=-1,
    listSubCmd=-1,
    shapeSubCmd=-1;

  int hand;
  char *result,*rptr;
  int i;
  int s,h,d,c;

  DistSet set=(DistSet) cd;

  if (subCmdInit) {
    handSubCmd=Keyword_addKey("hand");
    compileSubCmd=Keyword_addKey("compile");
    evalSubCmd=Keyword_addKey("eval");
    listSubCmd=Keyword_addKey("list");
    shapeSubCmd=Keyword_addKey("shape");
    subCmdInit=0;
  }

  if (objc==1) {
    Tcl_WrongNumArgs(interp,1,objv,"list");
    Tcl_AppendResult(interp,"\tor\n",0);
    Tcl_WrongNumArgs(interp,1,objv,"<handname>");
    Tcl_AppendResult(interp,"\tor\n",0);
    Tcl_WrongNumArgs(interp,1,objv,"hand <hand>");
    Tcl_AppendResult(interp,"\tor\n",0);
    Tcl_WrongNumArgs(interp,1,objv,"hand eval <s> <h> <d> <c>");
    Tcl_AppendResult(interp,"\tor\n",0);
    return TCL_ERROR;
  }

  if (NOSEAT==(hand=getHandNumFromObj(interp,objv[1]))) {
    int subCmdId=Keyword_getIdFromObj(interp,objv[1]);

    if (subCmdId==listSubCmd) {

      if (objc!=2) {
	Tcl_WrongNumArgs(interp,2,objv,0);
	return TCL_ERROR;
      }

      if (set->shapesList==NULL) {
	Tcl_Obj *explicit[DIST_COUNT],*current;
	int ret,value,count=0;
	for (i=0; i<DIST_COUNT; i++) {
	  current=DSElt(set,i);
	  ret=Tcl_GetBooleanFromObj(interp,current,&value);
	  if (ret!=TCL_OK) {
	    return ret;
	  }
	  if (value) {
	    explicit[count++]=shapeObjs[i];
	  }
	}
	Tcl_IncrRefCount(set->shapesList=Tcl_NewListObj(count,explicit));
      }
      Tcl_SetObjResult(interp,set->shapesList);
      return TCL_OK;
    }

    if (subCmdId==compileSubCmd) {
      char *name=Tcl_GetString(objv[0]);

      result=(char *)Tcl_Alloc(1+DIST_COUNT+strlen(name)+1024);
      sprintf(result,"shapeclass.binary %s {\n",name);
      rptr=result+strlen(result);

      for (i=1;i<=DIST_COUNT; i++) {
	Tcl_Obj *obj=DSElt(set,i-1);
	int isElt;
	int ret=Tcl_GetBooleanFromObj(interp,obj,&isElt);
	  
	if (ret!=TCL_OK) {
	  Tcl_Free(result);
	  return ret;
	}

	if (i%32==1) {
	  *(rptr++)='\t';
	}
	
	*(rptr++)=(isElt ? '1' : '0');
	if (i%32==0) {
	  *(rptr++)='\n';
	} else {
	  if (i%8==0) {
	    *(rptr++)=' ';
	  }
	} 
      } 

      strcpy(rptr,"\n}\n");
      Tcl_SetResult(interp,result,TCL_DYNAMIC);
      return TCL_OK;
    } 

    if (subCmdId==handSubCmd) {
      int retval,suit,hnum[4],lengths[4];

      if (objc!=3) {
	Tcl_WrongNumArgs(interp,2,objv,"<hand>");
	return TCL_ERROR;
      }

      retval=getHandHoldingsFromObj(interp,objv[2],hnum);
      if (retval!=TCL_OK) { 
	Tcl_AddErrorInfo(interp,"Argument was not a valid hand");
	return TCL_ERROR;
      }

      for (suit=0; suit<4; suit++) {
	lengths[suit]=counttable[hnum[suit]&8191];
      }

      s=lengths[0]; h=lengths[1]; d=lengths[2]; c=lengths[3];
    } else if (subCmdId==evalSubCmd) {
      if (objc!=6) {
	Tcl_WrongNumArgs(interp,2,objv,"<slen> <hlen> <dlen> <clen>");
	return TCL_ERROR;
      }

      Tcl_GetIntFromObj(interp,objv[2],&s);
      Tcl_GetIntFromObj(interp,objv[3],&h);
      Tcl_GetIntFromObj(interp,objv[4],&d);
      Tcl_GetIntFromObj(interp,objv[5],&c);

    } else if (subCmdId==shapeSubCmd) {
      int retval;
      Tcl_Obj **objv2=NULL;
      int objc2=0;
      if (objc!=3) {
	Tcl_WrongNumArgs(interp,2,objv,"{<slen> <hlen> <dlen> <clen>}");
	return TCL_ERROR;
      }

      retval=Tcl_ListObjGetElements(interp,objv[2],&objc2,&objv2);
      if (retval==TCL_ERROR || objc2!=4) {
	Tcl_SetResult(interp,"Error trying to parse shape ",TCL_STATIC);
	Tcl_AppendResult(interp,Tcl_GetString(objv[2]),NULL);
	return TCL_ERROR;
      }

      Tcl_GetIntFromObj(interp,objv2[0],&s);
      Tcl_GetIntFromObj(interp,objv2[1],&h);
      Tcl_GetIntFromObj(interp,objv2[2],&d);
      Tcl_GetIntFromObj(interp,objv2[3],&c);

    } else {

      char *command=Tcl_GetString(objv[0]);
      Tcl_AppendResult(interp,Tcl_GetString(objv[1]),
		       " is not a legitimate subcommand for the shapeclass ",
		       command,
		       NULL);
      return TCL_ERROR;
    }
  } else {
    FINISH(hand);
    s=count_suit(hand,SPADES);
    h=count_suit(hand,HEARTS);
    d=count_suit(hand,DIAMONDS);
    c=count_suit(hand,CLUBS);
  }
  
  if (s+h+d+c!=13) {
    Tcl_SetResult(interp,"Shape did not have 13 cards.",TCL_STATIC);
    return TCL_ERROR;
  }

  Tcl_SetObjResult(interp,set->array[s][h][d]);
  return TCL_OK;
}


static int ShapeProcDefine(interp,name,procBody)
     Tcl_Interp *interp;
     Tcl_Obj *name;
     Tcl_Obj *procBody;
{
  static Tcl_Obj *procCmd=NULL;
  Tcl_Obj *code[4];

  if (procCmd==NULL) {
    Tcl_IncrRefCount(procCmd=Tcl_NewStringObj("proc",4));
  }
  code[0]=procCmd;
  code[1]=name;
  code[2]=shapeParams;
  code[3]=procBody;

  return Tcl_EvalObjv(interp,4,code,TCL_EVAL_GLOBAL);
}

DistFunc shapefunc_compile(interp,name,procBody)
     Tcl_Interp *interp;
     Tcl_Obj *name;
     Tcl_Obj *procBody;
{
  int result;
  DistFunc func;
  char *nameStr;
  int spades,hearts,diamonds,clubs;
  Tcl_Obj *array[5],*value;

  func=newDistFunc(1);
  if (func==0) {
    return 0;
  }

  result=ShapeProcDefine(interp,name,procBody);

  nameStr=Tcl_GetString(name);
  if (result==TCL_ERROR) { 
    deleteDistFunc(func); 
    Tcl_AppendResult(interp,"\nCould not compile function ",nameStr,"\n",NULL);
    return 0;
  }

  array[0]=name;

  for (spades=0; spades<=13; spades++) {
    array[1]=lengthObjs[spades];
    for (hearts=0; hearts<=13-spades; hearts++) {
      array[2]=lengthObjs[hearts];
      for (diamonds=0; diamonds<=13-spades-hearts; diamonds++) {
	array[3]=lengthObjs[diamonds];
	clubs=13-spades-hearts-diamonds;
	array[4]=lengthObjs[clubs];
	result=Tcl_EvalObjv(interp,5,array,TCL_EVAL_GLOBAL);
	if (result==TCL_ERROR) { 
	  deleteDistFunc(func);
	  Tcl_AppendResult(interp,"\nCould not compile function ",nameStr,"\n",
			   NULL);
	  return 0;
	}
	
	value=Tcl_GetObjResult(interp);
	func->array[spades][hearts][diamonds]=value;
	Tcl_IncrRefCount(value);
      }
    }
  }
  Tcl_ResetResult(interp); /* Clear interp results... */
  return func;
}

DistSet shapeclass_compile(interp,name,procBody)
     Tcl_Interp *interp;
     Tcl_Obj *name;
     Tcl_Obj *procBody;
{

  int result;
   DistSet set;
  int spades,hearts,diamonds,clubs;
  Tcl_Obj *callCmd[5],*value;
  char *nameStr;
  set=newDistSet(1);

  result=ShapeProcDefine(interp,name,procBody);

  nameStr=Tcl_GetString(name);
  if (result==TCL_ERROR) { 
    deleteDistSet((ClientData)set); 

    Tcl_AppendResult(interp,"\nCould not compile function ",
		     nameStr,
		     NULL);
    return 0;
  }

  callCmd[0]=name;
  for (spades=0; spades<=13; spades++) {
    callCmd[1]=lengthObjs[spades];
    for (hearts=0; hearts<=13-spades; hearts++) {
      callCmd[2]=lengthObjs[hearts];
      for (diamonds=0; diamonds<=13-spades-hearts; diamonds++) {
	callCmd[3]=lengthObjs[diamonds];
	clubs=13-spades-hearts-diamonds;
	callCmd[4]=lengthObjs[clubs];
	result=Tcl_EvalObjv(interp,5,callCmd,TCL_EVAL_GLOBAL);

	if (result==TCL_ERROR) {
	  deleteDistSet((ClientData)set);
	  Tcl_AppendResult(interp,"\nCould not compile function ",
			   nameStr,
			   NULL);
	  return 0;
	}
	value=Tcl_GetObjResult(interp);
	set->array[spades][hearts][diamonds]=value;
	Tcl_IncrRefCount(value);
      }
    }
  }
  Tcl_ResetResult(interp);
  return set;
}

int tcl_shapefunc_lazy ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  Tcl_Obj *procBody=(Tcl_Obj *)cd;
  char *name=Tcl_GetString(objv[0]);
  DistFunc func=shapefunc_compile(interp,objv[0],procBody);

  if (func==(DistFunc)NULL) {
    Tcl_AppendResult(interp,"Error compiling\n",NULL);
    return TCL_ERROR;
  }
  Tcl_CreateObjCommand(interp,name,tcl_shapefunc_eval,(ClientData)func,
		       deleteDistFunc);
  return tcl_shapefunc_eval((ClientData) func,interp,objc,objv);
}

DistSet shapeclass_lazy_compile (interp,nameObj,procBody)
     Tcl_Interp *interp;
     Tcl_Obj *nameObj;
     Tcl_Obj *procBody;
{
  DistSet set=shapeclass_compile(interp,nameObj,procBody);
  char *name;

  if (set==(DistSet)NULL) {
    return 0;
  }

  name=Tcl_GetString(nameObj);
  Tcl_CreateObjCommand(interp,name,tcl_shapeclass_eval,(ClientData)set,
		       deleteDistSet);
  return set;
}
    
int tcl_shapeclass_lazy ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  Tcl_Obj *procBody=(Tcl_Obj *)cd;
  DistSet set=shapeclass_lazy_compile(interp,objv[0],procBody);

  if (set==(DistSet)0) {
    Tcl_AddErrorInfo(interp,"Error compiling shape class ");
    Tcl_AddErrorInfo(interp,Tcl_GetString(objv[0]));
    return TCL_ERROR;
  }
  return tcl_shapeclass_eval((ClientData) set,interp,objc,objv);

}

int tcl_shapeexpr_define ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  static Tcl_Obj *exprCmd=NULL;

  int isClass=(int)cd;
  Tcl_Obj *cmdElts[2];
  Tcl_Obj *command;
  char *name;

  if (exprCmd==NULL) {
    Tcl_IncrRefCount(exprCmd=Tcl_NewStringObj("expr",4));
  }

  if (objc!=3) { 
    Tcl_WrongNumArgs(interp,1,objv,"<name> <expr>");
    return TCL_ERROR;
  }
  name=Tcl_GetString(objv[1]);

  cmdElts[0]=exprCmd;
  cmdElts[1]=objv[2];
  Tcl_IncrRefCount(command=Tcl_NewListObj(2,cmdElts));

  if (isClass) {
    Tcl_CreateObjCommand(interp,name,tcl_shapeclass_lazy,(ClientData)command,&Tcl_ObjDelete);
  } else {
    Tcl_CreateObjCommand(interp,name,tcl_shapefunc_lazy,(ClientData)command,&Tcl_ObjDelete);	
  }
  return TCL_OK;
}

int tcl_shapefunc_define ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  char *name;
  if (objc!=3) { 
    Tcl_WrongNumArgs(interp,1,objv,"<name> <expr>");
    return TCL_ERROR;
  }

  name=Tcl_GetString(objv[1]);
  Tcl_IncrRefCount(objv[2]);

  Tcl_CreateObjCommand(interp,name,tcl_shapefunc_lazy,(ClientData)objv[2],Tcl_ObjDelete);
  return TCL_OK;
}

int tcl_shapeclass_define ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  char *name;
  if (objc!=3) { 
    Tcl_WrongNumArgs(interp,1,objv,"<name> <expr>");
    return TCL_ERROR;
  }

  name=Tcl_GetString(objv[1]);
  Tcl_IncrRefCount(objv[2]);
  Tcl_CreateObjCommand(interp,name,tcl_shapeclass_lazy,(ClientData)objv[2],Tcl_ObjDelete);
  return TCL_OK;
}

int tcl_shapeclass_define_binary ( TCL_PARAMS ) TCL_DECL
{
  DistSet set;
  CONST84 char *s;
  int i;
  if (argc!=3) { return TCL_ERROR; }

  set=newDistSet(1);
  for (i=0; i<DIST_COUNT; i++) {
    Tcl_IncrRefCount(set->array2[i]=Tcl_NewBooleanObj(0));
  }

  for (i=0,s=argv[2]; (*s) && (i<DIST_COUNT); s++) {
    switch (*s) {
    case '0':
      DSDelete(set,i++);
      break;
    case '1':
      DSAdd(set,i++);
      break;
    default:
      break;
    }
  }
  Tcl_CreateObjCommand(interp,argv[1],tcl_shapeclass_eval,(ClientData)set,Tcl_AllocDelete);
  return TCL_OK;
}

int Dist_Init(interp)
     Tcl_Interp *interp;
{
  InitializeLengths();
  Tcl_IncrRefCount(shapeParams=Tcl_NewStringObj("s h d c",7));

  Tcl_CreateObjCommand(interp,"shapefunc",tcl_shapefunc_define,NULL,NULL);
  Tcl_CreateObjCommand(interp,"shapeclass",tcl_shapeclass_define,NULL,NULL);
  Tcl_CreateObjCommand(interp,"shapecond",tcl_shapeexpr_define,(ClientData)1,NULL);
  Tcl_CreateObjCommand(interp,"shapeexpr",tcl_shapeexpr_define,(ClientData)0,NULL);
  Tcl_CreateCommand(interp,"shapeclass.binary",
		    tcl_shapeclass_define_binary,NULL,NULL);

  computeDistTable();
  return TCL_OK;
}
