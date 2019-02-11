#ifndef VDFSTRAV_C
#define VDFSTRAV_C

#ifdef ESASTREAMACCESS
#define ACCESSTYPE Esastream
#define ACCESSPARAMETER esastream
#else
#ifdef ESAMERGEDACCESS
#define ACCESSTYPE Emissionmergedesa
#define ACCESSPARAMETER emmesa
#else
#define ACCESSTYPE Virtualtree
#define ACCESSPARAMETER virtualtree
#endif
#endif

#ifdef ESASTREAMACCESS

#define READNEXTLCPTABVALUE(VAR)\
        retval = readnextUchar(&smalltmplcpvalue,&esastream->lcptabstream);\
        if(retval < 0)\
        {\
          return (Sint) -2;\
        }\
        if(retval == 0)\
        {\
          break;\
        }\
        if(smalltmplcpvalue == UCHAR_MAX)\
        {\
          retval = readnextPairUint(&exceptionpair,&esastream->llvtabstream);\
          if(retval < 0)\
          {\
            return (Sint) -3;\
          }\
          if(retval == 0)\
          {\
            ERROREOF("llvtab");\
            return (Sint) -4;\
          }\
          VAR = exceptionpair.uint1;\
        } else\
        {\
          VAR = (Uint) smalltmplcpvalue;\
        }
#else

#ifdef ESAMERGEDACCESS
#define READNEXTLCPTABVALUE(VAR)\
        retval = nextesamergedsuflcptabvalues(&(VAR),&previoussuffix,emmesa);\
        if(retval < 0)\
        {\
          return (Sint) -1;\
        }\
        if(retval == 0)\
        {\
          break;\
        }

#else
#define READNEXTLCPTABVALUE(VAR)\
        SEQUENTIALEVALLCPVALUE(VAR,currentindex+1,exception)

#endif  /* ESASTREAMACCESS */
#endif  /* ESAMERGEDACCESS */

#ifdef WITHLEFTCHAR

/*
  compute leftchar=text[previoussuffix-1] if previoussuffiX > 0 
       or leftchar=INITIALCHAR            otherwise
*/

