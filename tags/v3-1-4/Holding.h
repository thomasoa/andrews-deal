#ifndef __DDS_HOLDING_H__
#define __DDS_HOLDING_H__
#include "ddsInterface.h"

/*
 * This is just a utility class so you can say:
 *      cout << Holding(h) << endl;
 * when h is of type holding_t.
 */
struct Holding {
  holding_t _h;
  inline Holding(holding_t h) : _h(h) { }
};


inline ostream& operator <<(ostream &out,const Holding &holding) {
  static const char *cards="AKQJT98765432";
  int index=0;
  holding_t h = holding._h;

  if (h) {
    for (holding_t card=1<<12; card; card >>= 1, index++) {
      if (h & card) {
        out << cards[index];
      }
    }
  } else {
      out << "(void)";
  }
  return out;
}
#endif
