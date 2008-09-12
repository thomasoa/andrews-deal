
/* DDS 1.1.8   A bridge double dummy solver.                                            */
/* Copyright (C) 2006-2008 by Bo Haglund                                      */
/* Cleanups and porting to Linux and MacOSX (C) 2006 by Alex Martelli         */
/*                                                                                              */
/* This program is free software; you can redistribute it and/or              */
/* modify it under the terms of the GNU General Public License                */
/* as published by the Free software Foundation; either version 2             */
/* of the License, or (at your option) any later version.                     */ 
/*                                                                                              */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */
/*                                                                                              */
/* You should have received a copy of the GNU General Public License          */
/* along with this program; if not, write to the Free Software                */
/* Foundation, Inc, 51 Franklin Street, 5th Floor, Boston MA 02110-1301, USA. */

#include "dds.h"

extern "C" unsigned short int counttable[];

const int ContractInfo::nextSuitArray[4][4] = {
    { 1, 2, 3, 4},
    { 2, 0, 3, 4},
    { 1, 3, 0, 4},
    { 1, 2, 4, 0}
};

GLOBALS Globals;

int nodeTypeStore[4];
unsigned short int lowestWin[50][4];
int nodes;
int trickNodes;
int no[50];
int iniDepth;
int handToPlay;
int payOff, val;

struct pos lookAheadPos; /* Is initialized for starting  alpha-beta search */
struct moveType forbiddenMoves[14];
struct moveType initialMoves[4];
struct moveType cd;
struct movePlyType movePly[50];
int tricksTarget;
struct gameInfo game;
int estTricks[4];
FILE *fp2, *fp7, *fp11;
struct moveType * bestMove;
struct winCardType * temp_win;
int hiwinSetSize=0, hinodeSetSize=0;
int hilenSetSize=0;
int MaxnodeSetSize=0;
int MaxwinSetSize=0;
int MaxlenSetSize=0;
int nodeSetSizeLimit=0;
int winSetSizeLimit=0;
int lenSetSizeLimit=0;
LONGLONG maxmem, allocmem, summem;
int wmem, nmem, lmem;
int maxIndex;
int wcount, ncount, lcount;
int clearTTflag=FALSE, windex=-1;
int ttCollect=FALSE;
int suppressTTlog=FALSE;

int * highestRank;
struct adaptWinRanksType * adaptWins;

unsigned char cardRank[15], cardSuit[5], cardHand[4];
LONGLONG suitLengths=0;
struct posSearchType *rootnp[14][4];
struct winCardType **pw;
struct nodeCardsType **pn;
struct posSearchType **pl;

#ifdef CANCEL
int cancelOrdered=FALSE;
int cancelStarted=FALSE;
int threshold=CANCELCHECK;
#endif


#ifdef _MANAGED
#pragma managed(push, off)
#endif


extern "C" inline holding_t distinctUnplayedCards(holding_t origHolding, holding_t played,holding_t *sequence) {
   holding_t bitRank;
   holding_t unplayed = origHolding & (~played);
   holding_t result = 0;
   int inSequence=0;
   *sequence = 0;

   if (unplayed) {
     for (bitRank = (1<<12); bitRank; bitRank >>= 1) {
       if (unplayed & bitRank) {
          if (inSequence) {
            *sequence |= bitRank;
          } else {
            result |= bitRank;
            inSequence = 1;
          }
       } else {
          if (!(played & bitRank)) {
            inSequence = 0;
          }
       }
     }
   }
   return  result;
}

struct UnplayedCardsFinder {
protected:
  struct diagram starting;

public:

  UnplayedCardsFinder() {}

  void initialize(const struct diagram &diagram) {
    for (int hand=0; hand<4; hand++) {
      for (int suit=0; suit<4; suit++) {
        starting.cards[hand][suit] = diagram.cards[hand][suit];
      }
    }
  }


  inline holding_t getUnplayed(int hand, int suit, holding_t played,holding_t &sequence) {
    holding_t holding = starting.cards[hand][suit];
    return distinctUnplayedCards(holding,played,&sequence);
  }

} unplayedFinder;


