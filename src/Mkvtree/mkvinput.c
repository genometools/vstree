#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "errordef.h"
#include "chardef.h"
#include "debugdef.h"
#include "spacedef.h"
#include "megabytes.h"
#include "visible.h"
#include "multidef.h"
#include "alphadef.h"
#include "genfile.h"

#include "checkgzip.pr"
#include "readgzip.pr"
#include "alphabet.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "parsemultiform.pr"
#include "reverse.pr"

#define MAXINPUTSIZE32BITVERSION (UintConst(1) << 30)

static void transformstringlocalinplace(Uchar *sequence,
                                        Uint totallen,
                                        Uint *symbolmap)
{
  Uchar *tptr;

  for(tptr = sequence; tptr < sequence + totallen; tptr++)
  {
    *tptr = (Uchar) symbolmap[(Uint) *tptr];
  }
}

static Sint transformstringlocalandcheck(Uchar *sequence,Uint totallen,
                                         Uint *symbolmap,Uchar undefsymbol)
{
  Uchar acode, *tptr;

  for(tptr = sequence; tptr < sequence + totallen; tptr++)
  {
    acode = (Uchar) symbolmap[(Uint) *tptr];
    if(acode == undefsymbol)
    {
      if(INVISIBLE(*tptr))
      {
        switch(*tptr)
        {
          case '\n' : ERROR0("Illegal newline"); break;
          case '\t' : ERROR0("Illegal tabulator"); break;
          case ' '  : ERROR0("Illegal blank"); break;
          default: ERROR1("Illegal character '\\%lu'",(Showuint) *tptr);
        }
      } else
      {
        ERROR1("Illegal character '%c'",*tptr);
      }
      return (Sint) -1;
    }
    *tptr = acode;
  }
  return 0;
}

static void storedummydescription(Multiseq *multiseq,char *desc)
{
  Uint desclen = (Uint) strlen(desc);
  ALLOCASSIGNSPACE(multiseq->descspace.spaceUchar,NULL,Uchar,desclen+1);
  memcpy(multiseq->descspace.spaceUchar,desc,(size_t) desclen);
  multiseq->descspace.spaceUchar[desclen] = (Uchar) '\n';
  multiseq->descspace.nextfreeUchar = desclen+1;
  ALLOCASSIGNSPACE(multiseq->startdesc,NULL,Uint,UintConst(2));
  multiseq->startdesc[0] = 0;
  multiseq->startdesc[1] = desclen+1;
}