#ifdef DEBUG
#define CHECKIFLONGESTISDEFINED(V)\
        if(!(V)->longest.defined)\
        {\
          ERROR1("%s->longest is not defined",#V);\
          return (Sint) -3;\
        }
#else
#define CHECKIFLONGESTISDEFINED(V) /* Nothing */
#endif

#ifdef ESASTREAMACCESS
#define GETLEFTCHAR(VAR,POS)\
        retval = readnextUchar(&(VAR),&esastream->bwttabstream);\
        if(retval < 0)\
        {\
          return (Sint) -1;\
        }\
        if(retval == 0)\
        {\
          ERROREOF("bwttab");\
          return (Sint) -2;\
        }\
        CHECKIFLONGESTISDEFINED(esastream);\
        if((POS) == esastream->longest.uintvalue)\
        {\
          VAR = (Uchar) INITIALCHAR;\
        }
#else
#ifdef ESAMERGEDACCESS
#define GETLEFTCHAR(VAR,POS)\
        if(previoussuffix.startpos == 0)\
        {\
          VAR = (Uchar) INITIALCHAR;\
        } else\
        {\
          VAR = ACCESSENCODEDCHAR(emmesa->encseqtable + previoussuffix.idx,\
                                  previoussuffix.startpos-1);\
        }
#else
#define GETLEFTCHAR(VAR,POS)\
        CHECKIFLONGESTISDEFINED(virtualtree);\
        if((POS) == virtualtree->longest.uintvalue)\
        {\
          VAR = (Uchar) INITIALCHAR;\
        } else\
        {\
          VAR = virtualtree->bwttab[POS];\
        }
#endif  /* ESASTREAMACCESS */
#endif  /* ESAMERGEDACCESS */

#else
#define GETLEFTCHAR(VAR,IDX)          /* Nothing */

#endif  /* WITHLEFTCHAR */

#ifdef WITHSUFFIX

#ifdef ESASTREAMACCESS
#define READNEXTSUFTABVALUE(VAR)\
        retval = readnextUint(&(VAR),&esastream->suftabstream);\
        if(retval < 0)\
        {\
          return (Sint) -4; \
        }\
        if(retval == 0)\
        {\
          ERROREOF("suftab");\
          return (Sint) -5;\
        }
#else
#ifdef ESAMERGEDACCESS
#define READNEXTSUFTABVALUE(VAR)\
        /* Nothing */

#else
#define READNEXTSUFTABVALUE(VAR)\
        VAR = virtualtree->suftab[currentindex]
#endif  /* ESASTREAMACCESS */
#endif  /* ESAMERGEDACCESS */

#else
#define READNEXTSUFTABVALUE(VAR) /* Nothing */
#endif  /* WITHSUFFIX */

#ifndef STATICSTACKSPACE
#define STATICSTACKSPACE 512
#endif

#define ABOVETOP  stackptr[nextfreeNodeinfo]
#define TOP       stackptr[nextfreeNodeinfo-1]
#define BELOWTOP  stackptr[nextfreeNodeinfo-2]

#define PUSHDFS(D,B)\
        stackptr[nextfreeNodeinfo].depth = D;\
        stackptr[nextfreeNodeinfo].lastisleafedge = B;\
        nextfreeNodeinfo++

#define FIRSTSUFTABINDEX     0

#if !defined ESASTREAMACCESS && !defined ESAMERGEDACCESS
#define LASTSUFTABINDEX      (virtualtree->multiseq.totallength-1)
#endif

#define FIRSTLCPVALUE        0
#define FIRSTEXCEPTIONINDEX  0

#ifndef ASSIGNLEFTMOSTLEAF
#define ASSIGNLEFTMOSTLEAF(STACKELEM,VALUE)            /* Nothing */
#endif

#ifndef ASSIGNRIGHTMOSTLEAF
#define ASSIGNRIGHTMOSTLEAF(STACKELEM,VALUE,SUFVALUE,LCPVALUE)   /* Nothing */
#endif

#ifndef PROCESSLEAFEDGE
#define PROCESSLEAFEDGE(FIRSTSUCC)                     /* Nothing */
#endif

#ifndef PROCESSBRANCHEDGE
#define PROCESSBRANCHEDGE(FIRSTSUCC,D,L,R)             /* Nothing */
#endif

#ifndef PROCESSCOMPLETENODE
#define PROCESSCOMPLETENODE(STACKELEM)                 /* Nothing */
#endif

#ifndef PROCESSSPLITLEAFEDGE
#define PROCESSSPLITLEAFEDGE                           /* Nothing */
#endif

#ifndef DECLARERETCODE
#define DECLARERETCODE                                 /* Nothing */
#endif

#ifndef ALLOCATEEXTRASTACKELEMENTS
#define ALLOCATEEXTRASTACKELEMENTS(STPTR,SZ)           /* Nothing */
#endif

#ifndef REALLOCATEEXTRASTACKELEMENTS
#define REALLOCATEEXTRASTACKELEMENTS(STPTR,PSZ,SZ)     /* Nothing */
#endif

#ifndef FREENODESTACKSPACE
#define FREENODESTACKSPACE(SZ,STPTR)                   /* Nothing */
#endif

#ifndef GETSEARCHLENGTH
#define GETSEARCHLENGTH(ST)     (ST)->searchlength
#endif

#define CHECKSTACKSIZE(VAL)\
        if(!(nextfreeNodeinfo >= (Uint) (VAL)))\
        {\
          fprintf(stderr,"line %lu: stack is not >= %lu\n",\
                  (Showuint) __LINE__,(Showuint) (VAL));\
          exit(EXIT_FAILURE);\
        }

#ifdef ESASTREAMACCESS
DECLAREREADFUNCTION(Uchar)

DECLAREREADFUNCTION(Uint)

DECLAREREADFUNCTION(PairUint)
#else
#ifdef ESAMERGEDACCESS

#include "nextesamerged.c"

#endif
#endif

static Sint depthfirstvstreegeneric(State *state,
                                    ACCESSTYPE *ACCESSPARAMETER,
                                    Uint firstsuftabindex,
#if !defined ESASTREAMACCESS && !defined ESAMERGEDACCESS
                                    Uint lastsuftabindex,
#endif
                                    /*@unused@*/ Uint firstexceptionindex)
{
#if defined ESASTREAMACCESS || defined ESAMERGEDACCESS
  Sint retval;
#endif
#ifdef ESASTREAMACCESS
  __attribute__ ((unused)) Uchar smalltmplcpvalue = 0;
#endif
  __attribute__ ((unused)) BOOL firstedge;
  BOOL firstrootedge;
  Uint currentindex,
       currentlcp = 0, /* May be necessary if the lcpvalue is used after the
                          outer while loop */
       allocatedNodeinfo,
       nextfreeNodeinfo;
#ifdef ESASTREAMACCESS
  PairUint exceptionpair;
#else
#ifndef ESAMERGEDACCESS
  Uint exception = firstexceptionindex;
#endif
#endif
  Nodeinfo *stackptr,
           stackspace[STATICSTACKSPACE];
  DECLARERETCODE
#ifdef WITHSUFFIX
  __attribute__ ((unused)) Previoussuffixtype previoussuffix = 0;
#endif
#ifdef WITHPREVIOUSLCP
  Uint previouslcp;
#endif
#ifdef WITHLEFTCHAR
  __attribute__ ((unused)) Uchar leftchar = 0;
#endif

  allocatedNodeinfo = (Uint) STATICSTACKSPACE;
  nextfreeNodeinfo = 0;
  firstrootedge = True;
  stackptr = &stackspace[0];
#ifdef ESAMERGEDACCESS
  previoussuffix.startpos = 0;
  previoussuffix.idx = 0;
#endif
  ALLOCATEEXTRASTACKELEMENTS(stackptr,(Uint) STATICSTACKSPACE);
  PUSHDFS(FIRSTLCPVALUE,True);
  ASSIGNLEFTMOSTLEAF(TOP,firstsuftabindex);
  for(currentindex = firstsuftabindex;
#if !defined ESASTREAMACCESS && !defined ESAMERGEDACCESS
      currentindex <= lastsuftabindex;
#else
      /* Nothing */;
#endif
      currentindex++)
  {
    READNEXTLCPTABVALUE(currentlcp);
    READNEXTSUFTABVALUE(previoussuffix);
    GETLEFTCHAR(leftchar,currentindex);
    while(currentlcp < TOP.depth)    // splitting edge is reached 
    {
      if(TOP.lastisleafedge)       // last edge from top-node is leaf
      {                            // previoussuffix
        PROCESSLEAFEDGE(False);
      } else
      {
        PROCESSBRANCHEDGE(False,ABOVETOP.depth,
                                ABOVETOP.leftmostleaf,
                                ABOVETOP.rightmostleaf);
      }
      ASSIGNRIGHTMOSTLEAF(TOP,
                          currentindex,
                          previoussuffix,
                          currentlcp);
      PROCESSCOMPLETENODE(&TOP);
#ifdef DEBUG
      if(nextfreeNodeinfo == 0)
      {
        ERROR0("cannot pop from empty stack");
        return (Sint) -1;
      }
#endif
      nextfreeNodeinfo--;
    }
    CHECKSTACKSIZE(1);
    if(currentlcp == TOP.depth)
    {
      // add leaf edge to TOP-node
      if(firstrootedge && TOP.depth == 0)
      {
        firstedge = True;
        firstrootedge = False;
      } else
      {
        firstedge = False;
      }
      if(TOP.lastisleafedge)
      {
        PROCESSLEAFEDGE(firstedge);
      } else
      {
        PROCESSBRANCHEDGE(firstedge,ABOVETOP.depth,
                                    ABOVETOP.leftmostleaf,
                                    ABOVETOP.rightmostleaf);
        TOP.lastisleafedge = True;
      }
    } else
    {
#ifdef WITHPREVIOUSLCP
      previouslcp = ABOVETOP.depth;
#endif
      if(nextfreeNodeinfo >= allocatedNodeinfo)
      {
        if(allocatedNodeinfo == (Uint) STATICSTACKSPACE)
        {
          ALLOCASSIGNSPACE(stackptr,NULL,Nodeinfo,
                           nextfreeNodeinfo + STATICSTACKSPACE);
          memcpy(stackptr,&stackspace[0],
                 (size_t) (sizeof(Nodeinfo) * STATICSTACKSPACE));
          REALLOCATEEXTRASTACKELEMENTS(stackptr,(Uint) STATICSTACKSPACE,
                                       nextfreeNodeinfo);
        } else
        {
          ALLOCASSIGNSPACE(stackptr,stackptr,Nodeinfo,
                           allocatedNodeinfo+STATICSTACKSPACE);
          REALLOCATEEXTRASTACKELEMENTS(stackptr,
                                       allocatedNodeinfo,
                                       STATICSTACKSPACE);
        }
        allocatedNodeinfo += STATICSTACKSPACE;
      }
      PUSHDFS(currentlcp,True);
      CHECKSTACKSIZE(2);
      if(BELOWTOP.lastisleafedge)
      {
        // replace leaf edge by internal Edge
        ASSIGNLEFTMOSTLEAF(TOP,currentindex);
        PROCESSSPLITLEAFEDGE;
        PROCESSLEAFEDGE(True);
        BELOWTOP.lastisleafedge = False;
      } else
      {
        PROCESSBRANCHEDGE(True,previouslcp,
                               TOP.leftmostleaf,
                               TOP.rightmostleaf);
      }
    }
  }
  if(TOP.lastisleafedge)
  {
    GETLEFTCHAR(leftchar,currentindex);
    READNEXTSUFTABVALUE(previoussuffix);
    PROCESSLEAFEDGE(False);
    ASSIGNRIGHTMOSTLEAF(TOP,
                        currentindex,
                        previoussuffix,
                        currentlcp);
    PROCESSCOMPLETENODE(&TOP);
  }
  FREENODESTACKSPACE(allocatedNodeinfo,stackptr);
  if(allocatedNodeinfo > (Uint) STATICSTACKSPACE)
  {
    FREESPACE(stackptr);
  }
  return 0;
}

#ifdef  DISTRIBUTEDDFS

/*
  The idea of the distributed depth first traversal of the enhanced
  suffix array is to split the complete 0-interval [0,n] into 
  subintervals. Each subinterval is then processed independently
  from each other. To obtain the subintervals, we use table 
  virtualtree->bcktab. This contains virtualtree->numofcodes many 
  buckets dividing the [0,n]. According to the number $p$ of 
  processors specified by the option -numproc, we divide [0,n]
  into $p$ superbuckets by combining virtualtree->numofcodes/p
  consecutive buckets. This is done by the function 
  simplebck2superbck implemented in kurtz/superbck.c. Each superbucket
  is characterized by a structure of type Superbckinterval.
  It is very important to verify that the first lcp-value of
  the superbucket is not large than the search length,
  since otherwise we could miss matches.

  To test the correctness of our strategy, we iterate over an 
  array of all superbuckets from right to left and call 
  depthfirstvstreegeneric for the current superbucket.
  We could as well process the superbuckets in any order,
  or in parallel. So to obtain a parallel version of the
  depth first search (and all vmatch-algorithms using this)
  we could just distribute the p calls to the function 
  depthfirstvstreegeneric to p processors.
*/

static Sint depthfirstvstree(State *state,
                             Uint numberofprocessors,
                             Virtualtree *virtualtree)
{
  Superbckinterval *superbcktab, 
                   *superbckptr;

  if(numberofprocessors == UintConst(1))
  {
    return depthfirstvstreegeneric(state,
                                   virtualtree,
                                   FIRSTSUFTABINDEX,
                                   LASTSUFTABINDEX,
                                   FIRSTEXCEPTIONINDEX,
                                   virtualtree->multiseq.totallength);
  }
  ALLOCASSIGNSPACE(superbcktab,NULL,Superbckinterval,numberofprocessors);
  if(simplebck2superbck(superbcktab,
                        numberofprocessors,
                        virtualtree) != 0)
  {
    return (Sint) -1;
  }
  DEBUGCODE(1,showsuperbck(virtualtree,
                           superbcktab,
                           numberofprocessors));
  for(superbckptr = superbcktab + numberofprocessors - 1; 
      superbckptr >= superbcktab; superbckptr--)
  {
    if(GETSEARCHLENGTH(state) <= superbckptr->firstlcpvalue)
    {
      ERROR3("searchlength = %lu <= %lu = superbck[%lu].firstlcpvalue",
              (Showuint) GETSEARCHLENGTH(state),
              (Showuint) superbckptr->firstlcpvalue,
              (Showuint) (Uint) (superbckptr - superbcktab));
      return (Sint) -1;
    }
    DEBUG3(2,"# process superbck (%lu,%lu,%lu)\n",
              (Showuint) superbckptr->firstsuftabindex,
              (Showuint) superbckptr->lastsuftabindex,
              (Showuint) superbckptr->firstexceptionindex);
    if(depthfirstvstreegeneric(state,
                               virtualtree,
                               superbckptr->firstsuftabindex,
                               superbckptr->lastsuftabindex,
                               superbckptr->firstexceptionindex) != 0)
    {
      return (Sint) -2;
    }
  }
  FREESPACE(superbcktab);
  return 0;
}

#else

static Sint depthfirstvstree(State *state,
                             /*@unused@*/ __attribute__ ((unused))
                             Uint numberofprocessors,
                             ACCESSTYPE *ACCESSPARAMETER)
{
  return depthfirstvstreegeneric(state,
                                 ACCESSPARAMETER,
                                 FIRSTSUFTABINDEX,
#if !defined ESASTREAMACCESS && !defined ESAMERGEDACCESS
                                 LASTSUFTABINDEX,
#endif
                                 FIRSTEXCEPTIONINDEX);
}

#endif

#endif
