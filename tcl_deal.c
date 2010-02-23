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


#include <tcl.h>

#include <string.h>

#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
extern int getpid();

#include "deal.h"
#include "vector.h"
#include "stat.h"
#include "tcl_dist.h"
#include "formats.h"
#include "additive.h"
#include "stringbox.h"
#include "dealtypes.h"
#include "holdings.h"
#include <limits.h>
#include "getopt.h"

#include <stdlib.h>
#include "tcl_incl.h"

typedef struct formatter {
  char *(*fn)();
} *FormatFN;

Tcl_Interp *interp;

void Tcl_ObjDelete(ClientData data) {
  Tcl_DecrRefCount((Tcl_Obj *)data);
}

void Tcl_AllocDelete(ClientData data) {
  Tcl_Free((char *)data);
}

int (*next_deal)() = start_deal;

static Tcl_Obj *after_exp=0;
static Tcl_Obj *main_exp=0;

static int cachesize=0;
static Tcl_Obj *resetCmds=NULL;

static void create_cache_reset() {
  if (resetCmds==NULL) {
    Tcl_IncrRefCount(resetCmds=Tcl_NewListObj(0,0));
    cachesize=0;
  }
}

static int do_reset_commands(Tcl_Interp *interp) {
  if (resetCmds!=NULL && cachesize!=0) {
    /*
     * Allow the resetCmds to cache data in the 
     * next resetCmds
     */
    Tcl_Obj *oldCmds=resetCmds;
    int i,count;
    Tcl_Obj **code;
    int result;

    cachesize=0;
    resetCmds=NULL;

    result=Tcl_ListObjGetElements(interp,oldCmds,&count,&code);
    if (result==TCL_OK) {
      for (i=count-1; i>=0; i--) {
	result=Tcl_GlobalEvalObj(interp,code[i]);
	if (result==TCL_RETURN) {
	  /*fprintf(stderr,"Got return in reset\n");*/
	  return TCL_RETURN;
	}
	if (result==TCL_ERROR) {
	  return TCL_ERROR;
	}
      }
      Tcl_ListObjReplace(interp,oldCmds,0,count,0,0);
    }
    Tcl_DecrRefCount(oldCmds);
  }

  return TCL_OK;
}

static void add_reset_cmd(Tcl_Interp *interp,Tcl_Obj *code)
{
  int length;
  if (resetCmds==NULL) {
    create_cache_reset();
  }
  
  Tcl_ListObjLength(interp,resetCmds,&length);

  Tcl_ListObjAppendElement(interp,resetCmds,code);
  cachesize++;
}

static int add_reset_cmds(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int i=1;
  while (i<objc) {
    add_reset_cmd(interp,objv[i++]);
  }
  return TCL_OK;
}

int tcl_reset_deck (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  reset_deck();
  return TCL_OK;
}

int tcl_finish_deal (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  finish_deal();
  return TCL_OK;
}


int tcl_deal_deck (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  static char *result="exit";
  int retval;
  
  retval=do_reset_commands(interp);
  if (retval!=TCL_OK) {
    Tcl_SetResult(interp,result,TCL_STATIC);
    return retval;
  }

  retval=next_deal();

  if (retval==TCL_RETURN) {
    Tcl_SetResult(interp,result,TCL_STATIC);
  }
  return retval;
}

int tcl_format_deal (TCL_PARAMS) TCL_DECL
{
  FormatFN fmt=(FormatFN)cd;
  char *format;
  finish_deal();
  format=fmt->fn();
  if (format!=NULL) {
    Tcl_SetResult(interp,format,TCL_DYNAMIC);
    return TCL_OK;
  } else {
    Tcl_AppendResult(interp,argv[0]," failed due to error: ",
		     Tcl_PosixError(interp),NULL);
    return TCL_ERROR;
  }
}

int tcl_write_deal (TCL_PARAMS) TCL_DECL
{
  FormatFN fmt=(FormatFN)cd;
  char *format;
  FILE *file=stdout;
  finish_deal();
  format=fmt->fn();
  if (argc==2) {
    file=stderr;
  }
  if (format!=NULL) {
    fputs(format,file);
    Tcl_Free(format);
    return TCL_OK;
  } else {
    Tcl_AppendResult(interp,argv[0]," failed due to error: ",
		     Tcl_PosixError(interp),NULL);
    return TCL_ERROR;
  }
}

int tcl_flush_deal (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  return TCL_OK;	/* Default write flushing routing */
}

int tcl_rotate_deal (TCL_PARAMS) TCL_DECL
{
  if (argc!=2) {
    Tcl_AppendResult(interp,
		     "wrong # of args: \"",argv[0]," <rotate>\"",NULL);
    return TCL_ERROR;
  }
  rotate_deal(atoi(argv[1]));
  return TCL_OK;
}

