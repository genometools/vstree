
#include "match.h"
#include "alphadef.h"
#include "debugdef.h"
#include "codondef.h"
#include "scoredef.h"
#include "outinfo.h"

#include "codon.pr"
#include "alphabet.pr"

/*
  The following functions determine the sequence of the left and the right
  instance of a match.
*/

static Sint generatetranslatedcodoninbuffer(Alphabet *virtualalpha,
                                            Showseqinfo *showseqinfo,
                                            Matchflag storeflag, 
                                            Uint storelength)
{
  Sint retproteinlength; 
  Uint proteinlength = storelength/CODONLENGTH;

  ALLOCASSIGNSPACE(showseqinfo->codon2aminobufferorig,
                   showseqinfo->codon2aminobufferorig,
                   Uchar,
                   proteinlength);
  ALLOCASSIGNSPACE(showseqinfo->codon2aminobuffermapped,
                   showseqinfo->codon2aminobuffermapped,
                   Uchar,
                   proteinlength);
  if(storeflag & FLAGPPRIGHTREVERSE)
  {
    retproteinlength
      = translateDNAbackward(FLAG2TRANSNUM(storeflag),
                             0,
                             showseqinfo->codon2aminobufferorig,
                             showseqinfo->showseqorig,
                             storelength);
  } else
  {
    retproteinlength
      = translateDNAforward(FLAG2TRANSNUM(storeflag),
                            0,
                            showseqinfo->codon2aminobufferorig,
                            showseqinfo->showseqorig,
                            storelength);
  }
  if(retproteinlength < 0)
  {
    return (Sint) -1;
  }
  if((Uint) retproteinlength != proteinlength)
  {
    ERROR2("length of protein sequences differ: "
           "retproteinlength = %ld != %lu = proteinlength",
           (Showsint) retproteinlength,(Showuint) proteinlength);
    return (Sint) -2;
  }
  if(virtualalpha == NULL)
  {
    NOTIMPLEMENTED;  // virtualalpha == NULL
  }
  if(transformstringincopy(virtualalpha,
                           showseqinfo->codon2aminobuffermapped,
                           showseqinfo->codon2aminobufferorig,
                           proteinlength) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

void getleftseq(Showseqinfo *showseqinfo,
                StoreMatch *storematch,
                Multiseq *virtualmultiseq,
                /*@unused@*/ Alphabet *virtualalpha)
{
  showseqinfo->showseqcompare = virtualmultiseq->sequence;
  showseqinfo->showseqorig = virtualmultiseq->originalsequence;
  showseqinfo->showseqcompare += storematch->Storeposition1;
  showseqinfo->showseqorig += storematch->Storeposition1;
  showseqinfo->length = storematch->Storelength1;
}

Sint getrightseq(Showseqinfo *showseqinfo,
                 StoreMatch *storematch,
                 Multiseq *virtualmultiseq,
                 Multiseq *querymultiseq,
                 Alphabet *virtualalpha)
{
  Uint start;

  if(storematch->Storeflag & FLAGQUERY)
  {
    if(storematch->Storeflag & FLAGSELFPALINDROMIC)
    {
      showseqinfo->showseqcompare = virtualmultiseq->sequence;
      showseqinfo->showseqorig = virtualmultiseq->originalsequence;
    } else
    {
      showseqinfo->showseqcompare = querymultiseq->sequence;
      showseqinfo->showseqorig = querymultiseq->originalsequence;
    }
    start = storematch->Storeposition2;
  } else
  {
    if(HASINDEXEDQUERIES(virtualmultiseq))
    {
      showseqinfo->showseqcompare = virtualmultiseq->sequence;
      showseqinfo->showseqorig = virtualmultiseq->originalsequence;
      start = DATABASELENGTH(virtualmultiseq) + 
              UintConst(1) + 
              storematch->Storeposition2;
    } else
    {
      showseqinfo->showseqcompare = virtualmultiseq->sequence;
      showseqinfo->showseqorig = virtualmultiseq->originalsequence;
      start = storematch->Storeposition2;
    }
  }
  showseqinfo->showseqcompare += start;
  showseqinfo->showseqorig += start;
  if((storematch->Storeflag & FLAGQUERY) &&
     FLAG2TRANSNUM(storematch->Storeflag) != NOTRANSLATIONSCHEME)
  {
    if(generatetranslatedcodoninbuffer(virtualalpha,
                                       showseqinfo,
                                       storematch->Storeflag, 
                                       storematch->Storelength2) != 0)
    {
      return (Sint) -1;
    }
    showseqinfo->showseqorig = showseqinfo->codon2aminobufferorig;
    showseqinfo->showseqcompare = showseqinfo->codon2aminobuffermapped;
    showseqinfo->length = storematch->Storelength2/CODONLENGTH;
  } else
  {
    showseqinfo->length = storematch->Storelength2;
  }
  return 0;
}
