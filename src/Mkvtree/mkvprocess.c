#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "visible.h"
#include "debugdef.h"
#include "errordef.h"
#include "chardef.h"
#include "fhandledef.h"
#include "virtualdef.h"
#include "esafileend.h"

#include "filehandle.pr"
#include "outindextab.pr"
#include "readvirt.pr"
#include "accvirt.pr"
#include "bwtcode.pr"
#include "mkskip.pr"
#include "detpfxlen.pr"
#include "endianess.pr"
#include "scanpaths.pr"
#include "verbosealpha.pr"

#include "inputdef.h"
#include "mkvaux.h"

#include "cprsuf.pr"

#define EXTRASUFFIXLENGTH 5             // dot and suffix of length at most 4
#define NUMOFBASES        UintConst(4)  // number of bases in DNA alphabet

#define DNAALPHABET "aA\ncC\ngG\ntTuU\nnsywrkvbdhmNSYWRKVBDHM\n"
#define PROTEINALPHABET\
        "L\nV\nI\nF\nK\nR\nE\nD\nA\nG\nS\nT\nN\nQ\nY\nW\nP\nH\nM\nC\nXUBZ*-\n"


 Sint besesuffixsort(MKVaux *mkvaux,Virtualtree *virtualtree,
                     Uint numofchars,BOOL demandlcp,FILE *lcpfp,
                     Showverbose showverbose,DefinedUint *maxdepth);
 Sint besesuffixsortspecial(MKVaux *mkvaux,Virtualtree *virtualtree,
                            Uint numofchars,BOOL demandlcp,FILE *lcpfp,
                            Showverbose showverbose,DefinedUint *maxdepth);

/*@null@*/ static FILE *openthefile(Input *input,char *ending)
{
  char outputfilename[PATH_MAX+EXTRASUFFIXLENGTH+1], sbuf[PATH_MAX+50+1];
  FILE *fp;

  sprintf(outputfilename,"%s.%s",input->indexname,ending);
  if(filealreadyexists(outputfilename))
  {
    ERROR1("cannot create file \"%s\": it already exists",outputfilename);
    return NULL;
  }
  fp = CREATEFILEHANDLE(outputfilename,WRITEMODE);
  if(fp == NULL)
  {
    return NULL;
  }
  if(input->showverbose != NULL)
  {
    sprintf(sbuf,"create file \"%s\"",outputfilename);
    input->showverbose(sbuf);
  }
  return fp;
}

#ifdef WITHLCP
static Sint remaplcptab(Input *input,Virtualtree *virtualtree)
{
  char tmpfilename[PATH_MAX+EXTRASUFFIXLENGTH+1];
  Uint numofbytes;

  sprintf(tmpfilename,"%s.%s",input->indexname,"lcp");
  virtualtree->lcptab 
    = (Uchar *) CREATEMEMORYMAP(tmpfilename,False,&numofbytes);
  if(virtualtree->lcptab == NULL)
  {
    ERROR1("cannot read %s",tmpfilename);
    return (Sint) -1;
  }
  if(numofbytes != (virtualtree->multiseq.totallength+1))
  {
    ERROR2("file %s is must be of size %lu",
           tmpfilename,
           (Showuint) (virtualtree->multiseq.totallength+1));
    return (Sint) -2;
  }
  return 0;
}
#endif

static Sint makessptab(Input *input,Virtualtree *virtualtree)
{
  if(SOMETHINGOUT(input) && 
     virtualtree->multiseq.numofsequences > UintConst(1) &&
     input->storeonfile)
  {
    if(outindextab(input->indexname,
                   "ssp",
                   (void *) virtualtree->multiseq.markpos.spaceUint,
                   (Uint) sizeof(Uint),
                   virtualtree->multiseq.markpos.nextfreeUint) != 0)
     {
       return (Sint) -1;
     }
  }
  return 0;
}

