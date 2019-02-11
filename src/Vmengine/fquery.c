
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "types.h"
#include "divmodmul.h"
#include "intbits.h"
#include "debugdef.h"
#include "queryext.h"
#include "mumcand.h"
#include "errordef.h"
#include "chardef.h"
#include "spacedef.h"
#include "minmax.h"
#include "scoredef.h"
#include "virtualdef.h"
#include "vnodedef.h"
#include "matchstate.h"

#include "multiseq-adv.pr"
#include "matchsub.pr"
#include "alphabet.pr"
#include "cleanMUMcand.pr"
#include "mcontain.pr"
#include "multiseq.pr"

#include "procexqu.pr"
#include "initmstate.pr"

typedef Sint (*Matchsubagainstvirt)(void *,Uint,Uchar *,Uint);

typedef struct
{
  Virtualtree *virtualtree;
  Uint searchlength,
       querylength,
       mappower[UCHAR_MAX+1];
  void *mstateinfo;
  Querymatchextendfunction extendleftright;
  ArrayMUMcandidate maximaluniquecandtab;
  BOOL domaximaluniquematchcandidate;
} Substringinfo;

/*
  The following is the code fragment for processing the entry 
  \texttt{suftab[i]}. The output function is called if the suffix is 
  left maximal.
*/

#define SUFSTART(I) substringinfo->virtualtree->suftab[I]

#define PROCESSSUFFIX(I,L,MINPREFIX,OUTINFO,OUTPUTFUNCTION)\
        sufstart = SUFSTART(I);\
        DEBUG2(3,"suffix %lu: sufstart=%lu\n",(Showuint) (I),\
                                              (Showuint) SUFSTART(I));\
        DEBUG1(3,"leftchar of query = %lu\n",(Showuint) (L));\
        DEBUGCODE(3,{\
                      printf("leftchar of text: ");\
                      if(SUFSTART(I) == 0)\
                      {\
                        printf("undefined\n");\
                      } else\
                      {\
                        printf("%lu\n",(Showuint) substringinfo->virtualtree->\
                                                  multiseq.\
                                                  sequence[SUFSTART(I)-1]);\
                      }\
                    });\
        if((sufstart = SUFSTART(I)) == 0 ||\
           ISSPECIAL(L) ||\
           (L) != substringinfo->virtualtree->multiseq.sequence[sufstart-1])\
        {\
          TESTPREFIXLEN(MINPREFIX);\
          if(OUTPUTFUNCTION(OUTINFO,MINPREFIX,sufstart,qseqnum,\
                            (Uint) (qseqptr-qsubstring)) != 0)\
          {\
            return (Sint) -1;\
          }\
        }

/*
  The following macro checks if the prefix length is correctly calculated.
*/

#ifdef DEBUG
#define TESTPREFIXLEN(MINPREFIX)\
        Uint remaining = qseqlen - (Uint) (qseqptr-qsubstring),\
             suflen = substringinfo->virtualtree->multiseq.totallength - \
                      sufstart,\
             prefixlen = getprefixlen(substringinfo,qseqptr,remaining,\
                                      sufstart,suflen);\
        if(MINPREFIX != prefixlen)\
        {\
          fprintf(stderr,"minprefix= %lu!=%lu =prefixlen\n",\
                  (Showuint) (MINPREFIX),(Showuint) prefixlen);\
          exit(EXIT_FAILURE);\
        }\
        DEBUG1(3,"prefixlen=%lu okay\n",(Showuint) prefixlen)

static Uint getprefixlen(Substringinfo *substringinfo,Uchar *qseqptr,
                         Uint remaining,Uint sufstart,Uint suflen)
{
  Uint prefixlen;
  Uchar avalue, bvalue;

  for(prefixlen = 0; prefixlen < MIN(remaining,suflen); prefixlen++)
  {
    avalue = substringinfo->virtualtree->multiseq.sequence[sufstart+prefixlen];
    bvalue = qseqptr[prefixlen];
    if(avalue != bvalue || ISSPECIAL(avalue))
    {
       break;
    }
  }
  return prefixlen;
}
#else
#define TESTPREFIXLEN(MINPREFIX) /* Nothing */
#endif  // DEBUG

#ifdef COUNT
static Uint skipfailcount = 0;
static Uint skipsuccesscount = 0;
static Uint isomorphic = 0;
#endif

