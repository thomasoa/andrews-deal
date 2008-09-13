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

#include "tcl_incl.h"
extern void *tcl_create_additive _ANSI_ARGS_((Tcl_Interp *, 
			char *name,
			int (*)_ANSI_ARGS_((int holding,void* data)),
			void * data, Tcl_CmdDeleteProc));
extern int tcl_count_additive
	PROTO((ClientData ,Tcl_Interp *, int, Tcl_Obj * CONST *));
