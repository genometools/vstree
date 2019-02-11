#ifndef MCLDEF_H
#define MCLDEF_H

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "multidef.h"
#include "alphadef.h"
#include "match.h"

#define MATCHCLUSTERNAME "matchcluster"

typedef struct
{
  Uint matchnum0, 
       matchnum1;
  union
  {
    Uint gapsize;
    double overlap;
    struct
    {
      Uint minlen,
           edist;
    } uedata;
  } edgedata;
} Matchedge;

DECLAREARRAYSTRUCT(Matchedge);

typedef enum
{
  SimilarityMCL,
  GapMCL,
  OverlapMCL,
  UndefMCL
} Matchclustertype;

typedef struct
{
  BOOL defined;
  Uint errorrate,
       maxgapsize,
       minpercentoverlap;
  Matchclustertype matchclustertype;
  char *outprefix,
       *matchfile;
} Matchclustercallinfo;

typedef struct
{
  Multiseq *virtualmultiseq,
           *querymultiseq;
  Alphabet *virtualalpha;
  ArrayStoreMatch *matchtab;
  ArrayMatchedge matchedgetab;
  FILE *matchfileoutptr;
  char *mfargs,
       clusterfilenameprefix[PATH_MAX+1],
       clusterfilename[PATH_MAX+2*20+3+5+1];
  Sint (*mcllinkinfo)(void *,Matchedge *);
} Mclinfo;

#endif
