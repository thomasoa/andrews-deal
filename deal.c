/*
 * deal.c - Mostly Tcl-free code for dealing.  This is the "heart"
 * of Deal.
 *
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

#include <string.h>
#include <tcl.h>

#include <ctype.h>
#include <stdio.h>

#include "deal.h"

char *suitname[]={"spades","hearts","diamonds","clubs"};
char *handname[]={"north","east","south","west"};

RawDeal globalDeal;
RawDist globalDist;

struct deck_stacker stacker;

struct deck complete_deal;

/* These are here for backward compatibility reasons */
int *distributions[]={
  globalDist.hand[0].suit,
  globalDist.hand[1].suit,
  globalDist.hand[2].suit,
  globalDist.hand[3].suit
};

int *holdings[]={
  globalDeal.hand[0].suit,
  globalDeal.hand[1].suit,
  globalDeal.hand[2].suit,
  globalDeal.hand[3].suit
};

char suits[]="SHCD";
char cards[]="AKQJT98765432";

int verbose=0;

#define fast_mod(n,d) ((n)%(d))

int reset_deck() {
  int i;
  stacker.dealt=0;
  for (i=0; i<52; i++) {
    stacker.card[i] = NOCARD;
    stacker.whom[i]=NOSEAT;
    stacker.where[i]=-1;
  }
  for (i=0; i<4; i++) {
    stacker.handcount[i]=0;
  }
#ifdef DEBUG
  fprintf(stderr,"Reset deck\n");
#endif
  return TCL_OK;
}

void get_distribution_hand(hand) 
     int hand;
{
  int suit;
  for (suit=0; suit<4; suit++) {
    globalDist.hand[hand].suit[suit]=(int)
      counttable[globalDeal.hand[hand].suit[suit]];
  }
}

int count_deals=0;

void old_reset_deal(dealp) 
     struct deck *dealp;
{
  int card, hand;

  dealp->dealt=0;

  for (card=0; card<52; card++) {
    dealp->card[card]=card;
    dealp->whom[card]=NOSEAT;
    dealp->where[card]=card;
  }
 
  for (hand=0; hand<4; hand++) {
    dealp->handcount[hand]=0;
    dealp->finished[hand]=0;
  }
}

void reset_deal(dealp)
    struct deck *dealp;
{
  int hand,suit;
  static struct deck initializeOnce;
  static int needsInit=1;

  if (needsInit) {
    old_reset_deal(&initializeOnce);
    needsInit=0;
  }

  memcpy(dealp,&initializeOnce,sizeof(struct deck));

  for (hand=0; hand<4; hand++) {
    for (suit=0; suit<4; suit++) {
      globalDeal.hand[hand].suit[suit]=0;
    }
  }
}


#ifdef DEBUG
static void assert_deck(dealp)
     struct deck *dealp;
{
  int loc,card;
  for (loc=0; loc<52; loc++) {
    card=dealp->card[loc];
    if (dealp->where[card]!=loc) {
#ifdef __CENTERLINE__
      centerline_stop();
#else
      abort();
#endif
    }
  }
}
#endif
  
/* This amounts to a swap command, with lots of record keeping added
   to keep "where" and "card" as inverses */
void deal_put(dealp,card,whom)
     struct deck *dealp;
     int card,whom;
{
  int where=dealp->where[card];  /* Where the card is, currently */
  int loc=dealp->dealt;  /* Where it will be placed */
  int othercard=dealp->card[loc]; /* The card currently in this place */

  /* Swapping "loc" with "where" in the array */
  dealp->card[where]=othercard;
  dealp->card[loc]=card;

  dealp->where[othercard]=where;
  dealp->where[card]=loc;

  dealp->handcount[whom]++;
  dealp->whom[loc]=whom;  /* Give the card to whom */

  globalDeal.hand[whom].suit[SUIT(card)] |= (1 << (12- (RANK(card))));
  dealp->dealt++;
#ifdef DEBUG
  assert_deck(dealp);
#endif
}

void place_fixed_cards(dealp)
     struct deck *dealp;
{
  int i=0;
  while (i<stacker.dealt) {
    deal_put(dealp,stacker.card[i],stacker.whom[i]);
    i++;
  }
}

static int deal_random(dealt)
     int dealt;
{
#if USE_RAND48
   return dealt+(int) (drand48() *(double)(52-dealt));
#else
  return dealt+(int) (fast_mod((unsigned) random() , (52-dealt)));
#endif
}

void deal_hand(hand)
     int hand;
{
  int deck_dealt;
  deck_dealt=complete_deal.dealt;

  if (deck_dealt-complete_deal.handcount[hand]==39) {
    /*
     * If this is the last hand to be dealt, then simply
     * place undealt cards in this hand.  Cuts down on calls
     * to RNG.
     */
    while (complete_deal.handcount[hand]<13) {
      int card=complete_deal.card[deck_dealt++];
      deal_put(&complete_deal,card,hand);
    }
  } else {
    while (complete_deal.handcount[hand]<13) {
      int where=deal_random(complete_deal.dealt);
      int card=complete_deal.card[where];
#ifdef DEBUG
      if (complete_deal.where[card]<deck_dealt) abort();
#endif
      deal_put(&complete_deal,card,hand);
    }
  }
  complete_deal.finished[hand]=1;
  get_distribution_hand(hand);
}

