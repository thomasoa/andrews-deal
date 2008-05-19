#include "deal.h"
#include "tcl_incl.h"
#include "dealtypes.h"
#include "ddsInterface.h"

#ifdef BENCH
int totalNodes = 0;
#endif

static int LastTrump = -1;
static int CountCalls = 0;

static int tcl_double_dummy_reset(TCLOBJ_PARAMS) TCLOBJ_DECL
{
   LastTrump = -1;
   return TCL_OK;
}

/* This code directly borrows from Alex Martelli's Python interface to dds. */
static int tcl_double_dummy_solve(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int goal=-1;
  int defenseGoal = -1;
  int hand, suit,result;
  int mode;
  struct deal d;
  int status;

  struct futureTricks futp;
  memset(&(d.currentTrickSuit), 0, 3*sizeof(int));
  memset(&(d.currentTrickRank), 0, 3*sizeof(int));
  d.trump=4; /* notrump */
  d.first=WEST; /* west */
  finish_deal();
  
  if (objc > 2) {
    d.trump = getDenomNumFromObj(interp,objv[2]);
    if (d.trump==-1) {
     Tcl_AppendResult(interp,"Invalid denomination: ", Tcl_GetString(objv[2]), NULL);
     return TCL_ERROR;
    }
  }
  if (objc > 1) {
    int declarer = getHandNumFromObj(interp,objv[1]);
    if (declarer == NOSEAT) {
      Tcl_AppendResult(interp,"invalid seat name passed: ",Tcl_GetString(objv[1]),NULL);
      return TCL_ERROR;
    }
    d.first = (declarer+1)%4;
  }

  if (objc > 3) {
    if (TCL_ERROR == Tcl_GetIntFromObj(interp,objv[3],&goal) || (goal!=-1 && (goal<1 && goal>13))) {
       Tcl_AppendResult(interp,"Invalid tricks goal: ",Tcl_GetString(objv[3]),NULL);
       return TCL_ERROR;
    }
  }

  if (goal>0) {
    defenseGoal = 14-goal; /* Goal passed to DDS */
  }


  for (hand=0; hand<4; hand++) {
    /* Double dummy solver has north hand zero */
    int ddsHand = hand;
    for (suit=0; suit<4; suit++) {
      d.remainCards[ddsHand][suit] = globalDeal.hand[hand].suit[suit] << 2;
    }
  }
 
  if (d.trump != LastTrump) {
    if (CountCalls == 0) { 
       mode = 0;
    } else {
       mode = 1;
    }
  } else {
    mode = 2;
  }
  /* fprintf(stderr,"mode = %d\n",mode); */
  status = SolveBoard(d,defenseGoal,1,mode,&futp);
  LastTrump = d.trump;
  CountCalls++;
  
  if (status != 1) {
    Tcl_SetObjResult(interp,Tcl_NewIntObj(status));
    Tcl_AppendResult(interp,"dds failed due to error status from double dummy solver",NULL);
    return TCL_ERROR;
  }
#ifdef BENCH
  totalNodes += futp.totalNodes;
  printf("nodes=%d totalNodes=%d\n",futp.totalNodes,totalNodes);
#endif
  if (futp.score[0]==-2) {
    /* Defender takes all the tricks */
    result = 0;
  } else if (futp.score[0]==-1) {
    if (goal>0) {
      result = 1;
    } else {
      result = 13;
    }
  } else {
    result = 13-futp.score[0];
    if (goal>0) {
      result = (result>=goal);
    }
  }
  Tcl_SetObjResult(interp,Tcl_NewIntObj(result));
  return TCL_OK;
}

int DDS_Init(interp)
     Tcl_Interp *interp;
{
  DDSInitStart();
  Tcl_CreateObjCommand(interp,"tricks",tcl_double_dummy_solve,NULL,NULL);
  Tcl_CreateObjCommand(interp,"dds_reset",tcl_double_dummy_reset,NULL,NULL);
  return TCL_OK;
}
