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
/* The procedures in this file create "keywords" is a new Tcl
 * type which associates word with integer ids.
 *
 * This is mostly used in C routines to speed up processing
 * certain types of Tcl arguments - implicitly creating "enum"-like
 * behavior.  For example, the first time the code:
 * 
 *  hcp north
 *
 * is called, the internal representation of the argument, "north", is
 * converted to a Keyword Tcl type.  Every further time the code
 * is called, no further lookup is required.  This is a Tcl hack for
 * performance purposes.
 *
 * This is similar to the Tcl_GetIndexFromObj() routines, only there,
 * the keywords are local, not global.  Also, the Tcl_GetIndexFromObj
 * routine doesn't allow aliases.
 *
 * int Keyword_addKey(char *key) adds the key to the key database, if
 * it is not there already, and then returns the integer id of the key.
 * Integer ids are allocated sequentially.
 *
 * Typically, code which used Keyword_addKey might write something like:
 *
 *      static int lengthCmdID=-1, indexCmdID=-1, initialized=0;
 *      int cmdID;
 *      if (!initialized) {
 *         lengthCmdID=Keyword_addKey("length");
 *         indexCmdID=Keyword_addKey("index");
 *      }
 *
 *      cmdID=Keyword_getIdFromObj(interp,objv[1]);
 *      if (cmd==lengthCmdID) {
 *          ... do length sub-command ...
 *      } else if (cmd==indexCmdID) {
 *          ... do index sub-command ...
 *      } else {
 *         ... error code ... 
 *      }
 *
 */
#include "keywords.h"
#include "string.h"

Tcl_ObjType KeywordType;

static Tcl_HashTable keywordsTable;
static Tcl_HashTable backwardsLookup;

static int entryCount=0;

/**
 * If flags has KEYWORD_SET_DEFAULT, then makes this string
 * the default for this id.
 */
int Keyword_setId(const char *keyword,int id,int flags) {
  Tcl_HashEntry *entry=NULL;
  int isNew=0;
  char *dupKey;

  if (id==KEYWORD_INVALID_ID) { return KEYWORD_INVALID_ID; }
  dupKey=Tcl_Alloc(strlen(keyword)+1);
  strcpy(dupKey,keyword);

  entry=Tcl_CreateHashEntry(&keywordsTable,dupKey,&isNew);

  if (entry==NULL) { return KEYWORD_INVALID_ID; }
  if (!isNew) {
    id=(int)Tcl_GetHashValue(entry);
  } else {
    Tcl_SetHashValue(entry,id);
  }
  entry=NULL;
 
  /* Only the first occurence of an id gets a backwards lookup */
  entry=Tcl_CreateHashEntry(&backwardsLookup,(char *)id,&isNew);
  if (entry==NULL) { return TCL_ERROR; }

  if (isNew || (flags&KEYWORD_SET_DEFAULT)) {
    char *oldKey=(char *)Tcl_GetHashValue(entry);
    if (oldKey!=NULL) { Tcl_Free(oldKey); }
    Tcl_SetHashValue(entry,(ClientData)dupKey);
  }
  return id;
}

int 
Keyword_getId(char *key) {
  Tcl_HashEntry *entry=Tcl_FindHashEntry(&keywordsTable,key);
  if (entry==NULL) { return KEYWORD_INVALID_ID; }
  return (int)Tcl_GetHashValue(entry);
}

const char *Keyword_getKey(int id) {
  char *key;
  Tcl_HashEntry *entry=Tcl_FindHashEntry(&backwardsLookup,(char *)id);
  if (entry==NULL) {
    return NULL;
  }
  key=(char *)Tcl_GetHashValue(entry);
  return key;
}
	
int
Keyword_alias(char *alias,char *oldKey,int flags)
{
  int id=Keyword_getId(oldKey);
  if (id==KEYWORD_INVALID_ID) {
    id=Keyword_addKey(oldKey);
    if (id==KEYWORD_INVALID_ID) {
      return id;
    }
  }

  if (TCL_OK!=Keyword_setId(alias,id,flags)) {
    return KEYWORD_INVALID_ID;
  } else {
    return id;
  }
}

int Keyword_addKey(char *key) {
  int result=Keyword_setId(key,entryCount,0);
  if (result==KEYWORD_INVALID_ID) {
    return KEYWORD_INVALID_ID;
  } else {
    if (entryCount==result) { entryCount++; }
    return result;
  }
}

static void
kw_dupRepProc(Tcl_Obj *source, Tcl_Obj *dup)
{
  dup->internalRep.longValue=source->internalRep.longValue;
}

static void
kw_updateStringProc(Tcl_Obj *obj)
{
  int id=obj->internalRep.longValue;
  const char *key=Keyword_getKey(id);
#ifdef DEAL_MEMORY
  fprintf(stderr,"Getting string key for id %d - key %s\n",id,key);
#endif
  if (key==NULL) {
    obj->bytes=Tcl_Alloc(1);
    (obj->bytes)[0]=0;
  } else {
    obj->length=strlen(key);
    obj->bytes=Tcl_Alloc(obj->length+1);
    strcpy(obj->bytes,key);
  }
}


static int
kw_setFromAnyProc(Tcl_Interp *interp,Tcl_Obj *obj) {
  char *key=Tcl_GetString(obj);
  int id;
	
  if (key==NULL) {
    Tcl_AddErrorInfo(interp,
		     "Cannot get string rep for object in kw_setFromAnyProc\n");
    return TCL_ERROR;
  }

#ifdef DEAL_MEMORY
  fprintf(stderr,"Getting Keyword id for key %s\n",key);
#endif

  id=Keyword_getId(key);
  if (id==KEYWORD_INVALID_ID) { 
    Tcl_AddErrorInfo(interp,"Unknown keyword: ");
	Tcl_AddErrorInfo(interp,key);
    return TCL_ERROR;
  }

  obj->typePtr=&KeywordType;
  obj->internalRep.longValue=id;
  return TCL_OK;
}

int
Keyword_getIdFromObj(Tcl_Interp *interp,Tcl_Obj *obj)
{
  if (obj->typePtr!=&KeywordType) {
    int res;
    res=Tcl_ConvertToType(interp,obj,&KeywordType);
    if (res!=TCL_OK) { return KEYWORD_INVALID_ID; }
  }
  return obj->internalRep.longValue;
}

Tcl_Obj *Keyword_NewObj(int id)
{
  Tcl_Obj *obj;

  if (NULL==Keyword_getKey(id)) {
    return NULL;
  }

  obj=Tcl_NewObj();
  if (NULL==obj) {
    return NULL;
  }

  obj->internalRep.longValue=id;
  obj->typePtr=&KeywordType;
  Tcl_InvalidateStringRep(obj);
	
  return obj;
}

int Keyword_Init(Tcl_Interp *interp)
{
  Tcl_InitHashTable(&keywordsTable,TCL_STRING_KEYS);
  Tcl_InitHashTable(&backwardsLookup,TCL_ONE_WORD_KEYS);
  KeywordType.dupIntRepProc=&kw_dupRepProc;
  KeywordType.freeIntRepProc=NULL;
  KeywordType.name="Keywords";
  KeywordType.setFromAnyProc=&kw_setFromAnyProc;
  KeywordType.updateStringProc=&kw_updateStringProc;
  Tcl_RegisterObjType(&KeywordType);

  return TCL_OK;
}