#ifdef _MANAGED
#pragma managed(pop)
#endif

  int SolveBoard(struct deal dl, int target,
    int solutions, int mode, struct futureTricks *futp) {

  int k, n, cardCount, found, totalTricks, tricks, last, checkRes;
  int g, upperbound, lowerbound, first, i, j, forb, ind, flag, noMoves;
  int hand, suit;
  int mcurr;
  int noStartMoves;
  int handRelFirst;
  int latestTrickSuit[4];
  int latestTrickRank[4];
  int maxHand=0, maxSuit=0, maxRank;
  struct pos iniPosition;
  struct movePlyType temp;
  struct moveType mv;
  /*FILE *fp;*/
  
  /*InitStart();*/   /* Include InitStart() if inside SolveBoard,
                           but preferable InitStart should be called outside
                                         SolveBoard like in DllMain for Windows. */

  for (k=0; k<=13; k++) {
    forbiddenMoves[k].rank=0;
    forbiddenMoves[k].suit=0;
  }

  if (target<-1) {
    DumpInput(-5, dl, target, solutions, mode);
    return -5;
  }
  if (target>13) {
    DumpInput(-7, dl, target, solutions, mode);
    return -7;
  }
  if (solutions<1) {
    DumpInput(-8, dl, target, solutions, mode);
    return -8;
  }
  if (solutions>3) {
    DumpInput(-9, dl, target, solutions, mode);
    return -9;
  }
  
  if (target==-1) {
    tricksTarget=99;
  } else {
    tricksTarget=target;
  }

  cardCount=0;
  for (hand=0; hand<=3; hand++) {
    for (suit=0; suit<=3; suit++) {
      game.diagram.cards[hand][suit]=dl.remaining.cards[hand][suit];
      cardCount+=CountOnes(game.diagram.cards[hand][suit]);
    }
  }

  if (dl.currentTrickRank[2]) {
    if ((dl.currentTrickRank[2]<2)||(dl.currentTrickRank[2]>14)
      ||(dl.currentTrickSuit[2]<0)||(dl.currentTrickSuit[2]>3)) {
      DumpInput(-12, dl, target, solutions, mode);
      return -12;
    }
    handToPlay=rho(dl.first);
    handRelFirst=3;
    noStartMoves=3;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.diagram.cards[handToPlay][k]);
          break;
        }
      }
      latestTrickSuit[partner(dl.first)]=dl.currentTrickSuit[2];
      latestTrickRank[partner(dl.first)]=dl.currentTrickRank[2];
      latestTrickSuit[lho(dl.first)]=dl.currentTrickSuit[1];
      latestTrickRank[lho(dl.first)]=dl.currentTrickRank[1];
      latestTrickSuit[dl.first]=dl.currentTrickSuit[0];
      latestTrickRank[dl.first]=dl.currentTrickRank[0];
    }
  } else if (dl.currentTrickRank[1]) {
    if ((dl.currentTrickRank[1]<2)||(dl.currentTrickRank[1]>14)
      ||(dl.currentTrickSuit[1]<0)||(dl.currentTrickSuit[1]>3)) {
      DumpInput(-12, dl, target, solutions, mode);
      return -12;
    }
    handToPlay=partner(dl.first);
    handRelFirst=2;
    noStartMoves=2;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.diagram.cards[handToPlay][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[rho(dl.first)][k]!=0) {
          latestTrickSuit[rho(dl.first)]=k;
          latestTrickRank[rho(dl.first)]=
            InvBitMapRank(game.diagram.cards[rho(dl.first)][k]);
          break;
        }
      }
      latestTrickSuit[lho(dl.first)]=dl.currentTrickSuit[1];
      latestTrickRank[lho(dl.first)]=dl.currentTrickRank[1];
      latestTrickSuit[dl.first]=dl.currentTrickSuit[0];
      latestTrickRank[dl.first]=dl.currentTrickRank[0];
    }
  } else if (dl.currentTrickRank[0]) {
    if ((dl.currentTrickRank[0]<2)||(dl.currentTrickRank[0]>14)
      ||(dl.currentTrickSuit[0]<0)||(dl.currentTrickSuit[0]>3)) {
      DumpInput(-12, dl, target, solutions, mode);
      return -12;
    }
    handToPlay=lho(dl.first);
    handRelFirst=1;
    noStartMoves=1;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.diagram.cards[handToPlay][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[rho(dl.first)][k]!=0) {
          latestTrickSuit[rho(dl.first)]=k;
          latestTrickRank[rho(dl.first)]=
            InvBitMapRank(game.diagram.cards[rho(dl.first)][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[partner(dl.first)][k]!=0) {
          latestTrickSuit[partner(dl.first)]=k;
          latestTrickRank[partner(dl.first)]=
            InvBitMapRank(game.diagram.cards[partner(dl.first)][k]);
          break;
        }
      }
      latestTrickSuit[dl.first]=dl.currentTrickSuit[0];
      latestTrickRank[dl.first]=dl.currentTrickRank[0];
    }
  } else {
    handToPlay=dl.first;
    handRelFirst=0;
    noStartMoves=0;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.diagram.cards[handToPlay][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[rho(dl.first)][k]!=0) {
          latestTrickSuit[rho(dl.first)]=k;
          latestTrickRank[rho(dl.first)]=
            InvBitMapRank(game.diagram.cards[rho(dl.first)][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[partner(dl.first)][k]!=0) {
          latestTrickSuit[partner(dl.first)]=k;
          latestTrickRank[partner(dl.first)]=
            InvBitMapRank(game.diagram.cards[partner(dl.first)][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.diagram.cards[lho(dl.first)][k]!=0) {
          latestTrickSuit[lho(dl.first)]=k;
          latestTrickRank[lho(dl.first)]=
            InvBitMapRank(game.diagram.cards[lho(dl.first)][k]);
          break;
        }
      }
    }
  }

  game.contract=100+10*(dl.trump+1);
  game.first=dl.first;
  first=dl.first;
  game.noOfCards=cardCount;
  if (dl.currentTrickRank[0]!=0) {
    game.leadHand=dl.first;
    game.leadSuit=dl.currentTrickSuit[0];
    game.leadRank=dl.currentTrickRank[0];
  } else {
    game.leadHand=0;
    game.leadSuit=0;
    game.leadRank=0;
  }

  for (k=0; k<=2; k++) {
    initialMoves[k].suit=255;
    initialMoves[k].rank=255;
  }

  for (k=0; k<noStartMoves; k++) {
    initialMoves[noStartMoves-1-k].suit=dl.currentTrickSuit[k];
    initialMoves[noStartMoves-1-k].rank=dl.currentTrickRank[k];
  }

  if (cardCount % 4) {
    totalTricks=(cardCount-4)/4+2;
  } else {
    totalTricks=(cardCount-4)/4+1;
  }

  checkRes=CheckDeal(&cd);
  if (game.noOfCards<=0) {
    DumpInput(-2, dl, target, solutions, mode);
    return -2;
  }
  if (game.noOfCards>52) {
    DumpInput(-10, dl, target, solutions, mode);
    return -10;
  }
  if (totalTricks<target) {
    DumpInput(-3, dl, target, solutions, mode);
    return -3;
  }
  if (checkRes) {
    DumpInput(-4, dl, target, solutions, mode);
    return -4;
  }

  if (cardCount<=4) {
    maxSuit = latestTrickSuit[dl.first];
    maxRank = 0;
    for (k=0; k<=3; k++) {
      if ((dl.trump != maxSuit && dl.trump == latestTrickSuit[k]) ||
          (maxSuit == latestTrickSuit[k] && maxRank<latestTrickRank[k])) {
        maxRank=latestTrickRank[k];
        maxSuit=dl.trump;
        maxHand=k;
      }
    }
    futp->nodes=0;
    #ifdef BENCH
    futp->totalNodes=0;
    #endif
    futp->cards=1;
    futp->suit[0]=latestTrickSuit[handToPlay];
    futp->rank[0]=latestTrickRank[handToPlay];
    futp->equals[0]=0;
    if ((target==0)&&(solutions<3)) {
      futp->score[0]=0;
    } else if ((handToPlay==maxHand)||
               (partner(handToPlay)==maxHand)) {
      futp->score[0]=1;
    } else {
      futp->score[0]=0;
    }
    return 1;
  }

  if (mode!=2) {
    Wipe();
    winSetSizeLimit=WINIT;
    nodeSetSizeLimit=NINIT;
    lenSetSizeLimit=LINIT;
    allocmem=(WINIT+1)*sizeof(struct winCardType);
    allocmem+=(NINIT+1)*sizeof(struct nodeCardsType);
    allocmem+=(LINIT+1)*sizeof(struct posSearchType);
    winCards=pw[0];
    nodeCards=pn[0];
    posSearch=pl[0];
    wcount=0; ncount=0; lcount=0;
    InitGame(0, FALSE, first, handRelFirst,iniPosition);
  } else {
    InitGame(0, TRUE, first, handRelFirst,iniPosition);
  }

  unplayedFinder.initialize(game.diagram);

  nodes=0; trickNodes=0;
  iniDepth=cardCount-4;
  hiwinSetSize=0;
  hinodeSetSize=0;

  if (mode==0) {
    MoveGen(&lookAheadPos, iniDepth);
    if (movePly[iniDepth].last==0) {
      futp->nodes=0;
      #ifdef BENCH
      futp->totalNodes=0;
      #endif
      futp->cards=1;
      futp->suit[0]=movePly[iniDepth].move[0].suit;
      futp->rank[0]=movePly[iniDepth].move[0].rank;
      futp->equals[0]=
          movePly[iniDepth].move[0].sequence<<2;
      futp->score[0]=-2;
      return 1;
    }
  }

  if ((target==0)&&(solutions<3)) {
    MoveGen(&lookAheadPos, iniDepth);
    futp->nodes=0;
        #ifdef BENCH
    futp->totalNodes=0;
    #endif
    for (k=0; k<=movePly[iniDepth].last; k++) {
        futp->suit[k]=movePly[iniDepth].move[k].suit;
        futp->rank[k]=movePly[iniDepth].move[k].rank;
        futp->equals[k]=
          movePly[iniDepth].move[k].sequence<<2;
        futp->score[k]=0;
    }
    if (solutions==1)
        futp->cards=1;
    else
        futp->cards=movePly[iniDepth].last+1;
    return 1;
  }

  if ((target!=-1)&&(solutions!=3)) {
    val=ABsearch(&lookAheadPos, tricksTarget, iniDepth);
    #ifdef CANCEL
    if (cancelStarted) {
      cancelOrdered=FALSE;
      cancelStarted=FALSE;
      return 2;
    }
    #endif
    temp=movePly[iniDepth];
    last=movePly[iniDepth].last;
    noMoves=last+1;
    hiwinSetSize=winSetSize;
    hinodeSetSize=nodeSetSize;
    hilenSetSize=lenSetSize;
    if (nodeSetSize>MaxnodeSetSize)
      MaxnodeSetSize=nodeSetSize;
    if (winSetSize>MaxwinSetSize)
      MaxwinSetSize=winSetSize;
    if (lenSetSize>MaxlenSetSize)
      MaxlenSetSize=lenSetSize;
    if (val==1)
      payOff=tricksTarget;
    else
      payOff=0;
    futp->cards=1;
    ind=2;

    if (payOff<=0) {
      futp->suit[0]=movePly[game.noOfCards-4].move[0].suit;
      futp->rank[0]=movePly[game.noOfCards-4].move[0].rank;
        futp->equals[0]=(movePly[game.noOfCards-4].move[0].sequence)<<2;
        if (tricksTarget>1)
        futp->score[0]=-1;
        else
          futp->score[0]=0;
    } else {
      futp->suit[0]=bestMove[game.noOfCards-4].suit;
      futp->rank[0]=bestMove[game.noOfCards-4].rank;
        futp->equals[0]=(bestMove[game.noOfCards-4].sequence)<<2;
      futp->score[0]=payOff;
    }
  } else {
    g=estTricks[handToPlay];
    upperbound=(game.noOfCards+3)/4;
    lowerbound=0;
    do {
      if (g==lowerbound) {
        tricks=g+1;
      } else {
        tricks=g;
      }
      val=ABsearch(&lookAheadPos, tricks, iniDepth);
#ifdef CANCEL
      if (cancelStarted) {
        cancelOrdered=FALSE;
        cancelStarted=FALSE;
        return 2;
      }
#endif
      if (val==TRUE) {
        mv=bestMove[game.noOfCards-4];
      }
      hiwinSetSize=Max(hiwinSetSize, winSetSize);
      hinodeSetSize=Max(hinodeSetSize, nodeSetSize);
        hilenSetSize=Max(hilenSetSize, lenSetSize);
      if (nodeSetSize>MaxnodeSetSize)
        MaxnodeSetSize=nodeSetSize;
      if (winSetSize>MaxwinSetSize)
        MaxwinSetSize=winSetSize;
        if (lenSetSize>MaxlenSetSize)
        MaxlenSetSize=lenSetSize;
      if (val==FALSE) {
          upperbound=tricks-1;
          g=upperbound;
      } else {
         lowerbound=tricks;
         g=lowerbound;
      }
      InitSearch(&iniPosition, game.noOfCards-4,
        initialMoves, first, TRUE);
    } while (lowerbound<upperbound);

    payOff=g;
    temp=movePly[iniDepth];
    last=movePly[iniDepth].last;
    noMoves=last+1;
    ind=2;
    bestMove[game.noOfCards-4]=mv;
    futp->cards=1;
    if (payOff<=0) {
      futp->score[0]=0;
      futp->suit[0]=movePly[game.noOfCards-4].move[0].suit;
      futp->rank[0]=movePly[game.noOfCards-4].move[0].rank;
        futp->equals[0]=(movePly[game.noOfCards-4].move[0].sequence)<<2;
    } else {
      futp->score[0]=payOff;
      futp->suit[0]=bestMove[game.noOfCards-4].suit;
      futp->rank[0]=bestMove[game.noOfCards-4].rank;
      futp->equals[0]=(bestMove[game.noOfCards-4].sequence)<<2;
    }
    tricksTarget=payOff;
  }

  if ((solutions==2)&&(payOff>0)) {
    forb=1;
    ind=forb;
    while ((payOff==tricksTarget)&&(ind<(temp.last+1))) {
      forbiddenMoves[forb].suit=bestMove[game.noOfCards-4].suit;
      forbiddenMoves[forb].rank=bestMove[game.noOfCards-4].rank;
      forb++; ind++;
      /* All moves before bestMove in the move list shall be
      moved to the forbidden moves list, since none of them reached
      the target */
      mcurr=movePly[iniDepth].current;
      for (k=0; k<=movePly[iniDepth].last; k++)
        if ((bestMove[iniDepth].suit==movePly[iniDepth].move[k].suit)
          &&(bestMove[iniDepth].rank==movePly[iniDepth].move[k].rank))
          break;
      for (i=0; i<k; i++) {  /* All moves until best move */
        flag=FALSE;
        for (j=0; j<forb; j++) {
          if ((movePly[iniDepth].move[i].suit==forbiddenMoves[j].suit)
            &&(movePly[iniDepth].move[i].rank==forbiddenMoves[j].rank)) {
            /* If the move is already in the forbidden list */
            flag=TRUE;
            break;
          }
        }
        if (!flag) {
          forbiddenMoves[forb]=movePly[iniDepth].move[i];
          forb++;
        }
      }
      if (1/*(winSetSize<winSetFill)&&(nodeSetSize<nodeSetFill)*/) {
        InitSearch(&iniPosition, game.noOfCards-4,
          initialMoves, first, TRUE);
      } else {
        InitSearch(&iniPosition, game.noOfCards-4,
          initialMoves, first, FALSE);
      }
      val=ABsearch(&lookAheadPos, tricksTarget, iniDepth);
#ifdef CANCEL
      if (cancelStarted) {
        cancelOrdered=FALSE;
        cancelStarted=FALSE;
        return 2;
      }
#endif
      hiwinSetSize=winSetSize;
      hinodeSetSize=nodeSetSize;
          hilenSetSize=lenSetSize;
      if (nodeSetSize>MaxnodeSetSize)
        MaxnodeSetSize=nodeSetSize;
      if (winSetSize>MaxwinSetSize)
        MaxwinSetSize=winSetSize;
        if (lenSetSize>MaxlenSetSize)
        MaxlenSetSize=lenSetSize;
      if (val==TRUE) {
        payOff=tricksTarget;
        futp->cards=ind;
        futp->suit[ind-1]=bestMove[game.noOfCards-4].suit;
        futp->rank[ind-1]=bestMove[game.noOfCards-4].rank;
          futp->equals[ind-1]=(bestMove[game.noOfCards-4].sequence)<<2;
        futp->score[ind-1]=payOff;
      }
      else
        payOff=0;
    }
  } else if ((solutions==2)&&(payOff==0)&&((target==-1)||(tricksTarget==1))) {
    futp->cards=noMoves;
    /* Find the cards that were in the initial move list
    but have not been listed in the current result */
    n=0;
    for (i=0; i<noMoves; i++) {
      found=FALSE;
      if ((temp.move[i].suit==futp->suit[0])&&
        (temp.move[i].rank==futp->rank[0])) {
          found=TRUE;
      }
      if (!found) {
        futp->suit[1+n]=temp.move[i].suit;
        futp->rank[1+n]=temp.move[i].rank;
          futp->equals[1+n]=(temp.move[i].sequence)<<2;
        futp->score[1+n]=0;
        n++;
      }
    }
  }

  if ((solutions==3)&&(payOff>0)) {
    forb=1;
    ind=forb;
    for (i=0; i<last; i++) {
      forbiddenMoves[forb].suit=bestMove[game.noOfCards-4].suit;
      forbiddenMoves[forb].rank=bestMove[game.noOfCards-4].rank;
      forb++; ind++;

      g=payOff;
      upperbound=payOff;
      lowerbound=0;
      if (0) {
        InitSearch(&iniPosition, game.noOfCards-4,
          initialMoves, first, FALSE);
      } else {
          InitSearch(&iniPosition, game.noOfCards-4,
          initialMoves, first, TRUE);
      }

      do {
        if (g==lowerbound) {
          tricks=g+1;
        } else {
          tricks=g;
        }
        val=ABsearch(&lookAheadPos, tricks, iniDepth);
        #ifdef CANCEL
        if (cancelStarted) {
          cancelOrdered=FALSE;
          cancelStarted=FALSE;
          return 2;
        }
        #endif
        if (val==TRUE)
          mv=bestMove[game.noOfCards-4];
        hiwinSetSize=Max(hiwinSetSize, winSetSize);
        hinodeSetSize=Max(hinodeSetSize, nodeSetSize);
                hilenSetSize=Max(hilenSetSize, lenSetSize);
        if (nodeSetSize>MaxnodeSetSize)
          MaxnodeSetSize=nodeSetSize;
        if (winSetSize>MaxwinSetSize)
          MaxwinSetSize=winSetSize;
                if (lenSetSize>MaxlenSetSize)
          MaxlenSetSize=lenSetSize;
        if (val==FALSE) {
            upperbound=tricks-1;
            g=upperbound;
        } else {
            lowerbound=tricks;
            g=lowerbound;
        }
        if (0) {
          InitSearch(&iniPosition, game.noOfCards-4,
            initialMoves, first, FALSE);
        } else {
          InitSearch(&iniPosition, game.noOfCards-4,
            initialMoves, first, TRUE);
        }
      } while (lowerbound<upperbound);

      payOff=g;
      if (payOff==0) {
        last=movePly[iniDepth].last;
        futp->cards=temp.last+1;
        for (j=0; j<=last; j++) {
          futp->suit[ind-1+j]=movePly[game.noOfCards-4].move[j].suit;
          futp->rank[ind-1+j]=movePly[game.noOfCards-4].move[j].rank;
            futp->equals[ind-1+j]=(movePly[game.noOfCards-4].move[j].sequence)<<2;
          futp->score[ind-1+j]=payOff;
        }
        break;
      } else {
        bestMove[game.noOfCards-4]=mv;

        futp->cards=ind;
        futp->suit[ind-1]=bestMove[game.noOfCards-4].suit;
        futp->rank[ind-1]=bestMove[game.noOfCards-4].rank;
          futp->equals[ind-1]=(bestMove[game.noOfCards-4].sequence)<<2;
        futp->score[ind-1]=payOff;
      }   
    }
  } else if ((solutions==3)&&(payOff==0)) {
    futp->cards=noMoves;
    /* Find the cards that were in the initial move list
    but have not been listed in the current result */
    n=0;
    for (i=0; i<noMoves; i++) {
      found=FALSE;

      if ((temp.move[i].suit==futp->suit[0])&&
        (temp.move[i].rank==futp->rank[0])) {
          found=TRUE;
      }

      if (!found) {
        futp->suit[1+n]=temp.move[i].suit;
        futp->rank[1+n]=temp.move[i].rank;
          futp->equals[1+n]=(temp.move[i].sequence)<<2;
        futp->score[1+n]=0;
        n++;
      }
    }
  }

  for (k=0; k<=13; k++) {
    forbiddenMoves[k].suit=0;
    forbiddenMoves[k].rank=0;
  }

  futp->nodes=trickNodes;
  #ifdef BENCH
  futp->totalNodes=nodes;
  #endif
  /*if ((wcount>0)||(ncount>0)||(lcount>0)) {
    fp2=fopen("dyn.txt", "a");
        fprintf(fp2, "wcount=%d, ncount=%d, lcount=%d\n", 
          wcount, ncount, lcount);
    fprintf(fp2, "winSetSize=%d, nodeSetSize=%d, lenSetSize=%d\n", 
          winSetSize, nodeSetSize, lenSetSize);
        fprintf(fp2, "\n");
    fclose(fp2);
  }*/
  return 1;
}

const RelativeRanksFinder &rel=Globals.rel;
struct ttStoreType * ttStore;
struct nodeCardsType * nodeCards;
struct winCardType * winCards;
struct posSearchType * posSearch;
holding_t iniRemovedRanks[4];
int nodeSetSize=0; /* Index with range 0 to nodeSetSizeLimit */
int winSetSize=0;  /* Index with range 0 to winSetSizeLimit */
int lenSetSize=0;  /* Index with range 0 to lenSetSizeLimit */
int lastTTstore=0;

int _initialized=0;

extern "C" void DDSInitStart(void) {
  InitStart();
}

void InitStart(void) {
  int k,r,j;

  if (_initialized)
      return;
  _initialized = 1;

  nodeSetSizeLimit=NINIT;
  winSetSizeLimit=WINIT;
  lenSetSizeLimit=LINIT;

  maxmem=(5000001*sizeof(struct nodeCardsType)+
                   15000001*sizeof(struct winCardType)+
                   200001*sizeof(struct posSearchType));

  bestMove = (struct moveType *)calloc(50, sizeof(struct moveType));
  /*bestMove = new moveType [50];*/
  if (bestMove==NULL) {
    exit(1);
  }

  cardRank[2]='2'; cardRank[3]='3'; cardRank[4]='4'; cardRank[5]='5';
  cardRank[6]='6'; cardRank[7]='7'; cardRank[8]='8'; cardRank[9]='9';
  cardRank[10]='T'; cardRank[11]='J'; cardRank[12]='Q'; cardRank[13]='K';
  cardRank[14]='A';

  cardSuit[0]='S'; cardSuit[1]='H'; cardSuit[2]='D'; cardSuit[3]='C';
  cardSuit[4]='N';

  cardHand[0]='N'; cardHand[1]='E'; cardHand[2]='S'; cardHand[3]='W';

  temp_win = (struct winCardType *)calloc(5, sizeof(struct winCardType));
  if (temp_win==NULL)
    exit(1);

  summem=(WINIT+1)*sizeof(struct winCardType)+
             (NINIT+1)*sizeof(struct nodeCardsType)+
                 (LINIT+1)*sizeof(struct posSearchType);
  wmem=(WSIZE+1)*sizeof(struct winCardType);
  nmem=(NSIZE+1)*sizeof(struct nodeCardsType);
  lmem=(LSIZE+1)*sizeof(struct posSearchType);
  maxIndex=(int)(maxmem-summem)/((WSIZE+1) * sizeof(struct winCardType)); 

  pw = (struct winCardType **)calloc(maxIndex+1, sizeof(struct winCardType *));
  if (pw==NULL)
    exit(1);
  pn = (struct nodeCardsType **)calloc(maxIndex+1, sizeof(struct nodeCardsType *));
  if (pn==NULL)
    exit(1);
  pl = (struct posSearchType **)calloc(maxIndex+1, sizeof(struct posSearchType *));
  if (pl==NULL)
    exit(1);
  for (k=0; k<=maxIndex; k++) {
    if (pw[k]) {
      free(pw[k]);
    }
    pw[k]=NULL;
  }
  for (k=0; k<=maxIndex; k++) {
    if (pn[k]) {
      free(pn[k]);
    }
    pn[k]=NULL;
  }
  for (k=0; k<=maxIndex; k++) {
    if (pl[k]) {
      free(pl[k]);
    }
    pl[k]=NULL;
  }

  pw[0] = (struct winCardType *)calloc(winSetSizeLimit+1, sizeof(struct winCardType));
  if (pw[0]==NULL) 
    exit(1);
  allocmem=(winSetSizeLimit+1)*sizeof(struct winCardType);
  winCards=pw[0];
  pn[0] = (struct nodeCardsType *)calloc(nodeSetSizeLimit+1, sizeof(struct nodeCardsType));
  if (pn[0]==NULL)
    exit(1);
  allocmem+=(nodeSetSizeLimit+1)*sizeof(struct nodeCardsType);
  nodeCards=pn[0];
  pl[0] = (struct posSearchType *)calloc(lenSetSizeLimit+1, sizeof(struct posSearchType));
  if (pl[0]==NULL)
   exit(1);
  allocmem+=(lenSetSizeLimit+1)*sizeof(struct posSearchType);
  posSearch=pl[0];
  wcount=0; ncount=0; lcount=0;

  ttStore = (struct ttStoreType *)calloc(SEARCHSIZE, sizeof(struct ttStoreType));
  /*ttStore = new ttStoreType[SEARCHSIZE];*/
  if (ttStore==NULL) {
    exit(1);
  }

  highestRank = (int *) calloc(8192,sizeof(int));
  adaptWins = (struct adaptWinRanksType *)calloc(8192,
        sizeof(struct adaptWinRanksType));


  if (highestRank==NULL || adaptWins==NULL) {
    exit(1);
  }

  highestRank[0] = 0;
  for (j=0; j<14; j++) { adaptWins[0].winRanks[0] = 0; }

  for (r=2; r<=14; r++) {
    holding_t highestBitRank = BitRank(r);
    for (k=highestBitRank; k<2*highestBitRank; k++) {
      highestRank[k] = r;
      holding_t rest = k & (~highestBitRank);
      adaptWins[k].winRanks[0] = 0;
      for (j=1; j<14; j++) {
        adaptWins[k].winRanks[j] = highestBitRank | adaptWins[rest].winRanks[j-1];
      }
    }
  }

  return;
}


void InitGame(int gameNo, int moveTreeFlag, int first, int handRelFirst, struct pos &iniPosition) {

  int k, temp1, temp2,m;
  /*int points[4], tricks;
  int addNS, addEW, addMAX, trumpNS, trumpEW;
  struct gameInfo gm;*/

  #ifdef STAT
    fp2=fopen("stat.txt","w");
  #endif

  #ifdef TTDEBUG
  if (!suppressTTlog) {
    fp7=fopen("storett.txt","w");
    fp11=fopen("rectt.txt", "w");
    fclose(fp11);
    ttCollect=TRUE;
  }
  #endif        
  
  for (k=0; k<=3; k++) {
    for (m=0; m<=3; m++) {
      iniPosition.diagram.cards[k][m]=game.diagram.cards[k][m];
    }
  }

  iniPosition.stack[game.noOfCards-4].first=first;
  iniPosition.handRelFirst=handRelFirst;
  lookAheadPos=iniPosition;
  

  Globals.rel.initialize(game.diagram);
    
  temp1=game.contract/100;
  temp2=game.contract-100*temp1;
  int trump=temp2/10-1;
  Globals.setContract(trump);

  estTricks[1]=6;
  estTricks[3]=6;
  estTricks[0]=7;
  estTricks[2]=7;
  /*end: tricksest */

  #ifdef STAT
  fprintf(fp2, "Estimated tricks for hand to play:\n");        
  fprintf(fp2, "hand=%d  est tricks=%d\n", 
          handToPlay, estTricks[handToPlay]);
  #endif

  InitSearch(&lookAheadPos, game.noOfCards-4, initialMoves, first,
    moveTreeFlag);
  return;
}


void InitSearch(struct pos * posPoint, int depth, struct moveType startMoves[], int first, int mtd)  {

  int s, d, h, handRelFirst;
  int k, noOfStartMoves;       /* Number of start moves in the 1st trick */
  int hand[3], suit[3], rank[3];
  struct moveType move;
  unsigned short int startMovesBitMap[4][4]; /* Indices are hand and suit */
  const ContractInfo contract = Globals.getContract();

  for (h=0; h<=3; h++)
    for (s=0; s<=3; s++)
      startMovesBitMap[h][s]=0;

  handRelFirst=posPoint->handRelFirst;
  noOfStartMoves=handRelFirst;

  for (k=0; k<=2; k++) {
    hand[k]=RelativeHand(first,k);
    suit[k]=startMoves[k].suit;
    rank[k]=startMoves[k].rank;
    if (k<noOfStartMoves)
      startMovesBitMap[hand[k]][suit[k]] |= BitRank(rank[k]);
  }

  for (d=0; d<=49; d++) {
    /*bestMove[d].suit=0;*/
    bestMove[d].rank=0;
    /*bestMove[d].weight=0;
    bestMove[d].sequence=0; 0315 */
  }

  if ((RelativeHand(first,handRelFirst)==0)||
    (RelativeHand(first,handRelFirst)==2)) {
    nodeTypeStore[0]=MAXNODE;
    nodeTypeStore[1]=MINNODE;
    nodeTypeStore[2]=MAXNODE;
    nodeTypeStore[3]=MINNODE;
  } else {
    nodeTypeStore[0]=MINNODE;
    nodeTypeStore[1]=MAXNODE;
    nodeTypeStore[2]=MINNODE;
    nodeTypeStore[3]=MAXNODE;
  }

  k=noOfStartMoves;
  posPoint->stack[depth].first=first;
  posPoint->handRelFirst=k;
  posPoint->tricksMAX=0;

  if (k>0) {
    posPoint->stack[depth+k].move=startMoves[k-1];
    move=startMoves[k-1];
  }

  posPoint->stack[depth+k].high=first;

  while (k>0) {
    movePly[depth+k].current=0;
    movePly[depth+k].last=0;
    movePly[depth+k].move[0].suit=startMoves[k-1].suit;
    movePly[depth+k].move[0].rank=startMoves[k-1].rank;
    if (k<noOfStartMoves) {     /* If there is more than one start move */
      if (WinningMove(startMoves[k-1], move)) {
        posPoint->stack[depth+k].move.suit=startMoves[k-1].suit;
        posPoint->stack[depth+k].move.rank=startMoves[k-1].rank;
        posPoint->stack[depth+k].high=RelativeHand(first,noOfStartMoves-k);
        move=posPoint->stack[depth+k].move;
      } else {
        posPoint->stack[depth+k].move=posPoint->stack[depth+k+1].move;
        posPoint->stack[depth+k].high=posPoint->stack[depth+k+1].high;
      }
    }
    k--;
  }

  for (s=0; s<=3; s++) {
    posPoint->removedRanks[s]=0;
  }

  for (s=0; s<=3; s++) {      /* Suit */
    for (h=0; h<=3; h++) {    /* Hand */
      posPoint->removedRanks[s] |= posPoint->diagram.cards[h][s];
    }
  }

  for (s=0; s<=3; s++) {
    posPoint->removedRanks[s] = 8191 & ~(posPoint->removedRanks[s]);
  }

  for (s=0; s<=3; s++) {      /* Suit */
    for (h=0; h<=3; h++) {    /* Hand */
      posPoint->removedRanks[s] &= (~startMovesBitMap[h][s]);
    }
  }
        
  for (s=0; s<=3; s++) {
    iniRemovedRanks[s]=posPoint->removedRanks[s];
  }

  /* Initialize winning rank */
  holding_t aggHand[4][4];
  int maxHand = -1;

  for (s=0; s<=3; s++) {
    holding_t maxAgg = 0;
    for (h=0; h<=3; h++) {
      aggHand[h][s]=startMovesBitMap[h][s] | game.diagram.cards[h][s];
      if (aggHand[h][s]>maxAgg) {
        maxAgg=aggHand[h][s];
        maxHand=h;
      }
    }
    if (maxAgg!=0) {
      posPoint->winner[s].hand=maxHand;
      k=highestRank[aggHand[maxHand][s]];
      posPoint->winner[s].rank=k;

      maxAgg=0;
      for (h=0; h<=3; h++) {
        aggHand[h][s]&=(~BitRank(k));
        if (aggHand[h][s]>maxAgg) {
          maxAgg=aggHand[h][s];
          maxHand=h;
        }
      }
      if (maxAgg>0) {
        posPoint->secondBest[s].hand=maxHand;
        posPoint->secondBest[s].rank=highestRank[aggHand[maxHand][s]];
      } else {
        posPoint->secondBest[s].hand=-1;
        posPoint->secondBest[s].rank=0;
      }
    } else {
      posPoint->winner[s].hand=-1;
      posPoint->winner[s].rank=0;
      posPoint->secondBest[s].hand=-1;
      posPoint->secondBest[s].rank=0;
    }
  }

  for (s=0; s<=3; s++) {
    for (h=0; h<=3; h++) {
      posPoint->length[h][s]=
            (unsigned char)CountOnes(posPoint->diagram.cards[h][s]);
    }
  }

  #ifdef STAT
  for (d=0; d<=49; d++) {
    score1Counts[d]=0;
    score0Counts[d]=0;
    c1[d]=0;  c2[d]=0;  c3[d]=0;  c4[d]=0;  c5[d]=0;  c6[d]=0; c7[d]=0;
    c8[d]=0;
    no[d]=0;
  }
  #endif

  if (!mtd) {
            lenSetSize=0;  
    for (k=0; k<=13; k++) { 
          for (h=0; h<=3; h++) {
            rootnp[k][h]=&posSearch[lenSetSize];
            posSearch[lenSetSize].suitLengths=0;
            posSearch[lenSetSize].posSearchPoint=NULL;
            posSearch[lenSetSize].left=NULL;
            posSearch[lenSetSize].right=NULL;
            lenSetSize++;
          }
        }
    nodeSetSize=0;
    winSetSize=0;
  }
  
  #ifdef TTDEBUG
  if (!suppressTTlog) 
    lastTTstore=0;
  #endif

  recInd=0;

  return;
}

inline unsigned short int CountOnes(unsigned short int b) {
  return counttable[b];
  /*
  unsigned short int numb;

  for (numb=0; b!=0; numb++, b&=(b-1));
  return numb;
  */
}

int mexists, ready, hfirst;
int mcurrent, qtricks, out, sout, hout;
int res;
unsigned char cind;
int minimum, sopFound, nodeFound;
int score1Counts[50], score0Counts[50];
int sumScore1Counts, sumScore0Counts;
int c1[50], c2[50], c3[50], c4[50], c5[50], c6[50], c7[50], c8[50], c9[50];
int sumc1, sumc2, sumc3, sumc4, sumc5, sumc6, sumc7, sumc8, sumc9;
int scoreFlag;
int tricks, n;
int hh, ss, rr, mm, dd, fh;
int mcount, /*hand, */suit, rank, order;
int k, cardFound, currHand, a, found;
struct evalType evalData;
struct winCardType * np;
struct posSearchType * pp;
struct nodeCardsType * sopP;
struct nodeCardsType  * tempP;
holding_t aggr[4];
holding_t tricksLeft;
holding_t ranks;

int ABsearch(struct pos * posPoint, int target, int depth) {
    /* posPoint points to the current look-ahead position,
       target is number of tricks to take for the player,
       depth is the remaining search length, must be positive,
       the value of the subtree is returned.  */

  int moveExists, value;
  struct makeType makeData;
  struct nodeCardsType * cardsP;
  const ContractInfo contract = Globals.getContract();

  struct evalType Evaluate(const struct pos * posPoint);
  struct makeType Make(struct pos * posPoint, int depth);
  void Undo(struct pos * posPoint, int depth);

  #ifdef CANCEL
  if (nodes > threshold) {
    if (cancelOrdered) {
      cancelStarted=TRUE;
      return FALSE;
    } else {
      threshold+=CANCELCHECK;
    }
  }
  #endif

  /*cardsP=NULL;*/
  const int hand=RelativeHand(posPoint->stack[depth].first,posPoint->handRelFirst);

  nodes++;
  if (posPoint->handRelFirst==0) {
    trickNodes++;
    if (posPoint->tricksMAX>=target) {
      for (ss=0; ss<=3; ss++)
        posPoint->stack[depth].winRanks[ss]=0;

        #ifdef STAT
        c1[depth]++;
        
        score1Counts[depth]++;
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
            score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
              c3[dd], c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
              c6[dd], c7[dd], c8[dd]);
          }
        }
        #endif
   
      return TRUE;
    }
    if (((posPoint->tricksMAX+(depth>>2)+1)<target)&&(depth>0)) {
      for (ss=0; ss<=3; ss++)
        posPoint->stack[depth].winRanks[ss]=0;

        #ifdef STAT
        c2[depth]++;
        score0Counts[depth]++;
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
            score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
              c3[dd], c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
              c6[dd], c7[dd], c8[dd]);
          }
        }
        #endif

      return FALSE;
    }
            
    if (nodeTypeStore[hand]==MAXNODE) {
      qtricks=QuickTricks(posPoint, hand, depth, target, &res);
      if (res) {
        if (qtricks==0) { /* Tricks for MIN side gave cutoff */
          return FALSE;
        } else {
          return TRUE;
        }
#ifdef STAT
        c3[depth]++;
        score1Counts[depth]++;
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
                    score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
                    c3[dd], c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                    c6[dd], c7[dd], c8[dd]);
          }
        }
