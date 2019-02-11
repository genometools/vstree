#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "args.h"
#include "multidef.h"
#include "alphadef.h"
#include "debugdef.h"
#include "errordef.h"
#include "genfile.h"
#include "inputsymbol.h"
#include "spacedef.h"

#include "multiseq.pr"
#include "multiseq-adv.pr"

static void showseqs(Multiseq *multiseq)
{
  Uchar *start, *end;
  Uint i;
  size_t len;

  for(i=0; i<multiseq->numofsequences; i++)
  {
    (void) putchar(FASTASEPARATOR);
    if(i == 0)
    {
      start = multiseq->sequence;
    } else
    {
      start = multiseq->sequence + multiseq->markpos.spaceUint[i-1] + 1;
    }
    if(i == multiseq->numofsequences - 1)
    {
      end = multiseq->sequence + multiseq->totallength - 1;
    } else
    {
      end = multiseq->sequence + multiseq->markpos.spaceUint[i] - 1;
    }
    len = (size_t) (end - start + 1);
    if(fwrite(start,sizeof(Uchar),len,stdout) != len)
    {
      fprintf(stderr,"Cannot write %lu items of type Uchar",(Showuint) len);
      exit(EXIT_FAILURE);
    }
  }
}

MAINFUNCTION
{
  Uchar *text;
  Multiseq multiseq;
  Uint textlen, i, seqlen = 0, laststart = 0;

  CHECKARGNUM(2,"multiplefastfile");
  DEBUGLEVELSET;
  text = CREATEMEMORYMAP(argv[1],False,&textlen);
  if(text == NULL)
  {
    STANDARDMESSAGE;
  }
  initmultiseq(&multiseq);
  for(i=0; i<textlen; i++)
  {
    if(text[i] == FASTASEPARATOR)
    {
      if(seqlen > 0)
      {
        DEBUG2(3,"%lu %lu\n",(Showuint) laststart,
                             (Showuint) seqlen);
        addmultiseq(&multiseq,text+laststart,seqlen);
      }
      laststart = i+1;
      seqlen = 0;
    } else
    {
      seqlen++;
    }
  }
  DEBUG2(3,"%lu %lu\n",(Showuint) laststart,
                       (Showuint) seqlen);
  addmultiseq(&multiseq,text+laststart,seqlen);
  showseqs(&multiseq);
  if(DELETEMEMORYMAP(text) != 0)
  {
    STANDARDMESSAGE;
  }
  return EXIT_SUCCESS;
}
