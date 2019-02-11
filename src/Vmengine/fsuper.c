
#include "virtualdef.h"
#include "errordef.h"
#include "chardef.h"
#include "debugdef.h"
#include "vnodedef.h"

/*
  This file implements functions for enumerating virtual nodes.
*/

#define PROCESSLEAFEDGE(FIRSTSUCC)\
        if(FIRSTSUCC)\
        {\
          TOP.alwaysontop = True;\
        }

#define PROCESSBRANCHEDGE(FIRSTSUCC,D,L,R)\
        TOP.alwaysontop = False

#define PROCESSCOMPLETENODE(N)\
        state->vnode.left = (N)->leftmostleaf;\
        state->vnode.right = (N)->rightmostleaf;\
        state->vnode.offset = (N)->depth;\
        state->alwaysontop = (N)->alwaysontop;\
        if(selectsupermaxialrepeats(state) != 0)\
        {\
          return (Sint) -1;\
        }

#define ASSIGNLEFTMOSTLEAF(STACKELEM,VALUE)\
        (STACKELEM).leftmostleaf = VALUE
#define ASSIGNRIGHTMOSTLEAF(STACKELEM,VALUE,SUFVALUE,LCPVALUE)\
        (STACKELEM).rightmostleaf = VALUE

typedef struct
{
  Uint depth, 
       leftmostleaf,
       rightmostleaf;
  BOOL alwaysontop,
       lastisleafedge;
} Nodeinfo;

DECLAREARRAYSTRUCT(Nodeinfo);

/*
  The following type stores some information required in all
  functions.
*/

typedef struct
{
  Vnode vnode;
  BOOL alwaysontop;
  Uint searchlength;
  Virtualtree *virtualtree;
  void *outinfo;
  Outputfunction output;
  BOOL marktab[UCHAR_MAX+1];
} State;

static BOOL verifysupermaximality(State *state)
{
  Uint q;
  Uchar cc;
  BOOL marksep;

  if(state->vnode.offset < state->searchlength)
  {
    return False;
  }
  if(!state->alwaysontop)
  {
    return False;
  }
  for(q=0; q<state->virtualtree->alpha.mapsize-1; q++)
  {
    state->marktab[q] = False;
  }
  marksep = False;
  for(q=state->vnode.left; q<= state->vnode.right; q++)
  {
    if(q == state->virtualtree->longest.uintvalue)
    {
      if(marksep)
      {
        return False;
      }
      marksep = True;
    } else
    {
      cc = state->virtualtree->bwttab[q];
      if(ISNOTSPECIAL(cc))
      {
	if(state->marktab[cc])
	{
          return False;
	}
	state->marktab[cc] = True;
      }
    }
  }
  return True;
}

static Sint selectsupermaxialrepeats(State *state)
{
  Uint s, t;

  if(verifysupermaximality(state))
  {
    for(s=state->vnode.left; s < state->vnode.right; s++)
    {
      for(t=s+1; t<=state->vnode.right; t++)
      {
        if(state->output(state->outinfo,
                         state->vnode.offset,
                         state->virtualtree->suftab[s],
                         state->virtualtree->suftab[t]) != 0)
        {
          return (Sint) -1;
        }
      }
    }
  }
  return 0;
}

/*
  We make use of the source code stored in the file \texttt{vdfstrav.c}.
*/

typedef Uint Previoussuffixtype;
#include "vdfstrav.c"

/*EE
  The following function enumerates all leaves of a virtual 
  tree. To each leaf, the function processvleaf is applied.
*/

Sint findsupermax(Virtualtree *virtualtree,
                  /*@unused@*/ Uint numberofprocessors,
                  Uint searchlength,
                  /*@unused@*/ void *repeatgapspecinfo,
                  void *outinfo,
                  Outputfunction output)
{
  State state;

  if(!virtualtree->longest.defined)
  {
    ERROR0("findsupermax: longest not defined");
    return (Sint) -1;
  }
  state.virtualtree = virtualtree;
  state.searchlength = searchlength;
  state.outinfo = outinfo;
  state.output = output;
  if(depthfirstvstree(&state,UintConst(1),virtualtree) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}
