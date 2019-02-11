
#include <ctype.h>
#include "types.h"
#include "arraydef.h"
#include "fhandledef.h"
#include "debugdef.h"
#include "megabytes.h"
#include "redblackdef.h"
#include "chaindef.h"
#include "scoredef.h"
#include "genfile.h"
#include "chaincall.h"

#include "filehandle.pr"
#include "redblack.pr"
#include "chain2dim.pr"
#include "readnextline.pr"

#define WRAPUPRESOURCES\
        if(DELETEFILEHANDLE(matchfp) != 0)\
        {\
          return (Sint) -1;\
        }\
        FREEARRAY(&lbuf,Uchar)

#define READNUMS 5

#define CANNOTPARSELINE(S)\
        ERROR4("matchfile \"%s\", line %lu, column %lu: %s",\
                 matchfile,(Showuint) linenum,(Showuint) countcolumns,S)

typedef struct
{
  BOOL silent;
  Fragmentinfo *fragmentinfo;
  Uint chaincounter;
} Ofchainoutinfo;

static Sint analyzeopenformatfile(ArrayFragmentinfo *finfotab,
                                  double weightfactor,
                                  char *matchfile)
{
  ArrayUchar lbuf;
  char *matchline;
  Uint linenum = 0, countcolumns, storeinteger[READNUMS];
  FILE *matchfp;
  Scaninteger readint;
  Fragmentinfo *fiptr;

  matchfp = CREATEFILEHANDLE(matchfile,READMODE);
  if(matchfp == NULL)
  {
    return (Sint) -1;
  }
  INITARRAY(&lbuf,Uchar);
  while(True)
  {
    lbuf.nextfreeUchar = 0;
    if(readnextline(matchfp,&lbuf) == (Sint) EOF)
    {
      break;
    }
    linenum++;
    matchline = (char *) lbuf.spaceUchar;
    NOTSUPPOSEDTOBENULL(matchline);
    if(*matchline != '#')
    {
      countcolumns = 0;
      while(countcolumns < (Uint) READNUMS && 
            lbuf.spaceUchar != NULL &&
            matchline < (char *) (lbuf.spaceUchar + lbuf.nextfreeUchar))
      {
        while(isspace((Ctypeargumenttype) *matchline))
        {
          matchline++;
        }
        if (sscanf(matchline,"%ld",&readint) == 1 || readint < 0)
        {
          storeinteger[countcolumns] = (Uint) readint;
        } else
        {
          CANNOTPARSELINE("cannot read positive integer");
          WRAPUPRESOURCES;
          return (Sint) -1;
        }
        while(isdigit((Ctypeargumenttype) *matchline))
        {
          matchline++;
        }
        countcolumns++;
      }
      if(countcolumns < (Uint) (READNUMS - 1))
      {
        CANNOTPARSELINE("not enough integers: there must be at least four");
        WRAPUPRESOURCES;
        return (Sint) -2;
      }
      GETNEXTFREEINARRAY(fiptr,finfotab,Fragmentinfo,1024);
      fiptr->startpos[0] = storeinteger[0];
      fiptr->endpos[0] = storeinteger[1];
      fiptr->startpos[1] = storeinteger[2];
      fiptr->endpos[1] = storeinteger[3];
      if(fiptr->startpos[0] > fiptr->endpos[0])
      {
        CANNOTPARSELINE("startpos1 > endpos1");
        WRAPUPRESOURCES;
        return (Sint) -3;
      }
      if(fiptr->startpos[1] > fiptr->endpos[1])
      {
        CANNOTPARSELINE("startpos2 > endpos2");
        WRAPUPRESOURCES;
        return (Sint) -4;
      }
      if (countcolumns == (Uint) READNUMS)
      {
        fiptr->weight 
          = (Chainscoretype) (weightfactor * 
                              (double) storeinteger[READNUMS-1]);
      } else
      {
        Uint len1, len2;
        len1 = fiptr->endpos[0] - fiptr->startpos[0] + 1;
        len2 = fiptr->endpos[1] - fiptr->startpos[1] + 1;
        if(len1 < len2)
        {
          fiptr->weight 
            = (Chainscoretype) (weightfactor * (double) (3 * len1 - len2));
        } else
        {
          if(len2 < len1)
          {
            fiptr->weight 
              = (Chainscoretype) (weightfactor * (double) (3 * len2 - len1));
          }
        }
      }
    }
  }
  FREEARRAY(&lbuf,Uchar);
  if(DELETEFILEHANDLE(matchfp) != 0)
  {
    return (Sint) -4;
  }
  return 0;
}