/*
  The following function starts at a suffix of the input text,
  matching the current suffix of the query, say \(u\), with maximal length among
  all suffixes of the input string. The suffixes to
  the left and to the right are scanned as until a suffix is found
  that matches less than \texttt{searchlength} characters of \(u\).
  The matching length of the suffix scanned is maintained as the minimal
  prefix seen so far.
*/

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
      PROCESSSUFFIX(idx,
                    leftchar,
                    minprefix,
                    substringinfo->mstateinfo,
                    processexactquerymatch);
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
      PROCESSSUFFIX(idx,
                    leftchar,
                    minprefix,
                    substringinfo->mstateinfo,
                    processexactquerymatch);
    }
  } else // maxlcp >= UCHAR_MAX
  { // prefix length is >= 255
    PairUint *startexception, // pointer to start of lcp-exception interval
             *prevexception;  // pointer to previously found lcp-exception
    minprefix = maxlcp;
    startexception = prevexception = NULL;
    for(idx=witness; /* Nothing */; idx--)
    {
      PROCESSSUFFIX(idx,
                    leftchar,
                    minprefix,
                    substringinfo->mstateinfo,
                    processexactquerymatch);
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
                    minprefix,
                    substringinfo->mstateinfo,
                    processexactquerymatch);
    }
  }
  return 0;
}

static Sint processMUMcandiate(void *info,
                               Uint len,
                               Uint dbstart,
                               Uint queryseq,
                               Uint querystart)
{
  Substringinfo *substringinfo = (Substringinfo *) info;
  MUMcandidate *maximaluniquecandptr;

  DEBUG4(3,"processMUMcandiate %lu %lu %lu %lu\n",
            (Showuint) len,
            (Showuint) dbstart,
            (Showuint) queryseq,
            (Showuint) querystart);
  GETNEXTFREEINARRAY(maximaluniquecandptr,
                     &substringinfo->maximaluniquecandtab,
                     MUMcandidate,
                     1024);
  maximaluniquecandptr->mumlength = len;
  maximaluniquecandptr->dbstart = dbstart;
  maximaluniquecandptr->queryseq = queryseq;
  maximaluniquecandptr->querystart = querystart;
  return 0;
}

static Sint leftrightmaximaluniquematch(/*@unused@*/ Uint maxintvleft,
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
  Uint sufstart, // start position of suffix
       lcpval;            
  BOOL okay;
  Substringinfo *substringinfo = (Substringinfo *) info;
  
  DEBUG4(3,"maximaluniqueleftright(maxlcp=%lu,witness=%lu, "
           "leftchar=%lu,qseqlen=%lu",
                         (Showuint) maxlcp,
                         (Showuint) witness,
                         (Showuint) leftchar,
                         (Showuint) qseqlen);
  DEBUG2(3,",left=%lu,right=%lu)\n",(Showuint) left,(Showuint) right);
  if(maxlcp < UCHAR_MAX)   // prefix length is < 255
  {
    if((witness == left || 
        substringinfo->virtualtree->lcptab[witness] < (Uchar) maxlcp) &&
       (witness + 1 > right || 
        substringinfo->virtualtree->lcptab[witness+1] < (Uchar) maxlcp))
    {
      if(substringinfo->domaximaluniquematchcandidate)
      {
        PROCESSSUFFIX(witness,
                      leftchar,
                      maxlcp,
                      substringinfo->mstateinfo,
                      processexactquerymatch);
      } else
      {
        PROCESSSUFFIX(witness,
                      leftchar,
                      maxlcp,
                      (void *) substringinfo,
                      processMUMcandiate);
      }
    }
  } else
  {
    if(witness == left)
    {
      okay = True;
    } else
    {
      EVALLCP(substringinfo->virtualtree,lcpval,witness);
      okay = (lcpval < maxlcp) ? True : False;
    }
    if(okay)
    {
      if(witness + 1 < right)
      {
        EVALLCP(substringinfo->virtualtree,lcpval,witness+1);
        if(lcpval >= maxlcp)
        {
          okay = False;
        }
      }
    }
    if(okay)
    {
      if(substringinfo->domaximaluniquematchcandidate)
      {
        PROCESSSUFFIX(witness,
                      leftchar,
                      maxlcp,
                      substringinfo->mstateinfo,
                      processexactquerymatch);
      } else
      {
        PROCESSSUFFIX(witness,
                      leftchar,
                      maxlcp,
                      (void *) substringinfo,
                      processMUMcandiate);
      }
    }
  }
  return 0;
}

