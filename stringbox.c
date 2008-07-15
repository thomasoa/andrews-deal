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
#include "stringbox.h"
#include <string.h>

typedef Tcl_UniChar Char;

typedef struct string_box_base {
  int rows, columns;
  Char *start;
  int refcount;
} *StringBoxBase;

typedef struct string_box {
  StringBoxBase parent;
  int rows,columns;
  Char **strings;
} *StringBox;

static void clearStringBox(box)
     StringBox box;
{
  int rows=box->rows, columns=box->columns;
  int r,c;
  for (r=0; r<rows; r++) {
    for (c=0; c<columns; c++) {
      box->strings[r][c] = ' ';
    }
  }
}

static int writeStringBox(interp,box,row,column,string)
     Tcl_Interp *interp; /* May be null.  For error messages */
     StringBox box;
     int row;
     int column;
     Char *string;
{
  int rowcount=box->rows, columncount=box->columns;
  Char *boxstring;
  int startcolumn=column;
  Char c;
  if (row<0 || row>=rowcount || column>=columncount || column<0) {
    if (interp != (Tcl_Interp *)NULL) {
      char s[20];
      sprintf(s,"%d %d",row,column);
      Tcl_AppendResult(interp,"illegal parameters \"",s,"\"",NULL);
    }
    return TCL_ERROR;
  }
  boxstring=box->strings[row]+column;
  while ((c=*(string++))!=0) {
    if (c=='\n') {
      row++;
      if (row>=rowcount) return TCL_OK;
      column=startcolumn;
      boxstring=box->strings[row]+column;
      continue;
    }
    if (column<columncount) {
      *boxstring=c;
      boxstring++;
      column++;
    }
  }
  return TCL_OK;
}

static Char *formatStringBox(box,lengthPtr)
     StringBox box;
     int *lengthPtr;
{
  int row=0,col=0;
  Char *result=(Char*)Tcl_Alloc(sizeof(Char)*(box->rows*(box->columns+1)+1));
  Char *rindex=result;
  for (row=0; row<box->rows; row++) {
    for (col=0; col<box->columns; col++) {
      *(rindex++)=box->strings[row][col];
    }
    *(rindex++)='\n';
  }
  *(--rindex)=0;
  *lengthPtr = rindex - result;
  return result;
}

static Char *compactStringBox(box,lengthPtr)
     StringBox box;
     int *lengthPtr;
{
  int row,col;
  Char *result=(Char *)Tcl_Alloc(sizeof(Char)*((box->rows)*(box->columns+1)+1));
  Char *rindex=result;
  Char c,*lastnonspace;
  for (row=0; row<box->rows; row++) {
    lastnonspace=rindex;
    for (col=0; col<box->columns; col++) {
      *(rindex++)=c=box->strings[row][col];
      if (c!=' ') {
	lastnonspace=rindex;
      }
    }
    rindex=lastnonspace;
    *(rindex++)='\n';
  }
  *(--rindex)=0;
  *lengthPtr = rindex-result;
  return result;
}


static StringBox newStringBox(rows,columns)
     int rows;
     int columns;
{
  StringBoxBase parent;
  StringBox result;
  int i;
  if (rows<=0 || columns<=0) {
    return 0;
  }
  parent=(StringBoxBase)Tcl_Alloc(sizeof(struct string_box_base));
  parent->rows=rows;
  parent->columns=columns;
  parent->start=(Char *)Tcl_Alloc(rows*columns*sizeof(Char));
  parent->refcount=1;

  result=(StringBox)Tcl_Alloc(sizeof(struct string_box));
  result->rows=rows;
  result->columns=columns;
  result->strings=(Char **)Tcl_Alloc(rows*sizeof(Char **));

  for (i=0; i<rows; i++) {
    result->strings[i]=parent->start+i*columns;
  }
      
  result->parent=parent;

  clearStringBox(result);
  return result;
}

