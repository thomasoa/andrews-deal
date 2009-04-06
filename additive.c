/*
 * additive.c - Quick way to define integer additive holding procedures
 *         This is old code, somewhat superceded by "holdingProc"
 *         procedures
 *
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
 * This file encapsulates a specific class of bridge functions, namely,
 * "additive" functions.  An additive function is one that
 * is computed suit-by-suit, with the total equalling the sum
 * of the values for each suit.
 *
 * Examples include:
 *
 *        hcp        - high card points
 *        controls   - A=2, K=1
 *        other "vectors" - AKQ-points, etc
 *        losers     - the losing trick count
 *
 * All of these functions have the same interface, namely, they
 * can be called suit-by-suit or for the entire hand.  For example:
 *
 *        [hcp south] computes the total hcp for south
 *        [hcp south spades] computes the hcp held by south in spades
 * 
 * Additive functions are created with five parameters:
 *    interp - Tcl interpreter where the function is to be defined
 *    name   - The name for the additive function
 *    func   - ptr to function of type 
 *              "int func(int hand, int suit, void *data)"
 *    data   - Arbitrary data used by "func"
 *    freefunc - Function for freeing the data, ignored if NULL
 *
 * Note, additive functions are mostly superceded by the holdingProc
 * code in holdings.c. 
 *
 * Additive functions only return integers.
 *
 */


#include "deal.h"
#include "additive.h"
#include "dealtypes.h"


/* Data structure for holding information about an additive function */
typedef struct additivecounter {
  int (*func) PROTO((int /* holding */, void * /* data */));
  void *data;
  Tcl_CmdDeleteProc *freefunc;
} *AdditiveFunction;

#define AddFuncCall(addfunc,holding) \
        (addfunc)->func(holding,(addfunc)->data)

static AdditiveFunction newAdditiveFunction(func,data,freefunc)
     int (*func) PROTO((int /* holding */, void * /* data */));
     Tcl_CmdDeleteProc *freefunc;
     void *data;
{
  AdditiveFunction result=(AdditiveFunction) 
    Tcl_Alloc(sizeof(struct additivecounter));
  if (result==NULL) { return NULL; }

  result->func=func;
  result->data=data;
  result->freefunc=freefunc;
  return result;
}

static void deleteAdditiveFunction(ClientData ptr)
{
  AdditiveFunction addf=(AdditiveFunction)ptr;
  if (addf->freefunc!=0) {
    (addf->freefunc)(addf->data);
  }
  Tcl_Free((void*)ptr);
}

/*
 * This is the procedure as called from Tcl
 */
int tcl_count_additive( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  static int subCmdInit=1,
    handSubCmd=-1,
    holdingSubCmd=-1;
  int hand;
  int suit;
  AdditiveFunction addfunc=(AdditiveFunction)cd;
  int total=0;
  int i,subCmd;

  if (subCmdInit) {
    handSubCmd=Keyword_addKey("hand");
    holdingSubCmd=Keyword_addKey("holding");
    subCmdInit=0;
  }

  if (objc==1) {
    char *name=Tcl_GetString(objv[0]);
    Tcl_AppendResult(interp, "Usage Error: No args.  Us either:\n\t",
                     name," <handname> [ <suitname> ... ]\nor\n\t",
		     name," hand <hand> [ <suitname> ...]\nor\n\t",
                     name," holding <holding> [<holding> <holding> ...]",
                     (char *)NULL);
    return TCL_ERROR;
  }

  if (NOSEAT!=(hand=getHandNumFromObj(interp,objv[1]))) {

    /* First argument is a hand name - hand is set to the hand number */

    FINISH(hand); /* Make sure the hand is dealt */

    if (objc==2) {
      for (suit=0; suit<4; suit++) {
        total += AddFuncCall(addfunc,globalDeal.hand[hand].suit[suit]);
      } 
    } else {
      for (i=2; i<objc; i++) {
        suit=getSuitNumFromObj(interp,objv[i]);
        if (suit==NOSUIT) {
	  char *badparam=Tcl_GetString(objv[i]);
	  Tcl_AppendResult(interp,Tcl_GetString(objv[0]),": Got ", badparam, 
			   " when expecting a suit name",NULL);
	  return TCL_ERROR;
	}
        total += AddFuncCall(addfunc,globalDeal.hand[hand].suit[suit]);
      }
    }
    Tcl_SetObjResult(interp,Tcl_NewIntObj(total));
    return TCL_OK;
  }

  subCmd=Keyword_getIdFromObj(interp,objv[1]);

  if (subCmd==handSubCmd) {
    int total=0,suit,hnum[4];

    if (objc!=3) {
       Tcl_WrongNumArgs(interp,2,objv,"<hand>");
       return TCL_ERROR;
    }

    if (TCL_OK!=getHandHoldingsFromObj(interp,objv[2],hnum)) {
      Tcl_AppendResult(interp,"\nError: '",Tcl_GetString(objv[2]),
                              "' is not a proper hand",NULL);
      return TCL_ERROR;
    }
    for (suit=0; suit<4; suit++) {
      total += AddFuncCall(addfunc,hnum[suit]);
    }
    Tcl_SetObjResult(interp,Tcl_NewIntObj(total));
    return TCL_OK;
  }

  if (subCmd==holdingSubCmd) {
    int total=0;
    for (i=2; i<objc; i++) {
      int holding=getHoldingNumFromObj(interp,objv[i]);
      if (holding<0) { 
	Tcl_AppendResult(interp,"invalid holding ",
			 Tcl_GetString(objv[i]),NULL);
			 
	return TCL_ERROR;
      }
      total += AddFuncCall(addfunc,holding);
    }
    Tcl_SetObjResult(interp,Tcl_NewIntObj(total));
    return TCL_OK;
  }

  {
    Tcl_AppendResult(interp,
		     "Nonexistant hand name '",Tcl_GetString(objv[1]),
		     "' used with ", Tcl_GetString(objv[0])," command",(char *)NULL);
    return TCL_ERROR;
  }


}

void *tcl_create_additive(interp,name,func,data,freefunc) 
     Tcl_Interp *interp;
     char *name;
     int (*func) PROTO((int /* holding */, void * /* data */));
     void *data;
     Tcl_CmdDeleteProc freefunc;
{
  AdditiveFunction af=newAdditiveFunction(func,data,freefunc);
  Tcl_CreateObjCommand(interp,name,tcl_count_additive,(ClientData)af,
		       deleteAdditiveFunction);
  return (void *)af;
}
