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

/* Vectors are a basic sort of "additive evaluation"
 * which occurs often in bridge.
 *
 * Two common vectors are "Controls", where we count A=2
 * and K=1, and "High card points," where A=4, K=3, Q=2,
 * and J=1.
 *
 * See also 'additive.c' and 'holdings.c.'
 *
 * This file defines a number of different elements and
 * objects to support vectors in Tcl.
 *
 * Vectors are defined in Tcl with a 'defvector' call:
 *
 *   defvector AKQpoints 3 2 1
 *
 * Internally that defines a 'Lazy Vector' function -
 * one which only remembers the arguments passed in.
 *
 * The first time the vector is used, the 'Lazy Vector'
 * procedure builds up a table, in this case of 8 elements:
 *
 *       table[0]=0  - No AKQ
 *       table[1]=1  - Q
 *       table[2]=2  - K
 *       table[3]=3  - KQ
 *       table[4]=3  - A
 *       table[5]=4  - AQ
 *       table[6]=5  - AK
 *       table[7]=6  - AKQ
 *
 * After building this table, it converts itself from a
 * lazy procedure to an active procedure.
 *
 * This lets the user define a library of vectors without
 * concern for the expense of defining them - defining
 * is a lightweight operation.  The table is only built
 * when first used.
 *
 * Vectors only support integer values.  For doubles,
 * see holdings.c to handle other types.
 *
 */
#include <tcl.h>
#include "deck.h"
#include "tcl_incl.h"
#include "deal.h"
#include "vector.h"
#include "additive.h"

LazyVectorData newLazyVector() {
  LazyVectorData vec=(LazyVectorData)Tcl_Alloc(sizeof(struct lazy_vector_data));
  vec->num=0;
  return vec;
}


int tcl_vector_lazy PROTO((TCLOBJ_PARAMS));
int tcl_vector_define PROTO((TCL_PARAMS));

int tcl_vector_define ( TCL_PARAMS ) TCL_DECL
{
  int i;
  CONST84 char *name=argv[1];
  LazyVectorData vec;
  if (argc<=1) {
    Tcl_AppendResult(interp,"usage: ",argv[0],
		     " <name> <AceValue> <KingValue> ...",NULL);
    return TCL_ERROR;
  }
  if (argc>15) {
    Tcl_AppendResult(interp,"too many args: ",argv[0],NULL);
    return TCL_ERROR;
  }
  argc--; argv++;
  vec=newLazyVector();
  for (i=1; i<argc;i++) {
    vec->value[i-1]=atoi(argv[i]);
  }
  vec->num=argc-1;
  Tcl_CreateObjCommand(interp,name,tcl_vector_lazy,(ClientData)vec,Tcl_AllocDelete);
  return TCL_OK;
}

static int vector_eval(int holding,void *data)
{
  VectorTable vec=data;
  return VectorTableLookup(vec,holding);
}

int tcl_vector_lazy( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  LazyVectorData lazyvec=(LazyVectorData)cd;
  int num=lazyvec->num;
  int *val=lazyvec->value;
  VectorTable table;
  void *func;
  int i,j;
  int *tb;

  table=(int *)Tcl_Alloc(sizeof(int)*(1+(1 << num)));
  table[0]=13-num;
  tb=table+1;
  
  for (i=0; i< (1<<num); i++) {
    tb[i]=0;
  }

  for (i=0; i< (1<<num); i++) {
    for (j=0; j<num; j++) {
      if (i & (1<<(num-1-j))) {
        tb[i] += val[j];
      }
    }
  }

  func=tcl_create_additive(interp,Tcl_GetString(objv[0]),vector_eval,(ClientData)table,Tcl_AllocDelete);
  return tcl_count_additive((ClientData)func,interp,objc,objv);
}

int Vector_Init(interp)
     Tcl_Interp *interp;
{
  Tcl_CreateCommand(interp,"defvector",tcl_vector_define,NULL,NULL);
  return TCL_OK;
}
