#ifndef __DDS_INLINE_H__
#define __DDS_INLINE_H__

#include "ddsInterface.h"

/**
 * Repository for simple inline functions used by DDS
 */

inline int RelativeHand(int hand, int relative) {
  return (hand + relative)&3;
}

inline int partner(int hand) {
  return (hand^2); /* slightly faster */
}

inline int lho(int hand) {
  return RelativeHand(hand,1);
}

inline int rho(int hand) {
  return RelativeHand(hand,3);
}

inline holding_t BitRank(int rank) {
  /*
   * Trick calculation
   * Equivalent to 1<<(rank-2) for rank>=2, and 0 for rank<2.
   */
  return (1<<rank)>>2;
}


#endif
