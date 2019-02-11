#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include "esastream.h"
#include "fhandledef.h"
#include "spacedef.h"
#include "multidef.h"
#include "genfile.h"
#include "debugdef.h"

#include "filehandle.pr"
#include "alphabet.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "compfilenm.pr"

#define INITBufferedfile(STREAM,SUFFIX)\
        tmpfilename = COMPOSEFILENAME(indexname,SUFFIX);\
        (STREAM)->fp = CREATEFILEHANDLE(tmpfilename,READMODE);\
        if((STREAM)->fp == NULL)\
        {\
          return (Sint) -1;\
        }\
        (STREAM)->nextread = 0;\
        (STREAM)->nextfree = 0;\
        FREESPACE(tmpfilename)

Sint initUcharBufferedfile(UcharBufferedfile *stream,
                           const char *indexname,
                           const char *suffix)
{
  char *tmpfilename;

  INITBufferedfile(stream,suffix);
  return 0;
}

Sint initesastream(Esastream *esastream,
                   const char *indexname,
                   Uint demand)
{
  char *tmpfilename;
  Uint maxbranchdepth, specialindex;
  ArrayPairUint largelcpvalues;
  BOOL rcmindex, specialsymbols;

  if(parseprojectfile(&esastream->multiseq,
                      &esastream->longest,
                      &esastream->prefixlength,
                      &largelcpvalues,
                      &maxbranchdepth,
                      &rcmindex,
                      &specialindex,
                      indexname) != 0)
  {
    return (Sint) -1;
  }
  if(mapalphabetifyoucan(&specialsymbols,&esastream->alpha,indexname) != 0)
  {
    return (Sint) -2;
  }
  esastream->tistabstream.fp = NULL;
  esastream->bwttabstream.fp = NULL;
  esastream->suftabstream.fp = NULL;
  esastream->llvtabstream.fp = NULL;
  esastream->lcptabstream.fp = NULL;
  if(demand & TISTABSTREAM)
  {
    INITBufferedfile(&esastream->tistabstream,"tis");
  }
  if(demand & BWTTABSTREAM)
  {
    INITBufferedfile(&esastream->bwttabstream,"bwt");
  }
  if(demand & SUFTABSTREAM)
  {
    INITBufferedfile(&esastream->suftabstream,"suf");
    if(!esastream->longest.defined)
    {
      ERROR0("esastream->longest is not defined");
      return (Sint) -6;
    }
  }
  if(demand & LCPTABSTREAM)
  {
    INITBufferedfile(&esastream->lcptabstream,"lcp");
    if(fseek(esastream->lcptabstream.fp,(long) sizeof(Uchar),SEEK_SET) != 0)
    {
      ERROR1("fseek(esastream) failed: %s",strerror(errno));
      return (Sint) -8;
    }
    if(largelcpvalues.nextfreePairUint > 0)
    {
      INITBufferedfile(&esastream->llvtabstream,"llv");
    } 
  }
  return 0;
}

Sint closeesastream(Esastream *esastream)
{
  if(esastream->tistabstream.fp != NULL &&
     DELETEFILEHANDLE(esastream->tistabstream.fp) != 0)
  {
    return (Sint) -1;
  }
  if(esastream->bwttabstream.fp != NULL &&
     DELETEFILEHANDLE(esastream->bwttabstream.fp) != 0)
  {
    return (Sint) -2;
  }
  if(esastream->suftabstream.fp != NULL && 
     DELETEFILEHANDLE(esastream->suftabstream.fp) != 0)
  {
    return (Sint) -3;
  }
  if(esastream->lcptabstream.fp != NULL && 
     DELETEFILEHANDLE(esastream->lcptabstream.fp) != 0)
  {
    return (Sint) -4;
  }
  if(esastream->llvtabstream.fp != NULL && 
     DELETEFILEHANDLE(esastream->llvtabstream.fp) != 0)
  {
    return (Sint) -5;
  }
  freemultiseq(&esastream->multiseq);
  return 0;
}
