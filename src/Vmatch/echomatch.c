
//\Ignore{

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "types.h"
#include "minmax.h"
#include "multidef.h"
#include "errordef.h"
#include "debugdef.h"
#include "match.h"
#include "codondef.h"
#include "inputsymbol.h"
#include "iubdef.h"
#include "spacedef.h"
#include "fhandledef.h"
#include "alignment.h"
#include "frontdef.h"
#include "alphadef.h"
#include "outinfo.h"
#include "scoredef.h"
#include "genfile.h"
#include "galigndef.h"

#include "multiseq-adv.pr"
#include "xdropal2.pr"
#include "galign.pr"
#include "showalign.pr"
#include "filehandle.pr"
#include "readmulti.pr"
#include "matsort.pr"

#include "lrseq.pr"
#include "xmlfunc.pr"

//}

#define PUTONE(C) (void) putc((Fputcfirstargtype) (C),outfp);\
                  thislinelength++;\
                  if(thislinelength >= linewidth)\
                  {\
                    (void) putc('\n',outfp);\
                    thislinelength = 0;\
                  }

#define ERRORON\
        if(!errormode)\
        {\
          (void) putc((Fputcfirstargtype) ERRORCHAR,outfp);\
          errormode = True;\
        }

#define ERROROFF\
        if(errormode)\
        {\
          (void) putc((Fputcfirstargtype) NORMALCHAR,outfp);\
          errormode = False;\
        }

#ifdef DEBUG