#endif
      }
      if (!LaterTricksMIN(posPoint,hand,depth,target)) {
        return FALSE;
      }
    } else {
      qtricks=QuickTricks(posPoint, hand, depth, target, &res);
      if (res) {
        if (qtricks==0) {  /* Tricks for MAX side gave cutoff */
          return TRUE;
        } else {
          return FALSE;
        }
#ifdef STAT
        c4[depth]++;
        score0Counts[depth]++;
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
                    score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
                    c3[dd], c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                    c6[dd], c7[dd], c8[dd]);
          }
        }
#endif
      }
      if (LaterTricksMAX(posPoint,hand,depth,target)) {
        return TRUE;
      }
    }
  } else if (posPoint->handRelFirst==1) {
    ss=posPoint->stack[depth+1].move.suit;
    ranks=posPoint->diagram.cards[hand][ss] |
      posPoint->diagram.cards[partner(hand)][ss];
    found=FALSE; rr=0; qtricks=0; 
    if ( ranks >(BitRank(posPoint->stack[depth+1].move.rank) |
                 posPoint->diagram.cards[lho(hand)][ss])) {
        /* Own side has highest card in suit */
      if (!contract.trumpContract || ((ss==contract.trump) ||
                     (posPoint->diagram.cards[lho(hand)][contract.trump]==0)
                     || (posPoint->diagram.cards[lho(hand)][ss]!=0))) { 
        rr=highestRank[ranks];
        if (rr!=0) {
          found=TRUE;
          qtricks=1;
        } else {
          found=FALSE;
        }
      }
    } else if (contract.notTrumpWithTrump(ss) &&
      (((posPoint->diagram.cards[hand][ss]==0)
        && (posPoint->diagram.cards[hand][contract.trump]!=0))|| 
       ((posPoint->diagram.cards[partner(hand)][ss]==0)
         && (posPoint->diagram.cards[partner(hand)][contract.trump]!=0))))  {
        /* Own side can ruff */
      if ((posPoint->diagram.cards[lho(hand)][ss]!=0)||
          (posPoint->diagram.cards[lho(hand)][contract.trump]==0)) {
          found=TRUE;
        qtricks=1;
      }
    }                
    if (nodeTypeStore[hand]==MAXNODE) {
      if ((posPoint->tricksMAX+qtricks>=target)&&(found)&&
        (depth!=iniDepth)) {
         for (k=0; k<=3; k++)
             posPoint->stack[depth].winRanks[k]=0;
           if (rr!=0)
              posPoint->stack[depth].winRanks[ss] |= BitRank(rr);
         return TRUE;
      }
    } else {
      if (((posPoint->tricksMAX+((depth)>>2)+3-qtricks)<=target)&&
        (found)&&(depth!=iniDepth)) {
        for (k=0; k<=3; k++)
            posPoint->stack[depth].winRanks[k]=0;
        if (rr!=0)
             posPoint->stack[depth].winRanks[ss] |= BitRank(rr);
          return FALSE;
      }
    }
  }
  
  if (posPoint->handRelFirst==0) {  
    posPoint->computeOrderSet();
    tricks=depth>>2;
    posPoint->getSuitLengths(suitLengths);
            
    pp=SearchLenAndInsert(rootnp[tricks][hand], suitLengths, FALSE, &res);
        /* Find node that fits the suit lengths */
    if (pp!=NULL) {
      np=pp->posSearchPoint;
      if (np==NULL) {
        cardsP=NULL;
      } else {
        cardsP=FindSOP(posPoint, np, hand, target, tricks, &scoreFlag);
      }
      
      if ((cardsP!=NULL)&&(depth!=iniDepth)) {
        if (scoreFlag==1) {
          posPoint->winAdapt(depth, cardsP, posPoint->aggregate); 
                    
          if (cardsP->bestMoveRank!=0) {
            bestMove[depth].suit=cardsP->bestMoveSuit;
            bestMove[depth].rank=cardsP->bestMoveRank;
          }
#ifdef STAT
          c5[depth]++;
          if (scoreFlag==1) {
            score1Counts[depth]++;
          } else {
            score0Counts[depth]++;
          }
          if (depth==iniDepth) {
            fprintf(fp2, "score statistics:\n");
            for (dd=iniDepth; dd>=0; dd--) {
              fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
                      score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
                      c3[dd], c4[dd]);
              fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                      c6[dd], c7[dd], c8[dd]);
            }
          }
#endif

#ifdef TTDEBUG
          if (!suppressTTlog) { 
            if (lastTTstore<SEARCHSIZE) {
              ReceiveTTstore(posPoint, cardsP, target, depth);
            } else {
              ttCollect=FALSE;
            }
          }
#endif 
          return TRUE;
        } else {
          posPoint->winAdapt(depth, cardsP, posPoint->aggregate);
          if (cardsP->bestMoveRank!=0) {
            bestMove[depth].suit=cardsP->bestMoveSuit;
            bestMove[depth].rank=cardsP->bestMoveRank;
          }
#ifdef STAT
          c6[depth]++;
          if (scoreFlag==1) {
            score1Counts[depth]++;
          } else {
            score0Counts[depth]++;
          }
          if (depth==iniDepth) {
            fprintf(fp2, "score statistics:\n");
            for (dd=iniDepth; dd>=0; dd--) {
              fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
                score1Counts[dd], score0Counts[dd], c1[dd], c2[dd], c3[dd],
                  c4[dd]);
              fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                  c6[dd], c7[dd], c8[dd]);
            }
          }
#endif

#ifdef TTDEBUG
          if (!suppressTTlog) {
            if (lastTTstore<SEARCHSIZE) {
              ReceiveTTstore(posPoint, cardsP, target, depth);
            } else {
              ttCollect=FALSE;
            }
          }
#endif 
          return FALSE;
          }  
      }
    }
  }

  if (depth==0) {                    /* Maximum depth? */
    evalData=Evaluate(posPoint);        /* Leaf node */
    if (evalData.tricks>=target) {
      value=TRUE;
    } else {
      value=FALSE;
    }
    for (ss=0; ss<=3; ss++) {
      posPoint->stack[depth].winRanks[ss]=evalData.winRanks[ss];

#ifdef STAT
        c7[depth]++;
        if (value==1) {
          score1Counts[depth]++;
        } else {
          score0Counts[depth]++;
        }
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
              score1Counts[dd], score0Counts[dd], c1[dd], c2[dd], c3[dd],
              c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
              c6[dd], c7[dd], c8[dd]);
          }
        }
#endif
    }
    return value;
  } else { /* Not at maximum depth */
      moveExists=MoveGen(posPoint, depth);

        if ((posPoint->handRelFirst==3)&&(depth>=/*29*/33/*37*/)&&(depth!=iniDepth)) {
          movePly[depth].current=0;
          mexists=TRUE;
          ready=FALSE;
          while (mexists) {
            makeData=Make(posPoint, depth);
            depth--;

            posPoint->computeOrderSet();
            tricks=depth/4;
            hfirst=posPoint->stack[depth].first;
            posPoint->getSuitLengths(suitLengths);

            pp=SearchLenAndInsert(rootnp[tricks][hfirst], suitLengths, FALSE, &res);
                 /* Find node that fits the suit lengths */
            if (pp!=NULL) {
              np=pp->posSearchPoint;
              if (np==NULL) {
                tempP=NULL;
              } else {
                tempP=FindSOP(posPoint, np, hfirst, target, tricks, &scoreFlag);
              }

              if (tempP!=NULL) {
                if ((nodeTypeStore[hand]==MAXNODE)&&(scoreFlag==1)) {
                  posPoint->winAdapt(depth+1, tempP, posPoint->aggregate);
                  if (tempP->bestMoveRank!=0) {
                    bestMove[depth+1].suit=tempP->bestMoveSuit;
                    bestMove[depth+1].rank=tempP->bestMoveRank;
                  }
                  for (ss=0; ss<=3; ss++) {
                    posPoint->stack[depth+1].winRanks[ss] |= makeData.winRanks[ss];
                  }
                  Undo(posPoint, depth+1);
                  return TRUE;
                } else if ((nodeTypeStore[hand]==MINNODE)&&(scoreFlag==0)) {
                  posPoint->winAdapt(depth+1, tempP, posPoint->aggregate);
                  if (tempP->bestMoveRank!=0) {
                    bestMove[depth+1].suit=tempP->bestMoveSuit;
                    bestMove[depth+1].rank=tempP->bestMoveRank;
                  }
                  for (ss=0; ss<=3; ss++) {
                    posPoint->stack[depth+1].winRanks[ss] |= makeData.winRanks[ss];
                  }
                  Undo(posPoint, depth+1);
                  return FALSE;
                } else {
                  movePly[depth+1].move[movePly[depth+1].current].weight+=100;
                  ready=TRUE;
                }
              }
            }
            depth++;
            Undo(posPoint, depth);
            if (ready) {
              break;
            }
            if (movePly[depth].current<movePly[depth].last) {
              movePly[depth].current++;
              mexists=TRUE;
            } else {
              mexists=FALSE;
            }
          }
          if (ready) {
            InsertSort(movePly[depth].last+1, depth);
          }
        }

        movePly[depth].current=0;
        if (nodeTypeStore[hand]==MAXNODE) {
          value=FALSE;
          for (ss=0; ss<=3; ss++) {
            posPoint->stack[depth].winRanks[ss]=0;
          }
          
          while (moveExists)  {
            makeData=Make(posPoint, depth);        /* Make current move */

            value=ABsearch(posPoint, target, depth-1);

#ifdef CANCEL
            if (cancelStarted) {
              return FALSE;
            }
#endif
          
            Undo(posPoint, depth);      /* Retract current move */
            if (value==TRUE) {
              /* A cut-off? */
              for (ss=0; ss<=3; ss++) {
                posPoint->stack[depth].winRanks[ss] = 
                  posPoint->stack[depth-1].winRanks[ss] | makeData.winRanks[ss];
              }
              mcurrent=movePly[depth].current;
              bestMove[depth]=movePly[depth].move[mcurrent];
              goto ABexit;
            }  
            for (ss=0; ss<=3; ss++) {
              posPoint->stack[depth].winRanks[ss] |= 
                posPoint->stack[depth-1].winRanks[ss] | makeData.winRanks[ss];
            }
            
            moveExists=NextMove(posPoint, depth);
          }
        } else {                          /* A minnode */
          value=TRUE;
          for (ss=0; ss<=3; ss++) {
            posPoint->stack[depth].winRanks[ss]=0;
          }
        
          while (moveExists)  {
            makeData=Make(posPoint, depth);        /* Make current move */
            
            value=ABsearch(posPoint, target, depth-1);
      
#ifdef CANCEL
            if (cancelStarted) {
              return FALSE;
            }
#endif

            Undo(posPoint, depth);       /* Retract current move */
            if (value==FALSE) {
              /* A cut-off? */
              for (ss=0; ss<=3; ss++) {
                posPoint->stack[depth].winRanks[ss]=posPoint->stack[depth-1].winRanks[ss] |
                  makeData.winRanks[ss];
              }
              mcurrent=movePly[depth].current;
              bestMove[depth]=movePly[depth].move[mcurrent];
              goto ABexit;
            }
            for (ss=0; ss<=3; ss++) {
              posPoint->stack[depth].winRanks[ss] |=
                posPoint->stack[depth-1].winRanks[ss] | makeData.winRanks[ss];
            }
            
            moveExists=NextMove(posPoint, depth);
          }
        }
  }
  
 ABexit:
  if (depth>=4) {
    if(posPoint->handRelFirst==0) { 
      tricks=depth>>2;
      /*hand=posPoint->first[depth-1];*/
      if (value) {
        k=target;
      } else {
        k=target-1;
      }
      BuildSOP(posPoint, tricks, hand, target, depth,value, k);
      if (clearTTflag) {
         /* Wipe out the TT dynamically allocated structures
            except for the initially allocated structures.
            Set the TT limits to the initial values.
            Reset TT array indices to zero.
            Reset memory chunk indices to zero.
            Set allocated memory to the initial value. */

        Wipe();
        winSetSizeLimit=WINIT;
        nodeSetSizeLimit=NINIT;
        lenSetSizeLimit=LINIT;
        lcount=0;  
        allocmem=(lenSetSizeLimit+1)*sizeof(struct posSearchType);
        lenSetSize=0;
        posSearch=pl[lcount];  
        for (k=0; k<=13; k++) { 
          for (hh=0; hh<=3; hh++) {
            rootnp[k][hh]=&posSearch[lenSetSize];
            posSearch[lenSetSize].suitLengths=0;
            posSearch[lenSetSize].posSearchPoint=NULL;
            posSearch[lenSetSize].left=NULL;
            posSearch[lenSetSize].right=NULL;
            lenSetSize++;
          }
        }
        nodeSetSize=0;
        winSetSize=0;
        wcount=0; ncount=0; 
        allocmem+=(winSetSizeLimit+1)*sizeof(struct winCardType);
        winCards=pw[wcount];
        allocmem+=(nodeSetSizeLimit+1)*sizeof(struct nodeCardsType);
        nodeCards=pn[ncount];
        clearTTflag=FALSE;
        windex=-1;
      }
    } 
  }
