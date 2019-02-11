#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debugdef.h"
#include "spacedef.h"
#include "alphadef.h"
#include "genfile.h"
#include "fhandledef.h"
#include "multidef.h"

#include "alphabet.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "filehandle.pr"

typedef struct
{
  Uint minlength,
       *prefixtable;
  Multiseq *multiseq;
} Suffixprefixinfo;

static void computePrefixFunction(Uint *prefixtab,
                                  Uchar *pattern,
                                  Uint patterlen)
{
  Uchar b;
  Uint i, 
       vlen;

  prefixtab[1] = 0;
  vlen = 0;
  for(i=UintConst(2); i<=patterlen; i++)
  {
    b = pattern[i-1];
    while (vlen > 0 && pattern[vlen] != b)  // case (b) and (c)
    {
      vlen = prefixtab[vlen];
    }
    if(pattern[vlen] == b) // case (a)
    {
      vlen = vlen+1;
    }
    prefixtab[i] = vlen;
  }
}

static Uint kmpsuffixprefixmatch(Uint *prefixtab,
                                 Uchar *p,Uint m,Uchar *t,Uint n)
{
  Uchar b, c;    // temporary characters
  Uint j = 0,    // position in t corresponding to first character in P
       cpl = 0;  // current length of common prefix of t[j..n-1] and P

  computePrefixFunction(prefixtab,p,m);
  while(j+cpl < n)
  {
    if(cpl == m)
    {                                        // case (1)
      j = j + cpl - prefixtab[cpl];
      cpl = prefixtab[cpl];
    } else
    {                                        // case (2)
      b = p[cpl];
      c = t[j+cpl];
      if(b != c)
      {
        if(cpl > 0)
        {                                    // case (2a)
          j = j + cpl - prefixtab[cpl];
          cpl = prefixtab[cpl];
        } else
        {                                    // case (2b)
          j++;
        }
      } else
      {                                      // case (2c)
        cpl++;
      }
    }
  }
  return cpl;
}

#ifdef DEBUG
static void checksuffixprefix(Uchar *seq1,Uint len1,Uchar *seq2,Uint cpl)
{
  Uint i, startsuffix = len1 - cpl;


  for(i=0; i<cpl; i++)
  {
    if(seq1[startsuffix+i] != seq2[i])
    {
      fprintf(stderr,"seq1[%lu] = %lu != %lu = seq2[%lu]\n",
              (Showuint) i,(Showuint) seq1[i],
              (Showuint) startsuffix+i,(Showuint) seq2[startsuffix+i]);
      exit(EXIT_FAILURE);
    }
  }
}
#endif

static Sint findsuffixprefixmatch(void *info,
                                  Uint i,Uint start1,Uint len1,
                                  Uint j,Uint start2,Uint len2)
{
  Suffixprefixinfo *suffixprefixinfo = (Suffixprefixinfo *) info;
  Uchar *seq1, *seq2;
  Uint cpl;

  seq1 = suffixprefixinfo->multiseq->sequence + start1;
  seq2 = suffixprefixinfo->multiseq->sequence + start2;
  cpl = kmpsuffixprefixmatch(suffixprefixinfo->prefixtable,
                             seq2,len2,seq1,len1);
  if(cpl >= suffixprefixinfo->minlength)
  {
    printf("%lu %lu %lu\n",(Showuint) i,(Showuint) j,(Showuint) cpl);
    DEBUGCODE(2,checksuffixprefix(seq1,len1,seq2,cpl));
  }
  cpl = kmpsuffixprefixmatch(suffixprefixinfo->prefixtable,
                             seq1,len1,seq2,len2);
  if(cpl >= suffixprefixinfo->minlength)
  {
    printf("%lu %lu %lu\n",(Showuint) j,(Showuint) i,(Showuint) cpl);
    DEBUGCODE(2,checksuffixprefix(seq2,len2,seq1,cpl));
  }
  return 0;
}

static Sint allsuffixprefixmatch(Multiseq *multiseq,Uint minlength)
{
  ExtremeAverageSequences extremevalues;
  Suffixprefixinfo suffixprefixinfo;

  if(calculateseqparm(multiseq,&extremevalues) != 0)
  {
    return -1;
  }
  printf("# maximal sequence length = %lu\n",
         (Showuint) extremevalues.maxlength);
  suffixprefixinfo.minlength = minlength;
  suffixprefixinfo.multiseq = multiseq;
  ALLOCASSIGNSPACE(suffixprefixinfo.prefixtable,NULL,Uint,
                   extremevalues.maxlength+1);
  if(overallpairsofsequences(multiseq,&suffixprefixinfo,
                             findsuffixprefixmatch) != 0)
  {
    return -2;
  }
  FREESPACE(suffixprefixinfo.prefixtable);
  return 0;
}

static Sint mapandparsesequencefileassignalphabet(Alphabet *alpha,
                                                  Multiseq *multiseq,
                                                  const char *filename)
{
  Uchar *seq;
  Uint seqlen;

  initmultiseqfileinfo(multiseq);
  multiseq->originalsequence = NULL;
  seq = (Uchar *) CREATEMEMORYMAP(filename,True,&seqlen);
  if(seq == NULL || seqlen == 0)
  {
    return -1;
  }
  assignDNAalphabet(alpha);
  if(readmultiplefastafile(alpha,True,multiseq,seq,seqlen) != 0)
  {
    return -2;
  }
  return 0;
}

MAINFUNCTION
{
  Alphabet alpha;
  Multiseq multiseq;
  Uint minlength;
  Scaninteger readint;

  if(argc != 3)
  {
    fprintf(stderr,"%s <minlength> <inputfile>\n",argv[0]);
    return EXIT_FAILURE;
  }
  if(sscanf(argv[1],"%ld",(Scaninteger *) &readint) != 1 || readint < 0)
  {
    fprintf(stderr,"%s: first argument must be non-negative integer\n",argv[0]);
    return EXIT_FAILURE;
  }
  minlength = (Uint) readint;
  if(mapandparsesequencefileassignalphabet(&alpha,&multiseq,argv[2]) != 0)
  {
    STANDARDMESSAGE;
  }
  if(allsuffixprefixmatch(&multiseq,minlength) != 0)
  {
    STANDARDMESSAGE;
  }
  freemultiseq(&multiseq);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
