//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include "types.h"
#include "divmodmul.h"
#include "debugdef.h"
#include "spacedef.h"
#include "minmax.h"
#include "errordef.h"
#include "inputsymbol.h"
#include "chardef.h"
#include "alphadef.h"
#include "multidef.h"
#include "fhandledef.h"
#include "failures.h"
#include "genfile.h"
#include "cmp-tabdef.h"

#include "compfilenm.pr"
#include "dstrdup.pr"
#include "filehandle.pr"
#include "multiseq.pr"
#include "readgzip.pr"
#include "parsemultiform.pr"
#include "distri.pr"
#include "readnextline.pr"
#include "endianess.pr"
#include "checkgzip.pr"

//}

/*EE 
  This file implements the data type \texttt{Multiseq}.
*/

/*
  The following flags are used to specify in which way a sequence is shown
  by the function formatseq:
*/

#define FORMATSEQTENNERBLOCKS      UintConst(1)
#define FORMATSEQFORCEUPPER        (UintConst(1) << 1)
#define FORMATSEQFORCELOWER        (UintConst(1) << 2)
#define FORMATSEQCHARNUMBERING     (UintConst(1) << 3)

/*
  Store the next position where a description starts
*/

#define STORESTARTDESC\
        if(multiseq->numofsequences >= allocatedstartdesc)\
        {\
          allocatedstartdesc += 128;\
          ALLOCASSIGNSPACE(multiseq->startdesc,\
                           multiseq->startdesc,Uint,allocatedstartdesc);\
        }\
        multiseq->startdesc[multiseq->numofsequences]\
          = multiseq->descspace.nextfreeUchar