#ifdef STAT
  c8[depth]++;
  if (value==1) {
    score1Counts[depth]++;
  } else {
    score0Counts[depth]++;
  }
  if (depth==iniDepth) {
    if (fp2==NULL) {
      exit(0);        
    }          
    fprintf(fp2, "\n");
    fprintf(fp2, "top level cards:\n");
    for (hh=0; hh<=3; hh++) {
      fprintf(fp2, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp2, "suit=%c", cardSuit[ss]);
        for (rr=14; rr>=2; rr--)
          if (posPoint->diagram.cards[hh][ss] & BitRank(rr))
            fprintf(fp2, " %c", cardRank[rr]);
        fprintf(fp2, "\n");
      }
      fprintf(fp2, "\n");
    }
    fprintf(fp2, "top level winning cards:\n");
    for (ss=0; ss<=3; ss++) {
      fprintf(fp2, "suit=%c", cardSuit[ss]);
      for (rr=14; rr>=2; rr--) {
        if (posPoint->stack[depth].winRanks[ss] & BitRank(rr)) {
          fprintf(fp2, " %c", cardRank[rr]);
        }
      }
      fprintf(fp2, "\n");
    }
    fprintf(fp2, "\n");
    fprintf(fp2, "\n");

    fprintf(fp2, "score statistics:\n");
    sumScore0Counts=0;
    sumScore1Counts=0;
    sumc1=0; sumc2=0; sumc3=0; sumc4=0;
    sumc5=0; sumc6=0; sumc7=0; sumc8=0; sumc9=0;
    for (dd=iniDepth; dd>=0; dd--) {
      fprintf(fp2, "depth=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
              score1Counts[dd], score0Counts[dd], c1[dd], c2[dd], c3[dd], c4[dd]);
      fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd], c6[dd],
              c7[dd], c8[dd]);
      sumScore0Counts=sumScore0Counts+score0Counts[dd];
      sumScore1Counts=sumScore1Counts+score1Counts[dd];
      sumc1=sumc1+c1[dd];
      sumc2=sumc2+c2[dd];
      sumc3=sumc3+c3[dd];
      sumc4=sumc4+c4[dd];
      sumc5=sumc5+c5[dd];
      sumc6=sumc6+c6[dd];
      sumc7=sumc7+c7[dd];
      sumc8=sumc8+c8[dd];
      sumc9=sumc9+c9[dd];
    } 
    fprintf(fp2, "\n");
    fprintf(fp2, "score sum statistics:\n");
    fprintf(fp2, "\n");
    fprintf(fp2, "sumScore0Counts=%d sumScore1Counts=%d\n",
            sumScore0Counts, sumScore1Counts);
    fprintf(fp2, "nodeSetSize=%d  winSetSize=%d\n", nodeSetSize,
            winSetSize);
    fprintf(fp2, "sumc1=%d sumc2=%d sumc3=%d sumc4=%d\n",
            sumc1, sumc2, sumc3, sumc4);
    fprintf(fp2, "sumc5=%d sumc6=%d sumc7=%d sumc8=%d sumc9=%d\n",
            sumc5, sumc6, sumc7, sumc8, sumc9);
    fprintf(fp2, "\n");        
    fprintf(fp2, "\n");
    fprintf(fp2, "No of searched nodes per depth:\n");
    for (dd=iniDepth; dd>=0; dd--) {
      fprintf(fp2, "depth=%d  nodes=%d\n", dd, no[dd]);
    }
    fprintf(fp2, "\n");
    fprintf(fp2, "Total nodes=%d\n", nodes);
  }
#endif
    
  return value;
}


struct makeType Make(struct pos * posPoint, int depth)  {
  int r, s, t, u, w, firstHand;
  int suit, /*rank, */count, mcurr, h, q, done;
  struct makeType trickCards;
  struct moveType mo1, mo2;
  const ContractInfo contract = Globals.getContract();
  struct posStackItem &current = posPoint->stack[depth];
  const struct posStackItem &previous = posPoint->stack[depth+1];

  for (suit=0; suit<=3; suit++) {
    trickCards.winRanks[suit]=0;
  }

  firstHand=current.first;
  r=movePly[depth].current;

  if (posPoint->handRelFirst==3)  {   /* This hand is last hand */
    mo1 = movePly[depth].move[r];
    mo2 = previous.move;
    if (contract.betterMove(mo1,mo2)) {
      current.move=mo1;
      current.high=rho(firstHand);
    } else {
      current.move=mo2;
      current.high=previous.high;
    }

    /* Is the trick won by rank? */
    suit=current.move.suit;
    /*rank=posPoint->stack[depth+h].stack[depth].move.rank;*/
    count=0;
    for (h=0; h<=3; h++) {
      mcurr=movePly[depth+h].current;
      if (movePly[depth+h].move[mcurr].suit==suit) {
        count++;
      }
    }

    if (nodeTypeStore[current.high]==MAXNODE)
      posPoint->tricksMAX++;
      posPoint->stack[depth-1].first=current.high;   
      /* Defines who is first in the next move */

    t=rho(firstHand);
    posPoint->handRelFirst=0;      /* Hand pointed to by posPoint->first
                                    will lead the next trick */

    done=FALSE;
    for (s=3; s>=0; s--) {
      q=RelativeHand(firstHand,3-s);
      /* Add the moves to removed ranks */
      r=movePly[depth+s].current;
      w=movePly[depth+s].move[r].rank;
      u=movePly[depth+s].move[r].suit;
      posPoint->removeRank(u,w);

      if (s==0)
        posPoint->diagram.cards[t][u] &=
          (~BitRank(w));

      if (w==posPoint->winner[u].rank) {
        UpdateWinner(posPoint, u);
      } else if (w==posPoint->secondBest[u].rank) {
        UpdateSecondBest(posPoint, u);
      }

    /* Determine win-ranked cards */
      if ((q==current.high)&&(!done)) {
        done=TRUE;
        if (count>=2) {
          trickCards.winRanks[u]=BitRank(w);
          /* Mark ranks as winning if they are part of a sequence */
          trickCards.winRanks[u] |= movePly[depth+s].move[r].sequence; 
        }
      }
    }
  } else if (posPoint->handRelFirst==0) {   /* Is it the 1st hand? */
    posPoint->stack[depth-1].first=firstHand;   /* First hand is not changed in
                                            next move */
    current.high=firstHand;
    current.move=movePly[depth].move[r];

    t=firstHand;
    posPoint->handRelFirst=1;
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
    posPoint->diagram.cards[t][u] &= (~BitRank(w));
  } else {  /* Second or third hand in trick */
    mo1=movePly[depth].move[r];
    mo2=previous.move;
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
    if (contract.betterMove(mo1,mo2)) {
      current.move=mo1;
      current.high=RelativeHand(firstHand,posPoint->handRelFirst);
    } else {
      current.move=previous.move;
      current.high=previous.high;
    }
    
    t=RelativeHand(firstHand,posPoint->handRelFirst);
    posPoint->handRelFirst++;               /* Current hand is stepped */
    posPoint->stack[depth-1].first=firstHand;     /* First hand is not changed in
                                            next move */
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
    posPoint->diagram.cards[t][u] &= (~BitRank(w));
  }

  posPoint->length[t][u]--;

  no[depth]++;
    
  return trickCards;
}


void Undo(struct pos * posPoint, int depth)  {
  int r, s, t, u, w, firstHand;

  firstHand=posPoint->stack[depth].first;

  switch (posPoint->handRelFirst) {
    case 3: case 2: case 1:
     posPoint->handRelFirst--;
     break;
    case 0:
     posPoint->handRelFirst=3;
  }

  if (posPoint->handRelFirst==0) {          /* 1st hand which won the previous
                                            trick */
    t=firstHand;
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
  } else if (posPoint->handRelFirst==3)  {    /* Last hand */
    for (s=3; s>=0; s--) {
    /* Delete the moves from removed ranks */
      r=movePly[depth+s].current;
      w=movePly[depth+s].move[r].rank;
      u=movePly[depth+s].move[r].suit;
      //posPoint->removedRanks[u]=posPoint->removedRanks[u] & (~BitRank(w));
      posPoint->restoreRank(u,w);

      if (w>posPoint->winner[u].rank) {
        posPoint->secondBest[u].rank=posPoint->winner[u].rank;
        posPoint->secondBest[u].hand=posPoint->winner[u].hand;
        posPoint->winner[u].rank=w;
        posPoint->winner[u].hand=RelativeHand(firstHand,3-s);
      } else if (w>posPoint->secondBest[u].rank) {
        posPoint->secondBest[u].rank=w;
        posPoint->secondBest[u].hand=RelativeHand(firstHand,3-s);
      }
    }
    t=rho(firstHand);

        
    if (nodeTypeStore[posPoint->stack[depth-1].first]==MAXNODE) {
      /* First hand of next trick is winner of the current trick */
      posPoint->tricksMAX--;
    }
  } else {
    t=RelativeHand(firstHand,posPoint->handRelFirst);
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
  }    

  posPoint->diagram.cards[t][u] |= BitRank(w);

  posPoint->length[t][u]++;

  return;
}



struct evalType Evaluate(const struct pos * posPoint)  {
  int s, smax=0, k, firstHand, count;
  unsigned short int max;
  struct evalType eval;
  const ContractInfo contract = Globals.getContract();

  firstHand=posPoint->stack[0].first;

  for (s=0; s<=3; s++) {
    eval.winRanks[s]=0;
  }

  /* Who wins the last trick? */
  if (contract.trumpContract)  {            /* Highest trump card wins */
    max=0;
    count=0;
    for (s=0; s<=3; s++) {
      if (posPoint->diagram.cards[s][contract.trump]!=0) {
        count++;
      }
      if (posPoint->diagram.cards[s][contract.trump]>max) {
        smax=s;
        max=posPoint->diagram.cards[s][contract.trump];
      }
    }

    if (max>0) {        /* Trumpcard wins */
      if (count>=2) {
        eval.winRanks[contract.trump]=max;
      }

      if (nodeTypeStore[smax]==MAXNODE) {
        goto maxexit;
      } else {
        goto minexit;
      }
    }
  }

  /* Who has the highest card in the suit played by 1st hand? */

  k=0;
  while (k<=3)  {           /* Find the card the 1st hand played */
    if (posPoint->diagram.cards[firstHand][k]!=0) {     /* Is this the card? */
      break;
    }
    k++;
  }    

  count=0;
  max=0; 
  for (s=0; s<=3; s++)  {
    if (posPoint->diagram.cards[s][k]!=0) {
        count++;
    }
    if (posPoint->diagram.cards[s][k]>max)  {
      smax=s;
      max=posPoint->diagram.cards[s][k];
    }
  }

  if (count>=2) {
    eval.winRanks[k]=max;
  }

  if (nodeTypeStore[smax]==MAXNODE) {
    goto maxexit;
  } else {
    goto minexit;
  }

  maxexit:
  eval.tricks=posPoint->tricksMAX+1;
  return eval;

  minexit:
  eval.tricks=posPoint->tricksMAX;
  return eval;
}

void UpdateWinner(struct pos * posPoint, int suit) {
  int k, h, hmax=0;
  unsigned short int sb, sbmax;

  posPoint->winner[suit]=posPoint->secondBest[suit];

  sbmax=0;
  for (h=0; h<=3; h++) {
    sb=posPoint->diagram.cards[h][suit] & (~BitRank(posPoint->winner[suit].rank));
    if (sb>sbmax) {
      hmax=h;
      sbmax=sb;
    }
  }
  k=highestRank[sbmax];
  if (k!=0) {
    posPoint->secondBest[suit].hand=hmax;
    posPoint->secondBest[suit].rank=k;
  }
  else {
    posPoint->secondBest[suit].hand=-1;
    posPoint->secondBest[suit].rank=0;
  }

  return;
}

void UpdateSecondBest(struct pos * posPoint, int suit) {
  int k, h, hmax=0;
  unsigned short int sb, sbmax;

  sbmax=0;
  for (h=0; h<=3; h++) {
    sb=posPoint->diagram.cards[h][suit] & (~BitRank(posPoint->winner[suit].rank));
    if (sb>sbmax) {
      hmax=h;
      sbmax=sb;
    }
  }
  k=highestRank[sbmax];
  if (k!=0) {
    posPoint->secondBest[suit].hand=hmax;
    posPoint->secondBest[suit].rank=k;
  }
  else {
    posPoint->secondBest[suit].hand=-1;
    posPoint->secondBest[suit].rank=0;
  }

  return;
}

/*

void UpdateWinner(struct pos * posPoint, int suit) {
  int k;
  int h, hmax=0, flag;

  posPoint->winner[suit]=posPoint->secondBest[suit];

  k=posPoint->secondBest[suit].rank-1;
    while (k>=2) {
      flag=TRUE;
      for (h=0; h<=3; h++)
        if ((posPoint->diagram.cards[h][suit] & BitRank(k)) != 0) {
          hmax=h;
          flag=FALSE;
          break;
        }
      if (flag) {
        k--;
      } else {
        break;
      }
    }
    if (k<2) {
      posPoint->secondBest[suit].rank=0;
      posPoint->secondBest[suit].hand=0;
    } else {
      posPoint->secondBest[suit].rank=k;
      posPoint->secondBest[suit].hand=hmax;
    }
  return;
}


void UpdateSecondBest(struct pos * posPoint, int suit) {
  int k;
  int h, hmax=0, flag;

  k=posPoint->secondBest[suit].rank-1;
  while (k>=2) {
    flag=TRUE;
    for (h=0; h<=3; h++) {
      if ((posPoint->diagram.cards[h][suit] & BitRank(k)) != 0) {
        hmax=h;
        flag=FALSE;
        break;
      }
    }
    if (flag) {
      k--;
    } else {
      break;
    }
  }
  if (k<2) {
    posPoint->secondBest[suit].rank=0;
    posPoint->secondBest[suit].hand=0;
  }
  else {
    posPoint->secondBest[suit].rank=k;
    posPoint->secondBest[suit].hand=hmax;
  }
  return;
}
*/