int tcl_stacked_cards (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  Tcl_Obj *holdings[4];
  int handnum, suit;
  int h[4];

  if (objc!=2) {
    Tcl_AddErrorInfo(interp,"Usage: stacked_cards <handname>\n");
    return TCL_ERROR;
  }

  handnum=getHandNumFromObj(interp,objv[1]);
  if (handnum==NOSEAT) {
    Tcl_AddErrorInfo(interp,"Invalid hand name\n");
    return TCL_ERROR;
  }

  get_stacked_cards(handnum,h);
  for (suit=0; suit<4; suit++) {
    holdings[suit]=Tcl_NewHoldingObj(h[suit]);
  }
  Tcl_SetObjResult(interp,Tcl_NewListObj(4,holdings));
  return TCL_OK;
}

int tcl_rand_cmd( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  long res,tclres,modulus;
  res=random();
  if (objc>1) {
    tclres=Tcl_GetLongFromObj(interp,objv[1],&modulus);
    if (tclres==TCL_ERROR) { return TCL_ERROR; }
    res %= modulus;
    Tcl_SetObjResult(interp,Tcl_NewIntObj(res));
  }
  else {
    double dres = res * 1.0 / RANDOM_MAX;
    Tcl_SetObjResult(interp,Tcl_NewDoubleObj(dres));
  }
  return TCL_OK;
}

int tcl_after_set(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  if (objc!=2) {
    return TCL_ERROR;
  }
  if (after_exp != 0) { 
    Tcl_DecrRefCount(after_exp);
  } 
  Tcl_IncrRefCount(after_exp=Tcl_DuplicateObj(objv[1]));

  return TCL_OK;
}

int tcl_seed_deal(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int result,value;
  if (objc!=2) {
    return TCL_ERROR;
  }
  result=Tcl_GetIntFromObj(interp,objv[1],&value);
  if (result==TCL_OK) {
#ifdef USE_RAND48
    srand48(value);
#else
    srandom(value);
#endif
  }
  return TCL_OK;
}
	

int tcl_main_set(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  if (objc!=2) {
    return TCL_ERROR;
  }
  if (main_exp != 0) { Tcl_DecrRefCount(main_exp); }
  Tcl_IncrRefCount(main_exp=Tcl_DuplicateObj(objv[1]));
  return TCL_OK;
}

int tcl_deal_loop(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int result;
  if (objc!=2) {
    return TCL_ERROR;
  }

  while (1) {
    result=tcl_deal_deck(cd,interp,objc,objv);
    if (result==TCL_RETURN) {
      return TCL_RETURN;
    }
    if (result==TCL_ERROR) {
      return TCL_ERROR;
    }
    result=Tcl_GlobalEvalObj(interp,main_exp);
    if (result==TCL_ERROR) { 
      /* fprintf(stderr,"Error in eval loop: %s\n",Tcl_GetStringResult(interp)); */
      return result;
    }
    if (result==TCL_RETURN) {
      Tcl_Obj *output=Tcl_GetObjResult(interp);
      int value;
      if (Tcl_GetIntFromObj(interp,output,&value)!=TCL_ERROR && value>0) {
	break;
      }
    }
#ifdef DEBUG
    fprintf(stderr,"Rejected after %d cards dealt\n",complete_deal.dealt);
#endif
  }
  return Tcl_GlobalEvalObj(interp,objv[1]);
}

static Tcl_Obj *logicObj[2]={ NULL, NULL};

static int tcl_init(TCLOBJ_PARAMS) TCLOBJ_DECL
{
   return Tcl_Init(interp);
}

/*
 * implements the 'accept' and 'reject' commands
 */
