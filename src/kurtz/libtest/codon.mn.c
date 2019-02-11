#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "args.h"
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "fhandledef.h"
#include "spacedef.h"
#include "megabytes.h"
#include "genfile.h"
#include "inputsymbol.h"

#include "codon.pr"
#include "alphabet.pr"
#include "filehandle.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "sixframe.pr"
#include "readvirt.pr"

typedef struct
{
  FILE *fileptr;
  Multiseq *dnamultiseq;
  Uint transnum;
  BOOL withframe;
} Codonwriteinfo;

#define FWRITEPROTEIN(FWD)\
        (void) putc(FASTASEPARATOR,codonwriteinfo->fileptr);\
        if (codonwriteinfo->withframe || (FWD && frame == 0))\
        {\
          if(WRITETOFILEHANDLE(descstart,\
                               (Uint) sizeof(Uchar),\
                               desclength-1,\
                               codonwriteinfo->fileptr) != 0)\
          {\
            return (Sint) -1;\
          }\
        }\
        if(codonwriteinfo->withframe)\
        {\
          fprintf(codonwriteinfo->fileptr," frame %ld\n",\
                 (Showsint) ((FWD) ? frame + 1 : frame - 1));\
          showfastaformatted(codonwriteinfo->fileptr,proteinseq,\
                             (Uint) plen);\
        } else\
        {\
          (void) putc('\n',codonwriteinfo->fileptr);\
          if(WRITETOFILEHANDLE(proteinseq,\
                               (Uint) sizeof(Uchar),\
                               (Uint) plen,\
                               codonwriteinfo->fileptr) != 0)\
          {\
            return (Sint) -1;\
          }\
          (void) putc('\n',codonwriteinfo->fileptr);\
        }

static void showfastaformatted(FILE *fpout,const Uchar *start,Uint wlen)
{
  Uint idx, j;
  const Uint width = 60;

  for (idx = 0, j = 0; /* Nothing */; idx++)
  {
    (void) putc(start[idx],fpout);
    if (idx == wlen - 1)
    {
      fprintf(fpout,"\n");
      break;
    }
    j++;
    if (j >= width)
    {
      fprintf(fpout,"\n");
      j = 0;
    }
  }
}

static Sint translateDNA(void *info,Uint seqnum,Uchar *start,Uint len)
{
  Codonwriteinfo *codonwriteinfo = (Codonwriteinfo *) info;
  Uchar *proteinseq, *descstart;
  Sint frame, plen;
  Uint desclength;

  ALLOCASSIGNSPACE(proteinseq,NULL,Uchar,len/3+1);
  desclength = DESCRIPTIONLENGTH(codonwriteinfo->dnamultiseq,seqnum);
  descstart = DESCRIPTIONPTR(codonwriteinfo->dnamultiseq,seqnum);
  for(frame=0; frame <= 2; frame++)
  {
    plen = translateDNAforward(codonwriteinfo->transnum,
                               frame,proteinseq,start,len);
    if(plen < 0)
    {
      return -1;
    }
    FWRITEPROTEIN(True);
  }
  for(frame=0; frame >= -2; frame--)
  {
    plen = translateDNAbackward(codonwriteinfo->transnum,
                                frame,proteinseq,start,len);
    if(plen < 0)
    {
      return -1;
    }
    FWRITEPROTEIN(False);
  }
  FREESPACE(proteinseq);
  return 0;
}

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

