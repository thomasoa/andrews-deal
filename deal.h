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

#ifndef __DEAL_H__
#define __DEAL_H__

#include <stdlib.h>
#include "deck.h"
#include "tcl.h"

typedef struct _HandH {
	int suit[4];
} RawHand;

typedef struct _DealH {
	RawHand hand[4];
} RawDeal,RawDist;

extern RawDeal globalDeal;
extern RawDist globalDist;

#define HoldingHas(holding,rank) (holding & (1<<(12-(rank))))
#define HANDHASCARD(hand,card) HoldingHas(hand.suit[SUIT(card)],RANK(card))
#define DEALHANDHASCARD(deal,hnum,card) HANDHASCARD(deal.hand[hum],card)

#define HAS(hand,suit,rank) DEALHANDHASCARD(globalDeal,hand,suit,rank)
#define HASCARD(hand,card) HAS(hand,SUIT(card),RANK(card))
#define HASCARD2(hptr,card) HoldingHas(hptr[SUIT(card)],RANK(card))
#define count_suit(hnum,suitnum) globalDist.hand[hnum].suit[suitnum]

extern char suits[];
extern char cards[];
extern const unsigned short int counttable[];

/* 
   These exist for backward compatibility, and are
   initialized with pointers into globalDeal and globalDist.
 */
extern int *distributions[];
extern int *holdings[];



int reset_deck();
int deal_deck();
int read_deal();

/* char *format_deal(); */
char *format_deal_compact();
char *format_deal_verbose();


int start_deal();
void finish_deal();
void deal_hand PROTO((int /*hand*/));
/*
 * Lookup tables which key on characters to return
 * the associated hand, suit, and card numbers
 */
extern int hand_name_table[256];
extern int suit_name_table[256];
extern int card_name_table[256];

void init_name_tables();

int Dist_Init PROTO((Tcl_Interp *));
int Vector_Init PROTO((Tcl_Interp *));
int HandCmd_Init PROTO((Tcl_Interp *));
int DealControl_Init PROTO((Tcl_Interp *));
int DDS_Init PROTO((Tcl_Interp *));

int count_controls PROTO((int /* holding */, void */* dummy */));
int count_hcp PROTO((int /* holding */, void */* dummy */));
int count_losers PROTO((int /* holding */, void * /* dummy */));

int put_card PROTO((int, int));
int put_hand PROTO((int, char *));
int put_holdings PROTO((int, int*));
int put_holding PROTO((int hand, int suit, int card));

extern int count_deals;
extern int verbose;

int card_num PROTO((char *));
void rotate_deal PROTO((int));
void get_stacked_cards PROTO((int,int*));

struct deck_stacker {
    int card[52];  /* What card is in the position */
    int whom[52];  /* To whom is this card slated to go? */
    int where[52]; /* Is card placed, and where? */
    int handcount[4]; /* How man cards placed to hand so far */
    int dealt;     /* Placed cards */
};

struct deck {
    int card[52];  /* The ith card in the deck */
    int whom[52];  /* To whom does the card go? */
    int where[52]; /* Where in the deal is the card? whom is the inverse of card */
    int dealt;
    int handcount[4];
    int finished[4];
};

int to_whom PROTO ((int card));
extern struct deck_stacker stacker;
extern struct deck complete_deal;


#define FINISH(hand) if (complete_deal.finished[hand]==0) {deal_hand(hand);}

#define IDEAL_VERSION "0.9.0"

#endif

void __srandom PROTO((unsigned));
long __random();

#define srandom __srandom
#define random __random
#define RANDOM_MAX LONG_MAX