static void fillthegapvalues(Fragmentinfo *fragmentinfo,Uint numofmatches)
{
  Fragmentinfo *fiptr;
  Uint largestdim1 = 0, largestdim2 = 0;

  for(fiptr = fragmentinfo; fiptr < fragmentinfo + numofmatches; fiptr++)
  {
    if(largestdim1 < fiptr->endpos[0])
    {
      largestdim1 = fiptr->endpos[0];
    }
    if(largestdim2 < fiptr->endpos[1])
    {
      largestdim2 = fiptr->endpos[1];
    }
  }
  for(fiptr = fragmentinfo; fiptr < fragmentinfo + numofmatches; fiptr++)
  {
    fiptr->initialgap 
      = (Chainscoretype) (fiptr->startpos[0] + fiptr->startpos[1]);
    fiptr->terminalgap 
      = (Chainscoretype) (largestdim1 - fiptr->endpos[0] + 
                          largestdim2 - fiptr->endpos[1]);
  }
}

static Sint outopenformatchain(void *cpinfo,Chain *chain)
{
  Fragmentinfo *fiptr; 
  Ofchainoutinfo *ofchainoutinfo = (Ofchainoutinfo *) cpinfo;
  Uint i;

  printf("# chain %lu: length %lu score %ld\n",
         (Showuint) ofchainoutinfo->chaincounter,
         (Showuint) chain->chainedfragments.nextfreeUint,
         (Showsint) chain->scoreofchain);
  if(!ofchainoutinfo->silent)
  {
    for(i=0; i < chain->chainedfragments.nextfreeUint; i++)
    {
      fiptr = ofchainoutinfo->fragmentinfo + 
              chain->chainedfragments.spaceUint[i];
      printf("%lu %lu %lu %lu %ld\n",(Showuint) fiptr->startpos[0],
                                     (Showuint) fiptr->endpos[0],
                                     (Showuint) fiptr->startpos[1],
                                     (Showuint) fiptr->endpos[1],
                                     (Showsint) fiptr->weight);
    }
  }
  ofchainoutinfo->chaincounter++;
  return 0;
}

static Qsortcomparereturntype cmpFragmentinfo0(const void *keya,
                                               const void *keyb)
{
  if(((const Fragmentinfo *) keya)->startpos[0] < 
     ((const Fragmentinfo *) keyb)->startpos[0])
  {
    return (Qsortcomparereturntype) -1;
  }
  if(((const Fragmentinfo *) keya)->startpos[0] > 
     ((const Fragmentinfo *) keyb)->startpos[0])
  {
    return (Qsortcomparereturntype) 1;
  }
  return 0;
}

static Qsortcomparereturntype cmpFragmentinfo1(const void *keya,
                                               const void *keyb)
{
  if(((const Fragmentinfo *) keya)->startpos[1] < 
     ((const Fragmentinfo *) keyb)->startpos[1])
  {
    return (Qsortcomparereturntype) -1;
  }
  if(((const Fragmentinfo *) keya)->startpos[1] > 
     ((const Fragmentinfo *) keyb)->startpos[1])
  {
    return (Qsortcomparereturntype) 1;
  }
  return 0;
}