static StringBox subStringBox(interp,parentbox,rowloc,columnloc,rows,columns)
     Tcl_Interp *interp; /* For error messages only */
     StringBox parentbox;
     int rowloc,columnloc;
     int rows;
     int columns;
{
  StringBoxBase parent;
  StringBox result;
  int i;
  
  if (rows<=0 || columns<=0 || rowloc< 0 || columnloc<0) {
    goto error;
  }
  
  if (rows+rowloc>parentbox->rows || rows+columnloc>parentbox->columns) {
    goto error;
  }
  
  parent=parentbox->parent;
  parent->refcount++;
  
  result=(StringBox)Tcl_Alloc(sizeof(struct string_box));
  
  result->rows=rows;
  result->columns=columns;
  result->strings=(Char **)Tcl_Alloc(rows*sizeof(Char **));
  
  for (i=0; i<rows; i++) {
    result->strings[i]=parentbox->strings[rowloc+i]+columnloc;
  }
  
  result->parent=parent;
  
  clearStringBox(result);
  return result;
 error:

  if (interp != (Tcl_Interp *)NULL) {
    Tcl_AppendResult(interp, "subbox: illegal argument",NULL);
  }
  return 0;
}


	
void deleteStringBox(ClientData boxdata)
{
  StringBox box=(StringBox)boxdata;
  Tcl_Free((char *)box->strings);
#ifdef DEBUG
  fprintf(stderr,"Deleting stringbox\n");
#endif
  if (--(box->parent->refcount) <=0) {
    Tcl_Free((char *)box->parent->start);
    Tcl_Free((char *)box->parent);
#ifdef DEBUG
    fprintf(stderr,"Deleting parent box\n");
#endif
  }
  Tcl_Free((char *)box);
}