static Sint checksixframetranslation(BOOL withframe,
                                     Uint transnum,Multiseq *dnamultiseq)
{
  Alphabet proteinalphabet;
  Codonwriteinfo codonwriteinfo;
  Multiseq proteinmultiseq;
  Tmpfiledesc tmpfiledesc;

  if(!withframe)
  {
    inittmpfiledesc(&tmpfiledesc);
    if(MAKETMPFILE(&tmpfiledesc,NULL) != 0)
    {
      return (Sint) -1;
    }
    codonwriteinfo.fileptr = tmpfiledesc.tmpfileptr;
    printf("# write sequence to file %s\n",tmpfiledesc.tmpfilenamebuffer);
  } else
  {
    codonwriteinfo.fileptr = stdout;
  }
  codonwriteinfo.dnamultiseq = dnamultiseq;
  codonwriteinfo.transnum = transnum;
  codonwriteinfo.withframe = withframe;
  if(overalloriginalsequences(dnamultiseq,
                              &codonwriteinfo,
                              translateDNA) != 0)
  {
    return (Sint) -1;
  }
  assignProteinalphabet(&proteinalphabet);
  if(multisixframetranslateDNA(transnum,
                               True,
                               &proteinmultiseq,
                               dnamultiseq,
                               proteinalphabet.symbolmap) != 0)
  {
    return (Sint) -2;
  }
  if(proteinmultiseq.sequence == NULL)
  {
    fprintf(stderr,"(1) proteinmultiseq.sequence == NULL\n");
    exit(EXIT_FAILURE);
  }
  if(!withframe)
  {
    Multiseq readinmultiseq;
    Uchar *input;
    Uint inputlen;

    if(DELETEFILEHANDLE(tmpfiledesc.tmpfileptr) != 0)
    {
      return (Sint) -6;
    }
    input = CREATEMEMORYMAP(tmpfiledesc.tmpfilenamebuffer,True,&inputlen);
    if(input == NULL)
    {
      return (Sint) -7;
    }
    readinmultiseq.originalsequence = NULL;
    initmultiseqfileinfo(&readinmultiseq);
    if(readmultiplefastafile(&proteinalphabet,True,
                             &readinmultiseq,input,inputlen)  != 0)
    {
      return (Sint) -9;
    }
    if(proteinmultiseq.sequence == NULL)
    {
      fprintf(stderr,"(2) proteinmultiseq.sequence == NULL\n");
      exit(EXIT_FAILURE);
    }
    compareMultiseq(&proteinmultiseq,&readinmultiseq);
    freemultiseq(&readinmultiseq);
    if(unlink(tmpfiledesc.tmpfilenamebuffer) != 0)
    {
      ERROR2("cannot remove file \"%s\": %s",
              tmpfiledesc.tmpfilenamebuffer,strerror(errno));
      return (Sint) -10;
    }
    wraptmpfiledesc(&tmpfiledesc);
  }
  FREESPACE(proteinmultiseq.originalsequence);
  freemultiseq(&proteinmultiseq);
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  Uint transnum;
  Scaninteger readint;
  BOOL withframe;
#ifndef NOSPACEBOOKKEEPING
  Uint peak, mmpeak;
#endif

  CHECKARGNUM(4,"(frame|noframe) transnum filename");
  DEBUGLEVELSET;
  
  if(strcmp(argv[1],"frame") == 0)
  {
    withframe = True;
  } else
  {
    if(strcmp(argv[1],"noframe") == 0)
    {
      withframe = False;
    } else
    {
      fprintf(stderr,
              "%s: argument 1 must be keyword \"frame\" or \"noframe\"\n",
              argv[0]);
      return EXIT_FAILURE;
    }
  }
  if(sscanf(argv[2],"%ld",&readint) != 1 || readint < 0)
  {
    fprintf(stderr,"%s: argument 2 must be non-negative integer\n",argv[0]);
    return EXIT_FAILURE;
  }
  transnum = (Uint) readint;
  if(checktransnum(transnum) != 0)
  {
    STANDARDMESSAGE;
  }
  if(mapvirtualtreeifyoucan(&virtualtree,argv[3],OISTAB | DESTAB | SSPTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(!withframe)
  {
    if(showvirtualtreestatus(&virtualtree,argv[3],showonstdout) != 0)
    {
      STANDARDMESSAGE;
    }
  }
  if(checksixframetranslation(withframe,transnum,&virtualtree.multiseq) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
  outrubytranstable(transnum);
#ifndef NOSPACEBOOKKEEPING
  if(!withframe)
  {
    printf("# overall space peak: ");
    peak = getspacepeak();
    printf("main: %.2f MB (rate=%.2f), ",MEGABYTES(peak),
                              (double) peak/virtualtree.multiseq.totallength);
    mmpeak = mmgetspacepeak();
    printf("secondary: %.2f MB (rate=%.2f)\n",MEGABYTES(mmpeak),
                              (double) peak/virtualtree.multiseq.totallength);
  }
  checkspaceleak();
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