#define DECLAREMATCHSUBFUNCTION(NUM)\
        static Sint matchsubagainstvirtspeedup##NUM (void *info,\
                                                     Uint qseqnum,\
                                                     Uchar *qsubstring,\
                                                     Uint qseqlen)\
        {\
          Substringinfo *substringinfo = (Substringinfo *) info;\
          return  matchquerysubstring##NUM (substringinfo->virtualtree,\
                                            substringinfo->mappower,\
                                            substringinfo->searchlength,\
                                            substringinfo->extendleftright,\
                                            True,\
                                            qseqnum,\
                                            qsubstring,\
                                            qseqlen,\
                                            info);\
        }

 DECLAREMATCHSUBFUNCTION(0)
 DECLAREMATCHSUBFUNCTION(1)
 DECLAREMATCHSUBFUNCTION(2)
 DECLAREMATCHSUBFUNCTION(3)
 DECLAREMATCHSUBFUNCTION(4)
 DECLAREMATCHSUBFUNCTION(5)

static Sint matchqueryagainstvirtualtree(BOOL domaximaluniquematch,
                                         BOOL domaximaluniquematchcandidates,
                                         BOOL rcmode,
                                         Uint searchlength,
                                         void *mstateinfo,
                                         Querymatchextendfunction 
                                              extendleftright)
{
  Substringinfo substringinfo;
  Matchstate *matchstate = (Matchstate *) mstateinfo;
  Matchsubagainstvirt matchsubagainstvirt[] =
  {
    matchsubagainstvirtspeedup0,
    matchsubagainstvirtspeedup1,
    matchsubagainstvirtspeedup2,
    matchsubagainstvirtspeedup3,
    matchsubagainstvirtspeedup4,
    matchsubagainstvirtspeedup5
  };

  if(matchstate->matchparam.queryspeedup >=
     (Uint) sizeof(matchsubagainstvirt)/(Uint) sizeof(matchsubagainstvirt[0]))
  {
    ERROR1("illegal speedup value %lu",
            (Showuint) matchstate->matchparam.queryspeedup);
    return (Sint) -1;
  }
  if(searchlength < matchstate->virtualtree->prefixlength)
  {
    ERROR2("searchlength=%lu must be >= %lu=prefixlen",
            (Showuint) searchlength,
            (Showuint) matchstate->virtualtree->prefixlength);
    return (Sint) -2;
  }
  substringinfo.virtualtree = matchstate->virtualtree;
  substringinfo.searchlength = searchlength;
  substringinfo.mstateinfo = mstateinfo;
  substringinfo.querylength = matchstate->queryinfo->multiseq->totallength;
  substringinfo.extendleftright = extendleftright;
  substringinfo.domaximaluniquematchcandidate = 
    domaximaluniquematchcandidates;
  if(domaximaluniquematch && !domaximaluniquematchcandidates)
  {
    INITARRAY(&substringinfo.maximaluniquecandtab,MUMcandidate);
  }
  if(matchstate->virtualtree->alpha.mapsize == 0)
  {
    ERROR0("alphabet size must not be 0");
    return (Sint) -3;
  }
  initmappower(substringinfo.mappower,
               substringinfo.virtualtree->alpha.mapsize-1,
               substringinfo.virtualtree->prefixlength);
  // over all sequences of the query
  
  if(overallsequences(rcmode,
                      matchstate->queryinfo->multiseq,
                      (void *) &substringinfo,
                      matchsubagainstvirt[matchstate->matchparam.queryspeedup])
                      != 0)
  {
    return (Sint) -4;
  }
  //printf("# MUMTIME %s %.2f\n","vmatch.x",getruntime());
  if(domaximaluniquematch && !domaximaluniquematchcandidates)
  {
    if(mumuniqueinquery((void *) matchstate,
                        processexactquerymatch,
                        &substringinfo.maximaluniquecandtab) != 0)
    {
      return (Sint) -5;
    }
    FREEARRAY(&substringinfo.maximaluniquecandtab,MUMcandidate);
  }

#ifdef COUNT
  printf("# charcomp=%lu (%.2f)\n",
            (Showuint) charcomp,
            (double) charcomp/matchstate->queryinfo->multiseq->totallength);
  printf("# pushop=%lu (%.2f)\n",
            (Showuint) pushop,
            (double) pushop/matchstate->queryinfo->multiseq->totallength);
#endif
  return 0;
}

 Sint matchvirtagainstvirt(BOOL rcmode,Uint searchlength,
                           void *mstateinfo,
                           Querymatchextendfunction extendleftright)
{
  Uint i, j, sufstart, queryleft, queryright;
  Uchar leftchar, *qseqptr;
  Substringinfo substringinfo;
  Seqinfo seqinfo;
  Matchstate *matchstate = (Matchstate *) mstateinfo;
  Vnode vnode;  // Interval with offset in queryvirtualtree
  PairUint maxwit;  // (maximal prefix in interval vnode, suffix witnessing the
                    // maximal prefix)

  if(searchlength < matchstate->virtualtree->prefixlength)
  {
    ERROR2("searchlength=%lu must be >= %lu=prefixlen",
             (Showuint) searchlength,
             (Showuint) matchstate->virtualtree->prefixlength);
    return (Sint) -1;
  }
  if(rcmode)
  {
    ERROR0("cannot find palindromic matches when matching index against index");
    return (Sint) -2;
  }
  if(matchstate->queryinfo->prefixlength != 
     matchstate->virtualtree->prefixlength)
  {
    ERROR2("prefixlength(query) = %lu != %lu prefixlength(virtualtree)",
             (Showuint) matchstate->queryinfo->prefixlength,
             (Showuint) matchstate->virtualtree->prefixlength);
    return (Sint) -3;
  }
  if(matchstate->virtualtree->alpha.mapsize == 0)
  {
    ERROR0("alphabet size must not be 0");
    return (Sint) -4;
  }
  substringinfo.virtualtree = matchstate->virtualtree;
  substringinfo.searchlength = searchlength;
  substringinfo.mstateinfo = mstateinfo;
  substringinfo.extendleftright = extendleftright;
  vnode.offset = matchstate->queryinfo->prefixlength;
  for(i=0; i < MULT2(matchstate->queryinfo->numofcodes) ; i+=2)
  {
    queryleft = matchstate->queryinfo->bcktab[i];
    queryright = matchstate->queryinfo->bcktab[i+1];
    /*printf("%lu: querybck %lu %lu\n",
               (Showuint) DIV2(i),
               (Showuint) queryleft,
               (Showuint) queryright);*/
    if(queryleft < queryright)
    {
      vnode.left = matchstate->virtualtree->bcktab[i];
      vnode.right = matchstate->virtualtree->bcktab[i+1];
      /* printf("%lu: virtualbck %lu %lu\n",
                   (Showuint) DIV2(i),
                   (Showuint) vnode.left,
                   (Showuint) vnode.right);*/
      if(vnode.left < vnode.right)
      {
        vnode.right--;  // adjust right boundary
        for(j=queryleft; j<queryright; j++)
        {
          sufstart = matchstate->queryinfo->suftab[j];
          qseqptr = matchstate->queryinfo->multiseq->sequence + sufstart;
          findmaxprefixlen(matchstate->virtualtree,&vnode,qseqptr,
                           matchstate->queryinfo->multiseq->totallength
                             - sufstart,
                           &maxwit);  // calculate the interval
          if(maxwit.uint0 >= searchlength)  
          { // maximal prefix is long enough
            if(sufstart == 0)
            {
              leftchar = SEPARATOR;
            } else
            {
              leftchar = matchstate->queryinfo->multiseq->sequence[sufstart-1];
            }
            DEBUG1(3,"leftchar=%lu\n",(Showuint) leftchar);
            if(getseqinfo(matchstate->queryinfo->multiseq,
                          &seqinfo,sufstart) != 0)
            {
              return (Sint) -5;
            }
            DEBUG2(2,"sufstart=%lu,seqstart=%lu\n",
                      (Showuint) sufstart,
                      (Showuint) seqinfo.seqstartpos);
            if(extendleftright(vnode.left,
                               vnode.right,
                               maxwit.uint0,
                               maxwit.uint1,
                               leftchar,
                               vnode.left,
                               vnode.right,
                               &substringinfo,
                               matchstate->queryinfo->multiseq->sequence + 
                                 seqinfo.seqstartpos,
                               qseqptr,
                               seqinfo.seqlength,
                               seqinfo.seqnum) != 0)
            {
              return (Sint) -6;
            }
          }
        }
      }
    }
  }
  return 0;
}