static int tcl_string_box(TCLOBJ_PARAMS)
     TCLOBJ_DECL
{
  const char *command;
  StringBox box=(StringBox)cd;
  int len;
  if (objc==1) {
    int length;
    Char *result = formatStringBox(box,&length);
    Tcl_Obj *unicode = Tcl_NewUnicodeObj(result,length);
    Tcl_SetObjResult(interp,unicode);
    Tcl_Free((char *)result);
    return TCL_OK;
  }

  command=Tcl_GetStringFromObj(objv[1],&len);

  if (command==NULL) {
    return TCL_ERROR;
  }

  if ('w'==(*command) && strcmp(command,"write")==0) {
    int row, col;
    if (objc!=5) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", 
		       Tcl_GetString(objv[0]),
		       " write row column string\"",NULL);
      return TCL_ERROR;
    }
    if  (Tcl_GetIntFromObj(interp,objv[2],&row)!=TCL_OK ||
	 Tcl_GetIntFromObj(interp,objv[3],&col)!=TCL_OK) {
      Tcl_AppendResult(interp,"\nbad row or column arguments: row=\"",
		       Tcl_GetString(objv[2]),
		       "\", column=\"",
		       Tcl_GetString(objv[3]),"\"", NULL);
      return TCL_ERROR;
    }

    return writeStringBox(interp,box,row,col,Tcl_GetUnicode(objv[4]));

  }
  
  if (*command=='c' && strcmp(command,"clear")==0) {
    clearStringBox(box);
    return TCL_OK;
  }

  if (*command=='r' && strcmp(command,"rows")==0) {
    if (objc!=2) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", 
		       Tcl_GetString(objv[0]),
		       " ",command,"\"", NULL);
      return TCL_ERROR;
    }
    Tcl_SetObjResult(interp,Tcl_NewIntObj(box->rows));
    return TCL_OK;
  }

  if (*command=='c' && strcmp(command,"columns")==0) {
    if (objc!=2) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", 
		       Tcl_GetString(objv[0]),
		       " ",command,"\"", NULL);
      return TCL_ERROR;
    }
    Tcl_SetObjResult(interp,Tcl_NewIntObj(box->columns));
    return TCL_OK;
  }

  if (*command=='s' && strcmp(command,"subbox")==0) {
    StringBox subbox;
    int rows=0,columns=0,rowloc=0,columnloc=0;
    
    if (objc!=7 && objc!=5) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", 
		       Tcl_GetString(objv[0]),
		       " subbox name rowloc columnloc [ rows columns ]\"",
		       (Char *)NULL);
      return TCL_ERROR;
    }
    
    if (Tcl_GetIntFromObj(interp,objv[3],&rowloc) != TCL_OK) {
      if (rowloc<0) {
	Tcl_AppendResult(interp,"Illegal row location \"",
			 Tcl_GetString(objv[3]),"\" passed to subbox routine",NULL);
      }     
      return TCL_ERROR;
    }
    
    if (Tcl_GetIntFromObj(interp,objv[4],&columnloc) != TCL_OK) {
      if (columnloc<0) {
	Tcl_AppendResult(interp,"Illegal column location \"",
			 Tcl_GetString(objv[4]),"\" passed to subbox routine",NULL);
      }
      return TCL_ERROR;
    }
    if (objc==7) {
      if ((Tcl_GetIntFromObj(interp,objv[5],&rows) != TCL_OK)) {
	return TCL_ERROR;
      }
    
      if ((Tcl_GetIntFromObj(interp,objv[6],&columns) != TCL_OK)) {
	return TCL_ERROR;
      }
    }

    if (rowloc<0) {
      rowloc=(box->rows)+rowloc;
    }
    if (columnloc<0) {
      columnloc=(box->columns)+columnloc;
    }

    if (rows<=0) {
      rows=rows+(box->rows)-rowloc;
    }
    if (columns<=0) {
      columns=columns+(box->columns)-columnloc;
    }

    subbox=subStringBox(interp,box,rowloc,columnloc,rows,columns);

    if (subbox==(StringBox)0) {
      return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp,Tcl_GetString(objv[2]),tcl_string_box,subbox,deleteStringBox);

    return TCL_OK;
  }

  if (*command=='c' && strcmp(command,"compact")==0) {
    int length;
    Char *result = compactStringBox(box,&length);
    Tcl_Obj *unicode = Tcl_NewUnicodeObj(result,length);
    Tcl_SetObjResult(interp,unicode);
    Tcl_Free((char *)result);
    return TCL_OK;
  }

  Tcl_AppendResult(interp, "unknown stringbox operation: \"", 
		   Tcl_GetString(objv[0]), " ",
		   command," ...\"",NULL);
  return TCL_ERROR;
}

int tcl_create_string_box(TCLOBJ_PARAMS)
     TCLOBJ_DECL
{
  int rows,columns;
  StringBox box;
   
  if (objc!=4) {
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
		     Tcl_GetString(objv[0]),
		     " name rows columns\"",(Char *)NULL);
    return TCL_ERROR;
  }
    
  if ((Tcl_GetIntFromObj(interp,objv[2],&rows) != TCL_OK) || (rows<=0)) {
    Tcl_AddErrorInfo(interp,
		     "\n (reading value of <rows>)");
    return TCL_ERROR;
  }

  if (Tcl_GetIntFromObj(interp,objv[3],&columns) != TCL_OK || (columns<=0)) {
    Tcl_AddErrorInfo(interp,
		     "\n (reading value of <columns>)");
    return TCL_ERROR;
  }

  box=newStringBox(rows,columns);
  Tcl_CreateObjCommand(interp,Tcl_GetString(objv[1]),tcl_string_box,box,deleteStringBox);
  return TCL_OK;
}

int Stringbox_Init(interp)
     Tcl_Interp *interp;
{
#if 0
  int code;
  code = Tcl_PkgProvide(interp,"stringbox","1.0");
  if (code != TCL_OK) {
    return code;
  }
#endif
  Tcl_CreateObjCommand(interp,"stringbox",tcl_create_string_box,NULL,NULL);
  return TCL_OK;
}
