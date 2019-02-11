
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "vnodedef.h"
#include "spacedef.h"
#include "errordef.h"
#include "spacedef.h"
#include "chardef.h"
#include "minmax.h"

//}

/*
  This file implements functions for enumerating virtual nodes.
*/

#define PROCESSCOMPLETENODE(N)\
        state->vnode.left = (N)->leftmostleaf;\
        state->vnode.right = (N)->rightmostleaf;\
        state->vnode.offset = (N)->depth;\
        if(state->processvnode != NULL)\
        {\
          if(state->processvnode(state->info,&state->vnode) != 0)\
          {\
            return (Sint) -1;\
          }\
        }

#define GETSEARCHLENGTH(ST) 100

#define ASSIGNLEFTMOSTLEAF(STACKELEM,VALUE)\
        (STACKELEM).leftmostleaf = VALUE
#define ASSIGNRIGHTMOSTLEAF(STACKELEM,VALUE,SUFVALUE,LCPVALUE)\
        (STACKELEM).rightmostleaf = VALUE

/*
  For each node in the depth first traversal of the virtual suffix tree
  we need three infos.
*/

typedef struct
{
  Uint depth, 
       leftmostleaf,
       rightmostleaf;
  BOOL lastisleafedge;
} Nodeinfo;

DECLAREARRAYSTRUCT(Nodeinfo);

/*
  The following type stores some information required in all
  functions.
*/

typedef struct
{
  Vnode vnode;
  void *info;
  Sint (*processvnode)(void *,Vnode *);
} State;

/*
  We make use of the source code stored in the file \texttt{vdfstrav.c}.
*/

typedef Uint Previoussuffixtype;
#include "vdfstrav.c"

/*EE
  The following function enumerates all leaves of a virtual 
  tree. To each leaf, the function processvleaf is applied.
*/

Uint enumvleaves(Virtualtree *virtualtree,
                 void *info,
                 void (*processvleaf)(void *,Vleaf *))
{
  Uint i, currentlcp, nextlcp, exception = 0, countleaves = 0;
  Vleaf vleaf;

  SEQUENTIALEVALLCPVALUE(currentlcp,1,exception);
  vleaf.suffixnum = 0;
  vleaf.uniquelength = UintConst(1)+currentlcp;
  if(processvleaf != NULL)
  {
    processvleaf(info,&vleaf);
  }
  countleaves++;
  for(i=UintConst(2); i <= virtualtree->multiseq.totallength; i++)
  {
    SEQUENTIALEVALLCPVALUE(nextlcp,i,exception);
    vleaf.suffixnum = i-1;
    vleaf.uniquelength = UintConst(1) + MAX(currentlcp,nextlcp);
    if(processvleaf != NULL)
    {
      processvleaf(info,&vleaf);
    }
    countleaves++;
    currentlcp = nextlcp;
  }
  vleaf.suffixnum = virtualtree->multiseq.totallength;
  vleaf.uniquelength = UintConst(1)+currentlcp;
  if(processvleaf != NULL)
  {
    processvleaf(info,&vleaf);
  }
  countleaves++;
  return countleaves;
}

/*EE
  The following function enumerates all virtual nodes in the given
  virtual suffix tree. To each node enumerated,
  the function \texttt{processvnode} is applied. The return value is
  the number of enumerated virtual nodes.
*/

