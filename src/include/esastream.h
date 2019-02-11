#ifndef ESASTREAM_H
#define ESASTREAM_H

#include <stdio.h>
#include "types.h"
#include "alphadef.h"
#include "multidef.h"

#define TISTABSTREAM UintConst(1)
#define BWTTABSTREAM (UintConst(1) << 1)
#define SUFTABSTREAM (UintConst(1) << 2)
#define LCPTABSTREAM (UintConst(1) << 3)
#define FILEBUFFERSIZE 65536

#define ERROREOF(S)\
        ERROR3("file %s, line %lu: unexpected end of file when reading %s ",\
                __FILE__,(Showuint) __LINE__,S)

#define DECLAREBufferedfiletype(TYPE)\
        typedef struct\
        {\
          Uint nextfree,\
               nextread;\
          TYPE bufspace[FILEBUFFERSIZE];\
          FILE *fp;\
        } TYPE ## Bufferedfile

DECLAREBufferedfiletype(Uint);

DECLAREBufferedfiletype(Uchar);

DECLAREBufferedfiletype(PairUint);

typedef struct
{
  UintBufferedfile suftabstream;
  UcharBufferedfile bwttabstream,
                    lcptabstream,
                    tistabstream;
  PairUintBufferedfile llvtabstream;
  DefinedUint longest;
  Uint prefixlength;
  Multiseq multiseq;
  Alphabet alpha;
} Esastream;

#define DECLAREREADFUNCTION(TYPE)\
        static Sint readnext ## TYPE(TYPE *val,TYPE ## Bufferedfile *buf)\
        {\
          if(buf->nextread >= buf->nextfree)\
          {\
            buf->nextfree = (Uint) fread(buf->bufspace,\
                                         sizeof(TYPE),\
                                         (size_t) FILEBUFFERSIZE,\
                                         buf->fp);\
            if(ferror(buf->fp))\
            {\
              ERROR1("error when trying to read next %s",#TYPE);\
              return (Sint) -2;\
            }\
            buf->nextread = 0;\
            if(buf->nextfree == 0)\
            {\
              return (Sint) 0;\
            }\
          }\
          *val = buf->bufspace[buf->nextread++];\
          return 1;\
        }

#endif