int tcl_deal_control(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int i=2,value,result;
  Tcl_Obj *iffound,*ifnotfound;
  int data;
  int found,nfound;
  char *logic; /* Either "if" of "unless" */
  int length;

  if (logicObj[0]==NULL) {
    logicObj[0]=Tcl_NewBooleanObj(0);
    Tcl_IncrRefCount(logicObj[0]);
    logicObj[1]=Tcl_NewBooleanObj(1);
    Tcl_IncrRefCount(logicObj[1]);
  }

  data=(int)cd;  /* True if 'accept', false if 'reject' */
  if (objc==1) {
    Tcl_SetObjResult(interp,logicObj[data ? 1 : 0]);
    return TCL_RETURN;
  }

  logic=Tcl_GetStringFromObj(objv[1],&length);

  if (0==strcmp(logic,"if")) {
    iffound=logicObj[data!=0];
    found=TCL_RETURN;
    ifnotfound=logicObj[data==0];
    nfound=TCL_OK;
  } else if (0==strcmp(logic,"unless")) {
    iffound=logicObj[data==0];
    found=TCL_OK;
    ifnotfound=logicObj[data!=0];
    nfound=TCL_RETURN;
  } else {
    return TCL_ERROR;
  }

  for (i=2; i<objc; i++) {
    result=Tcl_ExprBooleanObj(interp,objv[i],&value);

    if (result==TCL_ERROR) { return TCL_ERROR; }

    if (value) {
      Tcl_IncrRefCount(iffound);
      Tcl_SetObjResult(interp,iffound);
      if (verbose>1) {
	fprintf(stderr,"Condition %s passed\n",Tcl_GetStringFromObj(objv[i],&length));
      }
      return found;
    }
  }
  Tcl_IncrRefCount(ifnotfound);
  Tcl_SetObjResult(interp,ifnotfound);
  return nfound;
}


DEAL31_API int *Deal_Init(Tcl_Interp *interp)
{
  int result;
  FormatFN compact=(struct formatter*)Tcl_Alloc(sizeof(struct formatter));
  FormatFN deffmt=(struct formatter*)Tcl_Alloc(sizeof(struct formatter));

  initializeLengths();
  init_name_tables();

  initializeDealTypes(interp);
  compact->fn=&format_deal_compact;
  Tcl_CreateCommand(interp,"write_deal_compact",tcl_write_deal,
		    (ClientData)compact,NULL);
  Tcl_CreateCommand(interp,"format_deal_compact",tcl_format_deal,
		    (ClientData)compact,NULL);

  deffmt->fn=&format_deal_verbose;
  Tcl_CreateCommand(interp,"write_deal",tcl_write_deal,
		    (ClientData)deffmt,NULL);
  Tcl_CreateCommand(interp,"format_deal",tcl_format_deal,
		    (ClientData)deffmt,NULL);

  Tcl_CreateCommand(interp,"write_deal_verbose",tcl_write_deal,
		    (ClientData)deffmt,NULL);
  Tcl_CreateObjCommand(interp,"flush_deal",tcl_flush_deal,
		       NULL,NULL);
  Tcl_CreateObjCommand(interp,"stacked",tcl_stacked_cards,
		       NULL,NULL);

  Tcl_CreateCommand(interp,"rotatedeal",tcl_rotate_deal,NULL,NULL);

  Tcl_CreateCommand(interp,"sdev",tcl_sdev_define,NULL,NULL);
  Tcl_CreateObjCommand(interp,"correlation",tcl_correlation_define,NULL,NULL);

  HandCmd_Init(interp);
  Dist_Init(interp);
  Vector_Init(interp);
  Stringbox_Init(interp);
  IDealHolding_Init(interp);
  DDS_Init(interp);


  Tcl_CreateObjCommand(interp,"deal_deck",tcl_deal_deck,NULL,NULL);
  Tcl_CreateObjCommand(interp,"reset_deck",tcl_reset_deck,NULL,NULL);
  Tcl_CreateObjCommand(interp,"finish_deal",tcl_finish_deal,NULL,NULL);
  Tcl_CreateObjCommand(interp,"main",tcl_main_set,NULL,NULL);
  Tcl_CreateObjCommand(interp,"deal_finished",tcl_after_set,NULL,NULL);
  Tcl_CreateObjCommand(interp,"deal_loop",tcl_deal_loop,NULL,NULL);

  Tcl_CreateObjCommand(interp,"seed_deal",tcl_seed_deal,NULL,NULL);
  Tcl_CreateObjCommand(interp,"reject",tcl_deal_control,(ClientData)0,NULL);
  Tcl_CreateObjCommand(interp,"accept",tcl_deal_control,(ClientData)1,NULL);
  Tcl_CreateObjCommand(interp,"rand",tcl_rand_cmd,NULL,NULL);

  Tcl_CreateObjCommand(interp,"deal_reset_cmds",add_reset_cmds,NULL,NULL);

  Tcl_CreateObjCommand(interp,"deal_init_tcl",tcl_init,NULL,NULL);

  result=Tcl_VarEval(interp,"source deal.tcl",NULL);
  if (result==TCL_ERROR) {
    tcl_error(interp);
  }

  Tcl_VarEval(interp,"reset_deck",NULL);

  return TCL_OK;
}

