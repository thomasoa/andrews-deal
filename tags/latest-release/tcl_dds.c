#include "deal.h"
#include "tcl_incl.h"
#include "dealtypes.h"
#include "ddsInterface.h"

#ifdef BENCH
int totalNodes = 0;
#endif

static int LastTrump = -1;
static int CountCalls = 0;

static int parse_diagram(Tcl_Interp *interp,Tcl_Obj *diagram, struct deal *aDeal, int *handLength) {
  int retval,length;
  int suitHoldings[4];
  int countHand[4];
  Tcl_Obj **hands;
  int hand,suit;

  for (suit=0; suit<4; suit++) {
    suitHoldings[suit]=0;
    for (hand=0; hand<4; hand++) {
      aDeal->remaining.cards[hand][suit]=0;
    } 
  }

  retval = Tcl_ListObjGetElements(interp,diagram,&length,&hands);
  if (retval !=TCL_OK) {
    return TCL_ERROR;
  }
  if (length!=4) {
    return TCL_ERROR;
  }

  for (hand=0; hand<4; hand++) {
    int suitCount;
    Tcl_Obj **suits;
    countHand[hand] = 0;
    retval = Tcl_ListObjGetElements(interp,hands[hand],&suitCount, &suits);
    if (retval != TCL_OK) {
      return TCL_ERROR;
    }
    if (suitCount>4) {
      return TCL_ERROR;
    }
    for (suit=0; suit<suitCount; suit++) {
      int holding;
      int suitLength;
      holding = getHoldingNumFromObj(interp,suits[suit]);
      if (holding<0) {
	return TCL_ERROR;
      }
      holding &= 8191;
      aDeal->remaining.cards[hand][suit] = holding;
      suitLength = counttable[holding];
      countHand[hand] += suitLength;
      if (suitHoldings[suit] & holding) {
	/* Not disjoint */
	return TCL_ERROR;
      }
      suitHoldings[suit] |= (holding);
    }
  }

  for (hand=1; hand<4; hand++) {
    if (countHand[hand] != countHand[0]) {
      /* Hands not all same size */
      return TCL_ERROR;
    }
  }
  *handLength = countHand[0];

  return TCL_OK;

}

static int tcl_double_dummy_reset(TCLOBJ_PARAMS) TCLOBJ_DECL
{
   LastTrump = -1;
   return TCL_OK;
}