#ifdef DEBUG
static void showstack(ArrayVnoderpref *stack)
{
  Uint stackdepth;

  for(stackdepth=0;stackdepth<stack->nextfreeVnoderpref; stackdepth++)
  {
    printf("stack[%lu]: (%lu,%lu,%lu,%lu)\n",
           (Showuint) stackdepth,
           (Showuint) stack->spaceVnoderpref[stackdepth].left,
           (Showuint) stack->spaceVnoderpref[stackdepth].right,
           (Showuint) stack->spaceVnoderpref[stackdepth].offset,
           (Showuint) stack->spaceVnoderpref[stackdepth].rpref);
  }
}
#endif

#ifdef DEBUG
#define DEBUGFINDMAXPREFIXLEN\
        vnodedebug.offset = vnode.offset;\
        vnodedebug.left = vnode.left;\
        vnodedebug.right = vnode.right;\
        findmaxprefixlen(matchstate->virtualtree,\
                         &vnodedebug,\
                         qseqptr,\
                         matchstate->queryinfo->multiseq->totallength-sufstart,\
                         &maxwit2);\
        if(maxwit.uint0 != maxwit2.uint0)\
        {\
          fprintf(stderr,"uint0 = %lu incorrect, must be %lu, ",\
                          (Showuint) maxwit.uint0,\
                          (Showuint) maxwit2.uint0);\
          fprintf(stderr,"witness = %lu,",(Showuint) maxwit.uint1);\
          fprintf(stderr,"witness1 = %lu\n",(Showuint) maxwit2.uint1);\
          exit(EXIT_FAILURE);\
        }\
        DEBUG1(3,"maxlcp=%lu is okay\n",(Showuint) maxwit.uint0)
