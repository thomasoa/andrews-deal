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
#include <string.h>
#include "formats.h"
#include "dealtypes.h"
#include "tcl_incl.h"

Tcl_Obj *tcl_format_suit(holding,voidObj)
     int holding;
     Tcl_Obj *voidObj;
{
  if ((holding&8191)==0 && voidObj!=NULL) {
    return voidObj;
  } else {
    return Tcl_NewHoldingObj(holding);
  }
}

Tcl_Obj *tcl_format_hand(hptrs,voidObj)
     int *hptrs;
     Tcl_Obj *voidObj;
{
  int suit;
  Tcl_Obj *holdingElts[4];

  for (suit=0; suit<4; suit++) {
    holdingElts[suit]=tcl_format_suit(hptrs[suit],voidObj);
  }

  return Tcl_NewListObj(4,holdingElts);
}
