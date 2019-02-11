
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "minmax.h"
#include "virtualdef.h"
#include "megabytes.h"
#include "alphadef.h"
#include "fhandledef.h"
#include "queryext.h"
#include "genfile.h"
#include "vmatfind-def.h"
#include "chardef.h"
#include "args.h"

#include "alphabet.pr"
#include "matchsub.pr"
#include "readmulti.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "readvirt.pr"
#include "filehandle.pr"

#define SUFSTART(I) substringinfo->virtualtree->suftab[I]

#define PROCESSSUFFIX(I,L,MINPREFIX)\
        sufstart = SUFSTART(I);\
        if((sufstart = SUFSTART(I)) == 0 ||\
           ISSPECIAL(L) ||\
           (L) != substringinfo->virtualtree->multiseq.sequence[sufstart-1])\
        {\
          if(substringinfo->processexactquerymatch(\
                        substringinfo->processexactquerymatchinfo,\
                        MINPREFIX,sufstart,qseqnum,\
                        (Uint) (qseqptr-qsubstring)) != 0)\
          {\
            return (Sint) -1;\
          }\
        }

#define USAGE "Usage: %s <searchlength> <indexname> [queryfilename]\n"

typedef Sint (*Processexactquerymatch)(void *info,
                                       Uint len,
                                       Uint subjectpos,
                                       Uint queryseq,
                                       Uint querystart);

typedef struct
{
  Virtualtree *virtualtree;
  Multiseq *querymultiseq;
  Processexactquerymatch processexactquerymatch;
  void *processexactquerymatchinfo;
  Uint searchlength;
  Uint *mappower;
} Substringinfo;     // \Typedef{Substringinfo}

static Sint simpleexactselfmatchoutput(/*@unused@*/ void *info,
                                       Uint len,
                                       Uint pos1,
                                       Uint pos2)
{
  if(pos1 > pos2)
  {
    Uint tmp = pos1;
    pos1 = pos2;
    pos2 = tmp;
  }
  printf("%lu %lu %lu\n",(Showuint) len,
                         (Showuint) pos1,
                         (Showuint) pos2);
  return 0;
}

static Sint simpleexactquerymatchoutput(/*@unused@*/ void *info,
                                        Uint len,
                                        Uint subjectpos,
                                        Uint queryseq,
                                        Uint querystart)
{
  printf("%lu %lu %lu %lu\n",(Showuint) len,
                             (Showuint) subjectpos,
                             (Showuint) queryseq,
                             (Showuint) querystart);
  return 0;
}