static void possiblysortopenformatfragments(Showverbose showverbose,
                                            Fragmentinfo *tab,
                                            Uint tablen,
                                            Uint presortdim)
{

  if(tablen > UintConst(1))
  {
    Qsortcomparefunction qsortcomparefunction;
    Fragmentinfo *fptr;
    BOOL fragmentsaresorted = True;

    qsortcomparefunction 
      = (presortdim == 0) ? cmpFragmentinfo0 : cmpFragmentinfo1;
    for(fptr = tab; fptr < tab + tablen - 1; fptr++)
    {
      if(qsortcomparefunction((const void *) fptr,
                              (const void *) (fptr+1)) == 1)
      {
        fragmentsaresorted = False;
        break;
      }
    }
    if(!fragmentsaresorted)
    {
      if(showverbose != NULL)
      {
        showverbose("input fragments are not yet sorted => sort them");
      }
      qsort(tab,(size_t) tablen,sizeof(Fragmentinfo),qsortcomparefunction);
    }
  }
}

#ifndef NOSPACEBOOKKEEPING
static void showchainingspacepeak(Showverbose showverbose,
                                  Uint numofmatches)
{
  if(showverbose != NULL && numofmatches > 0)
  {
    Uint spacepeak = getspacepeak() + 
                     getredblacktreespacepeak();
    char vbuf[80+1];
    sprintf(vbuf,"overall space peak: main=%.2f MB",MEGABYTES(spacepeak));
    showverbose(vbuf);
  }
}
#endif

Sint openformatchaining(Chaincallinfo *chaincallinfo)
{
  ArrayFragmentinfo finfotab;
  Chain chain;
  Ofchainoutinfo ofchainoutinfo;
  Sint retcode;
  Uint presortdim = UintConst(1);

  INITARRAY(&finfotab,Fragmentinfo);
  if(analyzeopenformatfile(&finfotab,
                           chaincallinfo->weightfactor,
                           chaincallinfo->matchfile) != 0)
  {
    return (Sint) -1;
  }
  if(chaincallinfo->withinborders)
  {
    ERROR0("option -withinborders not possible for open format chaining");
    return (Sint) -2;
  }
  if(chaincallinfo->showverbose != NULL)
  {
    char vbuf[PATH_MAX+80+1];
    sprintf(vbuf,"match file \"%s\" (open format) read",
                  chaincallinfo->matchfile);
    chaincallinfo->showverbose(vbuf);
  }
  possiblysortopenformatfragments(chaincallinfo->showverbose,
                                  finfotab.spaceFragmentinfo,
                                  finfotab.nextfreeFragmentinfo,
                                  presortdim);
  if(chaincallinfo->chainmode.chainkind != GLOBALCHAINING)
  {
    fillthegapvalues(finfotab.spaceFragmentinfo,finfotab.nextfreeFragmentinfo);
  }
  INITARRAY(&chain.chainedfragments,Uint);
  ofchainoutinfo.silent = chaincallinfo->silent;
  ofchainoutinfo.fragmentinfo = finfotab.spaceFragmentinfo;
  ofchainoutinfo.chaincounter = 0;
  retcode = fastchaining(&chaincallinfo->chainmode,
                         &chain,
                         finfotab.spaceFragmentinfo,
                         finfotab.nextfreeFragmentinfo,
                         True,
                         presortdim,
                         True,
                         outopenformatchain,
                         (void *) &ofchainoutinfo,
                         chaincallinfo->showverbose);
  if(retcode < 0)
  {
    FREEARRAY(&finfotab,Fragmentinfo);
    FREEARRAY(&chain.chainedfragments,Uint);
    return (Sint) -2;
  }
  if(retcode == SintConst(1))
  {
    printf("no chains of length > 1 with positive scores available\n");
  }
  FREEARRAY(&finfotab,Fragmentinfo);
#ifndef NOSPACEBOOKKEEPING
  showchainingspacepeak(chaincallinfo->showverbose,
                        finfotab.nextfreeFragmentinfo);
#endif
  FREEARRAY(&chain.chainedfragments,Uint);
  return 0;
}