int QuickTricks(struct pos * posPoint, const int hand, 
        const int depth, const int target, int *result) {
  holding_t ranks;
  int suit, sum, qtricks, commPartner, commRank=0, commSuit=-1, found=FALSE;
  int opps;
  int countLho, countRho, countPart, countOwn, lhoTrumpRanks=0, rhoTrumpRanks=0;
  int cutoff, k, lowestQtricks=0, count=0;
  const ContractInfo contract = Globals.getContract();
  
  *result=TRUE;
  qtricks=0;
  for (suit=0; suit<=3; suit++) {
    posPoint->stack[depth].winRanks[suit]=0;        
  }

  if ((depth<=0)||(depth==iniDepth)) {
    *result=FALSE;
    return qtricks;
  }

  if (nodeTypeStore[hand]==MAXNODE)  {
    cutoff=target-posPoint->tricksMAX;
  } else {
    cutoff=posPoint->tricksMAX-target+(depth>>2)+2;
  }
      
  commPartner=FALSE;
  for (suit=0; suit<=3; suit++) {
    if (contract.notTrumpWithTrump(suit)) {
      /* Trump contract, suit is not trump */
      if (posPoint->winner[suit].hand==partner(hand)) {
        /* Partner has winning card */
        if (posPoint->diagram.cards[hand][suit]!=0) {
        /* Own hand has card in suit */
          if (((posPoint->diagram.cards[lho(hand)][suit]!=0) ||
          /* LHO not void */
               (posPoint->diagram.cards[lho(hand)][contract.trump]==0))
          /* LHO has no trump */
              && ((posPoint->diagram.cards[rho(hand)][suit]!=0) ||
          /* RHO not void */
                  (posPoint->diagram.cards[rho(hand)][contract.trump]==0))) {
          /* RHO has no trump */
            commPartner=TRUE;
            commSuit=suit;
            commRank=posPoint->winner[suit].rank;
            break;
          }  
        }
      } else if (posPoint->secondBest[suit].hand==partner(hand)) {
        if ((posPoint->winner[suit].hand==hand)&&
            (posPoint->length[hand][suit]>=2)&&(posPoint->length[partner(hand)][suit]>=2)) {
          if (((posPoint->diagram.cards[lho(hand)][suit]!=0) ||
               (posPoint->diagram.cards[lho(hand)][contract.trump]==0))
              && ((posPoint->diagram.cards[rho(hand)][suit]!=0) ||
                  (posPoint->diagram.cards[rho(hand)][contract.trump]==0))) {
            commPartner=TRUE;
            commSuit=suit;
            commRank=posPoint->secondBest[suit].rank;
            break;
          }
        }
      }
    } else if (!contract.trumpContract) {
      if (posPoint->winner[suit].hand==partner(hand)) {
        /* Partner has winning card */
        if (posPoint->diagram.cards[hand][suit]!=0) {
        /* Own hand has card in suit */
          commPartner=TRUE;
          commSuit=suit;
          commRank=posPoint->winner[suit].rank;
          break;
        }
      } else if (posPoint->secondBest[suit].hand==partner(hand)) { 
        if ((posPoint->winner[suit].hand==hand)&&
            (posPoint->length[hand][suit]>=2)&&(posPoint->length[partner(hand)][suit]>=2)) {
          commPartner=TRUE;
          commSuit=suit;
          commRank=posPoint->secondBest[suit].rank;
          break;
        }
      }
    }
  }

  //int s;

  if (contract.trumpContract && (!commPartner) && 
    (posPoint->diagram.cards[hand][contract.trump]!=0) && 
      (posPoint->winner[contract.trump].hand==partner(hand))) {
    commPartner=TRUE;
    commSuit=contract.trump;
    commRank=posPoint->winner[contract.trump].rank;
  }


  if (contract.trumpContract) {
    lhoTrumpRanks=posPoint->length[lho(hand)][contract.trump];
    rhoTrumpRanks=posPoint->length[rho(hand)][contract.trump];
  }   

  for (suit=contract.firstSuit(); suit<4; suit=contract.nextSuit(suit)) {
    countOwn=posPoint->length[hand][suit];
    countLho=posPoint->length[lho(hand)][suit];
    countRho=posPoint->length[rho(hand)][suit];
    countPart=posPoint->length[partner(hand)][suit];
    opps=countLho | countRho;

    if (!opps && (countPart==0)) {
      if (countOwn==0) {
        continue;
      }
      if (contract.notTrumpWithTrump(suit)) {
        if ((lhoTrumpRanks==0) &&
          /* LHO has no trump */
          (rhoTrumpRanks==0)) {
          /* RHO has no trump */
          qtricks=qtricks+countOwn;
          if (qtricks>=cutoff) {
            return qtricks;
          }
          continue;
        } else {
          continue;
        }
      } else {
        qtricks=qtricks+countOwn;
        if (qtricks>=cutoff) {
          return qtricks;
        }
        
        continue;
      }
    } else {
      if (!opps && contract.isTrump(suit)) {
        sum=Max(countOwn, countPart);
        for (int suit2=0; suit2<=3; suit2++) {
          if ((sum>0)&&(suit2!=contract.trump)&&(countOwn>=countPart)&&(posPoint->length[hand][suit2]>0)&&
              (posPoint->length[partner(hand)][suit2]==0)) {
            sum++;
            break;
          }
        }
        if (sum>=cutoff) {
          return sum;
        }
      } else if (!opps) {
        sum=Min(countOwn,countPart);
        if (!contract.trumpContract) {
          if (sum>=cutoff) {
            return sum;
          }
        } else if ((suit!=contract.trump)&&(lhoTrumpRanks==0)&&(rhoTrumpRanks==0)) {
          if (sum>=cutoff) {
            return sum;
          }
        }
      }

      if (commPartner) {
        if (!opps && (countOwn==0)) {
          if (contract.notTrumpWithTrump(suit)) {
            if ((lhoTrumpRanks==0) &&
            /* LHO has no trump */
              (rhoTrumpRanks==0)) {
            /* RHO has no trump */
              qtricks=qtricks+countPart;
              posPoint->stack[depth].winRanks[commSuit] |= BitRank(commRank);
              if (qtricks>=cutoff) {
                return qtricks;
              }
              continue;
            } else {
              continue;
            }
          } else {
            qtricks=qtricks+countPart;
            posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
              BitRank(commRank);
            if (qtricks>=cutoff) 
              return qtricks;
            continue;
          }
        } else {
          if (!opps && contract.isTrump(suit)) {
            sum=Max(countOwn, countPart);
            for (int suit2=0; suit2<=3; suit2++) {
              if ((sum>0)&&(suit2!=contract.trump)&&(countOwn<=countPart)&&(posPoint->length[partner(hand)][suit2]>0)&&
                (posPoint->length[hand][suit2]==0)) {
                sum++;
                break;
              }
            }
            if (sum>=cutoff) {
              posPoint->stack[depth].winRanks[commSuit] |= BitRank(commRank);
              return sum;
            }
          } else if (!opps) {
            sum=Min(countOwn,countPart);
            if (!contract.trumpContract) {
              if (sum>=cutoff) 
                return sum;
            } else if ((suit!=contract.trump)&&(lhoTrumpRanks==0)&&(rhoTrumpRanks==0)) {
              if (sum>=cutoff) 
                return sum;
            }
          }
        } 
      }
    }
    /* 08-01-30 */
    if (posPoint->winner[suit].rank==0) {
      continue;
    }

    if (posPoint->winner[suit].hand==hand) {
      /* Winner found in own hand */
      if (contract.notTrumpWithTrump(suit)) {
        if (((countLho!=0) ||
          /* LHO not void */
          (lhoTrumpRanks==0))
          /* LHO has no trump */
          && ((countRho!=0) ||
          /* RHO not void */
          (rhoTrumpRanks==0))) {
          /* RHO has no trump */
          posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
            | BitRank(posPoint->winner[suit].rank);
          qtricks++;   /* A trick can be taken */
          /* 06-12-14 */
          if (qtricks>=cutoff) 
            return qtricks;

          if ((countLho<=1)&&(countRho<=1)&&(countPart<=1)&&
            (lhoTrumpRanks==0)&&(rhoTrumpRanks==0)) {
            qtricks=qtricks+countOwn-1;
            if (qtricks>=cutoff) {
              return qtricks;
            }
            continue;
          }
        }
        if (posPoint->secondBest[suit].hand==hand) {
          /* Second best found in own hand */
          if ((lhoTrumpRanks==0)&&
             (rhoTrumpRanks==0)) {
            /* Opponents have no trump */
            posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
              | BitRank(posPoint->secondBest[suit].rank);
            qtricks++;
            if ((countLho<=2)&&(countRho<=2)&&(countPart<=2)) {
              qtricks=qtricks+countOwn-2;
              if (qtricks>=cutoff) 
                return qtricks;
              continue;
            }
          }
        } else if ((posPoint->secondBest[suit].hand==partner(hand))
          &&(countOwn>1)&&(countPart>1)) {
          /* Second best at partner and suit length of own
             hand and partner > 1 */
          if ((lhoTrumpRanks==0)&&
             (rhoTrumpRanks==0)) {
          /* Opponents have no trump */
            posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
                | BitRank(posPoint->secondBest[suit].rank);
            qtricks++;
            if ((countLho<=2)&&(countRho<=2)&&((countPart<=2)||(countOwn<=2))) { 
                                /* 07-06-10 */
              qtricks=qtricks+Max(countOwn-2, countPart-2);
              if (qtricks>=cutoff) {
                return qtricks;
              }
              continue;
            }
          }
        }
      } else {
        posPoint->stack[depth].winRanks[suit] |= BitRank(posPoint->winner[suit].rank);
        qtricks++;
        /* 06-12-14 */
        if (qtricks>=cutoff) {
          return qtricks;
        }

        if ((countLho<=1)&&(countRho<=1)&&(countPart<=1)) {
          qtricks=qtricks+countOwn-1;
          if (qtricks>=cutoff) {
            return qtricks;
          }
          continue;
        }
        
        if (posPoint->secondBest[suit].hand==hand) {
          /* Second best found in own hand */
          posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
              | BitRank(posPoint->secondBest[suit].rank);
          qtricks++;
          if ((countLho<=2)&&(countRho<=2)&&(countPart<=2)) {
            qtricks=qtricks+countOwn-2;
            if (qtricks>=cutoff) {
              return qtricks;
            }
            continue;
          }
        } else if ((posPoint->secondBest[suit].hand==partner(hand))
            &&(countOwn>1)&&(countPart>1)) {
          /* Second best at partner and suit length of own
             hand and partner > 1 */
          posPoint->stack[depth].winRanks[suit] |= BitRank(posPoint->secondBest[suit].rank);
          qtricks++;
          if ((countLho<=2)&&(countRho<=2)&&((countPart<=2)||(countOwn<=2))) {  
          /* 07-06-10 */
            qtricks=qtricks+Max(countOwn-2,countPart-2);
            if (qtricks>=cutoff) {
              return qtricks;
            }
            continue;
          }
        }
      }
    } else {
      /* It was not possible to take a quick trick by own winning card in
         the suit */
      /* Partner winning card? */
      if ((posPoint->winner[suit].hand==partner(hand))&&(countPart>0)) {
        /* Winner found at partner*/
        if (commPartner) {
        /* There is communication with the partner */
          if (contract.notTrumpWithTrump(suit)) {
            if (((countLho!=0) ||
              /* LHO not void */
            (lhoTrumpRanks==0))
              /* LHO has no trump */
             && ((countRho!=0) ||
              /* RHO not void */
             (rhoTrumpRanks==0)))
              /* RHO has no trump */
              {
                posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
                  | BitRank(posPoint->winner[suit].rank);
                posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
                   BitRank(commRank);
                qtricks++;   /* A trick can be taken */
                /* 06-12-14 */
                if (qtricks>=cutoff) {
                  return qtricks;
                }
                if ((countLho<=1)&&(countRho<=1)&&(countOwn<=1)&&
                  (lhoTrumpRanks==0)&&
                  (rhoTrumpRanks==0)) {
                   qtricks=qtricks+countPart-1;
                   continue;
                }
              }
            if (posPoint->secondBest[suit].hand==partner(hand)) {
               /* Second best found in partners hand */
                if ((lhoTrumpRanks==0)&&
                 (rhoTrumpRanks==0)) {
                /* Opponents have no trump */
                  posPoint->stack[depth].winRanks[suit] |= BitRank(posPoint->secondBest[suit].rank);
                  posPoint->stack[depth].winRanks[commSuit] |= BitRank(commRank);
                  qtricks++;
                  if ((countLho<=2)&&(countRho<=2)&&(countOwn<=2)) {
                    continue;
                  }
                }
              } else if ((posPoint->secondBest[suit].hand==hand)&&
                  (countPart>1)&&(countOwn>1)) {
               /* Second best found in own hand and suit
                     lengths of own hand and partner > 1*/
                if ((lhoTrumpRanks==0)&&
                 (rhoTrumpRanks==0)) {
                /* Opponents have no trump */
                  posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
                   | BitRank(posPoint->secondBest[suit].rank);
                  posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
                    BitRank(commRank);
                  qtricks++;
                  if ((countLho<=2)&&(countRho<=2)&&
                    ((countOwn<=2)||(countPart<=2))) {  /* 07-06-10 */
                    qtricks=qtricks+
                      Max(countPart-2,countOwn-2);
                    if (qtricks>=cutoff) {
                      return qtricks;
                    }
                    continue;
                  }
                }
              }
              /* 06-08-24 */
              else if ((suit==commSuit)&&(posPoint->secondBest[suit].hand
                                          ==lho(hand))&&((countLho>=2)||(lhoTrumpRanks==0))&&
                ((countRho>=2)||(rhoTrumpRanks==0))) {
                ranks=0;
                for (k=0; k<=3; k++)
                  ranks=ranks | posPoint->diagram.cards[k][suit];
                for (rr=posPoint->secondBest[suit].rank-1; rr>=2; rr--) {
                  /* 3rd best at partner? */
                  if ((ranks & BitRank(rr))!=0) {
                    if ((posPoint->diagram.cards[partner(hand)][suit] &
                      BitRank(rr))!=0) {
                      found=TRUE;
                      break;
                    }
                    else {
                      found=FALSE;
                      break;
                    }
                  }
                  found=FALSE;
                }
                if (found) {
                  posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit] | BitRank(rr);
                  posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
                     BitRank(commRank);
                  qtricks++;
                  if ((countOwn<=2)&&(countLho<=2)&&(countRho<=2)&&
                    (lhoTrumpRanks==0)&&(rhoTrumpRanks==0)) 
                    qtricks=qtricks+countPart-2;
                }
              } 
          }
          else {
            posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
             | BitRank(posPoint->winner[suit].rank);
            posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
              BitRank(commRank);
            qtricks++;
            /* 06-12-14 */
            if (qtricks>=cutoff) {
              return qtricks;
            }

            if ((countLho<=1)&&(countRho<=1)&&(countOwn<=1)) {
              qtricks=qtricks+countPart-1;
              if (qtricks>=cutoff) {
                return qtricks;
              }
              continue;
            }
            if ((posPoint->secondBest[suit].hand==partner(hand))&&(countPart>0)) {
              /* Second best found in partners hand */
              posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
                | BitRank(posPoint->secondBest[suit].rank);
              posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
                BitRank(commRank);
              qtricks++;
              if ((countLho<=2)&&(countRho<=2)&&(countOwn<=2)) {
                qtricks=qtricks+countPart-2;
                if (qtricks>=cutoff)
                  return qtricks;
                continue;
              }
            }
            /* 06-08-19 */
            else if ((posPoint->secondBest[suit].hand==hand)
                  &&(countPart>1)&&(countOwn>1)) {
               /* Second best found in own hand and own and
                        partner's suit length > 1 */
              posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit]
               | BitRank(posPoint->secondBest[suit].rank);
              posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
                BitRank(commRank);
              qtricks++;
              if ((countLho<=2)&&(countRho<=2)&&((countOwn<=2)||(countPart<=2))) {  /* 07-06-10 */
                qtricks=qtricks+Max(countPart-2,countOwn-2);
                if (qtricks>=cutoff) {
                  return qtricks;
                }
                continue;
              }
            }
            /* 06-08-24 */
            else if ((suit==commSuit)&&(posPoint->secondBest[suit].hand
                                        ==lho(hand))) {
              ranks=0;
              for (k=0; k<=3; k++)
                ranks=ranks | posPoint->diagram.cards[k][suit];
              for (rr=posPoint->secondBest[suit].rank-1; rr>=2; rr--) {
                  /* 3rd best at partner? */
                if ((ranks & BitRank(rr))!=0) {
                  if ((posPoint->diagram.cards[partner(hand)][suit] &
                    BitRank(rr))!=0) {
                    found=TRUE;
                    break;
                  }
                  else {
                    found=FALSE;
                    break;
                  }
                }
                found=FALSE;
              }
              if (found) {
                posPoint->stack[depth].winRanks[suit]=posPoint->stack[depth].winRanks[suit] | BitRank(rr);
                posPoint->stack[depth].winRanks[commSuit]=posPoint->stack[depth].winRanks[commSuit] |
                  BitRank(commRank);
                qtricks++;
                if ((countOwn<=2)&&(countLho<=2)&&(countRho<=2)) { 
                  qtricks=qtricks+countPart-2;
                }
              }
            } 
          }
        }
      }
    }

    if (contract.notTrumpWithTrump(suit) &&(countOwn>0)&&(lowestQtricks==0)&&
        ((qtricks==0)||((posPoint->winner[suit].hand!=hand)&&
                        (posPoint->winner[suit].hand!=partner(hand))&&
        (posPoint->winner[contract.trump].hand!=hand)&&
                        (posPoint->winner[contract.trump].hand!=partner(hand))))) {
      if ((countPart==0)&&(posPoint->length[partner(hand)][contract.trump]>0)) {
        if (((countRho>0)||(posPoint->length[rho(hand)][contract.trump]==0))&&
            ((countLho>0)||(posPoint->length[lho(hand)][contract.trump]==0))) {
          lowestQtricks=1; 
          if (1>=cutoff) {
            return 1;
          }
          continue;
        } else if ((countRho==0)&&(countLho==0)) {
          if ((posPoint->diagram.cards[lho(hand)][contract.trump] |
               posPoint->diagram.cards[rho(hand)][contract.trump]) <
              posPoint->diagram.cards[partner(hand)][contract.trump]) {
            lowestQtricks=1; 

            rr=highestRank[posPoint->diagram.cards[partner(hand)][contract.trump]];

            if (rr!=0) {
              posPoint->stack[depth].winRanks[contract.trump]|=BitRank(rr);
              if (1>=cutoff) {
                return 1;
              }
            }
          }
          continue;
        } else if (countLho==0) {
          if (posPoint->diagram.cards[lho(hand)][contract.trump] <
              posPoint->diagram.cards[partner(hand)][contract.trump]) {
            lowestQtricks=1; 
            for (rr=14; rr>=2; rr--) {
              if ((posPoint->diagram.cards[partner(hand)][contract.trump] & BitRank(rr))!=0) {
                posPoint->stack[depth].winRanks[contract.trump] |= BitRank(rr);
                break;
              }
            }
            if (1>=cutoff) {
              return 1;
            }
          }
          continue;
        } else if (countRho==0) {
          if (posPoint->diagram.cards[rho(hand)][contract.trump] <
              posPoint->diagram.cards[partner(hand)][contract.trump]) {
            lowestQtricks=1; 
            for (rr=14; rr>=2; rr--) {
              if ((posPoint->diagram.cards[partner(hand)][contract.trump] &
                BitRank(rr))!=0) {
                posPoint->stack[depth].winRanks[contract.trump] |= BitRank(rr);
                break;
              }
            }
            if (1>=cutoff) {
              return 1;
            }
          }
          continue;
        }
      }
    }

    if (qtricks>=cutoff) {
      return qtricks;
    }

  }

  if (qtricks==0) {
    if ((!contract.trumpContract)||(posPoint->winner[contract.trump].rank==0)) {
      found=FALSE;
      for (ss=0; ss<=3; ss++) {
        if (posPoint->winner[ss].rank==0)
          continue;
        hh=posPoint->winner[ss].hand;
        if (nodeTypeStore[hh]==nodeTypeStore[hand]) {
          if (posPoint->length[hand][ss]>0) {
            found=TRUE;   
            break;
          }
        }
        else if ((posPoint->length[partner(hand)][ss]>0)&&
          (posPoint->length[hand][ss]>0)&&
                 (posPoint->length[partner(hh)][ss]>0)) { 
          count++;
                }
        }
        if (!found) {
          if (nodeTypeStore[hand]==MAXNODE) {
            if ((posPoint->tricksMAX+(depth>>2)-Max(0,count-1))<target) {
              for (ss=0; ss<=3; ss++) {
                if ((posPoint->length[hand][ss]>0)
                   &&(nodeTypeStore[posPoint->winner[ss].hand]==MINNODE))
                  posPoint->stack[depth].winRanks[ss]=
                    BitRank(posPoint->winner[ss].rank);
                else
                  posPoint->stack[depth].winRanks[ss]=0;
              }
              return 0;
            }
          }
          else {
            if ((posPoint->tricksMAX+1+Max(0,count-1))>=target) {
              for (ss=0; ss<=3; ss++) {
                if ((posPoint->length[hand][ss]>0) 
                   &&(nodeTypeStore[posPoint->winner[ss].hand]==MAXNODE))
                  posPoint->stack[depth].winRanks[ss]=
                    BitRank(posPoint->winner[ss].rank);
                else
                  posPoint->stack[depth].winRanks[ss]=0;
              }
              return 0;
            }
          }
      }
    }
  }

  *result=FALSE;
  return qtricks;
}


int LaterTricksMIN(struct pos *posPoint, const int hand, const int depth, const int target) {
  int hh, ss, sum=0;

  const ContractInfo contract = Globals.getContract();

  if ((!contract.trumpContract)||(posPoint->winner[contract.trump].rank==0)) {
    for (ss=0; ss<=3; ss++) {
      hh=posPoint->winner[ss].hand;
      if (nodeTypeStore[hh]==MAXNODE)
        sum+=Max(posPoint->length[hh][ss], posPoint->length[partner(hh)][ss]);
    }
    if ((posPoint->tricksMAX+sum<target)&&
      (sum>0)&&(depth>0)&&(depth!=iniDepth)) {
      if ((posPoint->tricksMAX+(depth>>2)<target)) {
        for (ss=0; ss<=3; ss++) {
          if (nodeTypeStore[posPoint->winner[ss].hand]==MINNODE)  
            posPoint->stack[depth].winRanks[ss]=BitRank(posPoint->winner[ss].rank);
          else
            posPoint->stack[depth].winRanks[ss]=0;
        }
        return FALSE;
      }
    } 
  } else if (contract.trumpContract && (posPoint->winner[contract.trump].rank!=0) && 
             (nodeTypeStore[posPoint->winner[contract.trump].hand]==MINNODE)) {
    if ((posPoint->length[hand][contract.trump]==0)&&
        (posPoint->length[partner(hand)][contract.trump]==0)) {
      if (((posPoint->tricksMAX+(depth>>2)+1-
            Max(posPoint->length[lho(hand)][contract.trump],
            posPoint->length[rho(hand)][contract.trump]))<target)
        &&(depth>0)&&(depth!=iniDepth)) {
        for (ss=0; ss<=3; ss++) {
          posPoint->stack[depth].winRanks[ss]=0;
        }
        return FALSE;
      }
    } else if (((posPoint->tricksMAX+(depth>>2))<target)&&
      (depth>0)&&(depth!=iniDepth)) {
      for (ss=0; ss<=3; ss++)
        posPoint->stack[depth].winRanks[ss]=0;
        posPoint->stack[depth].winRanks[contract.trump]=
          BitRank(posPoint->winner[contract.trump].rank);
        return FALSE;
    } else {
      hh=posPoint->secondBest[contract.trump].hand;
      if ((nodeTypeStore[hh]==MINNODE)&&(posPoint->secondBest[contract.trump].rank!=0))  {
        if (((posPoint->length[hh][contract.trump]>1) ||
             (posPoint->length[partner(hh)][contract.trump]>1))&&
          ((posPoint->tricksMAX+(depth>>2)-1)<target)&&(depth>0)&&(depth!=iniDepth)) {
          for (ss=0; ss<=3; ss++)
            posPoint->stack[depth].winRanks[ss]=0;
          posPoint->stack[depth].winRanks[contract.trump]=
              BitRank(posPoint->winner[contract.trump].rank) | 
              BitRank(posPoint->secondBest[contract.trump].rank) ;
          return FALSE;
        }
      }
    }        
  } else if (contract.trumpContract) {
    hh=posPoint->secondBest[contract.trump].hand; 
    if ((nodeTypeStore[hh]==MINNODE)&&
      (posPoint->length[hh][contract.trump]>1)&&
        (posPoint->winner[contract.trump].hand==rho(hh))
      &&(posPoint->secondBest[contract.trump].rank!=0)) {
      if (((posPoint->tricksMAX+(depth>>2))<target)&&
        (depth>0)&&(depth!=iniDepth)) {
        for (ss=0; ss<=3; ss++) {
          posPoint->stack[depth].winRanks[ss]=0;
        }
        posPoint->stack[depth].winRanks[contract.trump]= BitRank(posPoint->secondBest[contract.trump].rank) ; 
        return FALSE;
      }
    }
  }
  return TRUE;
}