Sint enumvnodes(Virtualtree *virtualtree,void *info,
                Sint (*processvnode)(void *,Vnode *))
{
  State state;

  state.info = info;
  state.processvnode = processvnode;
  if(depthfirstvstree(&state,UintConst(1),virtualtree) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

BOOL findparentinterval(Vnode *parent,
                        Vnode *child,
                        Uint globalleft,
                        Uint globalright,
                        Uint width,
                        Virtualtree *virtualtree)
{
  Uint idx, lcpvalue;

  if(child->offset == UintConst(1))
  {
    if(width > 0 && globalright - globalleft + 1 > width)
    {
      return False;
    }
    parent->left = globalleft;
    parent->right = globalright;
    parent->offset = 0;
  } else
  {
    if(child->left == globalleft && child->right == globalright)
    {
      if(width > 0 && globalright - globalleft + 1 > width)
      {
        return False;
      }
      parent->left = globalleft;
      parent->right = globalright;
      parent->offset = 0;
    } else
    {
      if(child->right == globalright)
      {
	EVALLCP(virtualtree,parent->offset,child->left);
      } else
      {
	if(child->left == globalleft)
	{
	  EVALLCP(virtualtree,parent->offset,child->right+1);
	} else
	{
	  EVALLCP(virtualtree,parent->offset,child->left);
	  EVALLCP(virtualtree,lcpvalue,child->right+1);
	  if(lcpvalue > parent->offset)
	  {
	    parent->offset = lcpvalue;
	  }
	}
      }
      if(width > 0)
      {
        for(idx=child->left; idx > globalleft; idx--)
        { // process suffixes to the left of witness
	  EVALLCP(virtualtree,lcpvalue,idx);
          if(lcpvalue < parent->offset || child->right - idx + 1  > width)
          {
            break;
          }
        }
        if(child->right - idx + 1  > width)
        {
          return False;
        }
      } else
      {
        for(idx=child->left; idx > globalleft; idx--)
        { // process suffixes to the left of witness
	  EVALLCP(virtualtree,lcpvalue,idx);
          if(lcpvalue < parent->offset)
          {
            break;
          }
        }
      }
      parent->left = idx;
      if(width > 0)
      {
        for(idx=child->right+1; idx <= globalright; idx++)
        { // process suffix to the right of witness
	  EVALLCP(virtualtree,lcpvalue,idx);
          if(lcpvalue < parent->offset || idx - parent->left > width)
          {
            break;
	  }
        }
        if(idx - parent->left > width)
        {
          return False;
        }
      } else
      {
        for(idx=child->right+1; idx <= globalright; idx++)
        { // process suffix to the right of witness
	  EVALLCP(virtualtree,lcpvalue,idx);
          if(lcpvalue < parent->offset)
          {
            break;
	  }
        }
      }
      parent->right = idx-1;
    }
  }
  return True;
}

#define SHOWIDX(V,IDX)\
        printf("# extend (%lu,%lu,%lu) to the %s: startposquery=%lu,"\
               "idx=%lu,lcpval=%lu,minprefix=%lu\n",\
               (Showuint) maxlcp,\
               (Showuint) maxintvleft,\
               (Showuint) maxintvright,\
               #V,\
               (Showuint) startposquery,\
               (Showuint) (IDX),\
               (Showuint) lcpval,\
               (Showuint) minprefix)

Sint findsiblings(Virtualtree *virtualtree,
                  Uint searchlength,
                  Uint maxintvleft,
                  Uint maxintvright,
                  Uint maxlcp,
                  Uint globalleft,
                  Uint globalright,
                  /*@unused@*/ void *info,
                  Uchar *qsubstring,
                  Uchar *qseqptr)
{
  Uint idx,                 // counter
       minprefix,           // minimal length of prefix in current group
       lcpval,              // temporary value of lcp
       lcplast,
       startposquery = (Uint) (qseqptr - qsubstring);
  
  DEBUG3(3,"leftrightsubmatch(maxlcp=%lu,intv=(%lu,%lu)\n",
                              (Showuint) maxlcp,
                              (Showuint) maxintvleft,
                              (Showuint) maxintvright);
  if(maxlcp < UCHAR_MAX)   // prefix length is < 255
  {
    minprefix = maxlcp;
    for(lcplast=idx=maxintvleft; /* Nothing */; idx--)
    { // process suffixes to the left of witness
      if(idx == globalleft ||
         ((lcpval = virtualtree->lcptab[idx])
                  < searchlength))
      {
        if(idx < maxintvleft)
        {
          printf("# overall left interval (%lu,%lu,%lu)\n",
                  (Showuint) minprefix,(Showuint) idx,(Showuint) (lcplast-1));
        }
        break;
      }
      if(minprefix > lcpval)
      {
        if(idx < maxintvleft)
        {
          printf("# overall left interval (%lu,%lu,%lu)\n",
                    (Showuint) minprefix,(Showuint) idx,(Showuint) (lcplast-1));
        }
        minprefix = lcpval;
        lcplast = idx;
      }
      SHOWIDX(left,idx-1);
    }
    minprefix = maxlcp;
    for(lcplast=idx=maxintvright+1; /* Nothing */ ; idx++)
    { // process suffix to the right of witness
      if(idx > globalright ||
         ((lcpval = virtualtree->lcptab[idx]) 
                  < searchlength))
      {
        if(idx > maxintvright+1)
        {
          printf("# overall right interval (%lu,%lu,%lu)\n",
                    (Showuint) minprefix,(Showuint) lcplast,
                    (Showuint) (idx-1));
        }
        break;
      }
      if(minprefix > lcpval)
      {
        if(idx > maxintvright+1)
        {
          printf("# overall right interval (%lu,%lu,%lu)\n",
                 (Showuint) minprefix,
                 (Showuint) lcplast,
                 (Showuint) (idx-1));
        }
        minprefix = lcpval;
        lcplast = idx;
      }
      SHOWIDX(right,idx);
    }
  } else // maxlcp >= UCHAR_MAX
  { // prefix length is >= 255
    PairUint *startexception, // pointer to start of lcp-exception interval
             *prevexception;  // pointer to previously found lcp-exception
    minprefix = maxlcp;
    startexception = prevexception = NULL;
    for(lcplast=idx=maxintvleft; /* Nothing */; idx--)
    {
      if(idx == globalleft)
      {
        break;
      }
      if((lcpval = virtualtree->lcptab[idx]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        { // find lcp-exception by binary search
          prevexception = startexception 
                        = getexception(virtualtree,idx);
        } 
        lcpval = (prevexception--)->uint1;
      }
      if(lcpval < searchlength)
      {
        break;
      }
      if(minprefix > lcpval)
      {
        minprefix = lcpval;
        lcplast = idx;
      }
      SHOWIDX(left,idx-1);
    }
    minprefix = maxlcp;
    startexception = prevexception = NULL;
    for(lcplast=idx=maxintvright+1; /* Nothing */ ; idx++)
    {
      if(idx > globalright)
      {
        break;
      }
      if((lcpval = virtualtree->lcptab[idx]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        {
          prevexception = startexception 
                        = getexception(virtualtree,idx);
        } 
        lcpval = (prevexception++)->uint1;
      }
      if(lcpval < searchlength)
      {
        break;
      }
      if(minprefix > lcpval)
      {
        minprefix = lcpval;
        lcplast = idx;
      }
      SHOWIDX(right,idx);
    }
  }
  return 0;
}
