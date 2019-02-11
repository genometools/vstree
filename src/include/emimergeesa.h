#ifndef SUFLCPBUF_H
#define SUFLCPBUF_H

#include <stdio.h>
#include "types.h"
#include "esastream.h"
#include "trieins-def.h"
#include "encseq-def.h"

#define SIZEOFMERGERESULTBUFFER BUFSIZ

typedef struct
{
  Uint idx,       // index of genome in list of all genomes
       startpos;  // in the range [0..totallength single index]
} Indexedsuffix;

typedef struct
{
  Uint nextaccessidx,  // in the range [0..SIZEOFMERGERESULTBUFFER]
       nextstoreidx,   // in the range [0..SIZEOFMERGERESULTBUFFER]
       lcptabstore[SIZEOFMERGERESULTBUFFER];
  Indexedsuffix suftabstore[SIZEOFMERGERESULTBUFFER];
  BOOL lastpage;
} Suflcpbuffer;

typedef struct
{
  Uint64 ident;                // can be arbitrary large
  Uint numofentries,           // in the range [0..numofindexes-1]
       numofindexes,           // number of indexes
       *nextpostable;          // in the range [0..totallength single index]
  Suflcpbuffer buf;
  Encodedsequence *encseqtable;
  Trierep trierep;
  Esastream *esastreamtable;
  Alphabet alpha;
} Emissionmergedesa;

#endif