#else
#define DEBUGFINDMAXPREFIXLEN 
#endif

 Sint matchvirtagainstvirt2(BOOL rcmode,Uint searchlength,
                            void *mstateinfo,
                            Querymatchextendfunction extendleftright)
{
  Uint i, j, lcpval, sufstart, queryleft, queryright;
  Uchar leftchar, *qseqptr;
  Substringinfo substringinfo;
  Seqinfo seqinfo;
  Matchstate *matchstate = (Matchstate *) mstateinfo;
  Vnoderpref vnodespace[MAXVSTACK],  
             vnode;  // Interval with offset in queryvirtualtree
  PairUint maxwit;  // (maximal prefix in interval vnode, suffix witnessing the
                    // maximal prefix)
  ArrayVnoderpref stack;
#ifdef COUNT
  Uint widthsaved = 0;
#endif
#ifdef DEBUG
  Vnode vnodedebug;
#endif
  DEBUGDECL(PairUint maxwit2);

  if(searchlength < matchstate->virtualtree->prefixlength)
  {
    ERROR2("searchlength=%lu must be >= %lu=prefixlen",
            (Showuint) searchlength,
            (Showuint) matchstate->virtualtree->prefixlength);
    return (Sint) -1;
  }
  if(rcmode)
  {
    ERROR0("cannot find palindromic matches when matching index against index");
    return (Sint) -2;
  }
  if(matchstate->queryinfo->prefixlength != 
     matchstate->virtualtree->prefixlength)
  {
    ERROR2("prefixlength(query) = %lu != %lu prefixlength(virtualtree)",
            (Showuint) matchstate->queryinfo->prefixlength,
            (Showuint) matchstate->virtualtree->prefixlength);
    return (Sint) -3;
  }
  if(matchstate->virtualtree->alpha.mapsize == 0)
  {
    ERROR0("alphabet size must not be 0");
    return (Sint) -4;
  }
  substringinfo.virtualtree = matchstate->virtualtree;
  substringinfo.searchlength = searchlength;
  substringinfo.mstateinfo = mstateinfo;
  substringinfo.extendleftright = extendleftright;
  INITARRAY(&stack,Vnoderpref);
  stack.spaceVnoderpref = &vnodespace[0];
  vnode.offset = matchstate->queryinfo->prefixlength;
  stack.spaceVnoderpref[0].offset = vnode.offset;
  stack.spaceVnoderpref[0].rpref = vnode.offset;
  for(i=0; i< MULT2(matchstate->queryinfo->numofcodes); i+=2)
  {
    queryleft = matchstate->queryinfo->bcktab[i];
    queryright = matchstate->queryinfo->bcktab[i+1];
    if(queryleft < queryright)
    {
      vnode.left = matchstate->virtualtree->bcktab[i];
      vnode.right = matchstate->virtualtree->bcktab[i+1];
      if(vnode.left < vnode.right)
      {
        vnode.right--;  // adjust right boundary
        stack.spaceVnoderpref[0].left = vnode.left;
        stack.spaceVnoderpref[0].right = vnode.right;
        stack.nextfreeVnoderpref = UintConst(1);
        for(j=queryleft; j<queryright; j++)
        {
          if(j > queryleft)
          {
            DEBUGCODE(2,showstack(&stack));
            lcpval = matchstate->queryinfo->lcptab[j];
            while(True)
            {
              if(stack.nextfreeVnoderpref == UintConst(1) ||
                 stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].rpref
                 < lcpval)
              {
                break;
              }
              stack.nextfreeVnoderpref--;
            }
            DEBUG1(3,"found suffix %lu=",(Showuint) j);
            DEBUG2(3,"%lu(lcpval=%lu) in ",
                       (Showuint) matchstate->queryinfo->suftab[j],
                       (Showuint) lcpval);
            DEBUG4(3,"(%lu,%lu,%lu,%lu)\n",
                    (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].left,
                    (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].right,
                    (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].offset,
                    (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].rpref);
          }
          sufstart = matchstate->queryinfo->suftab[j];
          qseqptr = matchstate->queryinfo->multiseq->sequence + sufstart;
#ifdef COUNT
          widthsaved 
            += ((vnode.right - vnode.left) - 
                (stack.spaceVnode[stack.nextfreeVnode-1].right - 
                 stack.spaceVnode[stack.nextfreeVnode-1].left));
#endif
          findmaxprefixlenstack(matchstate->virtualtree,
                               stack.spaceVnoderpref+stack.nextfreeVnoderpref-1,
                               qseqptr,
                               matchstate->queryinfo->multiseq->totallength 
                                 - sufstart,
                               &maxwit,&stack);  // calculate the interval
          DEBUGFINDMAXPREFIXLEN;
          if(maxwit.uint0 >= searchlength)  
          { // maximal prefix is long enough
            if(sufstart == 0)
            {
              leftchar = SEPARATOR;
            } else
            {
              leftchar = matchstate->queryinfo->multiseq->sequence[sufstart-1];
            }
            DEBUG1(3,"leftchar=%lu\n",(Showuint) leftchar);
            if(getseqinfo(matchstate->queryinfo->multiseq,
                          &seqinfo,sufstart) != 0)
            {
              return (Sint) -5;
            }
            DEBUG2(2,"sufstart=%lu,seqstart=%lu\n",
                      (Showuint) sufstart,
                      (Showuint) seqinfo.seqstartpos);
            if(extendleftright(vnode.left,
                               vnode.right,
                               maxwit.uint0,
                               maxwit.uint1,
                               leftchar,
                               vnode.left,
                               vnode.right,
                               &substringinfo,
                               matchstate->queryinfo->multiseq->sequence + 
                                 seqinfo.seqstartpos,
                               qseqptr,
                               seqinfo.seqlength,
                               seqinfo.seqnum) != 0)
            {
              return (Sint) -6;
            }
          }
        }
      }
    }
  }
