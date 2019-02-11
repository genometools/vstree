//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <ctype.h>
#include <string.h>
#include "types.h"
#include "chardef.h"
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "multidef.h"
#include "alphadef.h"
#include "fhandledef.h"

#include "filehandle.pr"
#include "alphabet.pr"
#include "checkgzip.pr"
#include "readgzip.pr"
#include "parsemultiform.pr"
#include "multiseq-adv.pr"

/*EE
  The following function computes the reverse complement for
  the sequence in memory area between \texttt{inseqstart} and
  \texttt{inseqend}. It write the result into the memory area
  \texttt{outseq}.
*/

Sint makereversecomplement(Uchar *outseq,Uchar *inseqstart,Uint len)
{
  Uchar c, *iptr, *cpptr = outseq;

  for(iptr = inseqstart + len - 1; iptr >= inseqstart; iptr--, cpptr++)
  {
    c = *iptr;
    if(c == (Uchar) WILDCARD)
    {
      *cpptr = (Uchar) WILDCARD;
    } else
    {
      if(c > (Uchar) 3)
      {
        ERROR1("reverse complement of %lu undefined",(Showuint) c);
        return (Sint) -1;
      }
      *cpptr = ((Uchar) 3) - c;  // the reverse complement
    }
  }
  return 0;
}

#define ASSIGNRC(BASEOUT,BASEIN)\
        switch(BASEIN)\
        {\
          case 'A' : BASEOUT = (Uchar) 'T'; break;\
          case 'a' : BASEOUT = (Uchar) 't'; break;\
          case 'C' : BASEOUT = (Uchar) 'G'; break;\
          case 'c' : BASEOUT = (Uchar) 'g'; break;\
          case 'G' : BASEOUT = (Uchar) 'C'; break;\
          case 'g' : BASEOUT = (Uchar) 'c'; break;\
          case 'T' : BASEOUT = (Uchar) 'A'; break;\
          case 't' : BASEOUT = (Uchar) 'a'; break;\
          default  : BASEOUT = BASEIN;\
        }

Sint makereversecomplementorig(Uchar *outseq,Uchar *inseqstart,Uint len)
{
  Uchar c, *iptr, *cpptr = outseq;

  for(iptr = inseqstart + len - 1; iptr >= inseqstart; iptr--, cpptr++)
  {
    c = *iptr;
    ASSIGNRC(*cpptr,c);
  }
  return 0;
}

/*EE
  The following function computes the reverse complement of
  a multiple sequence and returns a pointer to the resulting
  memory area. If something went wrong, the function returns 0.
  It is important to note that \texttt{multiseq-\symbol{62}sequence}
  is not reversed completely. Instead we reverse each of the 
  sequences concatenated in \texttt{multiseq-\symbol{62}sequence},
  independent of the other. In this way, the remaining parts of the
  multiple sequence (i.e.\ description, markpos etc) remains also 
  valued for the reverse complement of the sequence.
*/

/*@null@*/ Uchar *copymultiseqRC(Multiseq *multiseq)
{
  Uint i, start, end;
  Uchar *cpseq;

  ALLOCASSIGNSPACE(cpseq,NULL,Uchar,multiseq->totallength);
  for(i=0; i<multiseq->numofsequences; i++)
  {
    if(i == 0)
    {
      start = 0;
    } else
    {
      start = multiseq->markpos.spaceUint[i-1];
      cpseq[start++] = SEPARATOR;
    }
    if(i == multiseq->numofsequences - 1)
    {
      end = multiseq->totallength;
    } else
    {
      end = multiseq->markpos.spaceUint[i];
    }
    if(makereversecomplement(cpseq + start,
                             multiseq->sequence + start,end - start) != 0)
    {
      return NULL;
    }
  }
  return cpseq;
}

/*
  This function is derived from the function copymultiseqRC in the file
  readmulti.c.
  It should be included there.
  The existence of the orignialsequence in the multiseq is not tested!
*/

/*@null@*/ Uchar *copymultiseqRCorig(Multiseq *multiseq)
{
  Uint i, start, end;
  Uchar *cpseq;

  ALLOCASSIGNSPACE(cpseq,NULL,Uchar,multiseq->totallength);
  for(i=0; i<multiseq->numofsequences; i++)
  {
    if(i == 0)
    {
      start = 0;
    } else
    {
      start = multiseq->markpos.spaceUint[i-1];
      cpseq[start++] = SEPARATOR;
    }
    if(i == multiseq->numofsequences - 1)
    {
      end = multiseq->totallength;
    } else
    {
      end = multiseq->markpos.spaceUint[i];
    }
    if(makereversecomplementorig(cpseq + start,
                                 multiseq->originalsequence + start,
                                 end - start) != 0)
    {
      return NULL;
    }
  }
  return cpseq;
}

/*EE
  The following function reads in a \texttt{multiseq}.
  \texttt{storedesc} is true iff the sequence descriptions are
  to be stored. \texttt{alpha} is the alphabet of the sequence to
  parse. \texttt{numoffiles} is the number of files whose names
  are stored in the structure \texttt{allfiles}. The position of the
  file separators are written into \texttt{filesep}.
  The pointer to a function \texttt{showverbose} can be \texttt{NULL}. Then
  no verbose output is shown. If the pointer is not \texttt{NULL} then
  the function outputs the verbose messages. The function returns
  0, if everything went fine, and a negative error code, otherwise.
*/

