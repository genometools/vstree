#include <stdio.h>
#include "args.h"
#include "types.h"
#include "alphadef.h"
#include "multidef.h"
#include "spacedef.h"
#include "genfile.h"
#include "debugdef.h"
#include "errordef.h"

#include "multiseq.pr"
#include "alphabet.pr"
#include "multiseq-adv.pr"
#include "parsemultiform.pr"

static Sint mapandparsesequencefilewitheemptyalphabet(Alphabet *alpha,
                                                      Multiseq *multiseq,
                                                      const char *filename)
{
  Uchar *seq;
  Uint seqlen;
  BOOL dnatype;
  Sint retcode;
  Alphabet *alphaptr, localalpha;

  initmultiseqfileinfo(multiseq);
  multiseq->originalsequence = NULL;
  seq = (Uchar *) CREATEMEMORYMAP(filename,True,&seqlen);
  if(seq == NULL || seqlen == 0)
  {
    return -1;
  }
  retcode = parseMultiformat(&dnatype,seq,seqlen);
  if(retcode < 0) // error occurred
  {
    return -1;
  }
  if(retcode == 0)
  {
    ERROR1("input file \"%s\" is not in Fasta, GENBANK or EMBL format",
            filename);
    return -2;
  }
  if(alpha == NULL)
  {
    alphaptr = &localalpha;
  } else
  {
    alphaptr = alpha;
  }
  if(dnatype)
  {
    assignDNAalphabet(alphaptr);
  } else
  {
    assignProteinalphabet(alphaptr);
  }
  if(readmultiplefastafile(alphaptr,True,multiseq,seq,(Uint) retcode) != 0)
  {
    return -1;
  }
  return 0;
}

MAINFUNCTION
{
  Alphabet alpha;
  Multiseq multiseq;

  CHECKARGNUM(2,"filename");

  if(mapandparsesequencefilewitheemptyalphabet(&alpha,&multiseq,argv[1]) != 0)
  {
    STANDARDMESSAGE;
  }
  if(showmultiplefasta(stdout,False,UintConst(60),&alpha,&multiseq,False) != 0)
  {
    STANDARDMESSAGE;
  }
  freemultiseq(&multiseq);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
