#include "ddsInline.h"
/*
 * This header defines some inline functions which do global lookups for DDS
 * These functions are:
 *
 *  CountOnes(holding_t) - counts the cards in a holding
 *
 *  getHighestRank(holding_t) - returns the rank (2-14) of the highest
 *     card in the holding
 *
 *  getTopCards(holding_t h,int n) - returns a holding consisting of the top n cards
 *     in holding h   
 *
 * Your application must call initializeDDSLookup() to use these lookup tables
 */

struct topCardsType {
  holding_t topCards[14];
};

/* In dds code, counttable is local, but since I've already got a counttable in
   Deal, I reference that table instead */
extern "C" unsigned short int counttable[];

extern int highestRankLookup[8192];

extern struct topCardsType topCardsLookup[8192];

inline unsigned short int CountOnes(holding_t holding) {
  return counttable[holding];
}

inline int getHighestRank(holding_t holding) {
  return highestRankLookup[holding];
}

inline holding_t getTopCards(holding_t holding, int count) {
  return topCardsLookup[holding].topCards[count];
}

inline void initializeDDSLookup() {
  int n,rank;
  holding_t holding;
  highestRankLookup[0] = 0;
  for (n=0; n<14; n++) { topCardsLookup[0].topCards[n] = 0; }

  for (rank=2; rank<=14; rank++) {
    holding_t highestBitRank = BitRank(rank);
    for (holding=highestBitRank; holding<2*highestBitRank; holding++) {
      highestRankLookup[holding] = rank;
      holding_t rest = holding & (~highestBitRank);
      topCardsLookup[holding].topCards[0] = 0;
      for (n=1; n<14; n++) {
        topCardsLookup[holding].topCards[n] = 
	  highestBitRank | topCardsLookup[rest].topCards[n-1];
      }
    }
  }
}