int start_deal()
{
  reset_deal(&complete_deal);
  place_fixed_cards(&complete_deal);
  count_deals++;
  if (verbose && 0== (count_deals % 1000)) {
    fprintf(stderr,"Deal #%d\n",count_deals);
  }
  return TCL_OK;
}

void finish_deal() {
  int hand;
  for (hand=0; hand<4; hand++) {
    deal_hand(hand);
  }
}

int to_whom(card) 
     int card;
{
  int where;
  if (complete_deal.dealt!=52) {
    finish_deal();
  }
  where=complete_deal.where[card];
  return complete_deal.whom[where];
}
/*
 * Might want this at some later date, but for now
 *
int deal_deck ()
{
    int card,hand,count;
    
    start_deal();

    finish_deal();

    return TCL_OK;
}
 *
 *
 */

int read_deal()
{
  int hand=0, suit=SPADES,cardsHand=0,cardsDeck=0;
  int card;
  int c;

  reset_deck();

  while ((c=getchar()) != EOF) {
    switch (c) {
    case ' ':
      suit++;
      break;
    case '|':
      if (cardsHand!=13 || suit!=CLUBS || hand>=3) {
	return TCL_ERROR;
      }
      hand++; suit=SPADES; cardsHand=0;
      break;
    case '\n':
      if (cardsHand!=13 || cardsDeck!=52) {
	return TCL_ERROR;
      }
      start_deal();
      return TCL_OK;
    default:
      if (cardsHand>=13 || NORANK==(card=card_name_table[c])) {
	return TCL_ERROR;
      }
      cardsHand++; cardsDeck++;
      put_card(hand,CARD(card,suit));
      break;
    }
  }
  if (cardsDeck!=0) { return TCL_ERROR;}
  return TCL_RETURN;
}

char *format_deal_compact ()
{
  char s[4][26];
  char *result=(char *)Tcl_Alloc(52*2);
  char *s1[4];
  int hand,suit,denom;
  for (hand=0; hand<4; hand++) s1[hand]=s[hand];
  for (suit=0; suit<4; suit++) {
    for (denom=0; denom <13; denom++) {
      int card=CARD(denom,suit);
      int where=complete_deal.where[card];
      if (!(denom)) {
	for (hand=0; hand<4; hand++) {
	  *(s1[hand]++)=' ';
	}
      }
      *(s1[complete_deal.whom[where]]++)=cards[denom];
    }
  }
  for (hand=0; hand<4; hand++) 
    {
      *(s1[hand])=0;
    }
  sprintf(result,"%s|%s|%s|%s\n",1+s[0],1+s[1],1+s[2],1+s[3]);
  return result;
}

char *format_deal_verbose()
{
  static char suit_chars[]="SHDC";

  char a[4][4][14];
  char *p[4][4];
  char *result,*rp;
  int suit,hand,card,where;

  for(hand=0; hand<4; hand++)
    for(suit=0; suit<4; suit++)
      p[hand][suit]=&a[hand][suit][0];

  for (suit=0; suit<4; suit++)
    for (card=0; card <52; card += 4) {
      where=complete_deal.where[card+suit];
      *(p[complete_deal.whom[where]][suit]++)=cards[card>>2];
    }

  for(hand=0; hand<4; hand++)
    for(suit=0; suit<4; suit++)
      if (p[hand][suit] == &a[hand][suit][0]) {
	strcpy(p[hand][suit],"---");
      } else {
	*(p[hand][suit])=0;
      }

  rp=result=(char *)Tcl_Alloc(2048);

  for (suit=0; suit<4; suit++)  {
    sprintf(rp,"          %c : %s\n",suit_chars[suit],a[NORTH][suit]);
    rp=rp+strlen(rp);
  }

  for (suit=0; suit<4; suit++) {
    sprintf(rp," %c : %-13s  %c : %-13s\n",suit_chars[suit],a[WEST][suit],
	    suit_chars[suit],a[EAST][suit]);
    rp=rp+strlen(rp);
  }

  for (suit=0; suit<4; suit++) {
    sprintf(rp,"          %c : %s\n",suit_chars[suit],a[SOUTH][suit]);
    rp=rp+strlen(rp);
  }

  sprintf(rp,"---------------------------\n");

  return result;

}
  

int hand_name_table[256];
int suit_name_table[256];
int card_name_table[256];

void init_name_tables()
{
  int i;
  for (i=0; i<256; i++) {
    hand_name_table[i]=NOSEAT;
    suit_name_table[i]=NOSUIT;
    card_name_table[i]=NORANK;
  }
  hand_name_table['N']=hand_name_table['n']=NORTH;
  hand_name_table['E']=hand_name_table['e']=EAST;
  hand_name_table['S']=hand_name_table['s']=SOUTH;
  hand_name_table['W']=hand_name_table['w']=WEST;

  suit_name_table['S']=suit_name_table['s']=SPADES;
  suit_name_table['H']=suit_name_table['h']=HEARTS;
  suit_name_table['D']=suit_name_table['d']=DIAMONDS;
  suit_name_table['C']=suit_name_table['c']=CLUBS;

  for (i=0; i<13; i++) {
    card_name_table[(int)cards[i]]=i;
    card_name_table[(int)tolower(cards[i])]=i;
  }
}


