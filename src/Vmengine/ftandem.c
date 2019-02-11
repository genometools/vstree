
#include <string.h>
#include "virtualdef.h"
#include "divmodmul.h"
#include "errordef.h"
#include "chardef.h"
#include "debugdef.h"
#include "spacedef.h"
#include "vnodedef.h"

#include "matchstate.h"

#include "initmstate.pr"

#define PROCESSCOMPLETENODE(N)\
        if(TOP.depth >= state->matchparam.userdefinedleastlength)\
        {\
          if(processcompletenode(state,N) != 0)\
          {\
            return (Sint) -1;\
          }\
        }

#define GETSEARCHLENGTH(ST) ((ST)->matchparam.userdefinedleastlength)

#define ASSIGNLEFTMOSTLEAF(STACKELEM,VALUE)\
        (STACKELEM).leftmostleaf = VALUE
#define ASSIGNRIGHTMOSTLEAF(STACKELEM,VALUE,SUFVALUE,LCPVALUE)\
        (STACKELEM).rightmostleaf = VALUE

#define OUTTANDEM(L,S)\
        {\
          match.length1 = match.length2 = (L);\
          match.position1 = (S);\
          match.position2 = match.position1 + (L);\
          if(matchstate->processfinal(matchstate,&match) != 0)\
          {\
            return (Sint) -1;\
          }\
        }

#define SHOWTANDEM(V,W)\
        c1 = matchstate->virtualtree->multiseq.sequence[V];\
        if((W) + depth == matchstate->virtualtree->multiseq.totallength)\
        {\
          OUTTANDEM(depth,V);\
        } else\
        {\
          c2 = matchstate->virtualtree->multiseq.sequence[(W) + depth];\
          if(c1 != c2 || ISSPECIAL(c1) || ISSPECIAL(c2))\
          {\
            OUTTANDEM(depth,V);\
          }\
        }

#define CHECKPAIR(V,W)\
        if((V) + depth == (W))\
        {\
          SHOWTANDEM(V,W);\
        } else\
        {\
          if((W) + depth == (V))\
          {\
            SHOWTANDEM(W,V);\
          }\
        }

#define PROCESSSUFFIX(I)\
        sufstart = matchstate->virtualtree->suftab[I];\
        DEBUG2(3,"suffix %lu: sufstart=%lu\n",\
                  (Showuint) (I),\
                  (Showuint) matchstate->virtualtree->suftab[I]);\
	c1 = matchstate->virtualtree->multiseq.sequence[sufstart];\
	if(sufstart + totaldepth == matchstate->virtualtree->multiseq.totallength)\
        {\
          OUTTANDEM(DIV2(totaldepth),sufstart);\
        } else\
        {\
          c2 = matchstate->virtualtree->multiseq.sequence[sufstart + totaldepth];\
          if(c1 != c2 || ISSPECIAL(c1) || ISSPECIAL(c2))\
          {\
            OUTTANDEM(DIV2(totaldepth),sufstart);\
          }\
        }

typedef struct
{
  Uint depth,
       leftmostleaf,
       rightmostleaf;
  BOOL lastisleafedge;
} Nodeinfo;

DECLAREARRAYSTRUCT(Nodeinfo);

typedef Matchstate State;

