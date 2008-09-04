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
 * Commands defined in this file:
 *
 *    whogets - returns the name of the holder of a specific card
 *
 *    north,east,south,west,hand - rather complicated; see below
 * 
 *    hcp, controls, losers
 *
 *    spades,hearts,diamonds,clubs - returns suit counts
 *
 *    
 */
#include <tcl.h>

#include <string.h>

#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "deal.h"
#include "dealtypes.h"
#include "additive.h"
#include "formats.h"

#include "tcl_incl.h"
#include <stdlib.h>


#if (TCL_MINOR_VERSION==0)
int My_EvalObjv(Tcl_Interp *interp,int objc,Tcl_Obj **objv,int dummy)
{
  Tcl_Obj *list=Tcl_NewListObj(objc,objv);

  if (list==NULL) { return TCL_ERROR; }

  return Tcl_GlobalEvalObj(interp,list);
}
#endif

int tcl_deal_to_whom (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int card;
  
  if (objc!=2) {
    Tcl_WrongNumArgs(interp,1,objv,"<cardname>");
    return TCL_ERROR;
  }

  card=card_num(Tcl_GetString(objv[1]));
  if (card>=52||card<0) {
    Tcl_AppendResult(interp,Tcl_GetString(objv[0]), ": ", 
		     Tcl_GetString(objv[1])," is not a valid card name",NULL);
    return TCL_ERROR;
  }
  
  Tcl_SetObjResult(interp,getHandKeywordObj(interp,to_whom(card)));
  return TCL_OK;
}

int tcl_other_hand(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  Tcl_Obj *result;
  int hand;
  int rotation=(int)cd; /* 1 = LHO, 2 = partner, 3 = RHO */

  if (objc!=2) {
     Tcl_WrongNumArgs(interp,1,objv,"<handname>");
     return TCL_ERROR;
  }

  hand=getHandNumFromObj(interp,objv[1]);
  if (hand<0||hand>3) {
    Tcl_AppendResult(interp,Tcl_GetString(objv[0]),
		     ": Expected handname as first argument; got '",
		     Tcl_GetString(objv[1]),"'",NULL);
    return TCL_ERROR;
  }

  hand = (hand+rotation)%4;

  result=getHandKeywordObj(interp,hand);

  if (result==NULL) {
    /*NOTREACHED*/
    /* This should almost never happen -getHandKeywordObj doesn't create a
       new object, and the hand is assured to be an appropriate hand number */
    return TCL_ERROR;
  }

  Tcl_SetObjResult(interp,result);
  return TCL_OK;
}

int tcl_count_suit (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  static int doInit=1,
    HandCmdID=-1;
  static char *usage="<handname> | hand <handspec>";

  int suit,hand,length;
  int hArray[4],*hptr;
    
  if (doInit) {
    HandCmdID=Keyword_addKey("hand");
    doInit=0;
  }

  if (objc<2) { 
    Tcl_WrongNumArgs(interp,1,objv,usage);
    return TCL_ERROR;
  }

  hand=getHandNumFromObj(interp,objv[1]);
  if (NOSEAT==hand) {

    if (objc!=3) {
       Tcl_WrongNumArgs(interp,1,objv,usage);
       return TCL_ERROR;
    }

    if (HandCmdID!=Keyword_getIdFromObj(interp,objv[1])) {
      Tcl_AppendResult(interp,
		       "Illegal hand name, \"",Tcl_GetString(objv[1]),
		       "\", used with \"",
		       Tcl_GetString(objv[0]),"\" command",(char *)NULL);
      return TCL_ERROR;
    }

    if (TCL_OK!=getHandHoldingsFromObj(interp,objv[2],hArray)) {
      Tcl_AppendResult(interp,
		       "Badly specified hand, \"",Tcl_GetString(objv[2]),
		       "\", used with \"",
		       Tcl_GetString(objv[0]),"\" command",(char *)NULL);
      return TCL_ERROR;
    }

    hptr=hArray;

  } else {
    if (objc!=2) {
      Tcl_WrongNumArgs(interp,1,objv,usage);
      return TCL_ERROR;
    }
    FINISH(hand);
    hptr=holdings[hand];
  }

  suit=(int)cd;

  length=counttable[8191&hptr[suit]];
    
  Tcl_SetObjResult(interp,getLengthObj(length));
  return TCL_OK;
}