static int executeScript(Tcl_Interp *interp, 
                         const char *argv0,
                         int argc,
                         char *argv[],
                         const char *command) {
  int i, result;
  Tcl_Obj *list = Tcl_NewListObj(0,NULL);
  Tcl_SetVar(interp,"argv0",argv0,TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp,"argc",NULL,Tcl_NewIntObj(argc),TCL_GLOBAL_ONLY);

  for (i=0;i<argc;i++) {
    Tcl_ListObjAppendElement(interp,list,Tcl_NewStringObj(argv[i],strlen(argv[i])));
  }
  Tcl_SetVar2Ex(interp,"argv",NULL,list, TCL_GLOBAL_ONLY);
  result=Tcl_VarEval(interp,command,NULL);
  return result;
}

int old_main(argc,argv)
     int argc;
     char *argv[];
{

  int i;
  int count=10;
  char tcl_command_string[512];
  char *writecmd="write_deal";
  char *flushcmd="flush_deal";
  time_t for_seeding;
  Tcl_Obj *command;
  Tcl_Interp *interp=Tcl_CreateInterp();
  
  int opt;
  int result;
  extern int optind;
  extern char *optarg;

  time(&for_seeding);
#ifdef USE_RAND48
  srand48(for_seeding ^ getpid());
#else
  srandom(for_seeding);
#endif

  init_name_tables();
  
  Deal_Init(interp);

  while (-1!=(opt=getopt(argc,argv,"lve:S:N:E:W:i:ts:fo:VI:x:"))) {
    switch (opt) {
    case 'l':
      writecmd="write_deal_compact";
      break;
    case 'V':
      verbose=2;
      break;
    case 'v':
      verbose=1;
      break;
    case 'e':
      result=Tcl_VarEval(interp,optarg,NULL);
      if (result==TCL_ERROR) {
	tcl_error(interp);
      }
      break;

    case 'I':
      result=Tcl_VarEval(interp,"deal::input ",optarg,NULL);
      if (result==TCL_ERROR) {
	tcl_error(interp);
      }
      break;

    case 'S':
    case 'N':
    case 'E':
    case 'W':
      {
	int hand=hand_name_table[opt];
	int tclret=Tcl_VarEval(interp,
			       handname[hand]," is ",optarg,NULL);

	if (TCL_OK!=tclret) {
	  fprintf(stderr,"Failure attempts to stack hand %s\n",optarg);
	  Tcl_GlobalEval(interp,"puts stderr $errorInfo");
	  exit(1);
	}

      }
      break;

    case 's':
      for_seeding=atoi(optarg);
#ifdef USE_RAND48
      srand48(for_seeding);
#else
      srandom(for_seeding);
#endif

      break;
    case 'x':
      sprintf(tcl_command_string,"source %s",optarg);
      result = executeScript(interp, optarg,argc-optind,argv+optind, tcl_command_string);
      if (result==TCL_ERROR) {
	tcl_error(interp);
      }
      exit(0);
    case 'i':
      sprintf(tcl_command_string,"source %s",optarg);
      result=Tcl_VarEval(interp,tcl_command_string,NULL);
      if (result==TCL_ERROR) {
	tcl_error(interp);
      }
      break;
	  
    case 't':
      printDistTable();
      exit(1);
    default:
      fprintf(stderr,"usage:  %s [-v] [-s seed] [-i includeFile] [-I inputFormat] [count]\n",
	      argv[0]);
      exit(1);
    }
  }
  
  argc-=optind-1;
  argv+=optind-1;
  
  if (argc>1 && isdigit(*argv[1])) {
    count=atoi(argv[1]);
    argc--; argv++;
  }


  if (main_exp!=(Tcl_Obj *)0) {
    sprintf(tcl_command_string,"deal_loop %s",writecmd);
    argc--; argv++;
  } else {
    sprintf(tcl_command_string,"deal_deck ; %s",writecmd);
  }
 
  command=Tcl_NewStringObj(tcl_command_string,(int)strlen(tcl_command_string));

  Tcl_IncrRefCount(command);

  for (i=1; i<=count; i++) {
    const char *s;
    result=Tcl_GlobalEvalObj(interp,command);
    if (result==TCL_ERROR) { tcl_error(interp); }

    if (result==TCL_RETURN) { break; }

    s=Tcl_GetStringResult(interp);
    if (*s=='e' && (0==strcmp("exit",s))) {
      break;
    }

    if (verbose) {
      fprintf(stderr,"Deal %d found after %d tries\n",i,count_deals);
    }
  }
  result=Tcl_VarEval(interp,flushcmd,NULL);
  if (result==TCL_ERROR) { tcl_error(interp); exit(1); }
  if (after_exp) { 
    result=Tcl_GlobalEvalObj(interp,after_exp); 
    if (result==TCL_ERROR) { tcl_error(interp); exit(1); }
  }
  return 0;
}
