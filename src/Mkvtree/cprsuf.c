#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "arraydef.h"

#include "bwtcode.pr"

Uint *transsuftab(Uchar **sortedsuffixes,Uchar *sequence,Uint seqlen)
{
  Uint i, *suftab;

  suftab = (Uint *) sortedsuffixes;
  for(i=0; i<=seqlen; i++)
  {
    suftab[i] = (Uint) (sortedsuffixes[i] - sequence);
  }
  return suftab;
}

static void comparetabs(Uint *suftab,Uint *decodesuftab,Uint seqlen)
{
  Uint i;

/*
  for(i=0; i<=seqlen; i++)
  {
    printf("%lu: %lu %lu\n",(Showuint) i,
                            (Showuint) suftab[i],
                            (Showuint) decodesuftab[i]);
  }
*/
  for(i=0; i<=seqlen; i++)
  {
    if(suftab[i] != decodesuftab[i])
    {
      fprintf(stderr,"%lu: incorrect decoding: %lu != %lu\n",
                             (Showuint) i,
                             (Showuint) suftab[i],
                             (Showuint) decodesuftab[i]);
      exit(EXIT_FAILURE);
    }
  }
}

static void comparesequences(Uchar *sequence,Uchar *decodesequence,Uint seqlen)
{
  Uint i;

  for(i=0; i<seqlen; i++)
  {
    if(sequence[i] != decodesequence[i])
    {
      fprintf(stderr,"%lu: incorrect character: %lu != %lu\n",
                      (Showuint) i,
                      (Showuint) sequence[i],
                      (Showuint) decodesequence[i]);
      exit(EXIT_FAILURE);
    }
  }
}

Sint checksufencoding(BOOL special,Uint *suftab,Uint longest,
                      Uchar *sequence,Uint seqlen)
{
  Uchar *bwt, *decodesequence;
  Uint *decodesuftab;

  ALLOCASSIGNSPACE(bwt,NULL,Uchar,seqlen+1);
  ALLOCASSIGNSPACE(decodesequence,NULL,Uchar,seqlen+1);
  ALLOCASSIGNSPACE(decodesuftab,NULL,Uint,seqlen+1);

  if(encodeburrowswheeler(bwt,suftab,sequence,seqlen) != 0)
  {
    return (Sint) -1;
  }
  if(special)
  {
    decodeburrowswheelerspecial(decodesuftab,decodesequence,decodesuftab,bwt,
                                longest,seqlen);
  } else
  {
    decodeburrowswheeler(decodesuftab,decodesequence,decodesuftab,bwt,
                         longest,seqlen);
  }
  comparetabs(suftab,decodesuftab,seqlen);
  comparesequences(sequence,decodesequence,seqlen);
  FREESPACE(bwt);
  FREESPACE(decodesequence);
  FREESPACE(decodesuftab);
  return 0;
}