static Sint maketistab(Input *input,Virtualtree *virtualtree)
{
  if((input->demand & TISTAB) && input->storeonfile)
  {
    FILE *tisfp;

    if((tisfp = openthefile(input,"tis")) == NULL)
    {
      return (Sint) -1;
    }
    if(WRITETOFILEHANDLE(virtualtree->multiseq.sequence,
                         (Uint) sizeof(Uchar),
                         virtualtree->multiseq.totallength,
                         tisfp) != 0)
    {
      return (Sint) -2;
    }
    if(DELETEFILEHANDLE(tisfp) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

static Sint makeoistab(Input *input,Virtualtree *virtualtree)
{
  if((input->demand & OISTAB) && input->storeonfile && 
     virtualtree->multiseq.originalsequence != NULL)
  {
    FILE *oisfp;

    if((oisfp = openthefile(input,"ois")) == NULL)
    {
      return (Sint) -1;
    }
    if(WRITETOFILEHANDLE(virtualtree->multiseq.originalsequence,
                         (Uint) sizeof(Uchar),
                         virtualtree->multiseq.totallength,
                         oisfp) != 0)
    {
      return (Sint) -2;
    }
    if(DELETEFILEHANDLE(oisfp) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

static Sint makedestab(Input *input,Virtualtree *virtualtree)
{
  if(SOMETHINGOUT(input) && virtualtree->multiseq.descspace.spaceUchar != NULL
     && input->storeonfile)
  {
    FILE *fp;
    Uint seqnum;

    if((fp = openthefile(input,"des")) == NULL)
    {
      return (Sint) -1;
    }
    for(seqnum=0; seqnum<virtualtree->multiseq.numofsequences; seqnum++)
    {
      if(WRITETOFILEHANDLE(DESCRIPTIONPTR(&virtualtree->multiseq,seqnum),
                           (Uint) sizeof(Uchar),
                           DESCRIPTIONLENGTH(&virtualtree->multiseq,seqnum),
                           fp) != 0)
      {
        return (Sint) -2;
      }
    }
    if(DELETEFILEHANDLE(fp) != 0)
    {
      return (Sint) -3;
    }
    if((fp = openthefile(input,"sds")) == NULL)
    {
      return (Sint) -4;
    }
    if(WRITETOFILEHANDLE(virtualtree->multiseq.startdesc,
                         (Uint) sizeof(Uint),
                         virtualtree->multiseq.numofsequences+UintConst(1),
                         fp) != 0)
    {
      return (Sint) -5;
    }
    if(DELETEFILEHANDLE(fp) != 0)
    {
      return (Sint) -6;
    }
  }
  return 0;
}

#ifdef WITHLCP
static Sint makelargelcp(Input *input,Virtualtree *virtualtree)
{
  if((input->demand & LCPTAB) && input->storeonfile)
  {
    FILE *largelcpfp;

    if((largelcpfp = openthefile(input,LARGELCPTABSUFFIX)) == NULL)
    {
      return (Sint) -1;
    }
    if(WRITETOFILEHANDLE(virtualtree->largelcpvalues.spacePairUint,
                         (Uint) sizeof(PairUint),
                         virtualtree->largelcpvalues.nextfreePairUint,
                         largelcpfp) != 0)
    {
      return (Sint) -2;
    }
    if(DELETEFILEHANDLE(largelcpfp) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}
#endif

#ifdef DEBUG
#define UPDATELARGEBUCKET\
        if(mid - left >= UCHAR_MAX)\
        {\
          largebuckets++;\
          overhangsize += mid - left - UCHAR_MAX;\
        }
#else
#define UPDATELARGEBUCKET /* Nothing */
#endif

static Sint makebcktab(Input *input,Virtualtree *virtualtree,MKVaux *mkvaux)
{
  DEBUGDECL(Uint largebuckets = 0);
  DEBUGDECL(Uint overhangsize = 0);

  if(virtualtree->prefixlength > 0)
  {
    if(input->demand & BCKTAB)
    {
      Uint *bckptr, left = 0, mid, right, i, j, jstart;
  
      ALLOCASSIGNSPACE(virtualtree->bcktab,NULL,
                       Uint,UintConst(2) * virtualtree->numofcodes);
      bckptr = virtualtree->bcktab;
      right = mkvaux->leftborder[0];
      if(input->numofchars != NUMOFBASES || mkvaux->storecodes)
      {
        for(j = 0, i = 0; i < virtualtree->numofcodes; i++)
        {
          left = right;
          right = mkvaux->leftborder[i+1];
          for(jstart=j; i == GETCODE(mkvaux->arcodedistance.spaceUint[j]); j++)
              /* Nothing */ ;
          mid = right - (j-jstart);
          *bckptr++ = left;
          *bckptr++ = mid;
          UPDATELARGEBUCKET;
        }
      } else
      {
        for(i=0; i<virtualtree->numofcodes; i++)
        {
          left = right;
          right = mkvaux->leftborder[i+1];
          mid = right - mkvaux->specialtable[i];
          *bckptr++ = left;
          *bckptr++ = mid;
          UPDATELARGEBUCKET;
        }
      }
      DEBUG2(1,"# largebuckets=%lu(%.2f)\n",
                  (Showuint) largebuckets,
                  (double) largebuckets/virtualtree->numofcodes);
      DEBUG2(1,"# overhangsize=%lu(%.2f)\n",
                  (Showuint) overhangsize,
                  (double) overhangsize/virtualtree->multiseq.totallength);
      if(input->storeonfile)
      {
        FILE *bckfp;
        
        if((bckfp = openthefile(input,"bck")) == NULL)
        {
          return (Sint) -1;
        }
        if(WRITETOFILEHANDLE(virtualtree->bcktab,
                             (Uint) sizeof(Uint),
                             UintConst(2) * virtualtree->numofcodes,
                             bckfp) != 0)
        {
          return (Sint) -2;
        }
        if(DELETEFILEHANDLE(bckfp) != 0)
        {
          return (Sint) -3;
        }
        FREESPACE(virtualtree->bcktab);
      } else
      {
        virtualtree->constructed |= BCKTAB;
      }
    }
    FREEARRAY(&mkvaux->arcodedistance,Uint)
    FREESPACE(mkvaux->specialtable);
    FREESPACE(mkvaux->leftborder);
  }
  return 0;
}

static Sint makesuftab(Input *input,Virtualtree *virtualtree)
{
  if((input->demand & SUFTAB) && input->storeonfile)
  {
    FILE *suffp;

    if((suffp = openthefile(input,SUFTABSUFFIX)) == NULL)
    {
      return (Sint) -1;
    }
    if(WRITETOFILEHANDLE(virtualtree->suftab,
                         (Uint) sizeof(Uint),
                         virtualtree->multiseq.totallength+UintConst(1),
                         suffp) != 0)
    {
      return (Sint) -2;
    }
    if(DELETEFILEHANDLE(suffp) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

static Sint makebwttab(Input *input,Virtualtree *virtualtree)
{
  if(input->demand & BWTTAB)
  {
    if(input->storeonfile)
    {
      virtualtree->bwttab = (Uchar *) virtualtree->suftab;
    } else
    {
      ALLOCASSIGNSPACE(virtualtree->bwttab,NULL,Uchar,
                       virtualtree->multiseq.totallength+1);
      virtualtree->constructed |= BWTTAB;
    }
    if(encodeburrowswheeler(virtualtree->bwttab,
                            virtualtree->suftab,virtualtree->multiseq.sequence,
                            virtualtree->multiseq.totallength) != 0)
    {
      return (Sint) -1;
    }
    if(input->storeonfile)
    {
      FILE *bwtfp;

      if((bwtfp = openthefile(input,"bwt")) == NULL)
      {
        return (Sint) -2;
      }
      DEBUG1(2,"# longest=%lu\n",(Showuint) virtualtree->longest.uintvalue);
      if(WRITETOFILEHANDLE(virtualtree->bwttab,
                           (Uint) sizeof(Uchar),
                           virtualtree->multiseq.totallength+UintConst(1),
                           bwtfp) != 0)
      {
        return (Sint) -2;
      }
      /*
      outputUintvalue(bwtfp,virtualtree->longest.uintvalue);
      */
      if(DELETEFILEHANDLE(bwtfp) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  return 0;
}

#ifdef WITHLCP

static Sint makeprjtab(Input *input,Virtualtree *virtualtree)
{
  if(SOMETHINGOUT(input) && input->storeonfile)
  {
    FILE *prjfp;
    Uint i, startseq;

    if((prjfp = openthefile(input,PROJECTFILESUFFIX)) == NULL)
    {
      return (Sint) -1;
    }
#ifdef DEBUG
    if(virtualtree->multiseq.totalnumoffiles > 0)
    {
      for(i=0; i < virtualtree->multiseq.totalnumoffiles-1; i++)
      {
        DEBUG3(2,"# %lu: markpos=%lu,filesep=%lu\n",
                   (Showuint) i,
                   (Showuint) virtualtree->multiseq.markpos.spaceUint[i],
                   (Showuint) virtualtree->multiseq.filesep[i]);
        if(virtualtree->multiseq.sequence[virtualtree->multiseq.filesep[i]] 
           != SEPARATOR)
        {
          fprintf(stderr,"position %lu has no separator\n",
                          (Showuint) virtualtree->multiseq.filesep[i]);
          exit(EXIT_FAILURE);
        }
      }
    }
#endif  // DEBUG
    startseq = 0;
    for(i=0; i < virtualtree->multiseq.totalnumoffiles; i++)
    {
      fprintf(prjfp,"%s=",
                    (i < virtualtree->multiseq.totalnumoffiles - 
                         virtualtree->multiseq.numofqueryfiles) ? 
                                      "dbfile" : "queryfile");
      fprintf(prjfp,"%s %lu ",virtualtree->multiseq.allfiles[i].filenamebuf,
                (Showuint) virtualtree->multiseq.allfiles[i].filelength);
      if(i == virtualtree->multiseq.totalnumoffiles - 1)
      {
        fprintf(prjfp,"%lu\n",
                 (Showuint) (virtualtree->multiseq.totallength-startseq));
      } else
      {
        fprintf(prjfp,"%lu\n",
                  (Showuint) (virtualtree->multiseq.filesep[i]-startseq));
        startseq = virtualtree->multiseq.filesep[i]+1;
      }
    }
    fprintf(prjfp,"totallength=%lu\n",
            (Showuint) virtualtree->multiseq.totallength);
    fprintf(prjfp,"specialcharacters=%lu\n",
           (Showuint) virtualtree->multiseq.specialcharinfo.specialcharacters);
    fprintf(prjfp,"specialranges=%lu\n",
           (Showuint) virtualtree->multiseq.specialcharinfo.specialranges);
    fprintf(prjfp,"lengthofspecialprefix=%lu\n",
           (Showuint) virtualtree->multiseq.specialcharinfo
                                           .lengthofspecialprefix);
    fprintf(prjfp,"lengthofspecialsuffix=%lu\n",
           (Showuint) virtualtree->multiseq.specialcharinfo
                                           .lengthofspecialsuffix);
    fprintf(prjfp,"numofsequences=%lu\n",
            (Showuint) virtualtree->multiseq.numofsequences);
    fprintf(prjfp,"numofdbsequences=%lu\n",
            (Showuint) NUMOFDATABASESEQUENCES(&virtualtree->multiseq));
    fprintf(prjfp,"numofquerysequences=%lu\n",
            (Showuint) virtualtree->multiseq.numofquerysequences);
    if(input->storeonfile && virtualtree->longest.defined)
    {
      fprintf(prjfp,"longest=%lu\n",(Showuint) virtualtree->longest.uintvalue);
    }
    fprintf(prjfp,"prefixlength=%lu\n",(Showuint) virtualtree->prefixlength);
    if(input->demand & LCPTAB)
    {
      fprintf(prjfp,"largelcpvalues=%lu\n",
              (Showuint) virtualtree->largelcpvalues.nextfreePairUint);
      fprintf(prjfp,"maxbranchdepth=%lu\n",
              (Showuint) virtualtree->maxbranchdepth);
    } else
    {
      fprintf(prjfp,"largelcpvalues=0\n");
      fprintf(prjfp,"maxbranchdepth=0\n");
    }
    fprintf(prjfp,"integersize=%ld\n",(Showsint) (sizeof(Uint) * CHAR_BIT));
    fprintf(prjfp,"littleendian=%c\n",islittleendian() ? '1' : '0');
    if(input->rcm)
    {
      fprintf(prjfp,"specialindex=0\n");
    }
    if(input->transnum > 0)
    {
      fprintf(prjfp,"specialindex=%lu\n",(Showuint) input->transnum);
    }
    if(DELETEFILEHANDLE(prjfp) != 0)
    {
      return (Sint) -2;
    }
  }
  return 0;
}
#endif // WITHLCP

static Sint makealptab(Input *input,Virtualtree *virtualtree)
{
  if(SOMETHINGOUT(input) && input->storeonfile)
  {
    FILE *alpfp;
    
    if(virtualtree->specialsymbols)
    {
      if((alpfp = openthefile(input,"al1")) == NULL)
      {
        return (Sint) -1;
      }
      if(input->inputalpha.symbolmapfile != NULL)
      {
        FILE *fpin;
        Fgetcreturntype cc;

        fpin = scanpathsforfile("MKVTREESMAPDIR",
                                input->inputalpha.symbolmapfile);
        if(fpin == NULL)
        {
          return (Sint) -1;
        }
        while((cc = fgetc(fpin)) != EOF)
        {
          (void) putc(cc,alpfp);
        }
        if(DELETEFILEHANDLE(fpin) != 0)
        {
          return (Sint) -2;
        }
      } else
      {
        if(input->inputalpha.dnafile)
        {
          fprintf(alpfp,DNAALPHABET);
        } else
        {
          if(input->inputalpha.proteinfile)
          {
            fprintf(alpfp,PROTEINALPHABET);
          }
        }
      }
      if(DELETEFILEHANDLE(alpfp) != 0)
      {
        return (Sint) -3;
      }
    } else
    {
      if(virtualtree->prefixlength > 0)
      {
        Uint i;

        if((alpfp = openthefile(input,"al2")) == NULL)
        {
          return (Sint) -2;
        }
        (void) putc((Fputcfirstargtype) virtualtree->alpha.mapsize,alpfp);
        for(i=0; i<=UCHAR_MAX; i++)
        {
          if(virtualtree->alpha.symbolmap[i] != UCHAR_MAX)
          {
            (void) putc((Fputcfirstargtype) i,alpfp);
          }
        }
        if(DELETEFILEHANDLE(alpfp) != 0)
        {
          return (Sint) -2;
        }
      }
    }
  }
  return 0;
}

#ifdef WITHLCP
static Sint makestitab1(Input *input,Virtualtree *virtualtree)
{
  if(input->demand & STI1TAB)
  {
    Uint i;
    Uchar currentbucketleft = 0;
    BOOL mappedlcp = False;

    if(!(virtualtree->constructed & LCPTAB) && 
       !(virtualtree->mapped & LCPTAB))
    {
      mappedlcp = True;
      if(remaplcptab(input,virtualtree) != 0)
      {
        return (Sint) -1;
      }
    }
    ALLOCASSIGNSPACE(virtualtree->stitab1,NULL,Uchar,
                     virtualtree->multiseq.totallength+1);
    virtualtree->stitab1[virtualtree->suftab[0]] = 0;
    for(i=UintConst(1); i<=virtualtree->multiseq.totallength; i++)
    {
      if(virtualtree->lcptab[i] < (Uchar) virtualtree->prefixlength)
      {
        currentbucketleft = 0; // adjust to beginning of boundary
      } else
      {
        if(currentbucketleft < UCHAR_MAX)
        {
          currentbucketleft++;
        }
      }
      virtualtree->stitab1[virtualtree->suftab[i]] = currentbucketleft;
    }
    if(mappedlcp && DELETEMEMORYMAP(virtualtree->lcptab) != 0)
    {
      return (Sint) -2;
    }
    virtualtree->constructed |= STI1TAB;
    if(input->storeonfile)
    {
      FILE *sti1fp;
      if((sti1fp = openthefile(input,"sti1")) == NULL)
      {
        return (Sint) -3;
      }
      if(WRITETOFILEHANDLE(virtualtree->stitab1,
                           (Uint) sizeof(Uchar),
                           virtualtree->multiseq.totallength+UintConst(1),
                           sti1fp) != 0)
      {
        return (Sint) -4;
      }
      if(DELETEFILEHANDLE(sti1fp) != 0)
      {
        return (Sint) -5;
      }
    }
  }
  return 0;
}
#endif

#ifdef DEBUG
#include "qgram2code.c"

static Sint remapbcktab(Input *input,Virtualtree *virtualtree)
{
  char tmpfilename[PATH_MAX+EXTRASUFFIXLENGTH+1];
  Uint numofbytes;

  sprintf(tmpfilename,"%s.%s",input->indexname,"bck");
  virtualtree->bcktab 
    = (Uint *) CREATEMEMORYMAP(tmpfilename,False,&numofbytes);
  if(virtualtree->bcktab == NULL)
  {
    ERROR1("cannot read %s",tmpfilename);
    return (Sint) -1;
  }
  if(numofbytes != (UintConst(2) * 
                    virtualtree->numofcodes * (Uint) sizeof(Uint)))
  {
    ERROR2("file %s is must be of size %lu",tmpfilename,
           (Showuint) (2 * virtualtree->numofcodes * (Uint) sizeof(Uint)));
    return (Sint) -2;
  }
  return 0;
}

static Sint checkstitab1(Input *input,Virtualtree *virtualtree)
{

  if((virtualtree->constructed & STI1TAB) &&
     (virtualtree->constructed & SUFTAB))
  {
    Uint i, code = 0, left, right, *bckptr; 
    BOOL mappedbck = False,
         stitabconstructed = False;

    if(!(virtualtree->constructed & STITAB))
    {
      ALLOCASSIGNSPACE(virtualtree->stitab,NULL,Uint,
                       virtualtree->multiseq.totallength+1);
      for(i=0; i<=virtualtree->multiseq.totallength; i++)
      {
        virtualtree->stitab[virtualtree->suftab[i]] = i;
      }
      stitabconstructed = True;
      virtualtree->constructed |= STITAB;
    }
    if(!(virtualtree->constructed & BCKTAB) && 
       !(virtualtree->mapped & BCKTAB))
    {
      mappedbck = True;
      if(remapbcktab(input,virtualtree) != 0)
      {
        return (Sint) -1;
      }
    }
    for(i=0; i < virtualtree->multiseq.totallength - 
                 virtualtree->prefixlength + 1; 
        i++)
    {
      if(qgram2code(&code,
                    virtualtree->alpha.mapsize-1,
                    virtualtree->prefixlength,
                    virtualtree->multiseq.sequence+i))
      {
        bckptr = virtualtree->bcktab + MULT2(code);
        left = *bckptr;  // left boundary of bucket
        right = *(bckptr+1);
        if(right <= left)
        {
          fprintf(stderr,"Empty interval for code %lu\n",(Showuint) code);
          exit(EXIT_FAILURE);
        }
        right--;
        if(virtualtree->stitab[i] < left)
        {
          fprintf(stderr,"stitab[%lu]=%lu < left=%lu,code=%lu\n",
                          (Showuint) i,
                          (Showuint) virtualtree->stitab[i],
                          (Showuint) left,
                          (Showuint) code);
          exit(EXIT_FAILURE);
        }
        if(virtualtree->stitab[i] > right)
        {
          fprintf(stderr,"stitab[%lu]=%lu > right=%lu,code=%lu\n",
                          (Showuint) i,
                          (Showuint) virtualtree->stitab[i],
                          (Showuint) right,
                          (Showuint) code);
          exit(EXIT_FAILURE);
        }
        if(virtualtree->stitab1[i] < UCHAR_MAX)
        {
          if(left + virtualtree->stitab1[i] != virtualtree->stitab[i])
          {
            fprintf(stderr,"left + stitab1[%lu]=%lu+%lu != stitab[%lu]=%lu\n",
                            (Showuint) i,
                            (Showuint) virtualtree->stitab1[i],
                            (Showuint) left,
                            (Showuint) i,
                            (Showuint) virtualtree->stitab[i]);
            exit(EXIT_FAILURE);
          }
        }
      }
    }
    if(mappedbck && DELETEMEMORYMAP(virtualtree->bcktab) != 0)
    {
      return (Sint) -1;
    }
    if(stitabconstructed)
    {
      FREESPACE(virtualtree->stitab);
    }
  }
  return 0;
}
#endif

#ifdef WITHLCP
static Sint makeskptab(Input *input,Virtualtree *virtualtree)
{
  if(input->demand & SKPTAB)
  {
    if(input->storeonfile)
    {
      if(remaplcptab(input,virtualtree) != 0)
      {
        return (Sint) -1;
      }
      virtualtree->skiptab = (Uint *) virtualtree->suftab;
    } else
    {
      ALLOCASSIGNSPACE(virtualtree->skiptab,NULL,Uint,
                       virtualtree->multiseq.totallength+1);
    }
    makeskiptable(virtualtree->skiptab,virtualtree->lcptab,
                  &virtualtree->largelcpvalues,
                  virtualtree->multiseq.totallength);
    if(input->storeonfile)
    {
      FILE *skpfp;

      if((skpfp = openthefile(input,"skp")) == NULL)
      {
        return (Sint) -2;
      }
      if(WRITETOFILEHANDLE(virtualtree->skiptab,
                          (Uint) sizeof(Uint),
                          virtualtree->multiseq.totallength+UintConst(1),
                          skpfp) != 0)
      {
        return (Sint) -3;
      }
      if(DELETEFILEHANDLE(skpfp) != 0)
      {
        return (Sint) -4;
      }
      if(DELETEMEMORYMAP(virtualtree->lcptab) != 0)
      {
        return (Sint) -5;
      }
    } else
    {
      virtualtree->constructed |= SKPTAB;
    }
  }
  return 0;
}
#endif

static Sint freeeverything(Input *input,MKVaux *mkvaux,
                           Virtualtree *virtualtree,BOOL dosort)
{
  if(dosort)
  {
#ifdef WITHLCP
    FREESPACE(mkvaux->lcpsubtab);
    if(input->storeonfile)
    {
      FREEARRAY(&virtualtree->largelcpvalues,PairUint);
    }
#endif
  }
  return 0;
}

static void showseqlen(Input *input,Virtualtree *virtualtree)
{
  if(input->showverbose != NULL)
  {
    char sbuf[256] = {0};
    Sprintfreturntype start 
      = sprintf(sbuf,"total length of sequences: %lu",
                (Showuint) virtualtree->multiseq.totallength);
    if(virtualtree->multiseq.numofsequences > UintConst(1))
    {
      start += sprintf(sbuf+start," (including %lu separator%s)",
                       (Showuint) (virtualtree->multiseq.numofsequences - 1),
                       (virtualtree->multiseq.numofsequences == UintConst(2))
                               ? "" : "s");
    } 
    input->showverbose(sbuf);
    if(virtualtree->prefixlength > 0)
    {
      verbosealphabet(&virtualtree->alpha,input->showverbose);
    } 
  }
}

static Sint determinelongest(DefinedUint *longest,
                             Uint *suftab,Uint totallength)
{
  Uint i;

  for(i=0; i<=totallength; i++)
  {
    if(suftab[i] == 0)
    {
      longest->uintvalue = i;
      longest->defined = True;
      return 0;
    }
  }
  ERROR0("cannot find index i satisfying suftab[i] = 0");
  return (Sint) -1;
}

Sint mkvtreeprocess(Input *input,Virtualtree *virtualtree)
{
  MKVaux mkvaux;
  BOOL dosort;

  if(virtualtree->prefixlength > 0)
  {
    Uint maxprefixlen;

    if(virtualtree->prefixlength == AUTOPREFIXLENGTH)
    {
      ERROR0("prefix needs to be determined automatically");
      return (Sint) -1;
    }
    maxprefixlen 
      = vm_whatisthemaximalprefixlength(input->numofchars,
                                     virtualtree->multiseq.totallength,
                                     SIZEOFBCKENTRY,
                                     (Uint) PREFIXLENBITS);
    if(vm_checkprefixlength(maxprefixlen,
                         virtualtree->prefixlength) != 0)
    {
      return (Sint) -1;
    }
    vm_showmaximalprefixlength(input->showverbose,
                            maxprefixlen,
                            vm_recommendedprefixlength(
                                  input->numofchars,
                                  virtualtree->multiseq.totallength,
                                  SIZEOFBCKENTRY));
  }
#ifdef COUNT
  virtualtree->callgetexception = virtualtree->llvcachehit = 0;
#endif
  virtualtree->llvcachemin = virtualtree->llvcachemax = NULL;
  virtualtree->mapped = virtualtree->constructed = 0;
  virtualtree->bwttab = virtualtree->lcptab = NULL;
  virtualtree->suftab = virtualtree->skiptab = virtualtree->bcktab = NULL;
  showseqlen(input,virtualtree);
  if(input->storeonfile && SOMETHINGOUT(input))
  {
    if(removeexistingindex(input->showverbose,input->indexname) != 0)
    {
      return (Sint) -1;
    }
  }
  if(makessptab(input,virtualtree) != 0)
  {
    return (Sint) -2;
  }
  if(maketistab(input,virtualtree) != 0)
  {
    return (Sint) -3;
  }
  if(makeoistab(input,virtualtree) != 0)
  {
    return (Sint) -4;
  }
  if(makedestab(input,virtualtree) != 0)
  {
    return (Sint) -5;
  }
  mkvaux.specialtable = NULL;
  mkvaux.leftborder = NULL;
  INITARRAY(&mkvaux.arcodedistance,Uint);
  if((input->demand & (TISTAB | OISTAB))  && 
     !(input->demand & (BWTTAB | SUFTAB | LCPTAB | BCKTAB | 
                        DESTAB | STI1TAB | SKPTAB)))
  {
    dosort = False;
  } else
  {
    dosort = True;
  }
  if(dosort)
  {
    FILE *lcpfp = NULL;
    BOOL demandlcp = False;

#ifdef WITHLCP
    if(input->demand & LCPTAB)
    {
      demandlcp = True;
      if(input->storeonfile)
      {
        if((lcpfp = openthefile(input,LCPTABSUFFIX)) == NULL)
        {
          return (Sint) -6;
        }
      } else
      {
        ALLOCASSIGNSPACE(virtualtree->lcptab,
                         NULL,Uchar,
                         virtualtree->multiseq.totallength+1);
        virtualtree->constructed |= LCPTAB;
      }
    } else
    {
      demandlcp = False;
    }
#endif
    if(virtualtree->specialsymbols)
    {
      if(besesuffixsortspecial(&mkvaux,virtualtree,input->numofchars,
                               demandlcp,lcpfp,input->showverbose,
                               &input->maxdepth) != 0)
      {
        return (Sint) -6;
      }
    } else
    {
      if(besesuffixsort(&mkvaux,virtualtree,input->numofchars,
                        demandlcp,lcpfp,input->showverbose,
                        &input->maxdepth) != 0)
      {
        return (Sint) -7;
      }
    }
#ifdef WITHLCP
    if(input->demand & LCPTAB)
    {
      if(input->storeonfile)
      {
        if(DELETEFILEHANDLE(lcpfp) != 0)
        {
          return (Sint) -8;
        }
      }
    }
#endif
  }
#ifdef WITHLCP
  if(makelargelcp(input,virtualtree) != 0)
  {
    return (Sint) -8;
  }
#endif
  if(makebcktab(input,virtualtree,&mkvaux) != 0)
  {
    return (Sint) -9;
  }

  if(dosort)
  {
#ifdef SUFFIXPTR
    virtualtree->suftab = transsuftab(mkvaux.sortedsuffixes,
                                      virtualtree->multiseq.sequence,
                                      virtualtree->multiseq.totallength);
#else
    virtualtree->suftab = mkvaux.sortedsuffixes;
#endif
    virtualtree->constructed |= SUFTAB;
    if(determinelongest(&virtualtree->longest,
                        virtualtree->suftab,
                        virtualtree->multiseq.totallength) != 0)
    {
      return (Sint) -19;
    }
#ifdef DEBUG
    if(checksufencoding((virtualtree->specialsymbols > 0) ? True : False,
                        virtualtree->suftab,
                        virtualtree->longest.uintvalue,
                        virtualtree->multiseq.sequence,
                        virtualtree->multiseq.totallength) != 0)
    {
      return (Sint) -10;
    }
#endif
  }
  if(makesuftab(input,virtualtree) != 0)
  {
    return (Sint) -10;
  }
#ifdef WITHLCP
  if(makestitab1(input,virtualtree) != 0)
  {
    return (Sint) -12;
  }
#endif
#ifdef DEBUG
  if(checkstitab1(input,virtualtree) != 0)
  {
    return (Sint) -13;
  }
#endif
  if(makebwttab(input,virtualtree) != 0)
  {
    return (Sint) -16;
  }
#ifdef WITHLCP
  if(makeprjtab(input,virtualtree) != 0)
  {
    return (Sint) -17;
  }
#endif
  if(makealptab(input,virtualtree) != 0)
  {
    return (Sint) -18;
  }
#ifdef WITHLCP
  if(makeskptab(input,virtualtree) != 0)
  {
    return (Sint) -19;
  }
#endif
  if(!(input->demand & LCPTAB))
  {
    virtualtree->largelcpvalues.nextfreePairUint = 0;
  }
  if(freeeverything(input,&mkvaux,virtualtree,dosort) != 0)
  {
    return (Sint) -20;
  }
  return 0;
}