static Sint tandemleftright(Matchstate *matchstate,
                            Uint totaldepth,
                            Uint witness)
{
  Uint ind,        // counter
       lcpval,     // temporary value of lcp
       sufstart;   // start position of suffix
  Uchar c1, c2;
  Match match;
  
  DEBUG2(3,"tandemleftright(totaldepth=%lu,witness=%lu)\n",
            (Showuint) totaldepth,
            (Showuint) witness);
  match.distance = 0;
  match.flag = 0;
  match.seqnum2 = UNDEFSEQNUM2(&matchstate->virtualtree->multiseq);
  if(totaldepth < UCHAR_MAX)   // prefix length is < 255
  {
    for(ind = witness; /* Nothing */; ind--)
    { // process suffixes to the left of witness
      PROCESSSUFFIX(ind);
      if(ind == 0 || 
         ((lcpval = matchstate->virtualtree->lcptab[ind]) < totaldepth))
      {
        break;
      }
    }
    for(ind=witness+1; /* Nothing */ ; ind++)
    { // process suffix to the right of witness
      if(ind > matchstate->virtualtree->multiseq.totallength || 
         ((lcpval = matchstate->virtualtree->lcptab[ind]) < totaldepth))
      {
        break;
      }
      PROCESSSUFFIX(ind);
    }
  } else
  { // prefix length is >= 255
    PairUint *startexception, // pointer to start of lcp-exception interval
             *prevexception;  // pointer to previously found lcp-exception
    startexception = prevexception = NULL;
    for(ind=witness; /* Nothing */; ind--)
    {
      PROCESSSUFFIX(ind);
      if(ind == 0)
      {
        break;
      }
      if((lcpval = matchstate->virtualtree->lcptab[ind]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        { // find lcp-exception by binary search
          prevexception = startexception 
                        = getexception(matchstate->virtualtree,ind);
        }
        lcpval = (prevexception--)->uint1;
      }
      if(lcpval < totaldepth)
      {
        break;
      }
    }
    startexception = prevexception = NULL;
    for(ind=witness+1; /* Nothing */ ; ind++)
    {
      if(ind > matchstate->virtualtree->multiseq.totallength)
      {
        break;
      }
      if((lcpval = matchstate->virtualtree->lcptab[ind]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        {
          prevexception = startexception 
                        = getexception(matchstate->virtualtree,ind);
        } 
        lcpval = (prevexception++)->uint1;
      }
      if(lcpval < totaldepth)
      {
        break;
      }
      PROCESSSUFFIX(ind);
    }
  }
  return 0;
}

static Sint processsmallinterval(Matchstate *matchstate,
                                 Uint howmany,Uint depth,Uint left)
{
  Uchar c1, c2;
  Match match;

  match.distance = 0;
  match.flag = 0;
  match.seqnum2 = UNDEFSEQNUM2(&matchstate->virtualtree->multiseq);
  if(howmany == UintConst(2))
  {
    Uint s0 = matchstate->virtualtree->suftab[left],
         s1 = matchstate->virtualtree->suftab[left+1];
    CHECKPAIR(s0,s1);
  } else
  {
    Uint s0 = matchstate->virtualtree->suftab[left],
         s1 = matchstate->virtualtree->suftab[left + 1],
         s2 = matchstate->virtualtree->suftab[left + 2];
    CHECKPAIR(s0,s1);
    CHECKPAIR(s0,s2);
    CHECKPAIR(s1,s2);
  }
  return 0;
}

static Sint processcompletenode(Matchstate *matchstate,Nodeinfo *node)
{
  DEBUG3(3,"processcompletenode(%lu,%lu,%lu)\n",
           (Showuint) node->depth,
           (Showuint) node->leftmostleaf,
           (Showuint) node->rightmostleaf);
  if((node->rightmostleaf - node->leftmostleaf + UintConst(1)) <= UintConst(2))
  {
    if(processsmallinterval(matchstate,
                            node->rightmostleaf-node->leftmostleaf+1,
                            node->depth,node->leftmostleaf) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    Vnode vnode;
    PairUint maxwit;
    Uchar *query;

    vnode.offset = node->depth;
    vnode.left = node->leftmostleaf;
    vnode.right = node->rightmostleaf;
    DEBUG3(3,"visit (%lu,%lu,%lu)\n",
            (Showuint) vnode.offset,
            (Showuint) vnode.left,
            (Showuint) vnode.right);
    query = matchstate->virtualtree->multiseq.sequence 
            + matchstate->virtualtree->suftab[vnode.left] 
            - node->depth;
    findmaxprefixlen(matchstate->virtualtree,&vnode,query,MULT2(node->depth),
                     &maxwit);
    DEBUG2(3,"maxpref=(%lu,%lu)\n",
            (Showuint) maxwit.uint0,
            (Showuint) maxwit.uint1);
    if(maxwit.uint0 == MULT2(node->depth))
    {
      if(tandemleftright(matchstate,maxwit.uint0,maxwit.uint1) != 0)
      {
        return (Sint) -2;
      }
    }
  }
  return 0;
}

typedef Uint Previoussuffixtype;
#include "vdfstrav.c"

Sint findtandems(Virtualtree *virtualtree,
                 Matchparam *matchparam,
                 Bestflag bestflag,
                 Uint shownoevalue,
                 Uint showselfpalindromic,
                 SelectBundle *selectbundle,
                 void *procmultiseq,
                 Processfinalfunction processfinal,
                 Evalues *evalues,
                 BOOL domatchbuffering)
{
  Matchstate matchstate;

  if(HASINDEXEDQUERIES(&virtualtree->multiseq))
  {
    ERROR0("tandem repeat search does not allow query files in index");
    return (Sint) -1;
  }
  if(initMatchstate(&matchstate,
                    virtualtree,
                    NULL,
                    matchparam,
                    bestflag,
                    shownoevalue,
                    showselfpalindromic,
                    selectbundle,
                    0, // onlinequerynumoffset
                    procmultiseq,
                    DirectionForward,
                    False,
                    processfinal,
                    evalues,
                    domatchbuffering) != 0)
  {
    return (Sint) -1;
  }
  if(depthfirstvstree(&matchstate,
                      matchparam->numberofprocessors,
                      virtualtree) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}
