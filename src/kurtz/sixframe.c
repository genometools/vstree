
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "errordef.h"
#include "alphadef.h"
#include "arraydef.h"
#include "chardef.h"
#include "multidef.h"
#include "genfile.h"
#include "debugdef.h"
#include "scoredef.h"
#include "codondef.h"
#include "alignment.h"

#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "codon.pr"
#include "showalign.pr"

//}

#ifdef DEBUG
#define CHECKTRANSLATIONSPACE(EXTRA)\
        if(sfinfo->proteinseq.nextfreeUchar + (EXTRA) - 1 >=\
           sfinfo->proteinseq.allocatedUchar)\
        {\
          fprintf(stderr,"sequence %lu: array for storing translated "\
                 "protein sequence is too small: "\
                 "frame %ld needs extra %lu bytes but has only %lu left\n",\
                 (Showuint) seqnum,\
                 (Showsint) frame,\
                 (Showuint) (EXTRA),\
                 (Showuint) (sfinfo->proteinseq.allocatedUchar -\
                             sfinfo->proteinseq.nextfreeUchar));\
          exit(EXIT_FAILURE);\
        }

#define CHECKMARKPOSSPACE\
        if(sfinfo->markpostab.nextfreeUint >= \
           sfinfo->markpostab.allocatedUint)\
        {\
          fprintf(stderr,"not enough space in markpos\n");\
          exit(EXIT_FAILURE);\
        }
#else
#define CHECKTRANSLATIONSPACE(EXTRA) /* Nothing */
#define CHECKMARKPOSSPACE            /* Nothing */
#endif

typedef struct
{
  ArrayUchar proteinseq;
  ArrayPosition markpostab;
  Uint numofsequences;
  BOOL withdescription;
  ArrayUchar descspace;
  Uint *startdesc,
       transnum;
  Multiseq *dnamultiseq;
} Sixframeinfo;

Sint singlesixframetranslateDNA(void *info,Uint seqnum,Uchar *start,Uint len)
{
  Sixframeinfo *sfinfo = (Sixframeinfo *) info;
  Sint frame, plen;

  if(sfinfo->withdescription && 
     sfinfo->dnamultiseq->descspace.spaceUchar != NULL)
  {
    Uchar *descstart;
    Uint desclength;

    desclength = DESCRIPTIONLENGTH(sfinfo->dnamultiseq,seqnum);
    descstart = DESCRIPTIONPTR(sfinfo->dnamultiseq,seqnum);
    memcpy(sfinfo->descspace.spaceUchar + 
           sfinfo->descspace.nextfreeUchar,
           descstart,(size_t) desclength);
    sfinfo->startdesc[seqnum * MAXFRAMES] 
      = sfinfo->descspace.nextfreeUchar;
    sfinfo->descspace.nextfreeUchar += desclength;
    for(frame = SintConst(1); frame < (Sint) MAXFRAMES; frame++)
    {
      sfinfo->startdesc[seqnum * MAXFRAMES + frame] 
        = sfinfo->descspace.nextfreeUchar;
      sfinfo->descspace.spaceUchar[sfinfo->descspace.nextfreeUchar++] 
        = (Uchar) '\n';
    }
  }
  for(frame=0; frame <= SintConst(2); frame++)
  {
    CHECKTRANSLATIONSPACE(len/3+1);
    plen = translateDNAforward(sfinfo->transnum,
                               frame,
                               sfinfo->proteinseq.spaceUchar +
                                 sfinfo->proteinseq.nextfreeUchar,
                               start,
                               len);
    if(plen < 0)
    {
      return (Sint) -1;
    }
    sfinfo->proteinseq.nextfreeUchar += plen;
    CHECKMARKPOSSPACE;
    sfinfo->markpostab.spaceUint[sfinfo->markpostab.nextfreeUint++]
      = sfinfo->proteinseq.nextfreeUchar;
    CHECKTRANSLATIONSPACE(1);
    sfinfo->proteinseq.spaceUchar[sfinfo->proteinseq.nextfreeUchar++] 
      = SEPARATOR;
  }
  for(frame=0; frame >= SintConst(-2); frame--)
  {
    CHECKTRANSLATIONSPACE(len/3+1);
    plen = translateDNAbackward(sfinfo->transnum,
                                frame,
                                sfinfo->proteinseq.spaceUchar +
                                  sfinfo->proteinseq.nextfreeUchar,
                                start,
                                len);
    if(plen < 0)
    {
      return (Sint) -2;
    }
    sfinfo->proteinseq.nextfreeUchar += plen;
    if(frame != (Sint) -2 || seqnum < sfinfo->numofsequences - 1)
    {
      CHECKMARKPOSSPACE;
      sfinfo->markpostab.spaceUint[sfinfo->markpostab.nextfreeUint++]
        = sfinfo->proteinseq.nextfreeUchar;
      CHECKTRANSLATIONSPACE(1);
      sfinfo->proteinseq.spaceUchar[sfinfo->proteinseq.nextfreeUchar++] 
        = SEPARATOR;
    }
  }
  return 0;
}