int LaterTricksMAX(struct pos *posPoint, const int hand, const int depth, const int target) {
  int hh, ss, sum=0;
  const ContractInfo contract = Globals.getContract();
        
  if ((!contract.trumpContract)||(posPoint->winner[contract.trump].rank==0)) {
    for (ss=0; ss<=3; ss++) {
      hh=posPoint->winner[ss].hand;
      if (nodeTypeStore[hh]==MINNODE)
        sum+=Max(posPoint->length[hh][ss], posPoint->length[partner(hh)][ss]);
    }
    if ((posPoint->tricksMAX+(depth>>2)+1-sum>=target)&&
       (sum>0)&&(depth>0)&&(depth!=iniDepth)) {
      if ((posPoint->tricksMAX+1>=target)) {
        for (ss=0; ss<=3; ss++) {
          if (nodeTypeStore[posPoint->winner[ss].hand]==MAXNODE)  
            posPoint->stack[depth].winRanks[ss]=BitRank(posPoint->winner[ss].rank);
          else
            posPoint->stack[depth].winRanks[ss]=0;
        }
        return TRUE;
      }
    }
  } else if (contract.trumpContract && (posPoint->winner[contract.trump].rank!=0) &&
    (nodeTypeStore[posPoint->winner[contract.trump].hand]==MAXNODE)) {
    if ((posPoint->length[hand][contract.trump]==0)&&
        (posPoint->length[partner(hand)][contract.trump]==0)) {
      if (((posPoint->tricksMAX+Max(posPoint->length[lho(hand)][contract.trump],
                                    posPoint->length[rho(hand)][contract.trump]))>=target)
        &&(depth>0)&&(depth!=iniDepth)) {
        for (ss=0; ss<=3; ss++)
          posPoint->stack[depth].winRanks[ss]=0;
          return TRUE;
      }
    } else if (((posPoint->tricksMAX+1)>=target) &&(depth>0)&&(depth!=iniDepth)) {
      for (ss=0; ss<=3; ss++) {
        posPoint->stack[depth].winRanks[ss]=0;
      }
      posPoint->stack[depth].winRanks[contract.trump] =        BitRank(posPoint->winner[contract.trump].rank);
      return TRUE;
    }
    else {
      hh=posPoint->secondBest[contract.trump].hand;
      if ((nodeTypeStore[hh]==MAXNODE)&&(posPoint->secondBest[contract.trump].rank!=0))  {
        if (((posPoint->length[hh][contract.trump]>1) ||
             (posPoint->length[partner(hh)][contract.trump]>1))&&
          ((posPoint->tricksMAX+2)>=target)&&(depth>0)&&(depth!=iniDepth)) {
          for (ss=0; ss<=3; ss++) {
            posPoint->stack[depth].winRanks[ss]=0;
          }
          posPoint->stack[depth].winRanks[contract.trump]=
            BitRank(posPoint->winner[contract.trump].rank) | 
            BitRank(posPoint->secondBest[contract.trump].rank) ;
          return TRUE;
         }
      }
    }
  } else if (contract.trumpContract) {
    hh=posPoint->secondBest[contract.trump].hand; 
    if ((nodeTypeStore[hh]==MAXNODE)&&
        (posPoint->length[hh][contract.trump]>1)&&(posPoint->winner[contract.trump].hand==rho(hh))
      &&(posPoint->secondBest[contract.trump].rank!=0)) {
      if (((posPoint->tricksMAX+1)>=target)&&(depth>0)&&(depth!=iniDepth)) {
        for (ss=0; ss<=3; ss++)
          posPoint->stack[depth].winRanks[ss]=0;
          posPoint->stack[depth].winRanks[contract.trump]=
          BitRank(posPoint->secondBest[contract.trump].rank) ;  
        return TRUE;
      }
    }
    /*found=FALSE;
    for (ss=0; ss<=3; ss++) {
      if ((ss!=contract.trump)&&(posPoint->winner[ss].rank!=0)) {
        hh=posPoint->winner[ss].hand;
        if ((nodeTypeStore[hh]==MINNODE)||
          (posPoint->length[hand][ss]==0)||
          (posPoint->length[partner(hand)][ss]==0)) {
          found=TRUE;
          break;
        }
      }
    }
    if (!found) {
      sum=Max(posPoint->length[hand][contract.trump], 
        posPoint->length[partner(hand)][contract.trump]);
      if ((posPoint->tricksMAX+(depth>>2)+1-sum>=target)&&
         (depth>0)&&(depth!=iniDepth)) {
        if ((posPoint->tricksMAX+1>=target)&&(sum>0)) {
          for (ss=0; ss<=3; ss++) {
            if (ss!=contract.trump) {
              if (nodeTypeStore[posPoint->winner[ss].hand]==MAXNODE)  
                posPoint->stack[depth].winRanks[ss]=BitRank(posPoint->winner[ss].rank);
              else
                posPoint->stack[depth].winRanks[ss]=0;
            }
            else
              posPoint->stack[depth].winRanks[ss]=0;
          }
          return TRUE;
        }
      }
    }*/
  }
  return FALSE;
}


int recInd=0;

int MoveGen(const struct pos * posPoint, const int depth) { 

  int suit, k, m, n, r, s, t;
  int scount[4];
  int WeightAlloc(const struct pos *, struct moveType * mp, int depth,
                  int notVoidInSuit,int q, int first);

  for (k=0; k<4; k++) 
    lowestWin[depth][k]=0;
  
  m=0;
  r=posPoint->handRelFirst;
  const int first=posPoint->stack[depth].first;
  const int q=RelativeHand(first,r);
  
  s=movePly[depth+r].current;             /* Current move of first hand */
  t=movePly[depth+r].move[s].suit;        /* Suit played by first hand */
  const holding_t ris = posPoint->diagram.cards[q][t];

  if ((r!=0)&&(ris!=0)) {
  /* Not first hand and not void in suit */
    holding_t sequences;
    holding_t unplayedCardsRank;
    holding_t allCards, bitRank;
    holding_t sequence = 0;

    unplayedCardsRank = 
      unplayedFinder.getUnplayed(q,t, 
                                     (holding_t)posPoint->removedRanks[t],
                               sequences);
    allCards = unplayedCardsRank | sequences;
    while (allCards) {
      //cerr << Holding(allCards) << endl;
      bitRank = smallestRankInSuit(allCards);
      allCards ^= bitRank;
      if (unplayedCardsRank & bitRank) {
        movePly[depth].move[m].suit=t;
        movePly[depth].move[m].rank=InvBitMapRank(bitRank);
        movePly[depth].move[m].sequence= sequence;
        sequence = 0;
        m++;
      } else {
        sequence |= bitRank;
      }
    }

    if (m!=1) {
      for (k=0; k<=m-1; k++)  {
        movePly[depth].move[k].weight=WeightAlloc(posPoint,
                                                  &movePly[depth].move[k], depth, ris,q,first);
      }
    }

    movePly[depth].last=m-1;
    if (m!=1)
      InsertSort(m, depth);
    if (depth!=iniDepth) {
      return m;
    } else {
      m=AdjustMoveList();
      return m;
    }
  } else {                  /* First hand or void in suit */
    holding_t sequences;
    holding_t unplayedCardsRank;
    holding_t bitRank;
    holding_t allCards;
    holding_t sequence = 0;
    for (suit=0; suit<=3; suit++)  {
      unplayedCardsRank = 
        unplayedFinder.getUnplayed(q,suit, 
                                   (holding_t)(posPoint->removedRanks[suit]),
                                   sequences);
      allCards = unplayedCardsRank | sequences;
      while (allCards) {
        //cerr << Holding(allCards) << endl;
        bitRank = smallestRankInSuit(allCards);
        allCards ^= bitRank;
        if (unplayedCardsRank & bitRank) {
          movePly[depth].move[m].suit=suit;
          movePly[depth].move[m].rank=InvBitMapRank(bitRank);
          movePly[depth].move[m].sequence=sequence;
          sequence = 0;
          m++;
        } else {
          sequence |= bitRank;
        }
      }

    }

    for (k=0; k<=m-1; k++) {
        movePly[depth].move[k].weight=WeightAlloc(posPoint,
                                                  &movePly[depth].move[k], depth, ris,q,first);
    }
  
    movePly[depth].last=m-1;
    InsertSort(m, depth);
    if (r==0) {
      for (n=0; n<=3; n++) {
        scount[n]=0;
      }

      for (k=0; k<=m-1; k++) {
        if (scount[movePly[depth].move[k].suit]==1 /* 2 */) 
          continue;
        else {
          movePly[depth].move[k].weight+=500;
          scount[movePly[depth].move[k].suit]++;
        }
      }
      InsertSort(m, depth);
    }
    else {

      for (n=0; n<=3; n++) {
        scount[n]=0;
      }

      for (k=0; k<=m-1; k++) {
        if (scount[movePly[depth].move[k].suit]==1)  {
          continue;
        } else {
          movePly[depth].move[k].weight+=500;
          scount[movePly[depth].move[k].suit]++;
        }
      }
      InsertSort(m, depth);
    }

    if (depth!=iniDepth) {
     return m;
    } else {
      m=AdjustMoveList();
      return m;
    }  
  }
}


int WeightAlloc(const struct pos * posPoint, struct moveType * mp, const int depth,  const int notVoidInSuit,const int q, const int first) {
  int weight=0, suitAdd=0, leadSuit;
  holding_t k,l,kk,ll;
  int suitWeightDelta;
  int suitBonus=0;
  int winMove=FALSE;
  unsigned short suitCount, suitCountLH, suitCountRH;
  int countLH, countRH;
  const ContractInfo contract = Globals.getContract();

  const int suit=mp->suit;

  if ((!notVoidInSuit)||(posPoint->handRelFirst==0)) {
    suitCount=posPoint->length[q][suit];
    suitAdd=suitCount+suitCount;
  }

  switch (posPoint->handRelFirst) {
    case 0:
      suitCountLH=posPoint->length[lho(q)][suit];
      suitCountRH=posPoint->length[rho(q)][suit];

      if (contract.notTrumpWithTrump(suit) &&
          (((posPoint->diagram.cards[lho(q)][suit]==0) &&
            (posPoint->diagram.cards[lho(q)][contract.trump]!=0)) ||
         ((posPoint->diagram.cards[rho(q)][suit]==0) &&
          (posPoint->diagram.cards[rho(q)][contract.trump]!=0))))
          suitBonus=-12/*15*/;

      if (suitCountLH!=0) {
        countLH=(suitCountLH<<2);
      } else {
        countLH=depth+4;
      }
      if (suitCountRH!=0) {
        countRH=(suitCountRH<<2);
      } else {
        countRH=depth+4;
      }

      suitWeightDelta=suitBonus-((countLH+countRH)<<1);

      if (posPoint->winner[suit].rank==mp->rank) {
        if (contract.notTrumpWithTrump(suit)) {
          if ((posPoint->length[partner(first)][suit]!=0)||
              (posPoint->length[partner(first)][contract.trump]==0)) {
            if (((posPoint->length[lho(first)][suit]!=0)||
                 (posPoint->length[lho(first)][contract.trump]==0))&&
                ((posPoint->length[rho(first)][suit]!=0)||
                 (posPoint->length[rho(first)][contract.trump]==0)))
               winMove=TRUE;
          }
          else if (((posPoint->length[lho(first)][suit]!=0)||
                    (posPoint->diagram.cards[partner(first)][contract.trump]>
                     posPoint->diagram.cards[lho(first)][contract.trump]))&&
                   ((posPoint->length[rho(first)][suit]!=0)||
             (posPoint->diagram.cards[partner(first)][contract.trump]>
              posPoint->diagram.cards[rho(first)][contract.trump])))
             winMove=TRUE;
        }
        else 
          winMove=TRUE;                           
      }
      else if (posPoint->diagram.cards[partner(first)][suit] >
               (posPoint->diagram.cards[lho(first)][suit] |
         posPoint->diagram.cards[rho(first)][suit])) {
        if (contract.notTrumpWithTrump(suit)) {
          if (((posPoint->length[lho(first)][suit]!=0)||
               (posPoint->length[lho(first)][contract.trump]==0))&&
              ((posPoint->length[rho(first)][suit]!=0)||
               (posPoint->length[rho(first)][contract.trump]==0)))
             winMove=TRUE;
        }
        else
          winMove=TRUE;
      }                        
      else if (contract.notTrumpWithTrump(suit)) {
        if ((posPoint->length[partner(first)][suit]==0)&&
            (posPoint->length[partner(first)][contract.trump]!=0)) {
          if ((posPoint->length[lho(first)][suit]==0)&&
              (posPoint->length[lho(first)][contract.trump]!=0)&&
              (posPoint->length[rho(first)][suit]==0)&&
              (posPoint->length[rho(first)][contract.trump]!=0)) {
            if (posPoint->diagram.cards[partner(first)][contract.trump]>
                (posPoint->diagram.cards[lho(first)][contract.trump] |
               posPoint->diagram.cards[rho(first)][contract.trump]))
              winMove=TRUE;
          }
          else if ((posPoint->length[lho(first)][suit]==0)&&
                   (posPoint->length[lho(first)][contract.trump]!=0)) {
            if (posPoint->diagram.cards[partner(first)][contract.trump]
                > posPoint->diagram.cards[lho(first)][contract.trump])
               winMove=TRUE;
          }        
          else if ((posPoint->length[rho(first)][suit]==0)&&
                   (posPoint->length[rho(first)][contract.trump]!=0)) {
            if (posPoint->diagram.cards[partner(first)][contract.trump]
                > posPoint->diagram.cards[rho(first)][contract.trump])
              winMove=TRUE;
          }        
          else
            winMove=TRUE;
        }
      }
              
      if (winMove) {
        if (((posPoint->winner[suit].hand==lho(first))&&(suitCountLH==1))
            ||((posPoint->winner[suit].hand==rho(first))&&(suitCountRH==1)))
          weight=suitWeightDelta+40-(mp->rank);
        else if (posPoint->winner[suit].hand==first) {
          if (posPoint->secondBest[suit].hand==partner(first)&&
              (posPoint->secondBest[suit].rank!=0)) {
            weight=suitWeightDelta+50-(mp->rank);
          } else if (posPoint->winner[suit].rank==mp->rank) {
            weight=suitWeightDelta+31;
          } else {
            weight=suitWeightDelta+19-(mp->rank);
          }
        }
        else if (posPoint->winner[suit].hand==partner(first)) {
          /* If partner has winning card */
          if (posPoint->secondBest[suit].hand==first)
            weight=suitWeightDelta+50-(mp->rank);
          else 
            weight=suitWeightDelta+35-(mp->rank);  
        } 
        else if ((mp->sequence)&&
          (mp->rank==posPoint->secondBest[suit].rank))                        
          weight=suitWeightDelta+40/*35*/-(mp->rank);
        else
          weight=suitWeightDelta+30-(mp->rank);
        if ((bestMove[depth].suit==mp->suit)&&
          (bestMove[depth].rank==mp->rank)) 
          weight+=50/*45*//*35*/; 
      }
      else {
        if (((posPoint->winner[suit].hand==lho(first))&&(suitCountLH==1))
            ||((posPoint->winner[suit].hand==rho(first))&&(suitCountRH==1)))
          weight=suitWeightDelta+20-(mp->rank);
        else if (posPoint->winner[suit].hand==first&&
                    (posPoint->secondBest[suit].rank!=0)) {
          if (posPoint->secondBest[suit].hand==partner(first)) {
            weight=suitWeightDelta+35-(mp->rank);
          } else if (posPoint->winner[suit].rank==mp->rank)  {
            weight=suitWeightDelta+16;
          } else {
            weight=suitWeightDelta+4-(mp->rank);
          }
        }
        else if (posPoint->winner[suit].hand==partner(first)) {
          /* If partner has winning card */
          if (posPoint->secondBest[suit].hand==first) {
            weight=suitWeightDelta+35-(mp->rank);
          } else {
            weight=suitWeightDelta+20-(mp->rank);  
          }
        } 
        else if ((mp->sequence)&&
          (mp->rank==posPoint->secondBest[suit].rank)) 
          weight=suitWeightDelta+20-(mp->rank);
        else 
          weight=suitWeightDelta+4-(mp->rank);
        if ((bestMove[depth].suit==mp->suit)&&
          (bestMove[depth].rank==mp->rank)) 
          weight+=30/*25*//*35*/; 
      }
        
      break;

    case 1:
      leadSuit=posPoint->stack[depth+1].move.suit;
      if (leadSuit==suit) {
        if (BitRank(mp->rank)>
          (BitRank(posPoint->stack[depth+1].move.rank) |
           posPoint->diagram.cards[partner(first)][suit])) {
          if (contract.notTrumpWithTrump(suit)) {
            if ((posPoint->length[partner(first)][suit]!=0)||
                (posPoint->length[partner(first)][contract.trump]==0))
              winMove=TRUE;
            else if ((posPoint->length[rho(first)][suit]==0)
                     &&(posPoint->length[rho(first)][contract.trump]!=0)
                     &&(posPoint->diagram.cards[rho(first)][contract.trump]>
                  posPoint->diagram.cards[partner(first)][contract.trump]))
               winMove=TRUE;
          }
          else
            winMove=TRUE;
        }
        else if (posPoint->diagram.cards[rho(first)][suit]>
          (BitRank(posPoint->stack[depth+1].move.rank) |
           posPoint->diagram.cards[partner(first)][suit])) {         
          if (contract.notTrumpWithTrump(suit)) {
            if ((posPoint->length[partner(first)][suit]!=0)||
                (posPoint->length[partner(first)][contract.trump]==0)) {
              winMove=TRUE;
            }
          }
          else
            winMove=TRUE;
        } 
        else if (BitRank(posPoint->stack[depth+1].move.rank) >
                 (posPoint->diagram.cards[rho(first)][suit] |
           posPoint->diagram.cards[partner(first)][suit] |
          BitRank(mp->rank))) {  
          if (contract.notTrumpWithTrump(suit)) {
            if ((posPoint->length[rho(first)][suit]==0)&&
                (posPoint->length[rho(first)][contract.trump]!=0)) {
              if ((posPoint->length[partner(first)][suit]!=0)||
                  (posPoint->length[partner(first)][contract.trump]==0)) {
                winMove=TRUE;
              } else if (posPoint->diagram.cards[rho(first)][contract.trump] 
                         > posPoint->diagram.cards[partner(first)][contract.trump]) {
                 winMove=TRUE;
              }
            }  
          }
        } else {   /* winnerHand is partner to first */
          if (contract.notTrumpWithTrump(suit)) {
            if ((posPoint->length[rho(first)][suit]==0)&&
                (posPoint->length[rho(first)][contract.trump]!=0))
              winMove=TRUE;
          }  
        }
      }
      else {
        /* Leading suit differs from suit played by LHO */
        if (contract.isTrump(suit)) {
          if (posPoint->length[partner(first)][leadSuit]!=0)
            winMove=TRUE;
          else if (BitRank(mp->rank)>
                   posPoint->diagram.cards[partner(first)][contract.trump]) 
            winMove=TRUE;
          else if ((posPoint->length[rho(first)][leadSuit]==0)
                   &&(posPoint->length[rho(first)][contract.trump]!=0)&&
                   (posPoint->diagram.cards[rho(first)][contract.trump] >
             posPoint->diagram.cards[partner(first)][contract.trump]))
            winMove=TRUE;
        }        
        else if (contract.notTrumpWithTrump(leadSuit)) {
          /* Neither suit nor leadSuit is trump */
          if (posPoint->length[partner(first)][leadSuit]!=0) {
            if (posPoint->diagram.cards[rho(first)][leadSuit] >
                (posPoint->diagram.cards[partner(first)][leadSuit] |
              BitRank(posPoint->stack[depth+1].move.rank)))
              winMove=TRUE;
            else if ((posPoint->length[rho(first)][leadSuit]==0)
                     &&(posPoint->length[rho(first)][contract.trump]!=0))
              winMove=TRUE;
          }
          /* Partner to leading hand is void in leading suit */
          else if ((posPoint->length[rho(first)][leadSuit]==0)
                   &&(posPoint->diagram.cards[rho(first)][contract.trump]>
               posPoint->diagram.cards[partner(first)][contract.trump]))
            winMove=TRUE;
          else if ((posPoint->length[partner(first)][contract.trump]==0)
                   &&(posPoint->diagram.cards[rho(first)][leadSuit] >
            BitRank(posPoint->stack[depth+1].move.rank)))
            winMove=TRUE;
        }
        else {
            /* Either no trumps or leadSuit is trump, side with
                highest rank in leadSuit wins */
          if (posPoint->diagram.cards[rho(first)][leadSuit] >
                (posPoint->diagram.cards[partner(first)][leadSuit] |
             BitRank(posPoint->stack[depth+1].move.rank)))
             winMove=TRUE;                           
        }                          
      }
      
      kk=posPoint->diagram.cards[partner(first)][leadSuit];
      ll=posPoint->diagram.cards[rho(first)][leadSuit];
      k=kk & (-kk); l=ll & (-ll);  /* Only least significant 1 bit */
      if (winMove) {
        if (!notVoidInSuit) { 
          if (contract.isTrump(suit)) {
            weight=30-(mp->rank)+suitAdd;
          } else {
            weight=60-(mp->rank)+suitAdd;  /* Better discard than ruff since rho
                                                                wins anyway */
          }
        } else if (k > BitRank(mp->rank)) {
          weight=45-(mp->rank);    /* If lowest card for partner to leading hand 
                                                is higher than lho played card, playing as low as 
                                                possible will give the cheapest win */
        } else if ((ll > BitRank(posPoint->stack[depth+1].move.rank))&&
          (posPoint->diagram.cards[first][leadSuit] > ll))  {
          weight=60-(mp->rank);    /* If rho has a card in the leading suit that
                                    is higher than the trick leading card but lower
                                    than the highest rank of the leading hand, then
                                    lho playing the lowest card will be the cheapest
                                    win */
        } else if (mp->rank > posPoint->stack[depth+1].move.rank) {
          if (BitRank(mp->rank) < ll) 
            weight=75-(mp->rank);  /* If played card is lower than any of the cards of
                                                rho, it will be the cheapest win */
          else if (BitRank(mp->rank) > kk)
            weight=70-(mp->rank);  /* If played card is higher than any cards at partner
                                                of the leading hand, rho can play low, under the
                                    condition that he has a lower card than lho played */ 
          else {
            if (mp->sequence)
              weight=60-(mp->rank); 
            else
              weight=45-(mp->rank);
          }
        } 
        else if (posPoint->length[rho(first)][leadSuit]>0) {
          if (mp->sequence)
            weight=50-(mp->rank);  /* Plyiang a card in a sequence may promote a winner */
          else
            weight=45-(mp->rank);
        }
        else
          weight=45-(mp->rank);
      }
      else {
        if (!notVoidInSuit) {
          if (contract.isTrump(suit)) {
          /*if (ll > BitRank(posPoint->stack[depth+1].move.rank))
                          weight=-10-(mp->rank)+suitAdd;
                        else*/
              weight=15-(mp->rank)+suitAdd;  /* Ruffing is preferred, makes the trick
                                                          costly for the opponents */
          }
          else
            weight=-(mp->rank)+suitAdd;
        }
        else if ((k > BitRank(mp->rank))||
          (l > BitRank(mp->rank))) 
          weight=-(mp->rank);  /* If lowest rank for either partner to leading hand 
                                                or rho is higher than played card for lho,
                                                lho should play as low card as possible */                        
        else if (mp->rank > posPoint->stack[depth+1].move.rank) {                  
          if (mp->sequence) 
            weight=20-(mp->rank);
          else 
            weight=10-(mp->rank);
        }          
        else
          weight=-(mp->rank);  
      }

      break;

    case 2:
            
      leadSuit=posPoint->stack[depth+2].move.suit;
      if (WinningMove(*mp, (posPoint->stack[depth+1].move))) {
        if (suit==leadSuit) {
          if (contract.notTrumpWithTrump(leadSuit)) {
            if (((posPoint->length[rho(first)][suit]!=0)||
                 (posPoint->length[rho(first)][contract.trump]==0))&&
              (BitRank(mp->rank) >
               posPoint->diagram.cards[rho(first)][suit]))
              winMove=TRUE;
          }        
          else if (BitRank(mp->rank) >
                   posPoint->diagram.cards[rho(first)][suit])
            winMove=TRUE;
        }
        else {  /* Suit is trump */
          if (posPoint->length[rho(first)][leadSuit]==0) {
            if (BitRank(mp->rank) >
                posPoint->diagram.cards[rho(first)][contract.trump])
              winMove=TRUE;
          }
          else
            winMove=TRUE;
        }
      }        
      else if (posPoint->stack[depth+1].high==first) {
        if (posPoint->length[rho(first)][leadSuit]!=0) {
          if (posPoint->diagram.cards[rho(first)][leadSuit]
            < BitRank(posPoint->stack[depth+2].move.rank))        
            winMove=TRUE;
        } else if (!contract.trumpContract) {
          winMove=TRUE;
        } else if (contract.isTrump(leadSuit)) {
          winMove=TRUE;
        } else if (contract.notTrumpWithTrump(leadSuit) &&
                   (posPoint->length[rho(first)][contract.trump]==0)) {
          winMove=TRUE;
        }
      }
      
      if (winMove) {
        if (!notVoidInSuit) {
          if (posPoint->stack[depth+1].high==first) {
            if (contract.isTrump(suit)) {
              weight=30-(mp->rank)+suitAdd; /* Ruffs partner's winner */
            } else {
              weight=60-(mp->rank)+suitAdd;
            }
          } else if (WinningMove(*mp, (posPoint->stack[depth+1].move))) {
             /* Own hand on top by ruffing */
            weight=70-(mp->rank)+suitAdd;
          } else if (contract.isTrump(suit)) {
            /* Discard a trump but still losing */
            weight=15-(mp->rank)+suitAdd;
          } else {
            weight=30-(mp->rank)+suitAdd;
          }
        } else {
          weight=60-(mp->rank);  
        }
      } else {
        if (!notVoidInSuit) {
          if (WinningMove(*mp, (posPoint->stack[depth+1].move)))
             /* Own hand on top by ruffing */
            weight=40-(mp->rank)+suitAdd;
          else if (contract.isTrump(suit)) {
            /* Discard a trump but still losing */
            weight=-15-(mp->rank)+suitAdd;
          } else {
            weight=-(mp->rank)+suitAdd;
          }
        }
        else {
          if (WinningMove(*mp, (posPoint->stack[depth+1].move))) {
            if (mp->rank==posPoint->secondBest[leadSuit].rank)
              weight=25/*35*/;
            else if (mp->sequence)
              weight=20/*30*/-(mp->rank);
            else
              weight=10/*20*/-(mp->rank);
          }
          else  
            weight=-10/*0*/-(mp->rank);  
        } 
      }
            
      break;

    case 3:
      if (!notVoidInSuit) {
        if ((posPoint->stack[depth+1].high)==lho(first)) {
          /* If the current winning move is given by the partner */
          if (contract.isTrump(suit)) {
            /* Ruffing partners winner? */
            weight=14-(mp->rank)+suitAdd;
          } else {
            weight=30-(mp->rank)+suitAdd;
          }
        }
        else if (WinningMove(*mp, posPoint->stack[depth+1].move))  {
          /* Own hand ruffs */
          weight=30-(mp->rank)+suitAdd;
        } else if (suit==contract.trump) {
          weight=-(mp->rank);
        } else {
          weight=14-(mp->rank)+suitAdd;  
        }
      }
      else if ((posPoint->stack[depth+1].high)==(lho(first))) {
        /* If the current winning move is given by the partner */
        if (contract.isTrump(suit)) {
        /* Ruffs partners winner */
          weight=24-(mp->rank);
        } else {
          weight=30-(mp->rank);
        }
      } else if (WinningMove(*mp, posPoint->stack[depth+1].move)) {
        /* If present move is superior to current winning move and the
        current winning move is not given by the partner */
        weight=30-(mp->rank);
      } else {
        /* If present move is not superior to current winning move and the
        current winning move is not given by the partner */
        if (contract.isTrump(suit)) {
          /* Ruffs but still loses */
          weight=-(mp->rank);
        } else {
          weight=14-(mp->rank);
        }
      }
  }
  return weight;
}