#ifdef COUNT
  printf("# charcomp=%lu (%.2f)\n",
            (Showuint) charcomp,
            (double) charcomp/matchstate->queryinfo->multiseq->totallength);
  printf("# pushop=%lu (%.2f)\n",
            (Showuint) pushop,
            (double) pushop/matchstate->queryinfo->multiseq->totallength);
  printf("# charcomp/pushop=%.2f\n",(double) charcomp/pushop);
  printf("# widthsaved = %lu (%.2f)\n",
            (Showuint) widthsaved,
            (double) widthsaved/matchstate->queryinfo->multiseq->totallength);
#endif
  return 0;
}

 Sint matchvirtagainstvirt3(BOOL rcmode,Uint searchlength,
                            void *mstateinfo,
                            Querymatchextendfunction extendleftright)
{
  Uint j, lcpval, sufstart;
  Uchar leftchar, *qseqptr;
  Substringinfo substringinfo;
  Seqinfo seqinfo;
  Matchstate *matchstate = (Matchstate *) mstateinfo;
  Vnoderpref vnodespace[MAXVSTACK],
             vnode;  // Interval with offset in queryvirtualtree
  PairUint maxwit;  // (maximal prefix in interval vnode, suffix witnessing the
                    // maximal prefix)
  ArrayVnoderpref stack;
#ifdef COUNT
  Uint widthsaved = 0;
#endif
#ifdef DEBUG
  Vnode vnodedebug;
#endif
  DEBUGDECL(PairUint maxwit2);

  if(rcmode)
  {
    ERROR0("cannot find palindromic matches when matching index against index");
    return (Sint) -2;
  }
  if(matchstate->virtualtree->alpha.mapsize == 0)
  {
    ERROR0("alphabet size must not be 0");
    return (Sint) -4;
  }
  vnode.left = 0;
  vnode.right = matchstate->virtualtree->multiseq.totallength;
  vnode.offset = 0;
  substringinfo.virtualtree = matchstate->virtualtree;
  substringinfo.searchlength = searchlength;
  substringinfo.mstateinfo = mstateinfo;
  substringinfo.extendleftright = extendleftright;
  INITARRAY(&stack,Vnoderpref);
  stack.spaceVnoderpref = &vnodespace[0];
  stack.spaceVnoderpref[0].left = 0;
  stack.spaceVnoderpref[0].right 
    = matchstate->virtualtree->multiseq.totallength;
  stack.spaceVnoderpref[0].offset = 0;
  stack.spaceVnoderpref[0].rpref = 0;
  stack.nextfreeVnoderpref = UintConst(1);
  for(j=0; j<matchstate->queryinfo->multiseq->totallength; j++)
  {
    DEBUGCODE(3,showstack(&stack));
    lcpval = matchstate->queryinfo->lcptab[j];
    while(True)
    {
      if(stack.nextfreeVnoderpref == UintConst(1) ||
         stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].rpref < lcpval)
      {
        break;
      }
      stack.nextfreeVnoderpref--;
    }
    DEBUG1(3,"found suffix %lu=",(Showuint) j);
    DEBUG2(3,"%lu(lcpval=%lu) in ",
               (Showuint) matchstate->queryinfo->suftab[j],
               (Showuint) lcpval);
    DEBUG4(3,"(%lu,%lu,%lu,%lu)\n",
             (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].left,
             (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].right,
             (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].offset,
             (Showuint) stack.spaceVnoderpref[stack.nextfreeVnoderpref-1].rpref);
    sufstart = matchstate->queryinfo->suftab[j];
    qseqptr = matchstate->queryinfo->multiseq->sequence + sufstart;
    findmaxprefixlenstack(matchstate->virtualtree,
                          stack.spaceVnoderpref+stack.nextfreeVnoderpref-1,
                          qseqptr,
                          matchstate->queryinfo->multiseq->totallength
                             - sufstart,
                          &maxwit,&stack);  // calculate the interval
    DEBUGFINDMAXPREFIXLEN;
    if(maxwit.uint0 >= searchlength)  
    { // maximal prefix is long enough
      if(sufstart == 0)
      {
        leftchar = SEPARATOR;
      } else
      {
        leftchar = matchstate->queryinfo->multiseq->sequence[sufstart-1];
      }
      DEBUG1(3,"leftchar=%lu\n",(Showuint) leftchar);
      if(getseqinfo(matchstate->queryinfo->multiseq,&seqinfo,sufstart) != 0)
      {
        return (Sint) -5;
      }
      DEBUG2(2,"sufstart=%lu,seqstart=%lu\n",
              (Showuint) sufstart,
              (Showuint) seqinfo.seqstartpos);
      if(extendleftright(vnode.left,
                         vnode.right,
                         maxwit.uint0,
                         maxwit.uint1,
                         leftchar,
                         vnode.left,
                         vnode.right,
                         &substringinfo,
                         matchstate->queryinfo->multiseq->sequence + 
                            seqinfo.seqstartpos,
                         qseqptr,
                         seqinfo.seqlength,
                         seqinfo.seqnum) != 0)
      {
        return (Sint) -6;
      }
    }
  }