static void transformstringlocal(Uchar *sequence,
                                 Uint *symbolmap,
                                 Uchar *originalsequence,
                                 Uint totallen)
{
  Uint i;
  Uchar cc;

  for(i=0; i<totallen; i++)
  {
    cc = originalsequence[i];
    if(cc == SEPARATOR)
    {
      sequence[i] = (Uchar) SEPARATOR;
    } else
    {
      sequence[i] = (Uchar) symbolmap[(Uint) cc];
    }
  }
}

Sint multisixframetranslateDNA(Uint transnum,
                               BOOL withdescription,
                               Multiseq *proteinmultiseq,
                               Multiseq *multiseq,
                               Uint *proteinsymbolmap)
{
  Sixframeinfo sfinfo;

  initmultiseq(proteinmultiseq);
  initmultiseqfileinfo(proteinmultiseq);
  sfinfo.transnum = transnum;
  sfinfo.proteinseq.allocatedUchar = (multiseq->totallength/3+2)*MAXFRAMES;
  ALLOCASSIGNSPACE(sfinfo.proteinseq.spaceUchar,NULL,
                   Uchar,sfinfo.proteinseq.allocatedUchar);
  sfinfo.proteinseq.nextfreeUchar = 0;
  sfinfo.markpostab.allocatedUint = (multiseq->numofsequences * MAXFRAMES) - 1; 
  ALLOCASSIGNSPACE(sfinfo.markpostab.spaceUint,NULL,
                   Uint,sfinfo.markpostab.allocatedUint);
  sfinfo.markpostab.nextfreeUint = 0;
  sfinfo.numofsequences = multiseq->numofsequences;
  sfinfo.dnamultiseq = multiseq;
  sfinfo.withdescription = withdescription;
  if(withdescription && multiseq->descspace.spaceUchar != NULL)
  {
    sfinfo.descspace.allocatedUchar 
      = multiseq->descspace.nextfreeUchar + 
        multiseq->numofsequences * MAXFRAMES;
    ALLOCASSIGNSPACE(sfinfo.descspace.spaceUchar,NULL,Uchar,
                     sfinfo.descspace.allocatedUchar);
    sfinfo.descspace.nextfreeUchar = 0;
    ALLOCASSIGNSPACE(sfinfo.startdesc,NULL,Uint,
                     multiseq->numofsequences * MAXFRAMES + 1);
  }
  if(overalloriginalsequences(multiseq,&sfinfo,singlesixframetranslateDNA) != 0)
  {
    return (Sint) -1;
  }
  if(withdescription && multiseq->descspace.spaceUchar != NULL)
  {
    proteinmultiseq->descspace = sfinfo.descspace;
    sfinfo.startdesc[multiseq->numofsequences * MAXFRAMES]
      = sfinfo.descspace.nextfreeUchar;
    proteinmultiseq->startdesc = sfinfo.startdesc;
  }
  proteinmultiseq->markpos = sfinfo.markpostab;
  proteinmultiseq->numofsequences = multiseq->numofsequences * MAXFRAMES;
  proteinmultiseq->totallength = sfinfo.proteinseq.nextfreeUchar;
  // filesep not necessary
  // proteinmultiseq->totalnumoffiles = multiseq->totalnumoffiles;
  // numofqueryfiles already initialized
  // numofquerysequences already initialized
  // totalquerylength already initialized
  // allfiles already initialized
  // searchregexp already initialed
  proteinmultiseq->originalsequence = sfinfo.proteinseq.spaceUchar;
  // rcsequence already initialized
  // rcoriginalsequence already initialized
  ALLOCASSIGNSPACE(proteinmultiseq->sequence,NULL,Uchar,
                   proteinmultiseq->totallength);
  transformstringlocal(proteinmultiseq->sequence,
                       proteinsymbolmap,
                       proteinmultiseq->originalsequence,
                       proteinmultiseq->totallength);
  return 0;
}

Sint sixframeconvertmatch(Multiseq *dnamultiseq,
                          BOOL *palindromic,
                          Uint *dnarelpos,
                          Uint *dnastartpos,
                          Uint matchlength2,
                          Uint matchseqnum2,
                          Uint matchrelpos2)
{
  PairUint bndsdna;
  Uint frame;

  if(findboundaries(dnamultiseq,
                    matchseqnum2/MAXFRAMES,
                    &bndsdna) != 0)
  {
    return (Sint) -1;
  }
  frame = matchseqnum2 % MAXFRAMES;
  if(frame <= UintConst(2))
  {
    *dnarelpos = matchrelpos2 * CODONLENGTH + frame;
    *palindromic = False;
  } else
  {
    frame %= 3;
    *dnarelpos = bndsdna.uint1 - bndsdna.uint0 + 1 
                 - (matchrelpos2 + matchlength2) * CODONLENGTH 
                 - frame;
    *palindromic = True;
  }
  *dnastartpos = bndsdna.uint0;
  return 0;
}
