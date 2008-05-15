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
#include <stdio.h>
#include "tcl_incl.h"
#include "deck.h"
typedef unsigned char HandDist[4];

#define DIST_COUNT 560

extern HandDist dist_table[DIST_COUNT];

#define BitsPerChar 8
#define DistSetCharCount DIST_COUNT/BitsPerChar

typedef struct distset {
  Tcl_Obj ***array[14];
  Tcl_Obj **array1[105];
  Tcl_Obj *array2[560];
  Tcl_Obj *shapesList;
} *DistSet;

typedef struct distfunc {
  char *tclCode;
  Tcl_Obj ***array[14];
  Tcl_Obj **array1[105];
  Tcl_Obj *array2[560];
} *DistFunc;

typedef struct _DistTableEntry {
  int index;
  double probability;
} DistTableEntry;

typedef struct _DistTable {
    int count;
    DistTableEntry entry[DIST_COUNT];
} DistTable;

extern void computeDistTable _ANSI_ARGS_(());
extern DistSet newDistSet _ANSI_ARGS_((int));
extern DistFunc newDistFunc _ANSI_ARGS_((int));

#define DSElt(s,i) ((s)->array2[(i)])
#define DSAdd(s,i) Tcl_SetBooleanObj((s)->array2[(i)],1)
#define DSDelete(s,i) Tcl_SetBooleanObj((s)->array2[(i)],0)

#define DFVal(f,i) ((f)->array2[(i)])
#define DFSetValue(f,i,val) (((f)->array2[(i)])=val)