Tcl_Obj *tcl_hand_holdings(Tcl_Interp *interp,int *hArray)
{
  Tcl_Obj *holdingObjs[4];
  Tcl_Obj *list;
  int suit;
  for (suit=0; suit<4; suit++) {
    holdingObjs[suit]=Tcl_NewHoldingObj(hArray[suit]);
  }
  list=Tcl_NewListObj(4,holdingObjs);
  return list;
}

/*
 * tcl_hand_cmd implements the commands "north","east",
 * "south", and "west".
 *
 * Placing cards:
 *   north is AKQ JT98 765 432
 *   south is {432 765 JT98 AKQ}
 *   west gets AD
 *
 * Formatting output:
 *   % north
 *   AKQ JT98 765 432
 *   % south clubs
 *   AKQ
 *
 * Getting the shape or pattern:
 *   % north shape
 *   3 4 3 3
 *   % north pattern
 *   4 3 3 3
 */
static int tcl_hand_cmd( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  static int uninitialized=1, 
    IsID=-1,
    GetsID=-1,
    PatternID=-1,
    ShapeID=-1,
    HasID=-1,
    HandID=-1,
    VoidFlagID=-1;
     
  int hand=(int)cd;
  int suit;
  Tcl_Obj *voidObj=0;
  int argID;
  int retval;
  Tcl_Obj * CONST *objv0=objv; /* saved due to potential increment of objv */
  int usageStart=1;      /* set to 2 if this is of type hand <hand> ... */

  Tcl_Obj *handObj=NULL;
  static Tcl_Obj *stackHandCmd=NULL,*stackCardsCmd=NULL;
  int hnums[4];
  int *holdingsPtr;

  if (uninitialized) {
    /* Initialize once */
    IsID=Keyword_addKey("is");
    GetsID=Keyword_addKey("gets");
    PatternID=Keyword_addKey("pattern");
    ShapeID=Keyword_addKey("shape");
    HasID=Keyword_addKey("has");
    HandID=Keyword_addKey("hand");
    VoidFlagID=Keyword_addKey("-void");
    uninitialized=0;
    Tcl_IncrRefCount(stackHandCmd=Tcl_NewStringObj("::stack_hand",12));
    Tcl_IncrRefCount(stackCardsCmd=Tcl_NewStringObj("::stack_cards",13));
  }

  if (NOSEAT==hand) {
    if (objc>=2) {
      handObj=objv[1];
      usageStart+=2;
      objv++; objc--;
    } else {
      char *name=Tcl_GetString(*objv0);
      Tcl_AppendResult(interp,name[0]," is not a legitimate hand name",NULL);
      return TCL_ERROR;
    }
  }

  if (objc>1) {
    argID=Keyword_getIdFromObj(interp,objv[1]);
    if (argID==VoidFlagID) {
      voidObj=objv[2];
      objv+=2;
      objc-=2;
      argID=Keyword_getIdFromObj(interp,objv[1]);
    }
  }


  if (objc<=1) {
    if (handObj==NULL) {
      FINISH(hand);
      holdingsPtr=globalDeal.hand[hand].suit;
    } else {
      retval=getHandHoldingsFromObj(interp,handObj,hnums);
      if (retval!=TCL_OK) { 
        Tcl_AddErrorInfo(interp,"Error extracting holding from hand object");
        return retval;
      }
      holdingsPtr=hnums;
    }
    if (voidObj!=NULL) {
      Tcl_SetObjResult(interp,tcl_format_hand(holdingsPtr,voidObj));
    } else {
      Tcl_SetObjResult(interp,tcl_hand_holdings(interp,holdingsPtr));
    }
    return TCL_OK;
  }

  /* argID=Keyword_getIdFromObj(interp,objv[1]); */
  if (argID==KEYWORD_INVALID_ID) {
    Tcl_AppendResult(interp,
		     Tcl_GetString(*objv0),
		     ": invalid subcommand ",
		     Tcl_GetString(objv[1]),
		     NULL);
    return TCL_ERROR;
  }

  if (argID==IsID) {
    Tcl_Obj **command;
    int i;
    if (handObj!=NULL) {
      Tcl_AddErrorInfo(interp,
		       "hand subcommand 'is' cannot be used with hand command");
      return TCL_ERROR;
    }

    command=(Tcl_Obj**)Tcl_Alloc(objc*sizeof(Tcl_Obj*));
    command[0]=stackHandCmd;
    command[1]=objv[0];

    for (i=2; i<objc; i++) {
      if (0==strcmp(Tcl_GetString(objv[i]),"-")) {
          command[i]=Tcl_NewHoldingObj(0);
      } else {
          command[i]=objv[i];
      }
    }
    return Tcl_EvalObjv(interp,objc,command,TCL_EVAL_GLOBAL);  }

  if (argID==GetsID) {
    int card,holdings[4],suit;
    Tcl_Obj* CONST *suits=getAllSuitObjs();
    Tcl_Obj* stackcmd[4];
    stackcmd[0]=stackCardsCmd;
    stackcmd[1]=getHandKeywordObj(interp,hand);

    holdings[0]=holdings[1]=holdings[2]=holdings[3]=0;

    if (handObj!=NULL) {
      Tcl_AddErrorInfo(interp,
		       "hand subcommand 'gets' cannot be used with hand command.");
      return TCL_ERROR;
    }

    if (objc<=2) {
      Tcl_WrongNumArgs(interp,1+usageStart,objv0,"card [ card ... ]");
      return TCL_ERROR;
    }

    while (objc>2) {
      card=getCardNumFromObj(interp,objv[--objc]);
      if (card>=52||card<0) {
	Tcl_AppendResult(interp,Tcl_GetString(objv[objc])," is not a valid card name",NULL);
	return TCL_ERROR;
      }

      holdings[SUIT(card)] |= (1<<(12-RANK(card)));
    }

    for (suit=0; suit<4; suit++) {
      if (holdings[suit]!=0) {
        stackcmd[2]=suits[suit];
	Tcl_IncrRefCount(stackcmd[3]=Tcl_NewHoldingObj(holdings[suit]));
	if (TCL_OK!=Tcl_EvalObjv(interp,4,stackcmd,TCL_EVAL_GLOBAL)) {
	  Tcl_AppendResult(interp,"Forcing cards with 'gets' failed.",NULL);
	  return TCL_ERROR;
	}
	Tcl_DecrRefCount(stackcmd[3]);
      }
    }

    return TCL_OK;
  }

  if (handObj==NULL) {
    FINISH(hand);
    holdingsPtr=globalDeal.hand[hand].suit;
  } else {
    retval=getHandHoldingsFromObj(interp,handObj,hnums);
    if (retval!=TCL_OK) { return retval; }
    holdingsPtr=hnums;
  }

  usageStart += 1;
  if (argID==PatternID || argID==ShapeID) {
    int i,j,temp,d[4];
    Tcl_Obj *s=Tcl_NewListObj(0,NULL);
   
    if (objc>2) {
       Tcl_WrongNumArgs(interp,usageStart,objv0,0);
       return TCL_ERROR;
    }

    for (i=0; i<4 ; i++)  {
      d[i]=counttable[8191&holdingsPtr[i]];
    }
    if (argID==PatternID) {
      /* do bubble sort to return pattern */
      for (j=3; j>=1; j--)
  	for (i=0; i<j ; i++)
	  if (d[i]<d[i+1]) { temp=d[i]; d[i]=d[i+1]; d[i+1]=temp; }
    }

    for (i=0; i<4 ; i++)  {
      Tcl_ListObjAppendElement(interp,s,getLengthObj(d[i]));
    }

    Tcl_SetObjResult(interp,s);
    return TCL_OK;
  }

  if (argID==HasID) {
    int card;
    int count=0;

    if (objc<=2) {
       Tcl_WrongNumArgs(interp,usageStart,objv0,"<card> [<card>] ...");
       return TCL_ERROR;
    }

    while(objc>2) {
      card=getCardNumFromObj(interp,objv[--objc]);
      if (card>=52 || card<0) {
	Tcl_AppendResult(interp,Tcl_GetString(objv[objc])," is not a valid card name",NULL);
	return TCL_ERROR;
      }
      if (HASCARD2(holdingsPtr,card)) {
	count++;
      }
    }

    Tcl_SetObjResult(interp,Tcl_NewIntObj(count));
    return TCL_OK;
  }

  if (NOSUIT!=(suit=getSuitNumFromObj(interp,objv[1]))) {
    if (objc!=2) {
       Tcl_WrongNumArgs(interp,usageStart,objv0,0);
       return TCL_ERROR;
    }
    Tcl_SetObjResult(interp,tcl_format_suit(holdingsPtr[suit],voidObj));
    return TCL_OK;
  }  else {
      Tcl_AppendResult(interp,Tcl_GetString(objv[1]),
		       " is not a legitimate suit name\n",
		       Tcl_GetString(objv[0]),
		       "[<suit>]",NULL);
      return TCL_ERROR;
  }
  
  return TCL_ERROR;
  
}

