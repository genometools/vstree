#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "errordef.h"
#include "megabytes.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "esafileend.h"
#include "spacedef.h"
#include "chardef.h"
#include "inputsymbol.h"
#include "divmodmul.h"
#include "genfile.h"
#include "optdesc.h"
#include "fhandledef.h"
#include "inputdef.h"

#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "accvirt.pr"
#include "readmulti.pr"
#include "detpfxlen.pr"
#include "procopt.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "alphabet.pr"
#include "safescpy.pr"

#include "mkvprocess.pr"
#include "mkvtree.pr"

static void showonstdout(char *s)
{
  printf("%s\n",s);
}

#ifdef DEBUG
static Sint alsoechothesequenceinbothdirections(Multiseq *multiseq,
                                                Uchar *origsequence,
                                                Uchar *rcorigsequence,
                                                Uint seqlen,
                                                Uint seqnum,
                                                FILE *fpout)
{
  Uint rcmode, desclength;

  for(rcmode=0; rcmode<UintConst(2); rcmode++)
  {
    (void) putc(FASTASEPARATOR,fpout);
    desclength = DESCRIPTIONLENGTH(multiseq,seqnum);
    if(WRITETOFILEHANDLE(DESCRIPTIONPTR(multiseq,seqnum),
                         (Uint) sizeof(Uchar),
                         desclength,
                         fpout) != 0)
    {
      ERROR1("Cannot write %lu items of type Uchar",(Showuint) desclength);
      return (Sint) -1;
    }
    if(formatseq(fpout,
                 0,
                 NULL,
                 0,
                 UintConst(70),
                 rcmode ? rcorigsequence : origsequence,
                 seqlen) != 0)
    {
      return (Sint) -2;
    }
  }
  return 0;
}

static Sint mkrcsequences2indexbruteforce(Multiseq *rcplusmultiseq,
                                          Multiseq *multiseq)
{
  Uint seqnum, seqlen, inputlen;
  PairUint range;
  Uchar *rcorigsequence, *input;
  Tmpfiledesc tmpfiledesc;
  ExtremeAverageSequences extreme;
  Alphabet alpha;

  if(calculateseqparm(multiseq,&extreme) != 0)
  {
    return (Sint) -1;
  }
  ALLOCASSIGNSPACE(rcorigsequence,NULL,Uchar,extreme.maxlength);
  inittmpfiledesc(&tmpfiledesc);
  if(MAKETMPFILE(&tmpfiledesc,NULL) != 0)
  {
    return (Sint) -2;
  }
  printf("# write sequence to file %s\n",tmpfiledesc.tmpfilenamebuffer);
  for(seqnum=0; seqnum < multiseq->numofsequences; seqnum++)
  {
    if(findboundaries(multiseq,seqnum,&range) != 0)
    {
      return (Sint) -3;
    }
    seqlen = range.uint1 - range.uint0 + 1;
    if(makereversecomplementorig(rcorigsequence,
                                 multiseq->originalsequence + range.uint0,
                                 seqlen) != 0)
    {
      return (Sint) -4;
    }
    if(alsoechothesequenceinbothdirections(multiseq,
                                           multiseq->originalsequence 
                                             + range.uint0,
                                           rcorigsequence,
                                           seqlen,
                                           seqnum,
                                           tmpfiledesc.tmpfileptr) != 0)
    {
      return (Sint) -5;
    }
  }
  if(DELETEFILEHANDLE(tmpfiledesc.tmpfileptr) != 0)
  {
    return (Sint) -6;
  }
  FREESPACE(rcorigsequence);
  input = CREATEMEMORYMAP(tmpfiledesc.tmpfilenamebuffer,True,&inputlen);
  if(input == NULL)
  {
    return (Sint) -7;
  }
  assignDNAalphabet(&alpha);
  rcplusmultiseq->originalsequence = NULL;
  rcplusmultiseq->totalnumoffiles = multiseq->totalnumoffiles;
  if(multiseq->numofqueryfiles > 0)
  {
    ERROR0("no query files allowed when constructing "
           "reverse complemented index");
    return (Sint) -8;
  }
  rcplusmultiseq->numofqueryfiles = 0;
  copyfilenamesofMultiseq(rcplusmultiseq,multiseq);
  if(readmultiplefastafile(&alpha,True,
                           rcplusmultiseq,input,inputlen)  != 0)
  {
    return (Sint) -9;
  }
  if(unlink(tmpfiledesc.tmpfilenamebuffer) != 0)
  {
    ERROR2("cannot remove file \"%s\": %s",
            tmpfiledesc.tmpfilenamebuffer,strerror(errno));
    return (Sint) -10;
  }
  wraptmpfiledesc(&tmpfiledesc);
  return 0;
}