#ifdef COUNT
  printf("# charcomp=%lu (%.2f)\n",
            (Showuint) charcomp,
            (double) charcomp/matchstate->queryinfo->multiseq->totallength);
  printf("# pushop=%lu (%.2f)\n",
            (Showuint) pushop,
            (double) pushop/matchstate->queryinfo->multiseq->totallength);
  printf("# charcomp/pushop=%.2f\n",(double) charcomp/pushop);
  printf("# widthsaved = %lu (%.2f)\n",
            (Showuint) widthsaved,
            (double) widthsaved/matchstate->queryinfo->multiseq->totallength);
#endif
  return 0;
}
/*
  find the matches of the query against the virtual tree.
*/

static Sint findsubquerymatches(BOOL domaximaluniquematch,
                                BOOL domaximaluniquematchcandidates,
                                BOOL rcmode,
                                Virtualtree *virtualtree,
                                Uint onlinequerynumoffset,
                                Matchparam *matchparam,
                                Queryinfo *queryinfo,
                                Bestflag bestflag,
                                Uint shownoevalue,
                                Uint showselfpalindromic,
                                SelectBundle *selectbundle,
                                Querymatchextendfunction extendleftright,
                                void *procmultiseq,
                                Currentdirection currentdirection,
                                BOOL revmposorder,
                                Processfinalfunction processfinal,
                                Evalues *evalues,
                                BOOL domatchbuffering)
{
  Matchstate matchstate;
  
  if(initMatchstate(&matchstate,
                    virtualtree,
                    (void *) queryinfo,
                    matchparam,
                    bestflag,
                    shownoevalue,
                    showselfpalindromic,
                    selectbundle,
                    onlinequerynumoffset,
                    procmultiseq,
                    currentdirection,
                    revmposorder,
                    processfinal,
                    evalues,
                    domatchbuffering) != 0)
  {
    return (Sint) -1;
  }
  if(matchqueryagainstvirtualtree(domaximaluniquematch,
                                  domaximaluniquematchcandidates,
                                  rcmode,
                                  matchparam->seedlength,
                                  (void *) &matchstate,
                                  extendleftright) != 0)
  {
    return (Sint) -2;
  }
  if(bestflag == Allmaximalmatches && !MPARMEXACTMATCH(&matchparam->maxdist))
  {
    if(applymatchcontainer(&matchstate,
                           matchstate.processfinal,&matchstate.allmstore) != 0)
    {
      return (Sint) -3;
    }
    FREEARRAY(&matchstate.allmstore,Match);
    FREEARRAY(&matchstate.seedmstore,Match);
  }
  return 0;
}