/* Shell-1 */
/* K&R page 62: */
/*void shellSort(int n, int depth) {
  int gap, i, j;
  struct moveType temp;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      temp=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=temp;
      return;
    }
    else
      return;
  }
  for (gap=n>>1; gap>0; gap>>=1)
    for (i=gap; i<n; i++)
      for (j=i-gap; j>=0 && movePly[depth].move[j].weight<
         movePly[depth].move[j+gap].weight; j-=gap) {
        temp=movePly[depth].move[j];
        movePly[depth].move[j]=movePly[depth].move[j+gap];
        movePly[depth].move[j+gap]=temp;
      }
} */

/* Shell-2 */
/*void shellSort(int n, int depth)
{
  int i, j, increment;
  struct moveType temp;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      temp=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=temp;
      return;
    }
    else
      return;
  }
  increment = 3;
  while (increment > 0)
  {
    for (i=0; i < n; i++)
    {
      j = i;
      temp = movePly[depth].move[i];
      while ((j >= increment) && (movePly[depth].move[j-increment].weight < temp.weight))
      {
        movePly[depth].move[j] = movePly[depth].move[j - increment];
        j = j - increment;
      }
      movePly[depth].move[j] = temp;
    }
    if ((increment>>1) != 0)
      increment>>=1;
    else if (increment == 1)
      increment = 0;
    else
      increment = 1;
  }
} */


/* Insert-1 */
void InsertSort(int n, int depth) {
  int i, j;
  struct moveType a, temp;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      temp=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=temp;
      return;
    }
    else
      return;
  }

  a=movePly[depth].move[0];
  for (i=1; i<=n-1; i++) 
    if (movePly[depth].move[i].weight>a.weight) {
      temp=a;
      a=movePly[depth].move[i];
      movePly[depth].move[i]=temp;
    }
  movePly[depth].move[0]=a; 
  for (i=2; i<=n-1; i++) {  
    j=i;
    a=movePly[depth].move[i];
    while (a.weight>movePly[depth].move[j-1].weight) {
      movePly[depth].move[j]=movePly[depth].move[j-1];
      j--;
    }
    movePly[depth].move[j]=a;
  }
}  

/* Insert-2 */
/*void InsertSort(int n, int depth) {
  int i, j;
  struct moveType a;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      a=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=a;
      return;
    }
    else
      return;
  }
  for (j=1; j<=n-1; j++) {
    a=movePly[depth].move[j];
    i=j-1;
    while ((i>=0)&&(movePly[depth].move[i].weight<a.weight)) {
      movePly[depth].move[i+1]=movePly[depth].move[i];
      i--;
    }
    movePly[depth].move[i+1]=a;
  }
}  */


int AdjustMoveList(void) {
  int k, r, n, rank, suit;

  for (k=1; k<=13; k++) {
    suit=forbiddenMoves[k].suit;
    rank=forbiddenMoves[k].rank;
    for (r=0; r<=movePly[iniDepth].last; r++) {
      if ((suit==movePly[iniDepth].move[r].suit)&&
        (rank!=0)&&(rank==movePly[iniDepth].move[r].rank)) {
        /* For the forbidden move r: */
        for (n=r; n<=movePly[iniDepth].last; n++)
          movePly[iniDepth].move[n]=movePly[iniDepth].move[n+1];
        movePly[iniDepth].last--;
      }  
    }
  }
  return movePly[iniDepth].last+1;
}


          

int listNo;
struct winCardType * nextp;


inline int WinningMove(const struct moveType &mv1, const struct moveType &mv2) {
/* Return TRUE if move 1 wins over move 2, with the assumption that
   move 2 is the presently winning card of the trick */

  //const ContractInfo contract = Globals.getContract();
  return Globals.getContract().betterMove(mv1,mv2);
}


struct nodeCardsType * CheckSOP(struct pos * posPoint, struct nodeCardsType
  * nodep, int target, int tricks, int * result, int *value) {
    /* Check SOP if it matches the
    current position. If match, pointer to the SOP node is returned and
    result is set to TRUE, otherwise pointer to SOP node is returned
    and result set to FALSE. */

  /* 07-04-22 */ 
  if (nodeTypeStore[0]==MAXNODE) {
    if (nodep->lbound==-1) {  /* This bound values for
      this leading hand has not yet been determined */
      *result=FALSE;
      return nodep;
    }        
    else if ((posPoint->tricksMAX + nodep->lbound)>=target) {
      *value=TRUE;
      *result=TRUE;
      return nodep;
    }
    else if ((posPoint->tricksMAX + nodep->ubound)<target) {
      *value=FALSE;
      *result=TRUE;
      return nodep;
    }
  }
  else {
    if (nodep->ubound==-1) {  /* This bound values for
      this leading hand has not yet been determined */
      *result=FALSE;
      return nodep;
    }        
    else if ((posPoint->tricksMAX + (tricks + 1 - nodep->ubound))>=target) {
      *value=TRUE;
      *result=TRUE;
      return nodep;
    }
    else if ((posPoint->tricksMAX + (tricks + 1 - nodep->lbound))<target) {
      *value=FALSE;
      *result=TRUE;
      return nodep;
    }
  }

  *result=FALSE;
  return nodep;          /* No matching node was found */
}


struct nodeCardsType * UpdateSOP(struct pos * posPoint, struct nodeCardsType
  * nodep) {
    /* Update SOP node with new values for upper and lower
          bounds. */
        
    if ((posPoint->lbound > nodep->lbound) ||
                (nodep->lbound==-1))
      nodep->lbound=posPoint->lbound;
    if ((posPoint->ubound < nodep->ubound) ||
                (nodep->ubound==-1))
          nodep->ubound=posPoint->ubound;

    nodep->bestMoveSuit=posPoint->bestMoveSuit;
    nodep->bestMoveRank=posPoint->bestMoveRank;

    return nodep;
}


struct nodeCardsType * FindSOP(struct pos * posPoint,
  struct winCardType * nodeP, int firstHand, 
        int target, int tricks, int * valp) {
  struct nodeCardsType * sopP;
  struct winCardType * np;
  int s;

  np=nodeP; s=0;
  while ((np!=NULL)&&(s<4)) {
    if ((np->winMask & posPoint->orderSet[s])==
      np->orderSet)  {
      /* Winning rank set fits position */
      if (s==3) {
        sopP=CheckSOP(posPoint, np->first, target, tricks, &res, &val);
        *valp=val;
        if (res) {
          return sopP;
        }
        else {
          if (np->next!=NULL) {
            np=np->next;
          } else {
            np=np->prevWin;
            s--;
            if (np==NULL) {
              return NULL;
            }
            while (np->next==NULL) {
              np=np->prevWin;
              s--;
              if (np==NULL)  {
                /* Previous node is header node? */
                return NULL;
              }
            }
            np=np->next;
          }
        }
      }
      else if (s<4) {
        np=np->nextWin;
        s++;
      }
    }
    else {
      if (np->next!=NULL) {
        np=np->next;
      } else {
        np=np->prevWin;
        s--;
        if (np==NULL) {
          return NULL;
        }
        while (np->next==NULL) {
          np=np->prevWin;
          s--;
          if (np==NULL)  {
            /* Previous node is header node? */
            return NULL;
          }
        }
        np=np->next;
      }
    }
  }
  return NULL;
}


struct nodeCardsType * BuildPath(struct pos * posPoint, 
  struct posSearchType *nodep, int * result) {
  /* If result is TRUE, a new SOP has been created and BuildPath returns a
  pointer to it. If result is FALSE, an existing SOP is used and BuildPath
  returns a pointer to the SOP */

  int found, suit;
  struct winCardType * np, * p2, * nprev, * fnp, *pnp;
  struct winCardType temp;
  struct nodeCardsType * sopP=0, * p;

  np=nodep->posSearchPoint;
  nprev=NULL;
  suit=0;

  /* If winning node has a card that equals the next winning card deduced
  from the position, then there already exists a (partial) path */

  if (np==NULL) {   /* There is no winning list created yet */
   /* Create winning nodes */
    p2=&winCards[winSetSize];
    AddWinSet();
    p2->next=NULL;
    p2->nextWin=NULL;
    p2->prevWin=NULL;
    nodep->posSearchPoint=p2;
    p2->winMask=posPoint->winMask[suit];
    p2->orderSet=posPoint->winOrderSet[suit];
    p2->first=NULL;
    np=p2;           /* Latest winning node */
    suit++;
    while (suit<4) {
      p2=&winCards[winSetSize];
      AddWinSet();
      np->nextWin=p2;
      p2->prevWin=np;
      p2->next=NULL;
      p2->nextWin=NULL;
      p2->winMask=posPoint->winMask[suit];
      p2->orderSet=posPoint->winOrderSet[suit];
      p2->first=NULL;
      np=p2;         /* Latest winning node */
      suit++;
    }
    p=&nodeCards[nodeSetSize];
    AddNodeSet();
    np->first=p;
    *result=TRUE;
    return p;
  }
  else {   /* Winning list exists */
    while (1) {   /* Find all winning nodes that correspond to current
                position */
      found=FALSE;
      while (1) {    /* Find node amongst alternatives */
        if ((np->winMask==posPoint->winMask[suit])&&
           (np->orderSet==posPoint->winOrderSet[suit])) {
           /* Part of path found */
          found=TRUE;
          nprev=np;
          break;
        }
        if (np->next!=NULL) {
          np=np->next;
	} else {
          break;
        }
      }
      if (found) {
        suit++;
        if (suit>3) {
          sopP=UpdateSOP(posPoint, np->first);

          if (np->prevWin!=NULL) {
            pnp=np->prevWin;
            fnp=pnp->nextWin;
          } else {
            fnp=nodep->posSearchPoint;
          }

          temp.orderSet=np->orderSet;
          temp.winMask=np->winMask;
          temp.first=np->first;
          temp.nextWin=np->nextWin;
          np->orderSet=fnp->orderSet;
          np->winMask=fnp->winMask;
          np->first=fnp->first;
          np->nextWin=fnp->nextWin;
          fnp->orderSet=temp.orderSet;
          fnp->winMask=temp.winMask;
          fnp->first=temp.first;
          fnp->nextWin=temp.nextWin;

          *result=FALSE;
          return sopP;
        }
        else {
          np=np->nextWin;       /* Find next winning node  */
          continue;
        }
      }
      else
        break;        /* Node was not found */
    }  /* End outer while */

    /* Create additional node, coupled to existing node(s) */
    p2=&winCards[winSetSize];
    AddWinSet();
    /*np->next=p2;*/
    p2->prevWin=nprev;
    if (nprev!=NULL) {
      p2->next=nprev->nextWin;
      nprev->nextWin=p2;
    }
    else {
      p2->next=nodep->posSearchPoint;
      nodep->posSearchPoint=p2;
    }
    p2->nextWin=NULL;
    /*p2->next=NULL;*/
    p2->winMask=posPoint->winMask[suit];
    p2->orderSet=posPoint->winOrderSet[suit];
    p2->first=NULL;
    np=p2;          /* Latest winning node */
    suit++;

    /* Rest of path must be created */
    while (suit<4) {
      p2=&winCards[winSetSize];
      AddWinSet();/*winSetSize++;*/
      np->nextWin=p2;
      p2->prevWin=np;
      p2->next=NULL;
      p2->winMask=posPoint->winMask[suit];
      p2->orderSet=posPoint->winOrderSet[suit];
      p2->first=NULL;
      p2->nextWin=NULL;
      np=p2;         /* Latest winning node */
      suit++;
    }

  /* All winning nodes for SOP have been traversed and new created */
    p=&nodeCards[nodeSetSize];
    AddNodeSet();
    np->first=p;
    *result=TRUE;
    return p;
  }  
}