static Sint inputplain(BOOL specialsymbols,
                       BOOL readorig,
                       Multiseq *multiseq,
                       Alphabet *alpha,
                       BOOL autoprefixlength,
                       BOOL prefixlengthisnotnull,
                       Uchar *sequence,
                       Uint seqlen)
{
  initmultiseq(multiseq);
  multiseq->sequence = sequence;
  multiseq->totallength = seqlen;
  multiseq->numofsequences = UintConst(1);
  multiseq->numofquerysequences = 0;
  multiseq->totalquerylength = 0;

  if(specialsymbols)
  {
    storedummydescription(multiseq,"sequence 1");
    if(readorig)
    {
      ALLOCASSIGNSPACE(multiseq->originalsequence,NULL,Uchar,seqlen);
      readmultiplefastafileagain(multiseq->originalsequence,sequence,seqlen);
    } else
    {
      multiseq->originalsequence = NULL;
    }
    if(transformstringlocalandcheck(multiseq->sequence,
                                    multiseq->totallength,
                                    alpha->symbolmap,
                                    (Uchar) alpha->undefsymbol) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(autoprefixlength)
    {
      ERROR0("option -pl requires positive integer as parameter "
             "since alphabet size is unknown");
      return (Sint) -2;
    }
    if(prefixlengthisnotnull)  
    {
      findalphabet(alpha,
                   sequence,
                   seqlen,
                   (Uint) UNDEFCHAR);
      transformstringlocalinplace(multiseq->sequence,
                                  multiseq->totallength,
                                  alpha->symbolmap);
    }
  }
  return 0;
}

static Sint inputformatted(BOOL specialsymbols,
                           BOOL readorig,
                           Multiseq *multiseq,
                           BOOL storedesc,
                           Alphabet *alpha,
                           Uchar *sequence,
                           Uint seqlen)
{
  if(specialsymbols)
  {
    if(readorig)
    {
      ALLOCASSIGNSPACE(multiseq->originalsequence,NULL,Uchar,seqlen);
      readmultiplefastafileagain(multiseq->originalsequence,sequence,seqlen);
    } else
    {
      multiseq->originalsequence = NULL;
    }
    multiseq->totallength = seqlen;
    multiseq->sequence = sequence;
    if(readmultiplefastafilesep(alpha,
                                False,  // checkregexp
                                storedesc,
                                multiseq,
                                sequence,
                                seqlen) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    ERROR0("fasta, EMBL, and GenBank format requires one of the "
           "options -smap, -dna, or -protein");
    return (Sint) -2;
  }
  return 0;
}

Sint mkvtreeinput(BOOL specialsymbols,
                  BOOL readorig,
                  BOOL rev,
                  BOOL cpl,
                  BOOL autoprefixlength,
                  BOOL prefixlengthisnotnull,
                  Multiseq *multiseq,
                  BOOL storedesc,
                  Alphabet *alpha,
                  Showverbose showverbose)
{
  Uchar *sequence;
  Uint seqlen;
  Sint retcode;
  BOOL dnatype = True, 
       isgzipped;

  if(multiseq->totalnumoffiles == UintConst(1))
  {
    isgzipped = checkgzipsuffix(multiseq->allfiles[0].filenamebuf);
    if(isgzipped)
    {
      sequence = readgzippedfile(multiseq->allfiles[0].filenamebuf,
                                 &seqlen);
    } else
    {
      sequence = (Uchar *) CREATEMEMORYMAP(multiseq->allfiles[0].filenamebuf,
                                           True,&seqlen);
    }
    multiseq->allfiles[0].filelength = seqlen;
    multiseq->filesep[0] = UNDEFFILESEP;
    if(showverbose != NULL)
    {
      char sbuf[PATH_MAX+100+1];
      sprintf(sbuf,"reading%sfile \"%s\"",isgzipped ? " gzipped " : " ",
                                          multiseq->allfiles[0].filenamebuf);
      showverbose(sbuf);
    }
  } else
  {
    if(!specialsymbols)
    {
      ERROR0("multiple input files require one of the options "
             "-smap, -dna, or -protein");
      return (Sint) -2;
    }
    sequence = concatmanyfiles(showverbose,
                               multiseq->totalnumoffiles,
                               &multiseq->allfiles[0],
                               &multiseq->filesep[0],
                               &seqlen);
#ifdef DEBUG
    checkfilesep(multiseq->totalnumoffiles,
                 &multiseq->filesep[0],
                 sequence);
#endif
  }
  if(sequence == NULL)
  {
    return (Sint) -3;
  }
  retcode = parseMultiformat(&dnatype,sequence,seqlen);
  if(retcode < 0) // error occurred
  {
    return (Sint) -3;
  }
  if(retcode == 0)
  {
    if(inputplain(specialsymbols,
                  readorig,
                  multiseq,
                  alpha,
                  autoprefixlength,
                  prefixlengthisnotnull,
                  sequence,
                  seqlen) != 0)
    {
      return (Sint) -4;
    }
  } else
  {
    if(inputformatted(specialsymbols,readorig,multiseq,storedesc,
                      alpha,sequence,(Uint) retcode) != 0)
    {
      return (Sint) -5;
    }
  }
  if(multiseq->totallength == 0)
  {
    ERROR0("input sequences are all empty");
    return (Sint) -6;
  }
#ifndef SIXTYFOURBITS
  if(multiseq->totallength > MAXINPUTSIZE32BITVERSION)
  {
    ERROR1("input sequence length must be smaller than %.2f megabytes\n",
            MEGABYTES(MAXINPUTSIZE32BITVERSION));
    return (Sint) -7;
  }
#endif
  if(rev)
  {
    if(cpl)
    {
      if(showverbose != NULL)
      {
        showverbose("reverse complement sequence");
      }
      if(reversecomplementinplace(multiseq->sequence,
                                  multiseq->totallength) != 0)
      {
        return (Sint) -8;
      }
    } else
    {
      if(showverbose != NULL)
      {
        showverbose("reverse sequence");
      }
      reverseinplace(multiseq->sequence,multiseq->totallength);
    }
  } else
  {
    if(cpl)
    {
      if(showverbose != NULL)
      {
        showverbose("complement sequence");
      }
      if(onlycomplement(multiseq->sequence,multiseq->totallength) != 0)
      {
        return (Sint) -9;
      }
    }
  }
  return 0;
}
