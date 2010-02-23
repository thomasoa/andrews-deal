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
#ifndef __TCL_INCL__
#include <tcl.h>

#ifndef CONST84
#define CONST84
#endif

#ifdef __STDC__
#define  TCL_PARAMS ClientData cd,Tcl_Interp *interp,int argc, CONST84 char *argv[]
#define TCLOBJ_PARAMS ClientData cd,Tcl_Interp *interp,int objc,Tcl_Obj * CONST objv[]
#define  TCL_DECL
#define  TCLOBJ_DECL
#else
#define TCL_PARAMS cd,interp,argc,argv
#define TCLOBJ_PARAMS cd,interp,objc,objv
#define TCL_DECL ClientData cd; Tcl_Interp *interp; int argc; char *argv[];
#define  TCLOBJ_DECL ClientData cd; Tcl_Interp *interp; int objc; Tcl_Obj * CONST objv[];
/*int free();*/
#endif

#define USAGE(s) argv[0]," usage:\n\t",argv[0]," ",s
#define OBJUSAGE(s) Tcl_GetString(objv[0])," usage:\n\t",Tcl_GetString(objv[0])," ",s
#define AUnixError \
  Tcl_AppendResult(interp,argv[0]," failed due to error: ",\
		   Tcl_PosixError(interp),NULL); return TCL_ERROR;

/*
#define MyAlloc(string,bytes) (string)=(char *)Tcl_Alloc(bytes)
*/

#define tcl_error(interp)  \
  fprintf(stderr,"Tcl stack dump of error info:\n"); \
  fprintf(stderr,"%s\n",Tcl_GetVar2(interp,"errorInfo",NULL,0)); \
  exit(1);

void Tcl_AllocDelete _ANSI_ARGS_((ClientData data));
void Tcl_ObjDelete _ANSI_ARGS_((ClientData data));

#ifdef _WINDOWS
#define DEAL31_API __declspec(dllexport)
#else
#define DEAL31_API
#endif
#if (TCL_MAJOR_VERSION<8 || (TCL_MAJOR_VERSION==8 && TCL_MINOR_VERSION==0))
static int __dummyLength;
#define Tcl_GetString(obj) Tcl_GetStringFromObj(obj,&__dummyLength)
#endif

#if (TCL_MINOR_VERSION==0)
int My_EvalObjv _ANSI_ARGS_((Tcl_Interp *,int,Tcl_Obj **,int));
#define Tcl_EvalObjv(interp,objc,objv,dummy) My_EvalObjv(interp,objc,objv,dummy)
#endif



#define __TCL_INCL__
#endif