struct posSearchType * SearchLenAndInsert(struct posSearchType
        * rootp, LONGLONG key, int insertNode, int *result) {
/* Search for node which matches with the suit length combination 
   given by parameter key. If no such node is found, NULL is 
  returned if parameter insertNode is FALSE, otherwise a new 
  node is inserted with suitLengths set to key, the pointer to
  this node is returned.
  The algorithm used is defined in Knuth "The art of computer
  programming", vol.3 "Sorting and searching", 6.2.2 Algorithm T,
  page 424. */
 
  struct posSearchType *np, *p;                
                
  np=rootp;
  while (1) {
    if (key==np->suitLengths) {
        *result=TRUE;        
      return np;
    }  
    else if (key < np->suitLengths) {
      if (np->left!=NULL) {
        np=np->left;
      } else if (insertNode) {
        p=&posSearch[lenSetSize];
        AddLenSet();/*lenSetSize++;*/
        np->left=p;
        p->posSearchPoint=NULL;
        p->suitLengths=key;
        p->left=NULL; p->right=NULL;
        *result=TRUE;
        return p;
      } else {
        *result=FALSE;
        return NULL;
      }        
    } else {      /* key > suitLengths */
      if (np->right!=NULL) {
        np=np->right;
      } else if (insertNode) {
        p=&posSearch[lenSetSize];
        AddLenSet();/*lenSetSize++;*/
        np->right=p;
        p->posSearchPoint=NULL;
        p->suitLengths=key;
        p->left=NULL; p->right=NULL;
        *result=TRUE;
        return p;
      } else {
        *result=FALSE;
        return NULL;
      }        
    } 
  }
}

/* New algo */

void BuildSOP(struct pos * posPoint, int tricks, int firstHand, int target,
  const int depth, int scoreFlag, int score) {
  int ss, res;
  holding_t w;
  struct nodeCardsType * cardsP;
  struct posSearchType * np;

  for (ss=0; ss<=3; ss++) {
    w=posPoint->stack[depth].winRanks[ss];
    if (w==0) {
      posPoint->winMask[ss]=0;
      posPoint->winOrderSet[ss]=0;
      posPoint->leastWin[ss]=0;
    } else {
      posPoint->computeWinData(ss,w);
    }
  }

  /* 07-04-22 */
  if (scoreFlag) {
    if (nodeTypeStore[0]==MAXNODE) {
      posPoint->ubound=tricks+1;
      posPoint->lbound=target-posPoint->tricksMAX;
    }
    else {
      posPoint->ubound=tricks+1-target+posPoint->tricksMAX;
      posPoint->lbound=0;
    }
  }
  else {
    if (nodeTypeStore[0]==MAXNODE) {
      posPoint->ubound=target-posPoint->tricksMAX-1;
      posPoint->lbound=0;
    }
    else {
      posPoint->ubound=tricks+1;
      posPoint->lbound=tricks+1-target+posPoint->tricksMAX+1;
    }
  }        

  posPoint->getSuitLengths(suitLengths);
  
  np=SearchLenAndInsert(rootnp[tricks][firstHand], suitLengths, TRUE, &res);
  
  cardsP=BuildPath(posPoint, np, &res);
  if (res) {
    cardsP->ubound=posPoint->ubound;
    cardsP->lbound=posPoint->lbound;
    if (((nodeTypeStore[firstHand]==MAXNODE)&&(scoreFlag))||
        ((nodeTypeStore[firstHand]==MINNODE)&&(!scoreFlag))) {
      cardsP->bestMoveSuit=bestMove[depth].suit;
      cardsP->bestMoveRank=bestMove[depth].rank;
    }
    else {
      cardsP->bestMoveSuit=0;
      cardsP->bestMoveRank=0;
    }
    posPoint->bestMoveSuit=bestMove[depth].suit;
    posPoint->bestMoveRank=bestMove[depth].rank;
    for (ss=0; ss<=3; ss++) 
      cardsP->leastWin[ss]=posPoint->leastWin[ss];
  }
                      
  #ifdef STAT
    c9[depth]++;
  #endif  

  #ifdef TTDEBUG
  if ((res) && (ttCollect) && (!suppressTTlog)) {
    fprintf(fp7, "cardsP=%d\n", (int)cardsP);
    fprintf(fp7, "nodeSetSize=%d\n", nodeSetSize);
    fprintf(fp7, "ubound=%d\n", cardsP->ubound);
    fprintf(fp7, "lbound=%d\n", cardsP->lbound);
    fprintf(fp7, "target=%d\n", target);
    fprintf(fp7, "first=%c nextFirst=%c\n",
      cardHand[posPoint->stack[depth].first], cardHand[posPoint->stack[depth-1].first]);
    fprintf(fp7, "bestMove:  suit=%c rank=%c\n", cardSuit[bestMove.suit],
      cardRank[bestMove[depth].rank]);
    fprintf(fp7, "\n");
    fprintf(fp7, "Last trick:\n");
    fprintf(fp7, "1st hand=%c\n", cardHand[posPoint-]);
    for (k=3; k>=0; k--) {
      mcurrent=movePly[depth+k+1].current;
      fprintf(fp7, "suit=%c  rank=%c\n",
        cardSuit[movePly[depth+k+1].move[mcurrent].suit],
        cardRank[movePly[depth+k+1].move[mcurrent].rank]);
    }
    fprintf(fp7, "\n");
    for (hh=0; hh<=3; hh++) {
      fprintf(fp7, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp7, "suit=%c", cardSuit[ss]);
        for (rr=14; rr>=2; rr--)
          if (posPoint->diagram.cards[hh][ss] & BitRank(rr))
        fprintf(fp7, " %c", cardRank[rr]);
        fprintf(fp7, "\n");
      }
      fprintf(fp7, "\n");
    }
    fprintf(fp7, "\n");

    for (hh=0; hh<=3; hh++) {
      fprintf(fp7, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp7, "suit=%c", cardSuit[ss]);
        for (rr=1; rr<=13; rr++)
          if (posPoint->relRankInSuit[hh][ss] & BitRank(15-rr))
            fprintf(fp7, " %c", cardRank[rr]);
        fprintf(fp7, "\n");
      }
      fprintf(fp7, "\n");
    }
    fprintf(fp7, "\n");
  }
  #endif  
}


int CheckDeal(struct moveType * cardp) {
  int h, s, k, found;
  holding_t temp[4][4];

  for (h=0; h<=3; h++) {
    for (s=0; s<=3; s++) {
      temp[h][s]=game.diagram.cards[h][s];
    }
  }

  /* Check that all ranks appear only once within the same suit. */
  for (s=0; s<=3; s++)
    for (k=2; k<=14; k++) {
      found=FALSE;
      for (h=0; h<=3; h++) {
        if ((temp[h][s] & BitRank(k))!=0) {
          if (found) {
            cardp->suit=s;
            cardp->rank=k;
            return 1;
          }  
          else
            found=TRUE;
        }    
      }
    }

  return 0;
}


/* New algo */
/*
void WinAdapt(struct pos * posPoint, const int depth, const struct nodeCardsType * cp,
   holding_t aggr[]) {
   posPoint->winAdapt(depth,cp,aggr);
   return;
   int ss, rr, k;

   for (ss=0; ss<=3; ss++) {
     posPoint->stack[depth].winRanks[ss]=0;
     if (cp->leastWin[ss]==0)
       continue;
     k=1;
     for (rr=14; rr>=2; rr--) {
       if ((aggr[ss] & BitRank(rr))!=0) {
         if (k<=cp->leastWin[ss]) {
           posPoint->stack[depth].winRanks[ss]|=BitRank(rr);
           k++;
         }
         else
           break;
       }
     }
   } 
   return;
}
*/

int NextMove(struct pos *posPoint, const int depth) {
  int mcurrent;
  holding_t lw;
  struct moveType currMove;
  
  mcurrent=movePly[depth].current;
  currMove=movePly[depth].move[mcurrent];

  if (lowestWin[depth][currMove.suit]==0) {
    lw=posPoint->stack[depth].winRanks[currMove.suit];
    if (lw!=0) {
      lw=smallestRankInSuit(lw);  /* LSB */
    } else {
      lw=BitRank(15);
    }
    if (BitRank(currMove.rank)<lw) {
        lowestWin[depth][currMove.suit]=lw;
      while (movePly[depth].current<=movePly[depth].last-1) {
        movePly[depth].current++;
        mcurrent=movePly[depth].current;
        if (BitRank(movePly[depth].move[mcurrent].rank) >=
            lowestWin[depth][movePly[depth].move[mcurrent].suit]) 
            return TRUE;
      }
      return FALSE;
    } else {
      while (movePly[depth].current<=movePly[depth].last-1) {
        movePly[depth].current++;
        mcurrent=movePly[depth].current;
        suit=movePly[depth].move[mcurrent].suit;
        if ((currMove.suit==suit) ||
          (BitRank(movePly[depth].move[mcurrent].rank) >=
            lowestWin[depth][suit]))
          return TRUE;
      }
      return FALSE;
    }
/*
    else if (movePly[depth].current<=movePly[depth].last-1) {
      movePly[depth].current++;
        return TRUE;
    }
    else
      return FALSE;
*/
  }
  else {
    while (movePly[depth].current<=movePly[depth].last-1) { 
      movePly[depth].current++;
      mcurrent=movePly[depth].current;
      if (BitRank(movePly[depth].move[mcurrent].rank) >=
          lowestWin[depth][movePly[depth].move[mcurrent].suit]) 
          return TRUE;
    }
    return FALSE;
  }  
}


int DumpInput(int errCode, struct deal dl, int target,
    int solutions, int mode) {

  FILE *fp;
  int i, j, k;

  fp=fopen("dump.txt", "w");
  if (fp==NULL)
    return -1;
  fprintf(fp, "Error code=%d\n", errCode);
  fprintf(fp, "\n");
  fprintf(fp, "Deal data:\n");
  fprintf(fp, "trump=%d\n", dl.trump);
  fprintf(fp, "first=%d\n", dl.trump);
  for (k=0; k<=2; k++) {
    fprintf(fp, "index=%d currentTrickSuit=%d currentTrickRank=%d\n",  
       k, dl.currentTrickSuit[k], dl.currentTrickRank[k]);
  }
  for (i=0; i<=3; i++) {
    for (j=0; j<=3; j++) { 
      fprintf(fp, "hand=%d suit=%d remaining=%d\n", 
        i, j, dl.remaining.cards[i][j]);
    }
  }
  fprintf(fp, "\n");
  fprintf(fp, "target=%d\n", target);
  fprintf(fp, "solutions=%d\n", solutions);
  fprintf(fp, "mode=%d\n", mode);
  fclose(fp);
  return 0;
}      


void Wipe(void) {
  int k;

  for (k=1; k<=wcount; k++) {
    if (pw[k]) {
      free(pw[k]);
    }
    pw[k]=NULL;
  }
  for (k=1; k<=ncount; k++) {
    if (pn[k]) {
      free(pn[k]);
    }
    pn[k]=NULL;
  }
  for (k=1; k<=lcount; k++) {
    if (pl[k]) {
      free(pl[k]);
    }
    pl[k]=NULL;
  }
        
  allocmem=summem/*(WINIT+1)*sizeof(struct winCardType)+
          (NINIT+1)*sizeof(struct nodeCardsType)+
          (LINIT+1)*sizeof(struct posSearchType)*/;

  return;
}


void AddWinSet(void) {
  if (clearTTflag) {
    windex++;
    winSetSize=windex;
    /*fp2=fopen("dyn.txt", "a");
    fprintf(fp2, "windex=%d\n", windex);
    fclose(fp2);*/
    winCards=&temp_win[windex];
  } else if (winSetSize>=winSetSizeLimit) {
    /* The memory chunk for the winCards structure will be exceeded. */
    if ((allocmem+wmem)>maxmem) {
    /* Already allocated memory plus needed allocation overshot maxmem */
      windex++;
      winSetSize=windex;
      clearTTflag=TRUE;
      winCards=&temp_win[windex];
    }
    else {
      wcount++; winSetSizeLimit=WSIZE; 
      pw[wcount] = (struct winCardType *)calloc(winSetSizeLimit+1, sizeof(struct winCardType));
      if (pw[wcount]==NULL) {
        clearTTflag=TRUE;
        windex++;
        winSetSize=windex;
        winCards=&temp_win[windex];
      }
      else {
        allocmem+=(winSetSizeLimit+1)*sizeof(struct winCardType);
        winSetSize=0;
        winCards=pw[wcount];
      }
    }
  }
  else
    winSetSize++;
  return;
}

void AddNodeSet(void) {
  if (nodeSetSize>=nodeSetSizeLimit) {
    /* The memory chunk for the nodeCards structure will be exceeded. */
    if ((allocmem+nmem)>maxmem) {
    /* Already allocated memory plus needed allocation overshot maxmem */  
      clearTTflag=TRUE;
    }
    else {
      ncount++; nodeSetSizeLimit=NSIZE; 
      pn[ncount] = (struct nodeCardsType *)calloc(nodeSetSizeLimit+1, sizeof(struct nodeCardsType));
      if (pn[ncount]==NULL) {
        clearTTflag=TRUE;
      }
      else {
        allocmem+=(nodeSetSizeLimit+1)*sizeof(struct nodeCardsType);
        nodeSetSize=0;
        nodeCards=pn[ncount];
      }
    }
  }
  else
    nodeSetSize++;
  return;
}

void AddLenSet(void) {
  if (lenSetSize>=lenSetSizeLimit) {
  /* The memory chunk for the nodeCards structure will be exceeded. */
    if ((allocmem+lmem)>maxmem) { 
      /* Already allocated memory plus needed allocation overshot maxmem */
      clearTTflag=TRUE;
    } else {
      lcount++; lenSetSizeLimit=LSIZE; 
      pl[lcount] = (struct posSearchType *)calloc(lenSetSizeLimit+1, sizeof(struct posSearchType));

      if (pl[lcount]==NULL) {
        clearTTflag=TRUE;
      } else {
        allocmem+=(lenSetSizeLimit+1)*sizeof(struct posSearchType);
        lenSetSize=0;
        posSearch=pl[lcount];
      }
    }
  } else {
    lenSetSize++;
  }
  return;
} 

#ifdef TTDEBUG

void ReceiveTTstore(struct pos *posPoint, struct nodeCardsType * cardsP, 
  int target, int depth) {
/* Stores current position information and TT position value in table
  ttStore with current entry lastTTStore. Also stores corresponding
  information in log rectt.txt. */
  tricksLeft=0;
  for (hh=0; hh<=3; hh++)
    for (ss=0; ss<=3; ss++)
      tricksLeft=tricksLeft+posPoint->length[hh][ss];
  tricksLeft=tricksLeft/4;
  ttStore[lastTTstore].tricksLeft=tricksLeft;
  ttStore[lastTTstore].cardsP=cardsP;
  ttStore[lastTTstore].first=posPoint->stack[depth].first;
  if ((handToPlay==posPoint->stack[depth].first)||
      (handToPlay==partner(posPoint->stack[depth].first))) {
    ttStore[lastTTstore].target=target-posPoint->tricksMAX;
    ttStore[lastTTstore].ubound=cardsP->ubound[handToPlay];
    ttStore[lastTTstore].lbound=cardsP->lbound[handToPlay];
  }
  else {
    ttStore[lastTTstore].target=tricksLeft-
      target+posPoint->tricksMAX+1;
  }
  for (hh=0; hh<=3; hh++)
    for (ss=0; ss<=3; ss++)
      ttStore[lastTTstore].suit[hh][ss]=
        posPoint->diagram.cards[hh][ss];
  fp11=fopen("rectt.txt", "a");
  if (lastTTstore<SEARCHSIZE) {
    fprintf(fp11, "lastTTstore=%d\n", lastTTstore);
    fprintf(fp11, "tricksMAX=%d\n", posPoint->tricksMAX);
    fprintf(fp11, "leftTricks=%d\n",
      ttStore[lastTTstore].tricksLeft);
    fprintf(fp11, "cardsP=%d\n",
      ttStore[lastTTstore].cardsP);
    fprintf(fp11, "ubound=%d\n",
      ttStore[lastTTstore].ubound);
    fprintf(fp11, "lbound=%d\n",
      ttStore[lastTTstore].lbound);
    fprintf(fp11, "first=%c\n",
      cardHand[ttStore[lastTTstore].first]);
    fprintf(fp11, "target=%d\n",
      ttStore[lastTTstore].target);
    fprintf(fp11, "\n");
    for (hh=0; hh<=3; hh++) {
      fprintf(fp11, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp11, "suit=%c", cardSuit[ss]);
        for (rr=14; rr>=2; rr--)
          if (ttStore[lastTTstore].suit[hh][ss]
            & BitRank(rr))
            fprintf(fp11, " %c", cardRank[rr]);
         fprintf(fp11, "\n");
      }
      fprintf(fp11, "\n");
    }
    for (hh=0; hh<=3; hh++) {
      fprintf(fp11, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp11, "suit=%c", cardSuit[ss]);
        for (rr=1; rr<=13; rr++)
          if (posPoint->relRankInSuit[hh][ss] & BitRank(15-rr))
            fprintf(fp11, " %c", cardRank[rr]);
        fprintf(fp11, "\n");
      }
      fprintf(fp11, "\n");
    }
  }
  fclose(fp11);
  lastTTstore++;
}
#endif