#endif

static Sint initrcplus(Multiseq *rcplusmultiseq,Multiseq *multiseq)
{
  ALLOCASSIGNSPACE(rcplusmultiseq->startdesc,
                   NULL,Uint,MULT2(multiseq->numofsequences + 1));
  rcplusmultiseq->descspace.nextfreeUchar 
    = MULT2(multiseq->descspace.nextfreeUchar); 
  rcplusmultiseq->descspace.allocatedUchar 
    = rcplusmultiseq->descspace.nextfreeUchar;
  ALLOCASSIGNSPACE(rcplusmultiseq->descspace.spaceUchar,
                   NULL,Uchar,rcplusmultiseq->descspace.allocatedUchar);

  rcplusmultiseq->numofsequences = MULT2(multiseq->numofsequences);
  rcplusmultiseq->totallength = MULT2(multiseq->totallength) + 1;
  ALLOCASSIGNSPACE(rcplusmultiseq->sequence,NULL,Uchar,
                   rcplusmultiseq->totallength);

  rcplusmultiseq->markpos.nextfreeUint = 0; 
  rcplusmultiseq->markpos.allocatedUint 
    = MULT2(multiseq->numofsequences) - 1;
  ALLOCASSIGNSPACE(rcplusmultiseq->markpos.spaceUint,
                   NULL,Uint,rcplusmultiseq->markpos.allocatedUint);
  
  rcplusmultiseq->rcsequence = NULL;
  rcplusmultiseq->originalsequence = NULL;
  rcplusmultiseq->rcoriginalsequence = NULL;
  rcplusmultiseq->totalnumoffiles = multiseq->totalnumoffiles;
  if(HASINDEXEDQUERIES(multiseq))
  {
    ERROR0("no query files allowed when constructing "
           "reverse complemented index");
    return (Sint) -1;
  }
  rcplusmultiseq->numofqueryfiles = 0;
  copyfilenamesofMultiseq(rcplusmultiseq,multiseq);
#ifdef DEBUG
  if(multiseq->originalsequence == NULL)
  {
    ERROR0("need original sequence for the output: use option -ois");
    return (Sint) -2;
  }
#endif
  return 0;
}

static Sint mkrcsequences2index(Multiseq *rcplusmultiseq,Multiseq *multiseq)
{
  Uint seqnum, seqlen, offset = 0, desclength, descoffset = 0;
  PairUint range;
  Uchar *rcsequence, *descstart;
  ExtremeAverageSequences extreme;

  if(initrcplus(rcplusmultiseq,multiseq) != 0)
  {
    return (Sint) -1;
  }
  if(calculateseqparm(multiseq,&extreme) != 0)
  {
    return (Sint) -2;
  }
  ALLOCASSIGNSPACE(rcsequence,NULL,Uchar,extreme.maxlength);
  for(seqnum=0; seqnum < multiseq->numofsequences; seqnum++)
  {
    desclength = DESCRIPTIONLENGTH(multiseq,seqnum);
    rcplusmultiseq->startdesc[seqnum << 1] = descoffset;
    descstart = DESCRIPTIONPTR(multiseq,seqnum);
    memcpy(rcplusmultiseq->descspace.spaceUchar + descoffset,
           descstart,(size_t) desclength);
    descoffset += desclength;
    rcplusmultiseq->startdesc[(seqnum << 1) + 1] = descoffset;
    memcpy(rcplusmultiseq->descspace.spaceUchar + descoffset,
           descstart,(size_t) desclength);
    descoffset += desclength;
    if(findboundaries(multiseq,seqnum,&range) != 0)
    {
      return (Sint) -4;
    }
    seqlen = range.uint1 - range.uint0 + 1;
    memcpy(rcplusmultiseq->sequence + offset,
           multiseq->sequence + range.uint0,
           (size_t) seqlen);
    offset += seqlen;
    rcplusmultiseq->markpos.spaceUint[rcplusmultiseq->markpos.nextfreeUint++] 
      = offset;
    rcplusmultiseq->sequence[offset++] = SEPARATOR;
    if(makereversecomplement(rcsequence,
                             multiseq->sequence + range.uint0,
                             seqlen) != 0)
    {
      return (Sint) -5;
    }
    memcpy(rcplusmultiseq->sequence + offset,
           rcsequence,
           (size_t) seqlen);
    offset += seqlen;
    if(seqnum < multiseq->numofsequences - 1)
    {
      rcplusmultiseq->markpos.spaceUint[rcplusmultiseq->markpos.nextfreeUint++] 
        = offset;
      rcplusmultiseq->sequence[offset++] = SEPARATOR;
    }
  }
  rcplusmultiseq->startdesc[seqnum << 1] = descoffset;
  if(offset != rcplusmultiseq->totallength)
  {
    fprintf(stderr,"offset = %lu != %lu = totallength\n",
                   (Showuint) offset,(Showuint) rcplusmultiseq->totallength);
    exit(EXIT_FAILURE);
  }
  FREESPACE(rcsequence);
  return 0;
}