Sint readmultiseq(BOOL storedesc,
                  BOOL sixframematch,
                  const char *dnasymbolmapprotvsdna,
                  Alphabet *virtualalpha,
                  Uint numoffiles,
                  Multiseq *multiseq,
                  Showverbose showverbose)
{
  Sint retcode;
  BOOL dnatype = True, isgzipped;
  Alphabet *readinalphabet, dnaalphabet;

  if(numoffiles == UintConst(1))
  {
    isgzipped = checkgzipsuffix(multiseq->allfiles[0].filenamebuf);
    if(isgzipped)
    {
      multiseq->sequence = readgzippedfile(multiseq->allfiles[0].filenamebuf,
                                           &multiseq->totallength);

    } else
    {
      multiseq->sequence 
        = (Uchar *) CREATEMEMORYMAP(multiseq->allfiles[0].filenamebuf,True,
                                    &multiseq->totallength);
    }
    if(multiseq->sequence == NULL)
    {
      return (Sint) -1;
    }
    multiseq->allfiles[0].filelength = multiseq->totallength;
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
    multiseq->sequence = concatmanyfiles(showverbose,numoffiles,
                                         &multiseq->allfiles[0],
                                         &multiseq->filesep[0],
                                         &multiseq->totallength);
#ifdef DEBUG
    checkfilesep(numoffiles,&multiseq->filesep[0],multiseq->sequence);
#endif
    if(multiseq->sequence == NULL)
    {
      return (Sint) -1;
    }
  }
  retcode = parseMultiformat(&dnatype,multiseq->sequence,multiseq->totallength);
  if(retcode < 0) // error occurred
  {
    return (Sint) -2;
  }
  if(retcode == 0)
  {
    ERROR1("input file \"%s\" is not in Fasta, GENBANK or EMBL format",
            multiseq->allfiles[0].filenamebuf);
    return (Sint) -2;
  }
  multiseq->totallength = (Uint) retcode;
  if(sixframematch)
  {
    if(vm_isproteinalphabet(virtualalpha))
    {
      if(dnasymbolmapprotvsdna == NULL)
      {
        assignDNAalphabet(&dnaalphabet);
      } else
      {
        if(readsymbolmap(&dnaalphabet,(Uint) UNDEFCHAR,(Uint) WILDCARD,
                         dnasymbolmapprotvsdna) != 0)
        {
          return (Sint) -3;
        }
        if(!vm_isdnaalphabet(&dnaalphabet))
        {
          ERROR1("symbol map file \"%s\" is not an alphabet mapping for DNA",
                  dnasymbolmapprotvsdna);
          return (Sint) -4;
        }
      }
      readinalphabet = &dnaalphabet;
    } else
    {
      ERROR0("sixframe translation of query requires protein sequence index");
      return (Sint) -3;
    }
  } else
  {
    readinalphabet = virtualalpha;
  }
  if(readinalphabet->mapsize == 0)
  {
    ERROR0("need an alphabet for transforming fasta file");
    return (Sint) -3;
  }
  if(readmultiplefastafilesep(readinalphabet,
                              True,
                              storedesc,
                              multiseq,
                              multiseq->sequence,
                              multiseq->totallength) != 0)
  {
    return (Sint) -4;
  }
  if(multiseq->totallength == 0)
  {
    ERROR0("input sequences are all empty");
    return (Sint) -5;
  }
  return 0;
}

/*
  The following function reads the original sequence contained
  in the given list of files.
*/

Sint readmultiseqagain(Fileinfo *allfiles,
                       Uint numoffiles,
                       Uchar **originalsequence,
                       Showverbose showverbose)
{
  Uint totallength;
  Sint retcode;
  BOOL dnatype = True, isgzipped;

  if(numoffiles == UintConst(1))
  {
    isgzipped = checkgzipsuffix(allfiles[0].filenamebuf);
    if(isgzipped)
    {
      *originalsequence = readgzippedfile(allfiles[0].filenamebuf,
                                          &totallength);
    } else
    {
      *originalsequence 
         = (Uchar *) CREATEMEMORYMAP(allfiles[0].filenamebuf,True,
                                     &totallength);
    }
    if(*originalsequence == NULL)
    {
      return (Sint) -1;
    }
    if(showverbose != NULL)
    {
      char sbuf[PATH_MAX+50+1];
      sprintf(sbuf,"reading%sfile \"%s\" again",
                    isgzipped ? " gzipped " : " ",
                     allfiles[0].filenamebuf);
      showverbose(sbuf);
    }
  } else
  {
    *originalsequence = concatmanyfiles(showverbose,numoffiles,
                                        allfiles,
                                        NULL,
                                        &totallength);
    if(*originalsequence == NULL)
    {
      return (Sint) -1;
    }
  }
  retcode = parseMultiformat(&dnatype,*originalsequence,totallength);
  if(retcode < 0) // error occurred
  {
    return (Sint) -2;
  }
  if(retcode == 0)
  {
    ERROR1("input file \"%s\" is not in Fasta, GENBANK or EMBL format",
            allfiles[0].filenamebuf);
    return (Sint) -2;
  }
  totallength = (Uint) retcode;
  readmultiplefastafileagain(*originalsequence,*originalsequence,
                             totallength);
  return 0;
}