int tcl_stack_cards(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int hand;
  int index;
  int tclRet;

  if (objc<4||(objc%2)) {
    Tcl_WrongNumArgs(interp,1,objv,"handname suitname holding [suitname holding ...]");
    return TCL_ERROR;
  }

  if (NOSEAT==(hand=getHandNumFromObj(interp,objv[1]))) {
    Tcl_AddErrorInfo(interp,"error: first argument must be a hand name");
    return TCL_ERROR;
  }

  for (index=2; index<objc; index+=2) {
    int suit=getSuitNumFromObj(interp,objv[index]);
    int holding=getHoldingNumFromObj(interp,objv[index+1]);
    if (suit==NOSUIT || holding<0) {
      return TCL_ERROR;
    }
    tclRet=put_holding(hand,suit,holding);
    if (tclRet!=TCL_OK) { return tclRet; }
  }

  return TCL_OK;
}

int tcl_stack_hand(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int hand,tclret;
  int hnums[4];

  if (objc!=3&&objc!=6) {
    Tcl_WrongNumArgs(interp,1,objv,"{<spades> <hearts> <diamonds> <clubs>}");
    Tcl_AppendResult(interp," or ",NULL);
    Tcl_WrongNumArgs(interp,1,objv,"<spades> <hearts> <diamonds> <clubs>");
    return TCL_ERROR;
  }

  if (NOSEAT==(hand=getHandNumFromObj(interp,objv[1]))) {
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]),
		  ": first argument must be a hand name",
		  TCL_STATIC);
    return TCL_ERROR;
  }

  if (objc==3) {
    tclret=getHandHoldingsFromObj(interp,objv[2],hnums);
  } else {
    tclret=getHandHoldingsFromObjv(interp,objv+2,hnums);
  }

  if (tclret!=TCL_OK) {
    Tcl_SetResult(interp,"Hand stacking error",TCL_STATIC);
    return TCL_ERROR;
  }

  return put_holdings(hand,hnums);

}