static Sint leftrightsubmatch(/*@unused@*/ Uint maxintvleft,
                              /*@unused@*/ Uint maxintvright,
                              Uint maxlcp,
                              Uint witness,
                              Uchar leftchar,
                              Uint left,
                              Uint right,
                              void *info,
                              Uchar *qsubstring,
                              Uchar *qseqptr,
                              Uint qseqlen,
                              Uint qseqnum)
{
  Uint idx,                 // counter
       minprefix,           // minimal length of prefix in current group
       lcpval,              // temporary value of lcp
       sufstart;            // start position of suffix
  Substringinfo *substringinfo = (Substringinfo *) info;
  
  DEBUG4(3,"leftrightsubmatch(maxlcp=%lu,witness=%lu,leftchar=%lu,qseqlen=%lu",
                              (Showuint) maxlcp,
                              (Showuint) witness,
                              (Showuint) leftchar,
                              (Showuint) qseqlen);
  DEBUG2(3,",left=%lu,right=%lu)\n",(Showuint) left,(Showuint) right);
  if(maxlcp < UCHAR_MAX)   // prefix length is < 255
  {
    minprefix = maxlcp;
    for(idx=witness; /* Nothing */; idx--)
    { // process suffixes to the left of witness
      PROCESSSUFFIX(idx,leftchar,minprefix);
      if(idx == left || ((lcpval = substringinfo->virtualtree->lcptab[idx]) 
                                 < substringinfo->searchlength))
      {
        break;
      }
      if(minprefix > lcpval)
      {
        minprefix = lcpval;
      }
    }
    minprefix = maxlcp;
    for(idx=witness+1; /* Nothing */ ; idx++)
    { // process suffix to the right of witness
      if(idx > right || ((lcpval = substringinfo->virtualtree->lcptab[idx]) 
                                 < substringinfo->searchlength))
      {
        break;
      }
      if(minprefix > lcpval)
      {
        minprefix = lcpval;
      }
      PROCESSSUFFIX(idx,leftchar,minprefix);
    }
  } else // maxlcp >= UCHAR_MAX
  { // prefix length is >= 255
    PairUint *startexception, // pointer to start of lcp-exception interval
             *prevexception;  // pointer to previously found lcp-exception
    minprefix = maxlcp;
    startexception = prevexception = NULL;
    for(idx=witness; /* Nothing */; idx--)
    {
      PROCESSSUFFIX(idx,leftchar,minprefix);
      if(idx == left)
      {
        break;
      }
      if((lcpval = substringinfo->virtualtree->lcptab[idx]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        { // find lcp-exception by binary search
          prevexception = startexception 
                        = getexception(substringinfo->virtualtree,idx);
        } 
        lcpval = (prevexception--)->uint1;
      }
      if(lcpval < substringinfo->searchlength)
      {
        break;
      }
      if(minprefix > lcpval)
      {
        minprefix = lcpval;
      }
    }
    minprefix = maxlcp;
    startexception = prevexception = NULL;
    for(idx=witness+1; /* Nothing */ ; idx++)
    {
      if(idx > right)
      {
        break;
      }
      if((lcpval = substringinfo->virtualtree->lcptab[idx]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        {
          prevexception = startexception 
                        = getexception(substringinfo->virtualtree,idx);
        } 
        lcpval = (prevexception++)->uint1;
      }
      if(lcpval < substringinfo->searchlength)
      {
        break;
      }
      if(minprefix > lcpval)
      {
        minprefix = lcpval;
      }
      PROCESSSUFFIX(idx,
                    leftchar,
                    minprefix);
    }
  }
  return 0;
}

static Sint matchoverallsequences(void *info,
                                  Uint seqnum,
                                  Uchar *start,
                                  Uint len)
                                  
{
  Substringinfo *substringinfo = (Substringinfo *) info;

  if(matchquerysubstring2(substringinfo->virtualtree,
                          substringinfo->mappower,
			  substringinfo->searchlength,
                          leftrightsubmatch,
                          True, /* onlyleftmaximal */ 
                          seqnum,
                          start,
                          len,
                          substringinfo) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint findsubquerymatch(Virtualtree *virtualtree,
                              Multiseq *querymultiseq,
                              BOOL rcmode,
                              Uint searchlength,
                              Uint *mappower,
                              Processexactquerymatch processexactquerymatch,
                              void *processexactquerymatchinfo)
{
  Substringinfo substringinfo;

  substringinfo.virtualtree = virtualtree;
  substringinfo.querymultiseq = querymultiseq; // only used in DEBUG case
  substringinfo.searchlength = searchlength; // only used in DEBUG case
  substringinfo.processexactquerymatch = processexactquerymatch;
  substringinfo.processexactquerymatchinfo = processexactquerymatchinfo;
  substringinfo.mappower = mappower;
  if(rcmode)
  {
    if(querymultiseq->rcsequence == NULL)
    {
      querymultiseq->rcsequence = copymultiseqRC(querymultiseq);
      if(querymultiseq->rcsequence == NULL)
      {
        return (Sint) -1;
      }
    }
  }
  if(overallsequences(rcmode,
                      querymultiseq,
                      &substringinfo,
                      matchoverallsequences) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

Sint findallsubmatches(Virtualtree *virtualtree,
                       const char *queryfilename,
                       BOOL rcmode,
                       Uint userdefinedleastlength,
                       BOOL verbose,
                       Processexactquerymatch processexactquerymatch,
                       void *processexactquerymatchinfo)
{
  Sint retcode;
  Multiseq querymultiseq;
  Uint mappower[UCHAR_MAX+1];

  if(verbose)
  {
    printf("# alphasize = %lu\n",(Showuint) virtualtree->alpha.mapsize-1);
    printf("# searchlength = %lu\n",(Showuint) userdefinedleastlength);
  }
  if(queryfilename == NULL)
  {
    if(rcmode)
    {
      fprintf(stderr,"rcmode for selfmatch not implemented yet\n");
      exit(EXIT_FAILURE);
    }
    if(vmatmaxoutdynamic(virtualtree,
                         UintConst(1),
                         userdefinedleastlength,
                         NULL,
                         NULL,
                         simpleexactselfmatchoutput) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(userdefinedleastlength < virtualtree->prefixlength)
    {
      ERROR2("searchlength=%lu must be >= %lu=prefixlen",
                (Showuint) userdefinedleastlength,
                (Showuint) virtualtree->prefixlength);
        return (Sint) -1;
    }
    if(virtualtree->alpha.mapsize == 0)
    {
      ERROR0("alphabet size must not be 0");
      return (Sint) -2;
    }
    initmappower(mappower,
                 virtualtree->alpha.mapsize-1,
                 virtualtree->prefixlength);
    if(mapandparsesequencefile(&virtualtree->alpha,
                               &querymultiseq,
                               queryfilename) < 0)
    {
      return (Sint) -2;
    }
    if(verbose)
    {
      printf("# queryfile=%s of length %lu\n",
               queryfilename,
               (Showuint) querymultiseq.totallength);
    }
    retcode = findsubquerymatch(virtualtree,
                                &querymultiseq,
                                rcmode,
                                userdefinedleastlength,
                                mappower,
                                processexactquerymatch,
                                processexactquerymatchinfo);
    if(retcode < 0)
    {
      return (Sint) -3;
    }
    freemultiseq(&querymultiseq);
  }
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  Uint demand = 0;
  Scaninteger readint;
  const char *queryfile, *indexname;
  Uint userdefinedleastlength;

  DEBUGLEVELSET;
  if(argc == 4)
  {
    queryfile = argv[3];
    demand = LCPTAB | SUFTAB | STI1TAB | TISTAB | BCKTAB;
  } else
  {
    if(argc == 3)
    {
      queryfile = NULL;
      demand = LCPTAB | SUFTAB | TISTAB | BWTTAB;
    } else
    {
      fprintf(stderr,USAGE,argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if(sscanf(argv[1],"%ld",&readint) != 1 || readint < (Scaninteger) 1)
  {
    fprintf(stderr,"Usage: %s: first argument must be positive integer\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  userdefinedleastlength = (Uint) readint;
  indexname = argv[2];
  if(mapvirtualtreeifyoucan(&virtualtree,indexname,demand) != 0)
  {
    STANDARDMESSAGE;
  }
  if(findallsubmatches(&virtualtree,
                       queryfile,
                       False,
                       userdefinedleastlength,
                       False,
                       simpleexactquerymatchoutput,
                       NULL) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  printf("# overall space peak: main=%.2f MB, "
         "secondary=%.2f MB\n",
            MEGABYTES(getspacepeak()),
            MEGABYTES(mmgetspacepeak()));
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
