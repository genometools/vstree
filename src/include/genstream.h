#ifndef GENSTREAM_H
#define GENSTREAM_H
#include <stdio.h>
#include <zlib.h>

typedef struct
{
  unsigned char isgzippedstream;
  union
  {
    FILE *fopenstream;
    gzFile gzippedstream;
  } stream;
} Genericstream;

#endif