int count_controls(holding,dummy) 
     int holding; void *dummy;
{
  /*
   *
   * Since holdings contains a 13-bit 
   * representation of the holdings in the suit,
   * controls can be computed by taking holdings[hand][suit] >> 11
   *
   */
  return ((8191&holding)>>11);
}

int count_hcp(h,dummy) 
     int h;
     void *dummy;
{
  static int hcptable[]={
    0 /*no honors*/ ,1 /*J*/  ,2 /*Q*/  ,3 /*QJ*/,
    3 /*K*/ ,4 /*KJ*/ ,5 /*KQ*/ ,6 /*KQJ*/,
    4 /*A*/ ,5 /*AJ*/ ,6 /*AQ*/ ,7 /*AQJ*/,
    7 /*AK*/,8 /*AKJ*/,9 /*AKQ*/,10 /*AKQJ*/,
  };

  return hcptable[(8191&h)>>9];

}

int count_losers(holding,dummy)
     int holding;
     void * dummy;  /* For additive function implementation */
{ /* Really counts half-losers */
  int base=counttable[holding&8191];
  int losers=0;

  if (base==0) { return 0; }

  if (!HoldingHas(holding,ACE)) {
    losers +=2;
  }

  if (base>=2) {
    if (!HoldingHas(holding,KING)) {
      losers +=2;
    }
  }

  if (base>=3) {
    if (!HoldingHas(holding,QUEEN)) {
      losers +=2;
    } else {
      if (losers==4) {
	if(!HoldingHas(holding,JACK) && !HoldingHas(holding,TEN)) {
	  losers++;
	}
      }
    }
  }
  return losers;
}




int put_card(hand,card) int hand,card;
{
  int loc=stacker.dealt;
  if (stacker.handcount[hand]==13) {
    return TCL_ERROR;
  }
  if (stacker.where[card]!=-1) {
    /* Don't complain if stacking card to same hand */
    if (stacker.whom[stacker.where[card]]==hand) {
      return TCL_OK;
    } else {
      return TCL_ERROR;
    }
  } else {
    stacker.where[card]=loc;
  }
  stacker.handcount[hand]++;
  stacker.whom[loc]=hand;
  stacker.card[loc]=card;
  stacker.dealt++;
  return TCL_OK;
}

void get_stacked_cards(int hand,int holdings[])
{
  int index, suit,denom, card;

  for (suit=0; suit<4; suit++) { holdings[suit]=0; }

  for (index=0; index<stacker.dealt; index++) {
    if (stacker.whom[index]==hand) {
      card=stacker.card[index];
      suit=SUIT(card);
      denom=RANK(card);
      holdings[suit] |= (1<<(12-denom));
    }
  }
}

int put_holding(int hand,int suit, int holding)
{
    int card;
    for (card=12; card>=0; card--) {
      if (holding&1) { 
	if (TCL_ERROR==put_card(hand,CARD(card,suit))) {
            return TCL_ERROR;
        }
      }
      holding >>= 1;
    }
    return TCL_OK;
}

int put_holdings(int hand,int *holdings)
{
  int suit;
  for (suit=0; suit<4; suit++) {
    if (TCL_ERROR==put_holding(hand,suit,holdings[suit])) {
       return TCL_ERROR;
    }
    /* fprintf(stderr,"Adding holding %d to hand %d\n",h,hand); */
  }
  return TCL_OK;
}

int put_hand(int hand,char *hstring)
{
  int suit=SPADES;
  char s[4][13],*sptr;
  sscanf(hstring,"%s %s %s %s",s[SPADES],s[HEARTS],s[DIAMONDS],s[CLUBS]);
  for(suit=SPADES; suit<=CLUBS; suit++) {
    if (*s[suit]=='-') { continue; }
    sptr=s[suit];
    while (*sptr) {
      if (TCL_ERROR==put_card(hand,CARD(card_name_table[(int)*sptr],suit))) {
	return TCL_ERROR;
      }
      sptr++;
    }
  }
  return TCL_OK;
}

int card_num (string)
     char *string;
{
  return CARD(card_name_table[(int)string[0]],suit_name_table[(int)string[1]]);
}

void rotate_deal(rotation)
     int rotation;
{
  int loc,card;
  int whom[52];
  rotation = 4 - (rotation & 3);  /* same as %4, only always positive */
  finish_deal();
  for(card=0; card<52; card++) {
    loc=complete_deal.where[card];
    whom[card]=(complete_deal.whom[loc]+rotation) % 4;
  } 
  reset_deal(&complete_deal);
  for(card=0; card<52; card++) {
    deal_put(&complete_deal,card,whom[card]);
  }
  finish_deal();
}
