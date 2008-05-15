/*
 * Broken out from dds.h
 */
#ifndef __DDSINTERFACE_H__
#define __DDSINTERFACE_H__

#if 0
#define BENCH
#endif

#if defined(_WIN32)
#    define DLLEXPORT __declspec(dllexport)
#    define STDCALL __stdcall
#else
#    define DLLEXPORT
#    define STDCALL
#    define INT8 char
#endif

#ifdef __cplusplus
#    define EXTERN_C extern "C"
#else
#    define EXTERN_C
#endif


struct deal {
  int trump;
  int first;
  int currentTrickSuit[3];
  int currentTrickRank[3];
  unsigned int remainCards[4][4];
};

struct futureTricks {
  int nodes;
#ifdef BENCH
  int totalNodes;
#endif
  int cards;
  int suit[13];
  int rank[13];
  int equals[13];
  int score[13];
};

#include <string.h>

EXTERN_C DLLEXPORT int STDCALL SolveBoard(struct deal dl, 
  int target, int solutions, int mode, struct futureTricks *futp);

EXTERN_C DLLEXPORT void STDCALL DDSInitStart();

#endif