static Sint makercplusindex(Virtualtree *virtualtree,
                            char *indexname,
                            Showverbose showverbose)
{
  Input input;
  
  assignDNAalphabet(&virtualtree->alpha);
  input.storeonfile = True;
  input.rev = False;
  input.rcm = True;
  input.demand = SUFTAB | BWTTAB | LCPTAB | TISTAB;
  input.maxdepth.defined = False;
  input.maxdepth.uintvalue = 0;
  input.numofchars = virtualtree->alpha.mapsize - UintConst(1);
  if(safestringcopy(&input.indexname[0],indexname,PATH_MAX) != 0)
  {
    return (Sint) -1;
  }
  strcat(&input.indexname[0],RCMSUFFIX);
  input.inputalpha.dnafile = True;
  input.inputalpha.proteinfile = False;
  input.inputalpha.symbolmapfile = NULL;
  input.showverbose = showverbose;
  virtualtree->specialsymbols = True;
  virtualtree->prefixlength 
      = vm_recommendedprefixlength(input.numofchars,
                                   virtualtree->multiseq.totallength,
                                   SIZEOFBCKENTRY);
  if(mkvtreeprocess(&input,virtualtree) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree,
              rcplusvirtualtree;
  char *indexname;
  Sint ret;
  Uint transnum;
  Showverbose showverbose;
  DEBUGDECL(Multiseq rcplusmultiseqbf);

  if(extractoption("-v",argv,argc) > 0)
  {
    showverbose = showonstdout;
  } else
  {
    showverbose = NULL;
  }

  DEBUGLEVELSET;
  makeemptyvirtualtree(&virtualtree);
  ret = callmkvtreegeneric(True,
                           False,
                           argc,argv,
                           False,
                           &virtualtree,
                           True,
                           showverbose,
                           &transnum);
  if(ret == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  if(ret == (Sint) -1)
  {
    SIMPLESTANDARDMESSAGE;
  }
  if(ret < 0)
  {
    STANDARDMESSAGE;
  }
  makeemptyvirtualtree(&rcplusvirtualtree);
  if(mkrcsequences2index(&rcplusvirtualtree.multiseq,
                         &virtualtree.multiseq) != 0)
  {
    STANDARDMESSAGE;
  }
#ifdef DEBUG
  if(mkrcsequences2indexbruteforce(&rcplusmultiseqbf,
                                   &virtualtree.multiseq) != 0)
  {
    STANDARDMESSAGE;
  }
#endif
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifdef DEBUG
  compareMultiseq(&rcplusmultiseqbf,&rcplusvirtualtree.multiseq);
  freemultiseq(&rcplusmultiseqbf);
#endif
  if(extractindexname(&indexname,argv,argc) != 0)
  {
    STANDARDMESSAGE;
  }
  if(makercplusindex(&rcplusvirtualtree,indexname,showverbose) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(indexname);
  if(freevirtualtree(&rcplusvirtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  if(extractoption("-v",argv,argc) > 0)
  {
    printf("# overall space peak: main=%.2f MB, "
           " secondary=%.2f MB\n",
           MEGABYTES(getspacepeak()),
	   MEGABYTES(mmgetspacepeak()));
  }
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