int HandCmd_Init(Tcl_Interp *interp)
{
  int hand,suit;

  initializeLengths();

  initializeDealTypes(interp);

  for (suit=SPADES; suit<=CLUBS; suit++) {
    /* Put suit name in client data as well as command name to allow
       use of "rename" by user */
    Tcl_CreateObjCommand(interp,suitname[suit], tcl_count_suit,
			 (ClientData)suit, NULL);
  }
  
  for (hand=NORTH; hand<=WEST; hand++) {
    /* Put hand number in client data to allow use of "rename" by user */
    Tcl_CreateObjCommand(interp,handname[hand],tcl_hand_cmd,(ClientData)hand,NULL);
  }

  Tcl_CreateObjCommand(interp,"hand",tcl_hand_cmd,(ClientData)NOSEAT,NULL);

  Tcl_CreateObjCommand(interp,"whogets",tcl_deal_to_whom,NULL,NULL);

  tcl_create_additive(interp,"controls", count_controls, NULL, NULL);
  tcl_create_additive(interp,"hcp", count_hcp, NULL, NULL);
  tcl_create_additive(interp,"losers", count_losers, NULL, NULL);

  Tcl_CreateObjCommand(interp,"lho",tcl_other_hand,(ClientData)1,NULL);
  Tcl_CreateObjCommand(interp,"partner",tcl_other_hand,(ClientData)2,NULL);
  Tcl_CreateObjCommand(interp,"rho",tcl_other_hand,(ClientData)3,NULL);

  /*
   * These are not meant to be overridden
   */
  Tcl_CreateObjCommand(interp,"deck_stack_hand",tcl_stack_hand,NULL,NULL);
  Tcl_CreateObjCommand(interp,"deck_stack_cards",tcl_stack_cards,NULL,NULL);

  /* These two aliases are meant to be overridden */
  Tcl_CreateObjCommand(interp,"stack_hand",tcl_stack_hand,NULL,NULL);
  Tcl_CreateObjCommand(interp,"stack_cards",tcl_stack_cards,NULL,NULL);

  return TCL_OK;
}