#define CHECKMODE(B)\
        if(showmode & (B))\
        {\
          printf("file %s line %lu: %s is set\n",file,(Showuint) line,#B);\
        }

static void showshowmode(char *file,Sint line,Uint showmode)
{
  printf("showshowmode %lu\n",(Showuint) showmode);
  CHECKMODE(SHOWABSOLUTE);
  CHECKMODE(SHOWNODIST);
  CHECKMODE(SHOWNOEVALUE);
  CHECKMODE(SHOWDIRECT);
  CHECKMODE(SHOWPALINDROMIC);
  CHECKMODE(SHOWSELFPALINDROMIC);
  CHECKMODE(SHOWFILE);
}
#endif

static Sint echopospair(FILE *outfp,
                        Showdescinfo *showdesc,
                        Multiseq *multiseq,
                        Uint seqnum,
                        Uint relpos,
                        Uint descindex,
                        Uint seqnumdigits,
                        Uint posdigits)
{
  fprintf(outfp,"   ");
  if(showdesc->defined)
  {
    if(multiseq->descspace.spaceUchar == NULL)
    {
      fprintf(outfp,"sequence%lu",(Showuint) seqnum);
    } else
    {
      echothedescription(outfp,showdesc,multiseq,descindex);
    }
  } else
  {
    fprintf(outfp," %*lu",(Fieldwidthtype) seqnumdigits,(Showuint) seqnum);
  }
  fprintf(outfp," %*lu",(Fieldwidthtype) posdigits,(Showuint) relpos);
  return 0;
}

static Sint echomatchpart1(FILE *outfp,
                           Uint showmode,
                           Showdescinfo *showdesc,
                           Digits *digits,
                           Multiseq *virtualmultiseq,
                           StoreMatch *storematch)
{
  fprintf(outfp,"%*lu",(Fieldwidthtype) digits->length,
                       (Showuint) storematch->Storelength1);
  if(showmode & SHOWFILE)
  {
    Sint fnum = getfilenum(virtualmultiseq,storematch->Storeposition1);
    if(fnum < 0)
    {
      return (Sint) -1;
    }
    fprintf(outfp," %s",virtualmultiseq->allfiles[fnum].filenamebuf);
  }
  if(showmode & SHOWABSOLUTE)
  {
    fprintf(outfp," %*lu",(Fieldwidthtype) digits->position1,
                          (Showuint) storematch->Storeposition1);
  } else
  {
    if(echopospair(outfp,
                   showdesc,
                   virtualmultiseq,
                   storematch->Storeseqnum1,
                   storematch->Storerelpos1,
                   storematch->Storeseqnum1,
                   digits->seqnum1,
                   digits->position1) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint echomatchpart2(FILE *outfp,Uint showmode,Showdescinfo *showdesc,
                          Digits *digits,Multiseq *virtualmultiseq,
                          Multiseq *querymultiseq,StoreMatch *storematch)
{
  Multiseq *ms;
  Uint offset;

  fprintf(outfp,"%*lu",(Fieldwidthtype) digits->length,
                       (Showuint) storematch->Storelength2);
  if(storematch->Storeflag & FLAGQUERY)
  {
    ms = querymultiseq;
    offset = 0;
  } else
  {
    ms = virtualmultiseq;
    if(HASINDEXEDQUERIES(virtualmultiseq))
    {
      offset = DATABASELENGTH(virtualmultiseq)+UintConst(1);
    } else
    {
      offset = 0;
    }
  }
  if(showmode & SHOWFILE)
  {
    Sint fnum = getfilenum(ms,offset + storematch->Storeposition2);
    if(fnum < 0)
    {
      return (Sint) -1;
    }
    fprintf(outfp," %s",ms->allfiles[fnum].filenamebuf);
  }
  if(showmode & SHOWABSOLUTE)
  {
    fprintf(outfp," %*lu",(Fieldwidthtype) digits->position2,
                          (Showuint) storematch->Storeposition2);
  } else
  {
    if(storematch->Storeflag & FLAGQUERY)
    {
      if(echopospair(outfp,
                     showdesc,
                     ms,
                     storematch->Storeseqnum2,
                     storematch->Storerelpos2,
                     storematch->Storeseqnum2,
                     digits->seqnum2,
                     digits->position2) != 0)
      {
        return (Sint) -1;
      }
    } else
    {
      Uint descindex;

      if(HASINDEXEDQUERIES(virtualmultiseq))
      {
        descindex = storematch->Storeseqnum2 +
                    NUMOFDATABASESEQUENCES(virtualmultiseq);
      } else
      {
        descindex = storematch->Storeseqnum2;
      }
      if(echopospair(outfp,
                     showdesc,
                     virtualmultiseq,
                     storematch->Storeseqnum2,
                     storematch->Storerelpos2,
                     descindex,
                     digits->seqnum2,
                     digits->position2) != 0)
      {
        return (Sint) -2;
      }
    }
  }
  return 0;
}

static void vmechoexactmatch(FILE *outfp,Uint linewidth,
                             Showseqinfo *showseqinfo)
{
  Uint len, linestart = 0, i;

  while(True)
  {
    len = MIN(showseqinfo->length - linestart,linewidth);
    for(i = linestart; i < linestart+len; i++)
    {
      (void) putc((Fputcfirstargtype) showseqinfo->showseqorig[i],outfp);
    }
    linestart += len;
    if(i >= showseqinfo->length)
    {
      break;
    }
    (void) putc('\n',outfp);
  }
  (void) putc('\n',outfp);
}

#ifdef DEBUG
#define COUNTMISMATCHES countmismatches++
#else
#define COUNTMISMATCHES /* Nothing */
#endif

#define ASSIGNRC(BASE)\
        switch(BASE)\
        {\
          case 'A' : BASE = (Uchar) 'T'; break;\
          case 'a' : BASE = (Uchar) 't'; break;\
          case 'C' : BASE = (Uchar) 'G'; break;\
          case 'c' : BASE = (Uchar) 'g'; break;\
          case 'G' : BASE = (Uchar) 'C'; break;\
          case 'g' : BASE = (Uchar) 'c'; break;\
          case 'T' : BASE = (Uchar) 'A'; break;\
          case 't' : BASE = (Uchar) 'a'; break;\
        }

static Sint echohammingmatch(FILE *outfp,BOOL rightrcmode,BOOL showiub,
                             BOOL specialsymbols,Uint linewidth,
                             Showseqinfo *useqinfo,
                             Showseqinfo *vseqinfo,
                             Uint mismatches)
{
  Uint i, thislinelength = 0;
  Uchar acompare, aorig, bcompare, borig,
        *useqcompare = useqinfo->showseqcompare,
        *vseqcompare = vseqinfo->showseqcompare,
        *useqorig = useqinfo->showseqorig,
        *vseqorig = vseqinfo->showseqorig;
  DEBUGDECL(Uint countmismatches = 0);

  if(useqinfo->length != vseqinfo->length)
  {
    ERROR2("cannot echo hamming match of sequences of "
           " different lengths %lu and %lu",
           (Showuint) useqinfo->length,
           (Showuint) vseqinfo->length);
    return (Sint) -1;
  }
  if(specialsymbols)
  {
    for(i = 0; i < useqinfo->length; i++)
    {
      acompare = useqcompare[i];
      aorig = useqorig[i];
      if(rightrcmode)
      {
        bcompare = vseqcompare[useqinfo->length-1-i];
        borig = vseqorig[useqinfo->length-1-i];
        if(bcompare != (Uchar) WILDCARD)
        {
          if(bcompare > (Uchar) 3)
          {
            ERROR1("complement of %lu not defined",(Showuint) bcompare);
            return (Sint) -1;
          }
          bcompare = ((Uchar) 3) - bcompare;
          ASSIGNRC(borig);
        }
      } else
      {
        bcompare = vseqcompare[i];
        borig = vseqorig[i];
      }
      if(acompare == (Uchar) WILDCARD || bcompare == (Uchar) WILDCARD)
      {
        PUTONE('[');
        PUTONE(aorig);
        PUTONE(borig);
        PUTONE(']');
        COUNTMISMATCHES;
      } else
      {
        if(acompare != bcompare)
        {
          COUNTMISMATCHES;
          if(showiub)
          {
            PUTONE(IUBSYMBOL(acompare,bcompare))
          } else
          {
            PUTONE('[');
            PUTONE(aorig);
            PUTONE(borig);
            PUTONE(']');
          }
        } else
        {
          if(aorig != borig)
          {
            PUTONE('{');
            PUTONE(aorig);
            PUTONE(borig);
            PUTONE('}');
          } else
          {
            PUTONE(aorig);
          }
        }
      }
    }
  } else
  {
    for(i = 0; i < useqinfo->length; i++)
    {
      if(rightrcmode)
      {
        ERROR0("reverse complement requires DNA alphabet");
        return (Sint) -1;
      }
      acompare = useqcompare[i];
      aorig = useqorig[i];
      bcompare = vseqcompare[i];
      borig = vseqorig[i];
      if(acompare != bcompare)
      {
        COUNTMISMATCHES;
        PUTONE('[');
        PUTONE(aorig);
        PUTONE(borig);
        PUTONE(']');
      } else
      {
        if(aorig != borig)
        {
          PUTONE('{');
          PUTONE(aorig);
          PUTONE(borig);
          PUTONE('}');
        } else
        {
          PUTONE(aorig);
        }
      }
    }
  }
  (void) putc('\n',outfp);
#ifdef DEBUG
  if(countmismatches != mismatches)
  {
    fprintf(stderr,"countmismatches = %lu != %lu mismatches\n",
                    (Showuint) countmismatches,(Showuint) mismatches);
    exit(EXIT_FAILURE);
  }
#endif
  return 0;
}

static Sint fancyechohammingmatch(FILE *outfp,BOOL rightrcmode,BOOL showiub,
                                  BOOL specialsymbols,Uint linewidth,
                                  Showseqinfo *useqinfo,
                                  Showseqinfo *vseqinfo,
                                  /*@unused@*/ Uint mismatches)
{
  Uint i, thislinelength = 0;
  Uchar acompare, bcompare, aorig, borig,
        *useqcompare = useqinfo->showseqcompare,
        *vseqcompare = vseqinfo->showseqcompare,
        *useqorig = useqinfo->showseqorig,
        *vseqorig = vseqinfo->showseqorig;
  BOOL errormode = False;

  if(useqinfo->length != vseqinfo->length)
  {
    ERROR2("Cannot echo hamming match of sequences of different lengths %lu and %lu",
           (Showuint) useqinfo->length,
           (Showuint) vseqinfo->length);
    return (Sint) -1;
  }
  (void) putc((Fputcfirstargtype) NORMALCHAR,outfp);
  if(specialsymbols)
  {
    for(i = 0; i < useqinfo->length; i++)
    {
      acompare = useqcompare[i];
      aorig = useqorig[i];
      if(rightrcmode)
      {
        bcompare = vseqcompare[useqinfo->length-1-i];
        borig = vseqorig[useqinfo->length-1-i];
        if(bcompare != (Uchar) WILDCARD)
        {
          if(bcompare > (Uchar) 3)
          {
            ERROR1("complement of %lu not defined",(Showuint) bcompare);
            return (Sint) -1;
          }
          bcompare = ((Uchar) 3) - bcompare;
          ASSIGNRC(borig);
        }
      } else
      {
        bcompare = vseqcompare[i];
        borig = vseqorig[i];
      }
      if(acompare == (Uchar) WILDCARD || bcompare == (Uchar) WILDCARD)
      {
        ERRORON;
        PUTONE('[');
        PUTONE(aorig);
        PUTONE(borig);
        PUTONE(']');
      } else
      {
        if(acompare != bcompare)
        {
          ERRORON;
          if(showiub)
          {
            PUTONE(IUBSYMBOL(acompare,bcompare))
          } else
          {
            PUTONE('[');
            PUTONE(aorig);
            PUTONE(borig);
            PUTONE(']');
          }
        } else
        {
          ERROROFF;
          if(aorig != borig)
          {
            PUTONE('{');
            PUTONE(aorig);
            PUTONE(borig);
            PUTONE('}');
          } else
          {
            PUTONE(aorig);
          }
        }
      }
    }
  } else
  {
    for(i = 0; i < useqinfo->length; i++)
    {
      if(rightrcmode)
      {
        ERROR0("reverse complement requires DNA alphabet");
        return (Sint) -1;
      }
      acompare = useqcompare[i];
      aorig = useqorig[i];
      bcompare = vseqcompare[i];
      borig = vseqorig[i];
      if(acompare != bcompare)
      {
        ERRORON;
        PUTONE('[');
        PUTONE(aorig);
        PUTONE(borig);
        PUTONE(']');
      } else
      {
        ERROROFF;
        if(aorig != borig)
        {
          PUTONE('{');
          PUTONE(aorig);
          PUTONE(borig);
          PUTONE('}');
        } else
        {
          PUTONE(aorig);
        }
      }
    }
  }
  (void) putc('\n',outfp);
  return 0;
} 


static void showeditopinxml(Editoperation *al, Uint lenalg, FILE *outfp)
{
  GENFILE genfile;

  genfile.genfilemode  = STANDARD;
  genfile.fileptr.file = outfp;

  showeditopsgeneric(al,lenalg,False,True,UintConst(3),&genfile);
  closeMatchtag(outfp);
}

static void showxdropstringout(FILE *outfp,
                               BOOL fancy,
                               Uint showstring,
                               BOOL selfcomparison,
                               Showseqinfo *leftseqinfo,
                               Showseqinfo *rightseqinfo,
                               StoreMatch *storematch)
{
  ArrayEditoperation xdropalignment;
  Uint indelcount, saflag;

  INITARRAY(&xdropalignment,Editoperation);
  indelcount = onexdropalignment2(True,
                                  &xdropalignment,
                                  leftseqinfo->showseqcompare,
                                  (Sint) leftseqinfo->length,
                                  rightseqinfo->showseqcompare,
                                  (Sint) rightseqinfo->length,
                                  (Xdropscore) (storematch->Storeflag & 
                                                FLAGXDROP));
  MAKESAFLAG(saflag,True,fancy,selfcomparison);
  if(showstring & SHOWVMATCHXML)
  {
    showeditopinxml(xdropalignment.spaceEditoperation,
                    xdropalignment.nextfreeEditoperation,
                    outfp);
  } else
  {
    showalignment(saflag,
                  outfp,
                  showstring & MAXLINEWIDTH,
                  xdropalignment.spaceEditoperation,
                  xdropalignment.nextfreeEditoperation,
                  indelcount,
                  leftseqinfo->showseqcompare,
                  leftseqinfo->showseqorig,
                  leftseqinfo->length,
                  rightseqinfo->showseqcompare,
                  rightseqinfo->showseqorig,
                  rightseqinfo->length,
                  storematch->Storerelpos1,
                  storematch->Storerelpos2);
  }
  FREEARRAY(&xdropalignment,Editoperation);
}

static Sint shownormalstringout(FILE *outfp,
                                BOOL fancy,
                                Uint showstring,
                                BOOL hamming,
                                BOOL selfcomparison,
                                Showseqinfo *leftseqinfo,
                                Showseqinfo *rightseqinfo,
                                StoreMatch *storematch)
{
  Greedyalignreservoir greedyalignreservoir;
  Uint saflag;
  
  initgreedyalignreservoir(&greedyalignreservoir);
  if(storematch->Storedistance == 0)
  {
    alignequalstrings(&greedyalignreservoir.alignment,leftseqinfo->length);
  } else
  {
    if(hamming)
    {
      (void) hammingalignment(&greedyalignreservoir.alignment,
                              leftseqinfo->showseqcompare,
                              leftseqinfo->length,
                              rightseqinfo->showseqcompare);
    } else
    {
      if(greedyedistalign(&greedyalignreservoir,
                          True,
                          storematch->Storedistance,
                          leftseqinfo->showseqcompare,
                          (Sint) leftseqinfo->length,
                          rightseqinfo->showseqcompare,
                          (Sint) rightseqinfo->length) < 0)
      {
        return (Sint) -2;
      }
    }
  }
#ifdef DEBUG
  verifyedistalignment(__FILE__,(Uint) __LINE__,
                       greedyalignreservoir.alignment.spaceEditoperation,
                       greedyalignreservoir.alignment.nextfreeEditoperation,
                       leftseqinfo->length,
                       rightseqinfo->length,
                       (Xdropscore) storematch->Storedistance);
#endif
  MAKESAFLAG(saflag,True,fancy,selfcomparison);
  if(showstring & SHOWVMATCHXML)
  {
    showeditopinxml(greedyalignreservoir.alignment.spaceEditoperation,
                    greedyalignreservoir.alignment.nextfreeEditoperation,
                    outfp);
  } else
  {
    showalignment(saflag,
                  outfp,
                  showstring & MAXLINEWIDTH,
                  greedyalignreservoir.alignment.spaceEditoperation,
                  greedyalignreservoir.alignment.nextfreeEditoperation,
                  (Uint) ABS(storematch->Storedistance),
                  leftseqinfo->showseqcompare,
                  leftseqinfo->showseqorig,
                  leftseqinfo->length,
                  rightseqinfo->showseqcompare,
                  rightseqinfo->showseqorig,
                  rightseqinfo->length,
                  storematch->Storerelpos1,
                  storematch->Storerelpos2);
  }
  wraptgreedyalignreservoir(&greedyalignreservoir);
  return 0;
}

static Sint initseqinfoRC(Showseqinfo *seqinfoRC,Showseqinfo *seqinfo)
{
  ALLOCASSIGNSPACE(seqinfoRC->showseqcompare,NULL,Uchar,seqinfo->length);
  if(makereversecomplement(seqinfoRC->showseqcompare,
                           seqinfo->showseqcompare,
                           seqinfo->length) != 0)
  {
    return (Sint) -1;
  }
  ALLOCASSIGNSPACE(seqinfoRC->showseqorig,NULL,Uchar,seqinfo->length);
  if(makereversecomplementorig(seqinfoRC->showseqorig,
                               seqinfo->showseqorig,
                               seqinfo->length) != 0)
  {
    return (Sint) -2;
  }
  seqinfoRC->length = seqinfo->length;
  return 0;
}

/*
  The following function shows the matching substrings or the alignemts of
  the matching substring. The parameters are as follows:
  outfp:           file pointer to output the matches
  fancy:           show the output in fancy format
  showstring:      width of the output and special flags for output
  showmode:        mode of showing alignment
  virtualmultiseq: the virtual tree
  querymultiseq:   multisequence of the query
  storematch:      the matching structure
*/

static Sint echostringoutput(FILE *outfp,
                             BOOL fancy,
                             Uint showstring,
                             Multiseq *virtualmultiseq,
                             BOOL specialsymbols,
                             Multiseq *querymultiseq,
                             Alphabet *virtualalpha,
                             StoreMatch *storematch)
{
  Showseqinfo leftseqinfo, rightseqinfo, 
              leftseqinfoRC, rightseqinfoRC,
              *lsinfoptr, *rsinfoptr;
  BOOL selfcomparison, 
       hamming = False,
       leftrcmode,
       rightrcmode;

  if(storematch->Storeflag & FLAGPPLEFTREVERSE)
  {
    leftrcmode = True;
  } else
  {
    leftrcmode = False;
  }
  getleftseq(&leftseqinfo,storematch,virtualmultiseq,virtualalpha);
  if(storematch->Storeflag & (FLAGPALINDROMIC | FLAGSELFPALINDROMIC))
  {
    rightrcmode = True;
  } else
  {
    if(!(storematch->Storeflag & FLAGQUERY) &&
       (storematch->Storeflag & FLAGPPRIGHTREVERSE))
    {
      rightrcmode = True;
    } else
    {
      rightrcmode = False;
    }
  }
  if(storematch->Storeflag & FLAGSCOREMATCH)
  {
    return 0;
  }
  if(storematch->Storedistance == 0 && 
     (showstring & (SHOWALIGNABBREV | SHOWALIGNABBREVIUB)))
  {
    vmechoexactmatch(outfp,showstring & MAXLINEWIDTH,&leftseqinfo);
    return 0;
  }  
#ifdef WITHREGEXP
  if((storematch->Storeflag & FLAGQUERY) && 
     querymultiseq->searchregexp.spaceUchar != NULL && 
     querymultiseq->searchregexp.spaceUchar[storematch->Storeseqnum2])
  {
    vmechoexactmatch(outfp,showstring & MAXLINEWIDTH,&leftseqinfo);
    return 0;
  }
#endif
  rightseqinfo.codon2aminobufferorig = NULL;
  rightseqinfo.codon2aminobuffermapped = NULL;
  if(showstring & (SHOWPURELEFTSEQ | SHOWPURERIGHTSEQ))
  {
    if(showstring & SHOWPURELEFTSEQ)
    {
      vmechoexactmatch(outfp,showstring & MAXLINEWIDTH,&leftseqinfo);
    }
    if(showstring & SHOWPURERIGHTSEQ)
    {
      if(getrightseq(&rightseqinfo,
                     storematch,
                     virtualmultiseq,
                     querymultiseq,
                     virtualalpha) != 0)
      {
        return (Sint) -2;
      }
      (void) putc('\n',outfp);
      vmechoexactmatch(outfp,showstring & MAXLINEWIDTH,&rightseqinfo);
    }
    FREESPACE(rightseqinfo.codon2aminobufferorig);
    FREESPACE(rightseqinfo.codon2aminobuffermapped);
    return 0;
  } 
  if(getrightseq(&rightseqinfo,
                 storematch,
                 virtualmultiseq,
                 querymultiseq,
                 virtualalpha) != 0)
  {
    return (Sint) -3;
  }
  if(storematch->Storedistance < 0 && 
     (showstring & (SHOWALIGNABBREV | SHOWALIGNABBREVIUB)))
  {
    if((fancy ? fancyechohammingmatch : echohammingmatch)
                          (outfp,rightrcmode,
                          (showstring & SHOWALIGNABBREVIUB) ? True : False,
                          specialsymbols,
                          showstring & MAXLINEWIDTH,
                          &leftseqinfo,
                          &rightseqinfo,
                          (Uint) -storematch->Storedistance) != 0)
    {
      return (Sint) -4;
    }
    FREESPACE(rightseqinfo.codon2aminobufferorig);
    FREESPACE(rightseqinfo.codon2aminobuffermapped);
    return 0;
  } 
  if(storematch->Storedistance < 0)
  {
    hamming = True;
    storematch->Storedistance = -storematch->Storedistance;
  }
  if((storematch->Storeflag & FLAGQUERY) &&
    !(storematch->Storeflag & FLAGSELFPALINDROMIC))
  {
    selfcomparison = False;
  } else
  {
    selfcomparison = True;
  }
  if(leftrcmode)
  {
    if(initseqinfoRC(&leftseqinfoRC,&leftseqinfo) != 0)
    {
      return (Sint) -5;
    }
    lsinfoptr = &leftseqinfoRC;
  } else
  {
    lsinfoptr = &leftseqinfo;
  }
  if(rightrcmode)
  {
    if(initseqinfoRC(&rightseqinfoRC,&rightseqinfo) != 0)
    {
      return (Sint) -5;
    }
    rsinfoptr = &rightseqinfoRC;
  } else
  {
    rsinfoptr = &rightseqinfo;
  }
  if(storematch->Storeflag & FLAGXDROP)
  {
    showxdropstringout(outfp,
                       fancy,
                       showstring,
                       selfcomparison,
                       lsinfoptr,
                       rsinfoptr,
                       storematch);
  } else
  {
    if(shownormalstringout(outfp,
                           fancy,
                           showstring,
                           hamming,
                           selfcomparison,
                           lsinfoptr,
                           rsinfoptr,
                           storematch) != 0)
    {
      return (Sint) -6;
    }
  }
  if(leftrcmode)
  {
    FREESPACE(leftseqinfoRC.showseqcompare);
    FREESPACE(leftseqinfoRC.showseqorig);
  }
  if(rightrcmode)
  {
    FREESPACE(rightseqinfoRC.showseqcompare);
    FREESPACE(rightseqinfoRC.showseqorig);
  }
  FREESPACE(rightseqinfo.codon2aminobufferorig);
  FREESPACE(rightseqinfo.codon2aminobuffermapped);
  if(storematch->Storedistance != 0 && hamming)
  {
    storematch->Storedistance = -storematch->Storedistance;
  }
  return 0;
}

static Sint vmatchnormaloutmatch(FILE *outfp,
                                 BOOL fancy,
                                 Uint showmode,
                                 Showdescinfo *showdesc,
                                 Uint showstring,
                                 Digits *digits,
                                 Multiseq *virtualmultiseq,
                                 Multiseq *querymultiseq,
                                 StoreMatch *storematch)
{
  char modechar;

  if(showdesc == NULL)
  {
    fprintf(stderr,"programming error: showdesc is NULL\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && showstring > 0)
  {
    (void) putc((Fputcfirstargtype) POSITIONCHAR,outfp);
  }
  if(showstring & (SHOWPURELEFTSEQ | SHOWPURERIGHTSEQ))
  {
    (void) putc(FASTASEPARATOR,outfp);  // make the line containing the match info fasta header
  }
  if(echomatchpart1(outfp,
                    showmode,
                    showdesc,
                    digits,
                    virtualmultiseq,
                    storematch) != 0)
  {
    return (Sint) -1;
  }
  if(FLAG2TRANSNUM(storematch->Storeflag) == NOTRANSLATIONSCHEME)
  {
    if(storematch->Storeflag & FLAGPALINDROMIC)
    {
      modechar = PALINDROMICCHAR;
    } else
    {
      modechar = DIRECTCHAR;
    }
  } else
  {
    if(storematch->Storeflag & FLAGPPLEFTREVERSE)
    {
      if(storematch->Storeflag & FLAGPPRIGHTREVERSE)
      {
        modechar = PPREVREVCHAR;
      } else
      {
        modechar = PPREVFWDCHAR;
      }
    } else
    {
      if(storematch->Storeflag & FLAGPPRIGHTREVERSE)
      {
        modechar = PPFWDREVCHAR;
      } else
      {
        modechar = PPFWDFWDCHAR;
      }
    }
  }
  fprintf(outfp,"   %c ",modechar);
  if(echomatchpart2(outfp,showmode,showdesc,digits,
                    virtualmultiseq,
                    querymultiseq,storematch) != 0)
  {
    return (Sint) -2;
  }
  if(!(showmode & SHOWNODIST))
  {
    fprintf(outfp," %3ld",(Showsint) storematch->Storedistance);
  }
  if(!(showmode & SHOWNOEVALUE))
  {
    if(storematch->StoreEvalue >= 1.0e-99 || storematch->StoreEvalue == 0.0)
    {
      (void) putc(' ',outfp);
    }
    fprintf(outfp,"   %.2e",storematch->StoreEvalue);
  }
  if(!(showmode & SHOWNOSCORE))
  {
    fprintf(outfp," %*ld",(Fieldwidthtype) (digits->length + 1),
                          (Showsint) DISTANCE2SCORE(storematch));
  }
  if(!(showmode & SHOWNOIDENTITY))
  {
    double identity;

    IDENTITY(identity,storematch);
    if(identity < 100.0)
    {
      (void) putc(' ',outfp);
    }
    fprintf(outfp,"   %.2f",identity);
  }
  return 0;
}

/*EE
  The following function outputs a match (possibly including an alignment)
  to the file pointer \texttt{outfp}. The parameters are as follows:
  \begin{itemize}
  \item
  \texttt{fancy} is true iff the output contains interspersed characters
  for switching the mode when showing different parts of the match
  \item
  \texttt{showmode} is one of the bits \texttt{SHOWDIRECT} $\ldots$
  \texttt{SHOWFILE}.
  \item
  \texttt{showdesc} describes how to format the sequence description.
  If the desciption is the empty string, then the desciption is reported 
  as $\texttt{sequence}i$ where \(i\) is the sequence number.
  If \texttt{showdesc} is undefined, then the sequence number but 
  not the description is shown.
  \item
  If \texttt{showstring} is $>0$, then the string content (for exact matches)
  or the alignment of the matching strings (for hamming/edit distance matches)
  is shown formatted to \texttt{showstring} characters per line. 
  Also one of the bits \texttt{SHOWPURELEFT} or \texttt{SHOWPURERIGHT}
  may be set, in which case the string content of the 
  left/right instance of the match is shown (instead of an alignment).
  If \texttt{showstring=0}, then no matching substring or alignment is shown.
  \item
  The numbers output are formatted such that they appear 
  right adjusted in a certain column of the output.
  \texttt{digits} refers to a structure storing the width
  of these columns.
  \item
  \texttt{virtualmultiseq} is the virtual suffix tree of the index.
  \item
  \texttt{queryvirtualtree} is the virtual suffix tree of the query sequences.
  Not all components may be defined.
  \item
  \texttt{storematch} is the reference to the sturcture storing the match.
  \end{itemize}
  The function returns 0 upon success, and a negative error code in case
  of failure.
*/

Sint echomatch2file(FILE *outfp,
                    BOOL fancy,
                    Uint showmode,
                    Showdescinfo *showdesc,
                    Uint showstring,
                    Digits *digits,
                    Multiseq *virtualmultiseq,
                    Multiseq *querymultiseq,
                    Alphabet *virtualalpha,
                    StoreMatch *storematch)
{
  BOOL specialsymbols;
  DEBUGCODE(3,showshowmode(__FILE__,(Sint) __LINE__,showmode));

  if(showstring & SHOWVMATCHXML)
  {
    showdesc->replaceblanks = False;
    vmatchxmlmatch(outfp,False,showdesc,
                   virtualmultiseq,querymultiseq,storematch);
  } else
  {
    if(vmatchnormaloutmatch(outfp,fancy,showmode,showdesc,showstring,
                            digits,virtualmultiseq,querymultiseq,
                            storematch) != 0)
    {
      return (Sint) -1;
    }
  }
  specialsymbols = (showmode & SHOWNOSPECIAL) ? False : True;
  if(showstring > 0)
  {
    if(fancy)
    {
      (void) putc((Fputcfirstargtype) NORMALCHAR,outfp);
    }
    if(!(showstring & SHOWVMATCHXML))
    {
      (void) putc('\n',outfp);
    }
    if(echostringoutput(outfp,
                        fancy,
                        showstring,
                        virtualmultiseq,
                        specialsymbols,
                        querymultiseq,
                        virtualalpha,
                        storematch) != 0)
    {
      return (Sint) -2;
    }
    if(fancy)
    {
      (void) putc((Fputcfirstargtype) NORMALCHAR,outfp);
    }
  } else
  {
    if(fancy)
    {
      (void) putc((Fputcfirstargtype) NORMALCHAR,outfp);
    }
  }
  if(!(showstring & SHOWVMATCHXML))
  {
    (void) putc('\n',outfp);
  }
  return 0;
}

Sint immediatelyechothematch(void *voidoutinfo,
                             /*@unused@*/ Multiseq *virtualmultiseq,
                             /*@unused@*/ Multiseq *querymultiseq,
                             StoreMatch *storematch)
{
  Outinfo *outinfo = (Outinfo *) voidoutinfo;

  if(echomatch2file(outinfo->outfp,
                    False,
                    outinfo->showmode,
                    &outinfo->showdesc,
                    outinfo->showstring,
                    &outinfo->digits,
                    outinfo->outvms,
                    outinfo->outqms,
                    outinfo->outvmsalpha,
                    storematch) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

Sint simpleechomatch2file(FILE *outfp,
                          Multiseq *virtualmultiseq,
                          Multiseq *querymultiseq,
                          StoreMatch *storematch)
{
  Outinfo outinfo;

  outinfo.outfp = outfp;
  outinfo.showdesc.defined = False;
  if(storematch->Storeflag & FLAGPALINDROMIC)
  {
    outinfo.showmode = SHOWPALINDROMIC;
    if(storematch->Storeflag & FLAGSELFPALINDROMIC)
    {
      outinfo.showmode |= SHOWSELFPALINDROMIC;
    }
  } else
  {
    outinfo.showmode = SHOWDIRECT;
  }
  outinfo.showstring = UintConst(60);
  outinfo.sortmode = undefsortmode();
  outinfo.outvms = virtualmultiseq;
  outinfo.outqms = querymultiseq;
  ASSIGNDEFAULTDIGITS(&outinfo.digits);
  if(immediatelyechothematch((void *) &outinfo,
                             virtualmultiseq,
                             querymultiseq,
                             storematch) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

/*EE
  The following function returns a copy of the output produced by
  \texttt{echomatch2file} in main memory. This is done via a temporary file.
  The memory area must be freed and the temporary file must be removed,
  when it is not used any more. This must be done with the corresponding
  function \texttt{clearMatchram}. The arguments \texttt{fancy} $\ldots$
  \texttt{querymultiseq} are as in \texttt{echomatch2file}. The pointer
  \texttt{storematch} may refer to a complete memory area storing matches.
  The additonal arguments mean the following:
  \begin{itemize}
  \item
  \texttt{numofmatches} is the number of matches in the memory area
  referenced by \texttt{storematch} to be shown.
  \item
  \texttt{numofbytes}
  refers to a memory address when the size of the temporary file ist stored.
  \end{itemize}
  The return value is a pointer to the memory area storing the 
  contents of the temporary file. If anything went wrong, then
  the function returns \texttt{echomatch2ram}.
*/

/*@null@*/ Uchar *echomatch2ram(BOOL fancy,
                                Uint showmode,
                                Showdescinfo *showdesc,
                                Uint showstring,
                                Digits *digits,
                                Multiseq *virtualmultiseq,
                                Alphabet *virtualalpha,
                                Multiseq *querymultiseq,
                                StoreMatch *storematch,
                                Uint numofmatches,
                                Uint *numofbytes,
                                Tmpfiledesc *tmpfiledesc)
{
  Uint i;
  Uchar *eptr;

  if(numofmatches == 0)
  {
    ERROR0("numofmatches == 0 not allowed");
    return NULL;
  }
  if(MAKETMPFILE(tmpfiledesc,NULL) != 0)
  {
    return NULL;
  }
  for(i=0; i<numofmatches; i++)
  {
    if(echomatch2file(tmpfiledesc->tmpfileptr,
                      fancy,
                      showmode,
                      showdesc,
                      showstring,
                      digits,
                      virtualmultiseq,
                      querymultiseq,
                      virtualalpha,
                      storematch+i) != 0)
    {
      return NULL;
    }
  }
  if(DELETEFILEHANDLE(tmpfiledesc->tmpfileptr) != 0)
  {
    return NULL;
  }
  eptr = (Uchar *) CREATEMEMORYMAP(tmpfiledesc->tmpfilenamebuffer,
                                   True,numofbytes);
  if(eptr == NULL)
  {
    ERROR1("cannot read file \"%s\"",tmpfiledesc->tmpfilenamebuffer);
    return NULL;
  }
  return eptr;
}

/*EE
  The following function frees the temporary memory area and removes
  the temporary file used for producing the match.
*/

Sint clearMatchram(Uchar **eptr,Tmpfiledesc *tmpfiledesc)
{
  if(DELETEMEMORYMAP(*eptr) != 0)
  {
    return (Sint) -1;
  }
  if(unlink(tmpfiledesc->tmpfilenamebuffer) != 0)
  {
    ERROR2("Cannot remove file \"%s\": %s",
            tmpfiledesc->tmpfilenamebuffer,strerror(errno));
    return (Sint) -2;
  }
  return 0;
}