Sint findquerymatches(Virtualtree *virtualtree,
                      Uint onlinequerynumoffset,
                      Queryinfo *queryinfo,
                      BOOL domaximaluniquematch,
                      BOOL domaximaluniquematchcandidates,
                      BOOL rcmode,
                      Matchparam *matchparam,
                      Bestflag bestflag,
                      Uint shownoevalue,
                      Uint showselfpalindromic,
                      SelectBundle *selectbundle,
                      void *procmultiseq,
                      Currentdirection currentdirection,
                      BOOL revmposorder,
                      Processfinalfunction processfinal,
                      Evalues *evalues,
                      BOOL domatchbuffering)
{
  Querymatchextendfunction extendleftright;

  if(domaximaluniquematch)
  {
    extendleftright = leftrightmaximaluniquematch;
  } else
  {
    extendleftright = leftrightsubmatch;
  }
  if(findsubquerymatches(domaximaluniquematch,
                         domaximaluniquematchcandidates,
                         rcmode,
                         virtualtree,
                         onlinequerynumoffset,
                         matchparam,
                         queryinfo,
                         bestflag,
                         shownoevalue,
                         showselfpalindromic,
                         selectbundle,
                         extendleftright,
                         procmultiseq,
                         currentdirection,
                         revmposorder,
                         processfinal,
                         evalues,
                         domatchbuffering) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}