#define PRJERROR(WORD)\
        ERROR3("%s.%s does not contain line defining %s",indexname,\
                                                         PROJECTFILESUFFIX,\
                                                         #WORD)

/*EE
  Given a \texttt{multiseq}, and a \texttt{position} in 
  \texttt{multiseq}, the function \texttt{getfilenum} delivers the 
  file number for \texttt{position}. If this cannot be found, then a 
  negative error code is returned. The running time of \texttt{getfilenum} 
  is \(O(\log_{2}f)\), where \(f\) is the number of files stored
  in \texttt{multiseq}.
*/

Sint getfilenum(Multiseq *multiseq,Uint position)
{
  return getrecordnum(&multiseq->filesep[0],
                      multiseq->totalnumoffiles,
                      multiseq->totallength,position);
}

/*EE
  The following function initializes the fileinformation
  incorporated in a multiple sequence.
*/

void initmultiseqfileinfo(Multiseq *multiseq)
{
  Uint i;

  for(i=0; i<(Uint) MAXNUMBEROFFILES; i++)
  {
    multiseq->allfiles[i].filenamebuf = NULL;
    multiseq->allfiles[i].filelength = 0;
    multiseq->filesep[i] = UNDEFFILESEP;
  }
  multiseq->totalnumoffiles = 0;
  multiseq->numofqueryfiles = 0;
}

Sint overalloriginalsequences(Multiseq *multiseq,void *applyinfo,
                              Sint(*apply)(void *,Uint,Uchar *,Uint))
{
  Uint i;
  Uchar *seq, *start, *end;

  seq = multiseq->originalsequence;
  for(i = 0; i < multiseq->numofsequences; i++)
  {
    if(i == 0)
    {
      start = seq;
    } else
    {
      start = seq + multiseq->markpos.spaceUint[i-1] + 1;
    }
    if(i == multiseq->numofsequences - 1)
    {
      end = seq + multiseq->totallength;
    } else
    {
      end = seq + multiseq->markpos.spaceUint[i];
    }
    DEBUG3(3,"overalloriginalsequences: i=%lu, start=%lu, end=%lu\n",
              (Showuint) i,(Showuint) (start-seq),(Showuint) (end-seq));
    if(apply(applyinfo,i,start,(Uint) (end - start)) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

Sint overallpairsofsequences(Multiseq *multiseq,void *applyinfo,
                             Sint(*apply)(void *,Uint,Uint,Uint,
                                                Uint,Uint,Uint))
{
  Uint i, j, start1, len1, start2, len2;

  for(i = 0; i < multiseq->numofsequences - 1; i++)
  {
    if(i == 0)
    {
      start1 = 0;
    } else
    {
      start1 = multiseq->markpos.spaceUint[i-1] + 1;
    }
    if(i == multiseq->numofsequences - 1)
    {
      len1 = multiseq->totallength - start1;
    } else
    {
      len1 = multiseq->markpos.spaceUint[i] - start1;
    }
    for(j = i+1; j < multiseq->numofsequences; j++)
    {
      start2 = multiseq->markpos.spaceUint[j-1] + 1;
      if(j == multiseq->numofsequences - 1)
      {
        len2 = multiseq->totallength - start2;
      } else
      {
        len2 = multiseq->markpos.spaceUint[j] - start2;
      }
      DEBUG3(3,"overallpairsofsequences: (i=%lu,start1=%lu,len2=%lu,",
              (Showuint) i,(Showuint) start1,(Showuint) len2);
      DEBUG3(3,"j=%lu,start2=%lu,len2=%lu)\n",(Showuint) j,
                                              (Showuint) start2,
                                              (Showuint) len2);
      if(apply(applyinfo,i,start1,len1,j,start2,len2) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  return 0;
}

/*EE
  \texttt{addmultiseq} adds the sequence \texttt{seq} of length \texttt{len}
  to \texttt{multiseq}. The sequence \texttt{seq} is copied. 
*/

void addmultiseq(Multiseq *multiseq,Uchar *seq,Uint seqlen)
{
  Uint i, startcopy;
  Uchar *startcopyptr;
  
  if(multiseq->numofsequences == 0)
  {
    startcopy = 0;
    multiseq->totallength = seqlen;
  } else
  {
    startcopy = multiseq->totallength+1;
    multiseq->totallength += (seqlen+1);
  }
  ALLOCASSIGNSPACE(multiseq->sequence,multiseq->sequence,
                   Uchar,multiseq->totallength);
  if(multiseq->numofsequences > 0)
  {
    multiseq->sequence[startcopy-1] = SEPARATOR;
    STOREINARRAY(&(multiseq->markpos),Uint,128,startcopy-1);
  } 
  startcopyptr = multiseq->sequence + startcopy;
  for(i=0; i<seqlen; i++)
  {
    startcopyptr[i] = seq[i];
  }
  multiseq->numofsequences++;
}

Sint getunitinfo(Seqinfo *seqinfo,
                 Uint *unitseps,
                 Uint numofunits,
                 Uint totalwidth,
                 Uint position)
{
  Sint retcode;

  retcode = getrecordnum(unitseps,numofunits,totalwidth,position);
  if(retcode < 0)
  {
    return (Sint) -1;
  }
  seqinfo->seqnum = (Uint) retcode;
  if(seqinfo->seqnum == 0)
  {
    seqinfo->seqstartpos = 0;
    seqinfo->relposition = position;
    if(numofunits == UintConst(1))
    {
      seqinfo->seqlength = totalwidth;
    } else
    {
      seqinfo->seqlength = unitseps[0];
    }
  } else
  {
    seqinfo->seqstartpos = unitseps[seqinfo->seqnum-1] + 1;
    seqinfo->relposition = position - seqinfo->seqstartpos;
    if(seqinfo->seqnum == numofunits - 1)
    {
      seqinfo->seqlength = totalwidth - seqinfo->seqstartpos;
    } else
    {
      seqinfo->seqlength = unitseps[seqinfo->seqnum] - seqinfo->seqstartpos;
    }
  }
  DEBUG2(3,"position=%lu=>seqnum=%lu,",(Showuint) position,
                                       (Showuint) seqinfo->seqnum);
  DEBUG1(3,"seqlength=%lu,",(Showuint) seqinfo->seqlength);
  DEBUG1(3,"seqstartpos=%lu,",(Showuint) seqinfo->seqstartpos);
  DEBUG1(3,"seqrelposition=%lu\n",(Showuint) seqinfo->relposition);
  return 0;
}

/*EE
  The function \texttt{getseqinfo} computes the sequence information
  for \texttt{position} in \texttt{seqinfo}.
  If an error occurs, then a negative error code
  is returned. In case of success, the function returns 0.
  The running time of \texttt{getseqinfo} is \(O(\log_{2}k)\),
  where \(k\) is the number of sequences in \texttt{multiseq}. 
*/

Sint getseqinfo(Multiseq *multiseq,Seqinfo *seqinfo,Uint position)
{
  return getunitinfo(seqinfo,
                     multiseq->markpos.spaceUint,
                     multiseq->numofsequences,
                     multiseq->totallength,
                     position);
}

/*EE
  Given a unit number \texttt{unitnum} in the range \([0,\emph{numofunits}-1]\),
  the function \texttt{findunitboundaries} computes in the integer pair
  \texttt{range}, the first and last index of unit \texttt{unitnum}:
  \texttt{range->uint0} is the first index, and
  \texttt{range->uint1} is the last index. If the given unit
  number is not valid, then a negative error code is returned.
  In case of success, the return code is 0.
*/

Sint findunitboundaries(Uint *unitseppositions,
                        Uint numofunits,
                        Uint totallength,
                        Uint unitnum,
                        PairUint *range)
{
  if(unitnum >= numofunits)
  {
    ERROR2("unit %lu does not exist: maximal number of units is %lu",
            (Showuint) unitnum,(Showuint) numofunits);
    return (Sint) -1;
  }
  if(unitnum == 0)
  {
    range->uint0 = 0;
    if(numofunits == UintConst(1))
    {
      range->uint1 = totallength - 1;
    } else
    {
      range->uint1 = unitseppositions[unitnum] - 1;
    }
  } else
  {
    range->uint0 = unitseppositions[unitnum-1] + 1;
    if(unitnum == numofunits - 1)
    {
      range->uint1 = totallength - 1;
    } else
    {
      range->uint1 = unitseppositions[unitnum] - 1;
    }
  }
  DEBUG3(4,"findunitboundaries(unitnum=%lu)=(%lu,%lu)\n",
             (Showuint) unitnum,
             (Showuint) range->uint0,
             (Showuint) range->uint1);
  return 0;
}

/*EE
  Given a unit number \texttt{unitnum} in the range \([0,\emph{numofunits}-1]\),
  the function \texttt{findunitfirstpos} computes in 
  first index of unit \texttt{unitnum}.
  \texttt{range->uint1} is the last index. If the given unit
  number is not valid, then a negative error code is returned.
  In case of success, the return code is 0.
*/

Sint findunitfirstpos(Uint *unitseppositions,
                      Uint numofunits,
                      Uint unitnum,
                      Uint *firstpos)
{
  if(unitnum >= numofunits)
  {
    ERROR1("unit %lu does not exist",(Showuint) unitnum);
    return (Sint) -1;
  }
  if(unitnum == 0)
  {
    *firstpos = 0;
  } else
  {
    *firstpos = unitseppositions[unitnum-1] + 1;
  }
  return 0;
}

/*EE
  Given a sequence number \texttt{snum} in the range \([0,k-1]\),
  the function \texttt{findboundaries} computes in the integer pair
  \texttt{range}, the first and last index of \(T_{snum}\):
  \texttt{range->uint0} is the first index, and
  \texttt{range->uint1} is the last index. If the given sequence
  number is not valid, then a negative error code is returned.
  In case of success, the return code is 0.
*/

Sint findboundaries(Multiseq *multiseq,Uint snum,PairUint *range)
{
  return findunitboundaries(multiseq->markpos.spaceUint,
                            multiseq->numofsequences,
                            multiseq->totallength,
                            snum,
                            range);
}

/*EE
  Given a sequence number \texttt{snum} in the range \([0,k-1]\),
  the function \texttt{findfirstpos} computes 
  the first index of \(T_{snum}\). If the given sequence
  number is not valid, then a negative error code is returned.
  In case of success, the return code is 0.
*/

Sint findfirstpos(Multiseq *multiseq,Uint snum,Uint *firstpos)
{
  return findunitfirstpos(multiseq->markpos.spaceUint,
                          multiseq->numofsequences,
                          snum,
                          firstpos);
}

/*EE
  Given a \texttt{multiseq}, and a sequence number \texttt{seqnum} in 
  \texttt{multiseq}, the function \texttt{seqnum2filenum} delivers the 
  file number for sequence \texttt{seqnum}. If this cannot be found, then a 
  negative error code is returned. The running time of 
  \texttt{seqnum2filenum} is \(O(\log_{2}f)\), where \(f\) is the 
  number of files stored in \texttt{multiseq}.
*/

Sint seqnum2filenum(Multiseq *multiseq,Uint seqnum)
{
  PairUint range;

  if(findboundaries(multiseq,seqnum,&range) != 0)
  {
    return (Sint) -1;
  }
  return getfilenum(multiseq,range.uint0);
}

/*EE
  Given a sequence number \texttt{snum}, and a position \texttt{base} in 
  sequence \(T_{snum}\), then \texttt{findabspos} returns the 
  corresponding absolute position in \texttt{multiseq}. Additionally,
  it is checked whether sequence \(T_{snum}\) is of appropriate length.
  That is, we test, if the last position of the string of length
  \texttt{len} starting at position \texttt{base} in \(T_{snum}\) is 
  a substring of \(T_{snum}\). If an error occurs the function
  returns a negative value. In case of success, the function
  return 0.
*/

Sint findabspos(Multiseq *multiseq,Uint snum,Uint base,Uint len)
{
  Uint abspos; 
  PairUint range;

  if(findboundaries(multiseq,snum,&range) != 0)
  {
    return (Sint) -1;
  }
  abspos = range.uint0 + base;
  if(abspos + len - 1 > range.uint1)
  {
    ERROR4("abspos+len-1 = %lu+%lu-1 = %lu > right boundary of sequence %lu",
            (Showuint) abspos,(Showuint) len,(Showuint) (abspos+len-1),
            (Showuint) snum);
    return (Sint) -2;
  }
  return (Sint) abspos;
}

/*
  The function \texttt{checkmultiseq} checks if \texttt{multiseq} is consistent.
*/

#ifdef DEBUG
static void checkmultiseq(Multiseq *multiseq)
{
  Uint base = 0, snum = 0, i, currentseqlength;
  Sint abspos, tmpsnum;
  PairUint range;
  Seqinfo seqinfo;

  for(i=0; i<multiseq->numofsequences-1; i++)
  {
    DEBUG1(3,"%lu ",(Showuint) multiseq->markpos.spaceUint[i]);
  }
  DEBUG0(3,"\n");
  if(findboundaries(multiseq,0,&range) != 0)
  {
    fprintf(stderr,"findboundaries failed\n");
    exit(EXIT_FAILURE);
  }
  currentseqlength = range.uint1 - range.uint0 + 1;
  for(i=0; i<multiseq->totallength; i++)
  {
    if(multiseq->sequence[i] != SEPARATOR)
    {
      tmpsnum = getseqnum(multiseq,i);
      if(tmpsnum == (Sint) -1)
      {
        fprintf(stderr,"%s\n",messagespace());
        exit(EXIT_FAILURE);
      }
      if((Uint) tmpsnum != snum)
      {
        fprintf(stderr,"incorrect sequence number %ld for position %lu: "
                       "must be %lu",
               (Showsint) tmpsnum,(Showuint) i,(Showuint) snum);
        exit(EXIT_FAILURE);
      }
      abspos = findabspos(multiseq,snum,base,UintConst(1));
      if(abspos < 0)
      {
        fprintf(stderr,"%s\n",messagespace());
        exit(EXIT_FAILURE);
      }
      if((Uint) abspos != i)
      {
        fprintf(stderr,"incorrect abolute position %ld: must be %lu",
                (Showsint) abspos,(Showuint) i);
        exit(EXIT_FAILURE);
      }
      if(getseqinfo(multiseq,&seqinfo,i) != 0)
      {
        fprintf(stderr,"getseqinfo failed: %s\n",messagespace());
        exit(EXIT_FAILURE);
      }
      if(seqinfo.seqnum != snum)
      {
        fprintf(stderr,"seqinfo.seqnum = %lu != %lu = seqnum\n",
               (Showuint) seqinfo.seqnum,(Showuint) snum);
        exit(EXIT_FAILURE);
      }
      if(seqinfo.seqlength != currentseqlength)
      {
        fprintf(stderr,"seqinfo.seqlength = %lu != %lu = currentseqlength\n",
               (Showuint) seqinfo.seqlength,(Showuint) currentseqlength);
        exit(EXIT_FAILURE);
      }
      if(seqinfo.relposition != base)
      {
        fprintf(stderr,"seqinfo.relposition = %lu != %lu = base\n",
               (Showuint) seqinfo.relposition,(Showuint) base);
        exit(EXIT_FAILURE);
      }
      base++;
    } else
    {
      if(multiseq->markpos.spaceUint[snum] == i)
      {
        snum++;
        base = 0;
      }
      if(snum<multiseq->numofsequences)
      {
        if(findboundaries(multiseq,snum,&range) != 0)
        {
          fprintf(stderr,"findboundaries failed\n");
          exit(EXIT_FAILURE);
        }
        currentseqlength = range.uint1 - range.uint0 + 1;
      }
    }
  }
  DEBUG0(2,"# checkmultiseq okay\n");
}
#endif

/*EE
  The following function formats a sequence of length \texttt{len} 
  pointed to by \texttt{seq}, using lines of width \texttt{linewidth}.
  This sequence is over the list of \texttt{characters},  with undefined
  symbol equal to \texttt{undefsymbol}. The output is written to
  the file ptr \texttt{formatout}.
*/

Sint formatseq(FILE *formatout,
               Uint mapsize,
               Uchar *characters,
               Uint undefsymbol,
               Uint linewidth,
               const Uchar *seq,
               Uint len)
{
  Uint i, j, width = UintConst(1);
  Uchar c, wildcard;
  BOOL showcharnum = False;

  if(len == 0)
  {
    ERROR0("cannot format empty sequence");
    return (Sint) -1;
  }
  if(characters == NULL)
  {
    wildcard = (Uchar) 0;
  } else
  {
    wildcard = characters[WILDCARD];
  }
  for(i=0, j=0; /* Nothing */ ; i++)
  {
    c = seq[i];

    if(showcharnum)
    {
      fprintf(formatout, "  %*lu  ", (Fieldwidthtype) width, (Showuint) i+1); 
      showcharnum = False;
    }

    if(characters == NULL)
    {
      (void) putc((Fputcfirstargtype) c,formatout);
    } else
    {
      if(((Uint) c) >= mapsize - 1)
      {
        (void) putc((Fputcfirstargtype) wildcard,formatout);
      } 
      else
      {
        c = characters[c];
        if(((Uint) c) == undefsymbol)
        {
          ERROR1("undefined symbol %ld",(Showsint) seq[i]);
          return (Sint) -1;
        }
        (void) putc((Fputcfirstargtype) c,formatout);
      }
    }
    if(i == len - 1)
    {     
      (void) putc(NEWLINESYMBOL,formatout);
      break;
    }
    j++;
    if(j >= linewidth)
    {
      (void) putc(NEWLINESYMBOL,formatout);
      j = 0;
    }
  }
  return 0;
}

/*EE
  The following function shows a sequence for a given mapping
  given by table characters. Either the forward strand is output
  or the reverse strand.
*/

void echoexactmatch(BOOL forward,
                    FILE *outfp,
                    Uchar *characters,
                    Uint linewidth,
                    Uchar *s,
                    Uint slen)
{
  Uint len, linestart = 0, i;
  Uchar cc;

  while(True)
  {
    len = MIN(slen - linestart,linewidth);
    for(i = linestart; i < linestart+len; i++)
    {
      if(forward)
      {
        cc = s[i];
      } else
      {
        cc = (Uchar) 3 - s[i];
      }
      (void) putc((Fputcfirstargtype) characters[(Sint) cc],outfp);
    }
    linestart += len;
    if(i >= slen)
    {
      break;
    }
    (void) putc('\n',outfp);
  }
  (void) putc('\n',outfp);
}

/*EE
  The following function processes a multiple sequence and shows all
  sequences satisfying the given length ranges, defined by minlength
  and maxlength. minlength is defined iff minlengthdefined is True.
  maxlength is defined iff maxlengthdefined is True.
*/

Sint processsequence(Multiseq *multiseq,
                     BOOL minlengthdefined,
                     Uint minlength,
                     BOOL maxlengthdefined,
                     Uint maxlength,
                     Uint snum)
{
  Uint len;
  PairUint range;
  Uint desclength;

  if(findboundaries(multiseq,snum,&range) != 0)
  {
    return (Sint) -1;
  }
  len = range.uint1 - range.uint0 + 1;
  if((!minlengthdefined || len >= minlength) && 
     (!maxlengthdefined || len <= maxlength))
  {
    (void) putchar(FASTASEPARATOR);
    desclength = DESCRIPTIONLENGTH(multiseq,snum);
    if(WRITETOFILEHANDLE(DESCRIPTIONPTR(multiseq,snum),
                         (Uint) sizeof(Uchar),
                         desclength,
                         stdout) != 0)
    {
      ERROR1("Cannot write %lu items of type Uchar",
              (Showuint) desclength);
      return (Sint) -2;
    }
    if(formatseq(stdout,
                 0,
                 NULL,
                 0,
                 DEFAULTLINEWIDTH,
                 multiseq->originalsequence + range.uint0,
                 len) != 0)
    {
      return (Sint) -3;
    }
  } else
  {
    return 0;
  }
  return (Sint) len;
}

/* 
  The following structure aggregates the values needed to show a fasta file.
*/

typedef struct
{
  Alphabet *alpha;     // the alphabet
  Multiseq *multiseq;  // the multiple sequence structure
  Uint linewidth;      // the line width to show the alignment
  BOOL showseqnum;     // with out without the sequence number
  FILE *formatout;     // file pointer to show the alignment
} Fastaoutinfo;

/*
  The following function provides an example of a function to
  be used as the last argument of \texttt{overallsequences}.
  The function shows a sequence pointed to by \texttt{start} of length 
  \texttt{len} in fasta format. The number of the sequence is \texttt{seqnum}. 
  \texttt{info} is a \texttt{void} pointer to a structure of type 
  \texttt{Fastaoutinfo} to supply the function with the necessary global 
  information. If the corresponding component of the 
  \texttt{fastaoutinfo}-struct is set, then also the sequence number 
  is shown. 
*/

static Sint showfastasequence(void *info,Uint seqnum,Uchar *start,Uint len)
{
  Fastaoutinfo *fastaoutinfo = (Fastaoutinfo *) info;
  Uint desclength;

  (void) putc(FASTASEPARATOR,fastaoutinfo->formatout);
  if(fastaoutinfo->showseqnum)
  {
    fprintf(fastaoutinfo->formatout,"%lu ",(Showuint) seqnum);
  }
  // are there sequence descriptions
  if(fastaoutinfo->multiseq->descspace.spaceUchar != NULL)  
  {
    desclength = DESCRIPTIONLENGTH(fastaoutinfo->multiseq,seqnum);
    if(WRITETOFILEHANDLE(DESCRIPTIONPTR(fastaoutinfo->multiseq,seqnum),
                         (Uint) sizeof(Uchar),
                         desclength,
                         fastaoutinfo->formatout) != 0)
    {
      return (Sint) -1;
    }
  }
  if(formatseq(fastaoutinfo->formatout,
               fastaoutinfo->alpha->mapsize,
               fastaoutinfo->alpha->characters,
               fastaoutinfo->alpha->undefsymbol,fastaoutinfo->linewidth,
               start,len) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

/*EE
  The following function shows the \texttt{multiseq} in multiple fasta 
  format writing the output to the file pointer \texttt{formatout}. 
  \texttt{rcmode} is true, iff the reverse complemented sequence is
  shown. \texttt{linewidth} is the length of the line. The sequence is 
  over alphabet \texttt{alpha}. \texttt{showseqnum} is true, iff
  the sequence numbers are shown.
*/

Sint showmultiplefasta(FILE *formatout,BOOL rcmode,Uint linewidth,
                       Alphabet *alpha,Multiseq *multiseq,BOOL showseqnum)
{
  Fastaoutinfo fastaoutinfo;

  fastaoutinfo.linewidth = linewidth;
  fastaoutinfo.alpha = alpha;
  fastaoutinfo.multiseq = multiseq;
  fastaoutinfo.showseqnum = showseqnum;
  fastaoutinfo.formatout = formatout;
  if(overallsequences(rcmode,multiseq,&fastaoutinfo,showfastasequence) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

/*EE
  The following function reads a string of length 
  \texttt{inputlen} pointed to by \texttt{input} and stores
  the result in \texttt{multiseq}. The string is supposed to be in multiple 
  fasta format. Each sequence description begins with the symbol \texttt{>}. 
  If it does, then this symbol is skipped. The rest of the line is stored, if 
  \texttt{storedesc} is \texttt{True}. Otherwise, the rest of the line
  is discarded. The remaining lines (until the next symbol \texttt{>} or 
  the end of the input string) are scanned for alphanumeric
  characters which make up the sequence. White spaces are ignored.
  All other characters are checked, whether they are contained in 
  alphabet \texttt{alpha}. A valid character is transformed into 
  a number in the range \([0,l-1]\), where \(l\) is the size of 
  the alphabet. The input string must contain at least one 
  sequence, and all characters in the sequences must be valid w.r.t.\
  the given alphabet. Otherwise, an negative error code is returned.
  In case of success, the return code is 0.
*/

Sint readmultiplefastafile(Alphabet *alpha,BOOL storedesc,
                           Multiseq *multiseq,Uchar *input,Uint inputlen)
{
  Uchar acode, *inputptr, *newptr;
  Uint allocatedstartdesc = 0;
  BOOL indesc = False;

  initmultiseq(multiseq);
  newptr = multiseq->sequence = input;
  for(inputptr = input; inputptr < input + inputlen; inputptr++)
  {
    if(indesc)
    {
      if(storedesc)
      {
        STOREINARRAY(&multiseq->descspace,Uchar,4096,*inputptr);
      }
      if(*inputptr == NEWLINESYMBOL)
      {
        indesc = False;
      } 
    } else
    {
      if(*inputptr == FASTASEPARATOR)
      {
        if(storedesc)
        {
          STORESTARTDESC;
        }
        if(multiseq->numofsequences > 0)
        {
          STOREINARRAY(&multiseq->markpos,Uint,128,
                       (Uint) (newptr-multiseq->sequence));
          *newptr++ = SEPARATOR;
        }
        multiseq->numofsequences++;
        indesc = True;
      } else
      {
        if(!isspace((Ctypeargumenttype) *inputptr))
        {
          acode = (Uchar) alpha->symbolmap[(Sint) *inputptr];
          if(acode == (Uchar) alpha->undefsymbol)
          {
            ERROR1("illegal character '%c'",*inputptr);
            return (Sint) -1;
          }
          *newptr++ = acode;
        }
      }
    }
  }
  if(storedesc)
  {
    STORESTARTDESC;
  }
  if(multiseq->numofsequences == 0)
  {
    ERROR0("no sequences in multiple fasta file");
    return (Sint) -2;
  } 
  multiseq->totallength = (Uint) (newptr-multiseq->sequence);
  DEBUGCODE(2,checkmultiseq(multiseq));
  return 0;
}

Sint mapandparsesequencefile(Alphabet *alpha,
                             Multiseq *multiseq,
                             const char *filename)
{
  Uchar *seq;
  Uint seqlen;
  BOOL dnatype;
  Sint retcode;

  initmultiseqfileinfo(multiseq);
  multiseq->originalsequence = NULL;
  seq = (Uchar *) CREATEMEMORYMAP(filename,True,&seqlen);
  if(seq == NULL || seqlen == 0)
  {
    return (Sint) -1;
  }
  retcode = parseMultiformat(&dnatype,seq,seqlen);
  if(retcode < 0) // error occurred
  {
    return (Sint) -1;
  }
  if(retcode == 0)
  {
    ERROR1("input file \"%s\" is not in Fasta, GENBANK or EMBL format",
            filename);
    return (Sint) -2;
  }
  if(readmultiplefastafile(alpha,True,multiseq,seq,(Uint) retcode) != 0)
  {
    return (Sint) -1;
  }
  return (Sint) seqlen;
}

Sint mapandparsepossiblygzippedfastafile(Alphabet *alpha,
                                         Multiseq *multiseq,
                                         char *filename)
{
  BOOL isgzipped;
  Uchar *seq;
  Uint seqlen;

  initmultiseqfileinfo(multiseq);
  multiseq->originalsequence = NULL;
  isgzipped = checkgzipsuffix(filename);
  if(isgzipped)
  {
    seq = readgzippedfile(filename,&seqlen);
  } else
  {
    seq = (Uchar *) CREATEMEMORYMAP(filename,True,&seqlen);
  }
  if(seq == NULL || seqlen == 0)
  {
    return (Sint) -1;
  }
  printf("# reading%sfile \"%s\"\n",isgzipped ? " gzipped " : " ",filename);
  if(readmultiplefastafile(alpha,True,multiseq,seq,seqlen) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}


#ifdef WITHAGREP
/*
static BOOL checkagrepchar(Uchar *input,Uint inputlen)
{
  BOOL indesc = False;
  Uchar current, *inputptr;

  for(inputptr = input; inputptr < input + inputlen; inputptr++)
  {
    current = *inputptr;
    if(indesc)
    {
      if(current == NEWLINESYMBOL)
      {
        indesc = False;
      } 
    } else
    {
      if(current == FASTASEPARATOR)
      {
        if(inputptr > input && *(inputptr-1) != NEWLINESYMBOL)
        {
          return True;
        }
        indesc = True;
      } else
      {
        if(current == '[' ||
           current == ']' ||
           current == '#' ||
           current == '.' ||
           current == '^' ||
           current == '<')
        {
          return True;
        }
      }
    }
  }
  return False;
}
*/
#endif

/*EE
  The following function checks if there are query files in the index
  and at least one database files. Then the queryseparator positions
  is determined by the following function.
*/

Uint getqueryseppos(Multiseq *multiseq)
{
  if(!HASINDEXEDQUERIES(multiseq))
  {
    NOTSUPPOSED;
  }
  if(multiseq->totalnumoffiles <= multiseq->numofqueryfiles)
  {
    NOTSUPPOSED;
  }
  return multiseq->filesep[multiseq->totalnumoffiles-
                           multiseq->numofqueryfiles-
                           UintConst(1)];
}


/*EE
  The following function is a variation of \texttt{readmultiplefastafile}
  and reads a string \texttt{input} of length \texttt{inputlen}
  in multiple fasta format. The result is stored in 
  \texttt{multiseq}. In addition to \texttt{readmultiplefastafile},
  the function does the following:
  \begin{itemize}
  \item
  an array \texttt{filesep} is maintained. This contains the positions 
  separating two consecutive files.
  To achieve this, the function is supplied with the number
  \texttt{totalnumoffiles} of database files.
  \end{itemize}
*/

Sint readmultiplefastafilesep(Alphabet *alpha,
                              /*@unused@*/ BOOL checkmagic,
                              BOOL storedesc,
                              Multiseq *multiseq,
                              Uchar *input,
                              Uint inputlen)
{
  Uchar acode, current, *inputptr, *newptr;
  Uint pos,
       sepnum = 0, 
#ifdef WITHREGEXP
       keywordindex = 0, 
#endif
       allocatedstartdesc = 0, 
       linenum = UintConst(1);
  BOOL indesc = False, parsedmagic = False, lastwasspecial = False;

  initmultiseq(multiseq);
  newptr = multiseq->sequence = input;
  for(inputptr = input; inputptr < input + inputlen; inputptr++)
  {
    current = *inputptr;
    if(indesc)
    {
      if(storedesc)
      {
        STOREINARRAY(&multiseq->descspace,Uchar,4096,current);
      }
      if(current == NEWLINESYMBOL)
      {
        linenum++;
        indesc = False;
#ifdef WITHREGEXP
        keywordindex = 0;
        if(checkmagic)
        {
          STOREINARRAY(&multiseq->searchregexp,Uchar,128,parsedmagic);
        }
#endif
      } 
#ifdef WITHREGEXP
      if(checkmagic && !parsedmagic)
      {
        if(current == (Uchar) "REGEXP"[keywordindex])
        {
          if(keywordindex == UintConst(5))  // strlen("REGEXP") - 1
          {
            parsedmagic = True;
          } else
          {
            keywordindex++;
          }
        } else
        {
          keywordindex = 0;
        }
      }
#endif
    } else
    {
      if(current == FASTASEPARATOR)
      {
        if(inputptr > input && *(inputptr-1) != NEWLINESYMBOL)
        {
          // '>' occurs not at beginning of line
          if(!parsedmagic)
          {
            ERROR3("Illegal character '%c' in file \"%s\" line %lu",
                    current,
                    multiseq->allfiles[sepnum].filenamebuf,
                    (Showuint) linenum);
            return (Sint) -1;
          }
        } else
        {
          // '>' occurs at beginning of line, so it is a fasta header
          if(storedesc)
          {
            STORESTARTDESC;
          }
          if(multiseq->numofsequences > 0)
          {
            if(multiseq->filesep[sepnum] == (Uint) (inputptr - input))
            {
              multiseq->filesep[sepnum++] = (Uint) (newptr-multiseq->sequence);
              DEBUG2(2,"set filesep[%lu]=%lu\n",
                         (Showuint) (sepnum-1),
                         (Showuint) multiseq->filesep[sepnum-1]);
              linenum = 0;
            }
            STOREINARRAY(&multiseq->markpos,Uint,128,
                         (Uint) (newptr-multiseq->sequence));
            if(!lastwasspecial)
            {
              multiseq->specialcharinfo.specialranges++;  // namely a first SEPARATOR
            }
            multiseq->specialcharinfo.specialcharacters++; // namely a SEPARATOR
            lastwasspecial = True;
            *newptr++ = SEPARATOR;
          }
          parsedmagic = False;
          multiseq->numofsequences++;
          indesc = True;
        }
      } else
      {
        if(isspace((Ctypeargumenttype) current))
        {
          if(current == NEWLINESYMBOL)
          {
            linenum++;
          }
        } else
        {
          if(parsedmagic)
          {
            *newptr++ = current;
          } else
          {
            acode = (Uchar) alpha->symbolmap[(Sint) current];
            if(acode == (Uchar) alpha->undefsymbol)
            {
              ERROR3("Illegal character '%c' in file \"%s\" line %lu",
                      current,
                      multiseq->allfiles[sepnum].filenamebuf,
                      (Showuint) linenum);
              return (Sint) -1;
            }
            if(ISSPECIAL(acode))
            {
              if(!lastwasspecial)
              {
                multiseq->specialcharinfo.specialranges++;  // namely a first SEPARATOR
              }
              multiseq->specialcharinfo.specialcharacters++; // namely a WILDCARD
              lastwasspecial = True;
            } else
            {
              lastwasspecial = False;
            }
            *newptr++ = acode;
          } 
        }
      }
    }
  }
  if(storedesc)
  {
    STORESTARTDESC;
  }
  if(multiseq->numofsequences == 0)
  {
    ERROR0("no sequences in multiple fasta file");
    return (Sint) -2;
  } 
  multiseq->totallength = (Uint) (newptr-multiseq->sequence);
  for(pos = 0; pos < multiseq->totallength &&
               ISSPECIAL(multiseq->sequence[pos]); pos++)
  {
    multiseq->specialcharinfo.lengthofspecialprefix++;
  }
  for(pos = multiseq->totallength-1; ISSPECIAL(multiseq->sequence[pos]); pos--)
  {
    multiseq->specialcharinfo.lengthofspecialsuffix++;
    if(pos == 0)
    {
      break;
    }
  }
  if(multiseq->numofsequences >= UintConst(2))
  {
    Uint markposindex;

    for(markposindex = UintConst(1); 
        markposindex < multiseq->numofsequences-1; 
        markposindex++)
    {
      if(multiseq->markpos.spaceUint[markposindex-1] + 1 == 
         multiseq->markpos.spaceUint[markposindex])
      {
        ERROR1("sequence %lu is empty",(Showuint) markposindex-1);
        return (Sint) -3;
      }
    }
    if(multiseq->markpos.spaceUint[0] == 0)
    {
      ERROR0("sequence 0 is empty");
      return (Sint) -3;
    }
    if(multiseq->markpos.spaceUint[multiseq->numofsequences-2] == 
       multiseq->totallength - 1)
    {
      ERROR1("sequence %lu is empty",(Showuint) multiseq->numofsequences-1);
      return (Sint) -3;
    }
  }
  if(HASINDEXEDQUERIES(multiseq))
  {
    Uint queryseppos = getqueryseppos(multiseq);
    Sint retcode; 

    multiseq->totalquerylength = multiseq->totallength -
                                 queryseppos -
                                 UintConst(1);
    retcode = getseqnum(multiseq,queryseppos+1);
    if(retcode < 0)
    {
      return (Sint) -2;
    }
    multiseq->numofquerysequences 
      = multiseq->numofsequences - (Uint) retcode;
  }
  DEBUGCODE(2,checkmultiseq(multiseq));
  return 0;
}

/*EE
  The following function reads in just the sequence part of a multiple
  fasta file. It does not store the positions of the marking symbols and 
  discards the sequence descriptions. Moreover, the input sequence
  is not transformed. It is assumed that the file is in fasta format.
  No check for this is performed. 
*/

void readmultiplefastafileagain(Uchar *newseq,Uchar *input,Uint inputlen)
{
  Uchar *inputptr, *newptr = newseq;
  BOOL indesc = False;

  for(inputptr = input; inputptr < input + inputlen; inputptr++)
  {
    if(indesc)
    {
      if(*inputptr == NEWLINESYMBOL)
      {
        indesc = False;
      }
    } else
    {
      if(*inputptr == FASTASEPARATOR)
      {
        if(inputptr > input)
        {
          *newptr++ = SEPARATOR;
        }
        indesc = True;
      } else
      {
        if(!isspace((Ctypeargumenttype) *inputptr))
        {
          *newptr++ = *inputptr;
        }
      }
    }
  }
}

/*@null@*/ static Uchar *justopenthefile(char *filename,Uint *filelength)
{
  if(checkgzipsuffix(filename))
  {
    return readgzippedfile(filename,filelength);
  } else
  {
    return CREATEMEMORYMAP(filename,False,filelength);
  }
}

/*
  The following function opens the file given by name \texttt{filename}.
  If an error occurs, the return value is -1. It checks if the file
  is in FASTA-format (i.e. the first non-white space character is $>$).
  It determined how many initial white-space characters at the beginning
  of the file can be skipped. Moreover, it determines the filelength.
*/

static Sint openandcheckfile(char *filename,
                             Uchar **seqptrtab,
                             BOOL *fastaformat,
                             Uint *skipemptyprefix,
                             Uint *filelength)
{
  BOOL isfasta = False;
  Uint i, tmpskip = 0;
  Uchar cc;

  *seqptrtab = justopenthefile(filename,filelength);
  if(*seqptrtab == NULL)
  {
    return (Sint) -1;
  }
  for(i=0; i<*filelength; i++)
  {
    cc = (*seqptrtab)[i];
    if(isspace(cc))
    {
      tmpskip++;
    } else
    {
      isfasta = (cc == FASTASEPARATOR) ? True : False;
      break;
    }
  }
  *fastaformat = isfasta;
  *skipemptyprefix = tmpskip;
  return 0;
}
  
/*EE
  The following function concatenates the contents of
  several files in a block of consecutive memory cells
  and returns a pointer to the block. If an error occurs, the function 
  returns \texttt{NULL}.
  \texttt{totalnumoffiles} is the number of input files. The filenames
  are supposed to be stored in \texttt{allfiles}. The remaining 
  \texttt{filelength} component in \texttt{allfiles} and the
  separator positions in \texttt{filesep} are set. 
  The total length of the concatenated files including the possibly
  inserted extra lines is written into textlen.
  If the file does not begin with the symbol \texttt{>} (i.e.\ it is not a 
  file in FASTA-Format), then a line \texttt{>filename} 
  is prepended to the contents of the file.
*/

/*@null@*/ Uchar *concatmanyfiles(Showverbose showverbose,
                                  Uint totalnumoffiles,
                                  Fileinfo *allfiles,
                                  Uint *filesep,
                                  Uint *totalbytes)
{ 
  Sint retcode;
  Uint fnum, bytes2concat = 0, *skipemptyprefix, filelength;
  size_t filenamelength2show;
  Uchar *concatenation, *mappedfile;
  BOOL *fastaformattab;

  if(totalnumoffiles <= UintConst(1))
  {
    ERROR0("concatmanyfiles wants to read at least two files");
    return NULL;
  }
  ALLOCASSIGNSPACE(fastaformattab,NULL,BOOL,totalnumoffiles);
  ALLOCASSIGNSPACE(skipemptyprefix,NULL,Uint,totalnumoffiles);
  for(fnum=0; fnum<totalnumoffiles; fnum++)
  {
    retcode = openandcheckfile(allfiles[fnum].filenamebuf,
                               &mappedfile,
                               &fastaformattab[fnum],
                               &skipemptyprefix[fnum],
                               &allfiles[fnum].filelength);
    if(retcode < 0)
    {
      return NULL;
    }
    bytes2concat += (allfiles[fnum].filelength - skipemptyprefix[fnum]);
    if(!fastaformattab[fnum])
    {
      bytes2concat += 3 + strlen(allfiles[fnum].filenamebuf);
    }
    if(DELETEMEMORYMAP(mappedfile) != 0)
    {
      FREESPACE(mappedfile);
    }
  }
  ALLOCASSIGNSPACE(concatenation,NULL,Uchar,bytes2concat);
  bytes2concat = 0;
  for(fnum=0; fnum<totalnumoffiles; fnum++)
  {
    mappedfile = justopenthefile(allfiles[fnum].filenamebuf,&filelength);
    if(mappedfile == NULL)
    {
      return NULL;
    }
    if(fnum > 0 && filesep != NULL)
    {
      filesep[fnum-1] = bytes2concat;
    }
    if(!fastaformattab[fnum])
    {
      filenamelength2show = strlen(allfiles[fnum].filenamebuf);
      if(checkgzipsuffix(allfiles[fnum].filenamebuf))
      {
        filenamelength2show -= 3; // for the suffix .gz at the end
      } 
      printf(">%*.*s\n",
              (Fieldwidthtype) filenamelength2show,
              (Fieldwidthtype) filenamelength2show,
              allfiles[fnum].filenamebuf);
      sprintf((char *) (concatenation+bytes2concat),">%*.*s\n",
              (Fieldwidthtype) filenamelength2show,
              (Fieldwidthtype) filenamelength2show,
              allfiles[fnum].filenamebuf);
      bytes2concat += 2 + (Uint) filenamelength2show;
    }
    if(showverbose != NULL)
    {
      char sbuf[PATH_MAX+50+1];
      sprintf(sbuf,"reading file \"%s\"",allfiles[fnum].filenamebuf);
      showverbose(sbuf);
    }
    memcpy((void *) (concatenation+bytes2concat), 
           mappedfile + skipemptyprefix[fnum],
           (size_t) (allfiles[fnum].filelength - skipemptyprefix[fnum]));
    bytes2concat += (allfiles[fnum].filelength - skipemptyprefix[fnum]);
    if(!fastaformattab[fnum])
    {
      concatenation[bytes2concat++] = (Uchar) NEWLINESYMBOL;
    }
    if(DELETEMEMORYMAP(mappedfile) != 0)
    {
      FREESPACE(mappedfile);
    }
  }
  FREESPACE(skipemptyprefix);
  FREESPACE(fastaformattab);
  *totalbytes = bytes2concat;
  return concatenation;
}

/*EE
  The following function shows the description for sequence number
  \texttt{seqnum} w.r.t.\ the multiple sequence \texttt{multiseq}.
  The parameter \texttt{showdesc} defines the format of the sequence
  description.
*/

void echothedescription(FILE *outfp,Showdescinfo *showdesc,Multiseq *multiseq,
                        Uint seqnum)
{
  Uint i, len;
  Uchar *desc;

  if(!showdesc->defined)
  {
    fprintf(stderr,"programming error: format specification for "
                   "description is undefined\n");
    exit(EXIT_FAILURE);
  }
  len = DESCRIPTIONLENGTH(multiseq,seqnum)-1;
  if(showdesc->maxlength > 0 && 
     showdesc->maxlength + showdesc->skipprefix < len)
  {
    len = showdesc->maxlength + showdesc->skipprefix;
  }
  desc = DESCRIPTIONPTR(multiseq,seqnum);
  for(i=showdesc->skipprefix; i<len; i++)
  {
    if(isspace((Ctypeargumenttype) desc[i]))
    {
      if(showdesc->untilfirstblank)
      {
        break;
      }
      if(showdesc->replaceblanks)
      {
        (void) putc('_',outfp);
      } else
      {
        (void) putc((Fputcfirstargtype) desc[i],outfp);
      }
    } else
    {
      (void) putc((Fputcfirstargtype) desc[i],outfp);
    }
  }
}

/*
  The following function is to supplied as an argument to
  \texttt{overallsequences}. It computes the length distribution.
*/

static Sint addtothedistri(void *distri, 
                           /*@unused@*/ Uint seqnum,
                           /*@unused@*/ Uchar *start,
                           Uint len)
{
  adddistribution((ArrayUint *) distri,len);
  return 0;
}

/*EE
  The following function computes the length distribution of the
  sequences in \texttt{multiseq} and shows the result on
  \texttt{formatout}. If an error occurs, the function returns
  a negative error code. Otherwise, the error code is 0.
*/

Sint lengthdistribution(FILE *formatout,Multiseq *multiseq)
{
  ArrayUint dist;
  Uint i;
  
  INITARRAY(&dist,Uint);
  if(overallsequences(False,multiseq,&dist,addtothedistri) != 0)
  {
    return (Sint) -1;
  }
  NOTSUPPOSEDTOBENULL(dist.spaceUint);
  for(i=0; i<dist.nextfreeUint; i++)
  {
    if(dist.spaceUint[i] > 0)
    {
      fprintf(formatout,"%lu %lu\n",(Showuint) i,(Showuint) dist.spaceUint[i]);
    }
  }
  FREEARRAY(&dist,Uint);
  return 0;
}

/*EE
  The following function calculates the minimum length, the maximum
  length and the average length of the sequences in the multiple
  sequence. If something went wrong, then an negative error code is 
  returned.
*/

Sint calculateseqparm(Multiseq *multiseq,
                      ExtremeAverageSequences *extreme)
{
  Uint snum, seqlen, seqlensum = 0;
  PairUint range;

  extreme->minlengthseqnum = multiseq->numofsequences;  // undefined
  extreme->maxlengthseqnum = multiseq->numofsequences;  // undefined
  extreme->minlength = multiseq->totallength;  // undefined
  extreme->maxlength = 0;
  for(snum=0; snum<multiseq->numofsequences; snum++)
  {
    if(findboundaries(multiseq,snum,&range) != 0)
    {
      return (Sint) -1;
    }
    seqlen = range.uint1 - range.uint0 + 1;
    if(seqlen > extreme->maxlength)
    {
      extreme->maxlength = seqlen;
      extreme->maxlengthseqnum = snum;
    }
    if(seqlen < extreme->minlength)
    {
      extreme->minlength = seqlen;
      extreme->minlengthseqnum = snum;
    }
    seqlensum += seqlen;
  }
  extreme->averageseqlen = (double) seqlensum / multiseq->numofsequences;
  return 0;
}

/*EE
  The following function calculates the length of two sequences
  in the multiple sequence which are of maximal lengths. If there are less 
  than two sequences, then an negative error code is returned.
*/

Sint calculatetwolongestseqlength(Multiseq *multiseq,Uint *maxlength1,
                                  Uint *maxlength2)
{
  Uint i;
  ArrayUint storelengthdistribution;

  if(multiseq->numofsequences < UintConst(2))
  {
    ERROR0("multiple sequence contains less than two sequences");
    return (Sint) -2;
  }
  INITARRAY(&storelengthdistribution,Uint);
  if(overallsequences(False,multiseq,(void *) &storelengthdistribution,
                      addtothedistri) != 0)
  {
    return (Sint) -1;
  }
  if(storelengthdistribution.nextfreeUint == 0)
  {
    ERROR0("multiple sequence only contains no sequence");
    return (Sint) -2;
  }
  *maxlength2 = storelengthdistribution.nextfreeUint-1;
  NOTSUPPOSEDTOBENULL(storelengthdistribution.spaceUint);
  if(storelengthdistribution.spaceUint[storelengthdistribution.nextfreeUint-1] 
     >= UintConst(2))
  {
    *maxlength1 = storelengthdistribution.nextfreeUint-1;
  } else
  {
    for(i=storelengthdistribution.nextfreeUint-2; i>0; i--)
    { 
      if(storelengthdistribution.spaceUint[i] > 0)
      { 
        *maxlength1 = i;
        break;
      }
    }
    if(i == 0)
    {
      ERROR0("multiple sequence only contains one sequence");
      return (Sint) -3;
    }
  }
  FREEARRAY(&storelengthdistribution,Uint);
  return 0;
}

/*EE
  The following function shows for all sequences of a multiseq
  the corresponding description the minimum length, the maximum
  length and the average length of the sequences in the multiple
  sequence. If something went wrong, then an negative error code is 
  returned.
*/

Sint showdescandlength(Multiseq *multiseq)
{
  Uint snum;
  PairUint range;
  Uint desclength;

  if(multiseq->descspace.spaceUchar == NULL)
  {
    for(snum=0; snum<multiseq->numofsequences; snum++)
    {
      if(findboundaries(multiseq,snum,&range) != 0)
      {
        return (Sint) -1;
      }
      printf("%lu %lu\n",(Showuint) snum,
                         (Showuint) (range.uint1 - range.uint0 + 1));
    }
  } else
  {
    for(snum=0; snum<multiseq->numofsequences; snum++)
    {
      if(findboundaries(multiseq,snum,&range) != 0)
      {
        return (Sint) -2;
      }
      printf("%lu %lu ",(Showuint) snum,
                        (Showuint) (range.uint1 - range.uint0 + 1));
      desclength = DESCRIPTIONLENGTH(multiseq,snum);
      if(WRITETOFILEHANDLE(DESCRIPTIONPTR(multiseq,snum),
                           (Uint) sizeof(Uchar),
                           desclength,
                           stdout) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  return 0;
}

/*EE
  The following function copies the \texttt{multiseq2} to \texttt{multiseq1}.
*/

void copyMultiseq(Multiseq *multiseq1,Multiseq *multiseq2)
{
  *multiseq1 = *multiseq2;
}

/*EE
  The following function copies the filename buffer of 
  \texttt{multiseqsrc} to \texttt{multiseqdest}.
*/

void copyfilenamesofMultiseq(Multiseq *multiseqdest,Multiseq *multiseqsrc)
{
  Uint i;

  for(i=0; i<multiseqsrc->totalnumoffiles; i++)
  {
    multiseqdest->allfiles[i].filelength = multiseqsrc->allfiles[i].filelength;
    ASSIGNDYNAMICSTRDUP(multiseqdest->allfiles[i].filenamebuf,
                        multiseqsrc->allfiles[i].filenamebuf);
  }
}

/*
  The following functions parse the project file which ends with the
  PRJSUFFIX.
*/

static Sint parsemultiseqprojectfilefromfileptr(Multiseq *multiseq,
                                                DefinedUint *longestptr,
                                                Uint *prefixlengthptr,
                                                ArrayPairUint *largelcpvalues,
                                                Uint *maxbranchdepthptr,
                                                BOOL *rcmindex,
                                                Uint *sixframeindex,
                                                FILE *fp,
                                                const char *indexname)
{
  Uint i, j, sep, readnumofdbsequences, maxline;
  Scaninteger readint1, readint2, integersize;
  char *tmpfilename;

  maxline = maximumlinelength(fp);
  rewind(fp);
  ALLOCASSIGNSPACE(tmpfilename,NULL,char,maxline+1);
  for(i=0; fscanf(fp,"dbfile=%s %ld %ld\n",
                   tmpfilename,
                   &readint1,
                   &readint2) == 3; i++)
  {
    multiseq->allfiles[i].filelength = (Uint) readint1;
    multiseq->filesep[i] = (Uint) readint2;
    ASSIGNDYNAMICSTRDUP(multiseq->allfiles[i].filenamebuf,tmpfilename);
  }
  for(j=0; fscanf(fp,"queryfile=%s %ld %ld\n",
             tmpfilename,
             &readint1,
             &readint2) == 3; j++)
  {
    multiseq->allfiles[i+j].filelength = (Uint) readint1;
    multiseq->filesep[i+j] = (Uint) readint2;
    ASSIGNDYNAMICSTRDUP(multiseq->allfiles[i+j].filenamebuf,tmpfilename);
  }
  FREESPACE(tmpfilename);
  multiseq->totalnumoffiles = i+j;
  multiseq->numofqueryfiles = j;
  sep = 0;
  for(i=0; i < multiseq->totalnumoffiles; i++)
  {
    sep += multiseq->filesep[i];
    multiseq->filesep[i] = sep++;
  }
  if(fscanf(fp,"totallength=%ld\n",&readint1) != 1)
  {
    PRJERROR(totallength);
    return (Sint) -2;
  }
  multiseq->totallength = (Uint) readint1;
  if(fscanf(fp,"specialcharacters=%ld\n",&readint1) != 1)
  {
    PRJERROR(specialcharacters);
    return (Sint) -5;
  }
  multiseq->specialcharinfo.specialcharacters = (Uint) readint1;
  if(fscanf(fp,"specialranges=%ld\n",&readint1) != 1)
  {
    PRJERROR(specialranges);
    return (Sint) -5;
  }
  multiseq->specialcharinfo.specialranges = (Uint) readint1;
  if(fscanf(fp,"lengthofspecialprefix=%ld\n",&readint1) != 1)
  {
    PRJERROR(lengthofspecialprefix);
    return (Sint) -5;
  }
  multiseq->specialcharinfo.lengthofspecialprefix = (Uint) readint1;
  if(fscanf(fp,"lengthofspecialsuffix=%ld\n",&readint1) != 1)
  {
     PRJERROR(lengthofspecialsuffix);
     return (Sint) -5;
  }
  multiseq->specialcharinfo.lengthofspecialsuffix = (Uint) readint1;
  if(fscanf(fp,"numofsequences=%ld\n",&readint1) != 1)
  {
    PRJERROR(numofsequences);
    return (Sint) -3;
  } 
  multiseq->numofsequences = (Uint) readint1;
  if(fscanf(fp,"numofdbsequences=%ld\n",&readint1) != 1)
  {
    PRJERROR(numofdbsequences);
    return (Sint) -4;
  } 
  readnumofdbsequences = (Uint) readint1;
  if(fscanf(fp,"numofquerysequences=%ld\n",&readint1) != 1)
  {
    PRJERROR(numofquerysequences);
    return (Sint) -5;
  }
  multiseq->numofquerysequences = (Uint) readint1;
  if(NUMOFDATABASESEQUENCES(multiseq) != readnumofdbsequences)
  {
    ERROR3("inconsistent number of sequences: "
           "dbseq=%lu, queryseq=%lu, total=%lu",
            (Showuint) readnumofdbsequences,
            (Showuint) multiseq->numofquerysequences,
            (Showuint) multiseq->numofsequences);
    return (Sint) -6;
  }
  if(HASINDEXEDQUERIES(multiseq))
  {
    multiseq->totalquerylength
      = multiseq->totallength - 
        getqueryseppos(multiseq)
        - UintConst(1);
  }
  if(fscanf(fp,"longest=%ld\n",&readint1) != 1)
  {
    longestptr->defined = False;
  } else
  {
    longestptr->defined = True;
    longestptr->uintvalue = (Uint) readint1;
  }
  if(fscanf(fp,"prefixlength=%ld\n",&readint1) != 1)
  {
    PRJERROR(prefixlength);
    return (Sint) -7;
  }
  *prefixlengthptr = (Uint) readint1;
  if(fscanf(fp,"largelcpvalues=%ld\n",&readint1) == 1)
  {
    largelcpvalues->nextfreePairUint = (Uint) readint1;
  } else
  {
    largelcpvalues->nextfreePairUint = 0;
  }
  largelcpvalues->spacePairUint = NULL;
  if(fscanf(fp,"maxbranchdepth=%ld\n",&readint1) == 1)
  {
    *maxbranchdepthptr = (Uint) readint1;
  } else
  {
    *maxbranchdepthptr = 0;
  }
  if(fscanf(fp,"integersize=%ld\n",&integersize) != 1)
  {
    integersize = (Scaninteger) 32;
  } else
  {
    if(integersize != (Scaninteger) 32 && integersize != (Scaninteger) 64)
    {
      ERROR2("%s.%s contains illegal line defining the integer size",
               indexname,PROJECTFILESUFFIX);
      return (Sint) -8;
    }
  }
  if(integersize != (Scaninteger) (sizeof(Uint) * CHAR_BIT))
  {
    ERROR2("index was generated for %lu-bit integers while "
           "this program uses %lu-bit integers",
           (Showuint) integersize,
           (Showuint) (sizeof(Uint) * CHAR_BIT));
    return (Sint) -9;
  }
  if(fscanf(fp,"littleendian=%ld\n",&readint1) != 1)
  {
    ERROR2("%s.%s contains illegal line defining the endianness",
               indexname,PROJECTFILESUFFIX);
    return (Sint) -8;
  }
  if(islittleendian())
  {
    if(readint1 != (Scaninteger) 1)
    {
      ERROR0("computer has little endian byte order, while index was build on "
             "computer with big endian byte order");
      return (Sint) -9;
    }
  } else
  {
    if(readint1 == (Scaninteger) 1)
    {
      ERROR0("computer has big endian byte order, while index was build on "
             "computer with little endian byte order");
      return (Sint) -10;
    }
  }
  if(fscanf(fp,"specialindex=%ld\n",&readint1) == 1)
  {
    if(readint1 == 0)
    {
      *rcmindex = True;
    } else
    {
      if(readint1 >= (Scaninteger) 1)
      {
        *sixframeindex = (Uint) readint1;
      } else
      {
        ERROR2("%s.%s contains illegal line defining special index",
               indexname,PROJECTFILESUFFIX);
        return (Sint) -10;
      }
    }
  }
  return 0;
}

/*EE
  The following function opens a file of the virtual index file.
  If this is successfull, then the file pointer to
  the file is returned. If the latter file does not exist, 
  then \texttt{NULL} is returned.
*/

/*@null@*/ static FILE *fileopenvstreetabfile(const char *indexname,
                                              const char *suffix)
{
  FILE *fp;
  char *tmpfilename = COMPOSEFILENAME(indexname,suffix);

  fp = CREATEFILEHANDLE(tmpfilename,READMODE);
  FREESPACE(tmpfilename);
  return fp;
}

/*EE
  The following function tries to close a virtual index file.
  If this this is not successful, then a corresponding error
  message is returned.
*/

Sint parseprojectfile(Multiseq *multiseq,
                      DefinedUint *longestptr,
                      Uint *prefixlengthptr,
                      ArrayPairUint *largelcpvalues,
                      Uint *maxbranchdepthptr,
                      BOOL *rcmindex,
                      Uint *sixframeindex,
                      const char *indexname)
{
  FILE *fp;

  initmultiseq(multiseq);
  multiseq->originalsequence = NULL;
  fp = fileopenvstreetabfile(indexname,PROJECTFILESUFFIX);
  if(fp == NULL)
  {
    return (Sint) -1;
  }
  if(parsemultiseqprojectfilefromfileptr(multiseq,
                                         longestptr,
                                         prefixlengthptr,
                                         largelcpvalues,
                                         maxbranchdepthptr,
                                         rcmindex,
                                         sixframeindex,
                                         fp,
                                         indexname) != 0)
  {
    return (Sint) -2;
  }
  if(DELETEFILEHANDLE(fp) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

/*@null@*/ Uchar *createtistabmap(Uint totallength,const char *indexname)
{
  Uint numofbytes;
  __attribute__ ((unused)) Uint indexsize = 0;
  Uchar *sequence;
  char *tmpfilename;

  tmpfilename = COMPOSEFILENAME(indexname,"tis");
  sequence = (Uchar *) CREATEMEMORYMAP(tmpfilename,False,&numofbytes);
  if(sequence == NULL)
  {
    return NULL;
  }
  EXPECTED(indexsize,numofbytes,tmpfilename,
           totallength * (Uint) sizeof(Uchar),NULL);
  FREESPACE(tmpfilename);
  return sequence;
}

Sint mapmultiseqifyoucan(Uint *indexsize,
                         Multiseq *multiseq,
                         const char *indexname,
                         BOOL demandTISTAB,
                         BOOL demandOISTAB,
                         BOOL demandDESTAB,
                         BOOL demandSSPTAB)
{
  Uint numofbytes;
  char *tmpfilename;

  if(demandTISTAB)
  {
    multiseq->sequence = createtistabmap(multiseq->totallength,indexname);
    (*indexsize) += multiseq->totallength * (Uint) sizeof(Uchar);
    IFNULLRETURN(multiseq->sequence);
  }
  if(demandOISTAB)
  {
    tmpfilename = COMPOSEFILENAME(indexname,"ois");
    multiseq->originalsequence
      = (Uchar *) CREATEMEMORYMAP(tmpfilename,False,&numofbytes);
    IFNULLRETURN(multiseq->originalsequence);
    EXPECTED(*indexsize,numofbytes,tmpfilename,
             multiseq->totallength * (Uint) sizeof(Uchar),(Sint) -1);
    FREESPACE(tmpfilename);
  }
  if(demandSSPTAB || demandTISTAB)
  {
    if(multiseq->numofsequences == UintConst(1))
    {
      multiseq->markpos.spaceUint = NULL;
      multiseq->markpos.allocatedUint = multiseq->markpos.nextfreeUint = 0;
    } else
    {  // GENERIC
      tmpfilename = COMPOSEFILENAME(indexname,"ssp");
      multiseq->markpos.spaceUint 
        = (Uint *) CREATEMEMORYMAP(tmpfilename,False,&numofbytes);
      IFNULLRETURN(multiseq->markpos.spaceUint);
      EXPECTED(*indexsize,numofbytes,tmpfilename,
               (multiseq->numofsequences-1) * (Uint) sizeof(Uint),(Sint) -1);
      multiseq->markpos.allocatedUint
        = multiseq->markpos.nextfreeUint 
            = multiseq->numofsequences-1;
      FREESPACE(tmpfilename);
    }
  }
  if(demandDESTAB)
  {
    tmpfilename = COMPOSEFILENAME(indexname,"des");
    multiseq->descspace.spaceUchar 
      = (Uchar *) CREATEMEMORYMAP(tmpfilename,True,&numofbytes);
    IFNULLRETURN(multiseq->descspace.spaceUchar);
    // XXX the following line was added as suggested by robert Homann
    multiseq->descspace.nextfreeUchar = numofbytes; 
    FREESPACE(tmpfilename);
    tmpfilename = COMPOSEFILENAME(indexname,"sds");
    multiseq->startdesc
      = (Uint *) CREATEMEMORYMAP(tmpfilename,True,&numofbytes);
    IFNULLRETURN(multiseq->startdesc);
    EXPECTED(*indexsize,numofbytes,tmpfilename,
             (multiseq->numofsequences+1) * (Uint) sizeof(Uint),(Sint) -1);
    FREESPACE(tmpfilename);
  } else
  {
    multiseq->descspace.spaceUchar = NULL;
  }
  return 0;
}

//\Ignore{

#ifdef DEBUG

#define COMPAREMULTISEQINT(INT)\
        if(multiseq1->INT != multiseq2->INT)\
        {\
          fprintf(stderr,"%s.%s: %lu != %lu\n","comparemultiseq",#INT,\
                  (Showuint) multiseq1->INT,(Showuint) multiseq2->INT);\
          exit(EXIT_FAILURE);\
        }\
        printf("# %s.%s=%lu\n","comparemultiseq",#INT,(Showuint) multiseq1->INT)

void compareFileinfo(char *tag,Fileinfo *tab1,Fileinfo *tab2,Uint len)
{
  Uint i;

  for(i=0; i < len; i++)
  {
    if(strcmp(tab1[i].filenamebuf,tab2[i].filenamebuf) != 0)
    {
      fprintf(stderr,"(%s.uint0)[%lu]: \"%s\" != \"%s\"\n",tag,(Showuint) i,
                      tab1[i].filenamebuf,tab2[i].filenamebuf);
      exit(EXIT_FAILURE);
    }
    if(tab1[i].filelength != tab2[i].filelength)
    {
      fprintf(stderr,"(%s.uint1)[%lu]: %lu != %lu\n",tag,
                      (Showuint) i,
                      (Showuint) tab1[i].filelength,
                      (Showuint) tab2[i].filelength);
      exit(EXIT_FAILURE);
    }
  }
  printf("# %s of length %lu identical\n",tag,(Showuint) len);
}

void compareMultiseq(Multiseq *multiseq1,Multiseq *multiseq2)
{
  COMPAREMULTISEQINT(totallength);
  compareUchartab("comparemultiseq.sequence",multiseq1->sequence,
                  multiseq2->sequence,multiseq1->totallength);
  COMPAREMULTISEQINT(numofsequences);
  if(multiseq1->numofsequences > UintConst(1))
  {
    compareUinttab("comparemultiseq.markpos",multiseq1->markpos.spaceUint,
                    multiseq2->markpos.spaceUint,multiseq1->numofsequences-1);
  }
  if(multiseq1->startdesc != NULL && multiseq2->startdesc != NULL)
  {
    compareUinttab("comparemultiseq.startdesc",multiseq1->startdesc,
                    multiseq2->startdesc,multiseq1->numofsequences+1);
    compareUchartab("comparemultiseq.descspace",multiseq1->descspace.spaceUchar,
                    multiseq2->descspace.spaceUchar,
                    multiseq1->startdesc[multiseq1->numofsequences]-1);
  }
  COMPAREMULTISEQINT(totalnumoffiles);
  COMPAREMULTISEQINT(numofqueryfiles);
  COMPAREMULTISEQINT(numofquerysequences);
  COMPAREMULTISEQINT(totalquerylength);
  if(multiseq1->totalnumoffiles > 0)
  {
    compareUinttab("comparemultiseq.filesep",
                   &multiseq1->filesep[0],
                   &multiseq2->filesep[0],
                   multiseq1->totalnumoffiles-1);
    compareFileinfo("comparemultiseq.fileinfo",
                    &multiseq1->allfiles[0],
                    &multiseq2->allfiles[0],
                    multiseq1->totalnumoffiles);
  }
  printf("# compareMultiseq okay\n");
}
#endif

//}