static int tcl_dds(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  static int doInit=1,
    DiagramFlagID=-1,
    GoalFlagID=-1,
    NoReuseFlagID=-1,
    ReuseFlagID=-1,
    LeaderFlagID=-1, 
    TrickFlagID=-1;

  int goal=-1;
  int playerGoal = -1;
  int hand, suit,result;
  int mode=-1;
  int declarer = 2;
  struct deal d;
  int status;
  int handLength;
  int cardsPlayedToTrick=0;
  int totalTricksPlayed=0;
  int played[4];
  Tcl_Obj *diagram = NULL;

  struct futureTricks futp;
  if (doInit) {
    ReuseFlagID=Keyword_addKey("-reuse");
    NoReuseFlagID=Keyword_addKey("-noreuse");
    DiagramFlagID=Keyword_addKey("-diagram");
    GoalFlagID=Keyword_addKey("-goal");
    LeaderFlagID=Keyword_addKey("-leader");
    TrickFlagID=Keyword_addKey("-trick");
    doInit=0;
  }

  memset(&(d.currentTrickSuit), 0, 3*sizeof(int));
  memset(&(d.currentTrickRank), 0, 3*sizeof(int));
  d.trump=4; /* notrump */
  d.first=-1; /* unknown */

  for (suit=0; suit<4; suit++) {
    played[suit] = 0;
  }

  while (objc > 1) {
    int id = Keyword_getIdFromObj(interp,objv[1]);
    objv++; objc--;
    if (id == ReuseFlagID) {
      mode=2;
      continue;
    }
    if (id == NoReuseFlagID) {
      mode=1;
      continue;
    }
    if (id == DiagramFlagID || id == GoalFlagID || id == LeaderFlagID || id == TrickFlagID) {
      Tcl_Obj *arg = objv[1];
      if (objc>1) {
	objc--; objv++;
      } else {
	return TCL_ERROR;
      }
      
      if (id == DiagramFlagID) {
	if (diagram!=NULL) {
	  return TCL_ERROR;
	}
	diagram = arg;
      } else if (id == GoalFlagID ) {
	if (TCL_ERROR == Tcl_GetIntFromObj(interp,arg,&goal) || (goal!=-1 && (goal<1 && goal>13))) {
	  Tcl_AppendResult(interp,"Invalid tricks goal: ",Tcl_GetString(arg),NULL);
	  return TCL_ERROR;
	}
	
      } else if (id == LeaderFlagID) {
        d.first = getHandNumFromObj(interp,arg);
        if (d.first == NOSEAT) {
          Tcl_AppendResult(interp,"invalid seat name passed to -leader: ",Tcl_GetString(arg),NULL);
          return TCL_ERROR;
        }
      } else if (id == TrickFlagID) {
         Tcl_Obj **cards;
         int playNo,rank;
         if (cardsPlayedToTrick!=0) {
           Tcl_AppendResult(interp,"No additional tricks after an incomplete trick",NULL);
         }

         if (TCL_ERROR == Tcl_ListObjGetElements(interp,arg,&cardsPlayedToTrick,&cards) || 
		cardsPlayedToTrick>4) {
           Tcl_AppendResult(interp,"Invalid -trick argument: ",Tcl_GetString(arg),NULL);
           return TCL_ERROR;
         }

         for (playNo=0; playNo<cardsPlayedToTrick; playNo++) {
           int card=getCardNumFromObj(interp,cards[playNo]);
           if (card==NOCARD) {
             Tcl_AppendResult(interp,"Invalid card ", Tcl_GetString(cards[playNo]), " in -trick argument: ",Tcl_GetString(arg),NULL);
             return TCL_ERROR;
           }
           rank = RANK(card);
           suit = SUIT(card);
           played[suit] |= 1<<(12-rank);
           if (cardsPlayedToTrick<4) {
             d.currentTrickRank[playNo]=14-rank;
             d.currentTrickSuit[playNo]=suit;
           }
         }

         if (cardsPlayedToTrick==4) {
           totalTricksPlayed++;
           cardsPlayedToTrick=0;
         }
      }
      continue;

    }
    objv--; objc++;
    break;
  }

  if (objc > 2) {
    d.trump = getDenomNumFromObj(interp,objv[2]);
    if (d.trump==-1) {
     Tcl_AppendResult(interp,"Invalid denomination: ", Tcl_GetString(objv[2]), NULL);
     return TCL_ERROR;
    }
  }

  if (objc > 1) {
    declarer = getHandNumFromObj(interp,objv[1]);
    if (declarer == NOSEAT) {
      Tcl_AppendResult(interp,"invalid declarer name passed: ",Tcl_GetString(objv[1]),NULL);
      return TCL_ERROR;
    }
  }

  if (d.first==-1) {
    d.first = (declarer+1)%4;
  }


  if (diagram != NULL) {
    int retval = parse_diagram(interp,diagram, &d,&handLength);
    if (retval != TCL_OK) {
      return TCL_ERROR;
    }
  } else {
    finish_deal();
    handLength = 13;
    for (hand=0; hand<4; hand++) {
      /* Double dummy solver has north hand zero */
      int ddsHand = hand;
      for (suit=0; suit<4; suit++) {
	d.remaining.cards[ddsHand][suit] = globalDeal.hand[hand].suit[suit];
      }
    }
  }

  for (hand=0; hand<4; hand++) {
    for (suit=0; suit<4; suit++) {
      d.remaining.cards[hand][suit] &= (~played[suit]);
    }
  }
  handLength = handLength - totalTricksPlayed;
 
  if (goal>0) {
    if ((d.first + cardsPlayedToTrick + declarer)&1) {
      playerGoal = (handLength + 1)-goal; /* Goal passed to DDS */
    } else {
      playerGoal = goal; /* Goal passed to DDS */
    }
  }

  if (CountCalls == 0) {
    mode = 0;
  } else if ( mode == -1 ) {
    if (d.trump != LastTrump || diagram != NULL) {
      mode = 1;
    } else {
      mode = 2;
    }
  }

  /* fprintf(stderr,"mode = %d\n",mode); */
  status = SolveBoard(d,playerGoal,1,mode,&futp);
  LastTrump = d.trump;
  CountCalls++;
  
  if (status != 1) {
    Tcl_SetObjResult(interp,Tcl_NewIntObj(status));
    Tcl_AppendResult(interp,": dds failed due to error status from double dummy solver",NULL);
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
      result = handLength;
    }
  } else {

    if ((d.first + cardsPlayedToTrick + declarer)&1) {
      result = handLength-futp.score[0];
    } else {
      result = futp.score[0];
    }

    if (goal>0) {
      result = (result>=goal);
    }
  }
  Tcl_SetObjResult(interp,Tcl_NewIntObj(result));
  return TCL_OK;
}

/* This code directly borrows from Alex Martelli's Python interface to dds. */
static int tcl_double_dummy_solve(TCLOBJ_PARAMS) TCLOBJ_DECL
{
  int goal=-1;
  int leaderGoal = -1;
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
    leaderGoal = 14-goal; /* Goal passed to DDS */
  }


  for (hand=0; hand<4; hand++) {
    /* Double dummy solver has north hand zero */
    int ddsHand = hand;
    for (suit=0; suit<4; suit++) {
      d.remaining.cards[ddsHand][suit] = globalDeal.hand[hand].suit[suit];
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
  status = SolveBoard(d,leaderGoal,1,mode,&futp);
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
  Tcl_CreateObjCommand(interp,"dds",tcl_dds,NULL,NULL);
  Tcl_CreateObjCommand(interp,"dds_reset",tcl_double_dummy_reset,NULL,NULL);
  return TCL_OK;
}
