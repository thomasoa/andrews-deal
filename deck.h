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

#ifndef __DECK_H__
#define __DECK_H__
#define SPADES 0
#define HEARTS 1
#define DIAMONDS 2
#define CLUBS 3

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

#define NOSUIT 4
#define NOSEAT 4
#define NORANK 15
#define NOCARD -1

#define ACE 0
#define KING 1
#define QUEEN 2
#define JACK 3
#define TEN 4
#define NINE 5
#define EIGHT 6
#define SEVEN 7
#define SIX 8
#define FIVE 9
#define FOUR 10
#define THREE 11
#define TWO 12

#define CARDSperSUIT 13
#define CARD(denom,suit) ((int)(denom)*4+(int)(suit))
#define RANK(card) ((card)/4)
#define SUIT(card) ((card)%4)

extern char *suitname[];
extern char *handname[];


#if defined(__STDC__)
#define PROTO(params) params
#else
#define PROTO(params) ()
#endif

#endif

#ifdef HPUX
#define random rand
#define srandom srand
#endif

#ifdef USE_RAND48
double drand48();
void srand48();
#endif
