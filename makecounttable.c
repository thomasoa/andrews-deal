/*
 * makecounttable.c - This is a standalone utility which, when run,
 *   creates the file counttable.c.  counttable.c is just a lookup
 *   table method to count the bits in a holding.
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
#include <stdio.h>

/*
 * This is a speed consideration issue.
 * I've added "bit representations" of the
 * suits holding for each hand. AQT7, for example,
 * is represented as:
 *
 *     1 0 1 0 1 0 0 1 0 0 0 0 0
 *
 * the "counttable" is just a lookup table which holds
 * the number of bits set, that is, the number of cards 
 * held in the suit.  This table has 2^13=8192 entries, so instead
 * of shipping a 33k file with the source kit, I ship
 * this smaller source file which creates the source file on the
 * fly.
 * 
 * This sort of table could also be used to compute losers in
 * a suit or HCP in a suit.  In the case of HCP, just examine
 * the table in "deal.c", in the hcp_count function.
 *
 */
int counttable[8192]; /* Fast lookup for suit lengths */
void make_counttable()
{
  int i,j;
  for (i=0; i<8192; i++) {
    counttable[i]=0;
    for (j=0; j<13; j++) {
      if (i & (1<<j)) {counttable[i]++;}
    }
  }   
}

int main() {
  int i;
  make_counttable();
  printf("unsigned short int counttable[]={");
  for (i=0; i<8192; i++) {
    if (0==i%16) {
      printf("\n");
    }
    printf("%3d",counttable[i]);
    if (i!=8191) {
      printf(",");
    }
  }
  printf("\n};\n");
  return 0;
}
