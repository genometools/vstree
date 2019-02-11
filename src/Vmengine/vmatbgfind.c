
/*
  vmatbgfind.c: Find maximal pairs with bounded gap
  Author: Henning Stehr <h.stehr@tuhh.de>
  Date: 27/Sep/2004

  To include this file into a project:
  - define Bounds datastructure with boundfunctions and/or constants
  - define output function for reporting pairs
  - map virtual tree (as in testvmatbgfind.c)
*/

/*--------------------------- Preprocessor directives ------------------------*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "arraydef.h"
#include "errordef.h"
#include "assert.h"
#include "boundedgaps.h"    // Datastructure: Repeatgapspec
#include "leafblocktree.h"  // Datastructure: Leafblocktree, Flag: SHOWMEMREPOS

#define INITIALCHAR     (state->alphabetsize+1)
#define WITHPREVIOUSLCP

#define PROCESSLEAFEDGE(FIRSTSUCC)\
        if(processleafedge(state,FIRSTSUCC,&TOP,leftchar,previoussuffix) != 0)\
        {\
          return (Sint) -1;\
        }

#define PROCESSBRANCHEDGE(FIRSTSUCC,D,L,R)\
        if(processbranch(state,FIRSTSUCC,&TOP,D,virtualtree) != 0)\
        {\
          return (Sint) -2;\
        }

/*--------------------------- Type definitions -------------------------------*/
 
typedef struct
{
  BOOL lastisleafedge;
  Uint depth;
} Nodeinfo; // required by vdfstrav.c

DECLAREARRAYSTRUCT(Nodeinfo);

typedef Leafblocktree* LeafblocktreePtr;

DECLAREARRAYSTRUCT(LeafblocktreePtr);

/* state */

typedef struct
{
  BOOL                   initialized;
  Uint                   depth,            // value changes with each new match
                         searchlength,
                         alphabetsize;
  void                   *outinfo;         // Info-record for output function
  Outputfunction         output;           // Output function
  ArrayUint              sizestack;        // stack for number of children  
  double                 p;                // p parameter for skiplists
  Uint                   maxsize,          // maxsize parameter for skiplists
                         maxlevel,         // maximal node level
                         sizestackincrement; // cells to be alloc'd on overflow
  Bounds                 *bounds;          // user boundfunctions and constants
  Memrepos               memrepos;         // repository of dynamic arrays
} State;                                   // \Typedef{State}

/*---------------------------- Stack operations ------------------------------*/

/* stack opertions for leaflist stack */

#define PUSHLEAFLISTDEEP(TREE)\
        PUSHLEAFBLOCKTREEDEEP(TREE, state->maxlevel)

#define POPLEAFLISTDEEP(TREE)\
        POPLEAFBLOCKTREEDEEP(TREE, state->maxlevel)

/* stack operations for size stack */

#define TOPSIZE (state->sizestack.spaceUint[state->sizestack.nextfreeUint-1])
#define PUSHSIZE(SIZE)\
        CHECKARRAYSPACE(&state->sizestack,Uint,state->sizestackincrement);\
        state->sizestack.spaceUint[state->sizestack.nextfreeUint] = SIZE;\
        state->sizestack.nextfreeUint++
#define POPSIZE (state->sizestack.spaceUint[--state->sizestack.nextfreeUint]);

/*---------------------- Memory management functions -------------------------*/
Sint compareleafblocktrees(Leafblocktree *lbt1, Leafblocktree *lbt2)
{
  Sint     i, error;
  Skiplist *llt1,
           *llt2,
           *bst1,
           *bst2;
  Skipnode **llt1hp, **llt2hp,
           **bst1hp, **bst2hp,
           ***llt1tp, ***llt2tp,
           ***bst1tp, ***bst2tp;
  
  llt1 = lbt1->leaflisttree;
  llt2 = lbt2->leaflisttree;
  bst1 = lbt1->blockstarttree;
  bst2 = lbt2->blockstarttree;

  llt1hp = llt1->headpointers;
  llt2hp = llt2->headpointers;
  bst1hp = bst1->headpointers;
  bst2hp = bst2->headpointers;

  llt1tp = llt1->tracepointers;
  llt2tp = llt2->tracepointers;
  bst1tp = bst1->tracepointers;
  bst2tp = bst2->tracepointers;

  if(llt1->currentlistsize != llt2->currentlistsize) return (Sint) -1;
  if(llt1->maxlistsize != llt2->maxlistsize) return (Sint) -2;
  if(llt1->maxnodelevel != llt2->maxnodelevel) return (Sint) -3;
  if(llt1->p != llt2->p) return (Sint) -4;
  if(llt1->currentelement != llt2->currentelement) return (Sint) -5;

  if(bst1->currentlistsize != bst2->currentlistsize) return (Sint) -6;
  if(bst1->maxlistsize != bst2->maxlistsize) return (Sint) -7;
  if(bst1->maxnodelevel != bst2->maxnodelevel) return (Sint) -8;
  if(bst1->p != bst2->p) return (Sint) -9;
  if(bst1->currentelement != bst2->currentelement) return (Sint) -10;

  error = (Sint) -100;
  for(i = 0; i < (Sint) llt1->maxnodelevel; i++)
  {
    if(llt1hp[i] != llt2hp[i]) return (error - i);
  }

  error = (Sint) -200;
  for(i = 0; i < (Sint) llt1->maxnodelevel; i++)
  {
    if(llt1tp[i] != llt2tp[i]) return (error - i);
  }    

  error = (Sint) -300;
  for(i = 0; i < (Sint) bst1->maxnodelevel; i++)
  {
    if(bst1hp[i] != bst2hp[i]) return (error - i);
  }

  error = (Sint) -400;
  for(i = 0; i < (Sint) bst1->maxnodelevel; i++)
  {
    if(bst1tp[i] != bst2tp[i]) return (error - i);
  }

  return 0;
}

/*---------------------------- Functions -------------------------------------*/
 
static Sint processleafedge(State *state,BOOL firstsucc, Nodeinfo *father,
                            Uchar leftchar,Uint leafnumber)
{
  Sint retval;
  Uint i;

  DEBUG4(4,"Leaf %lu: firstsucc=%s, leftchar=%lu, fatherdepth=%lu\n",
            (Showuint) leafnumber,
            SHOWBOOL(firstsucc),
	    (Showuint) leftchar,
	    (Showuint) father->depth);
   
  /* if father already out of searchlength, do nothing */
  if(father->depth < state->searchlength)
  {
    return 0;
  }

  /*
    create a new tree with current position
    put it on the tree stack
    if firstsuccessor then put 1 on size stack else stacktop++
  */

  retval = initleafblocktree(
		   &state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE1], 
		   leafnumber, state->p, state->maxsize,
		   &state->memrepos);
  if(retval != 0)
  {
    ERROR0("Creating a new leafblocktree failed.\n");
  }
  PUSHLEAFLISTDEEP(state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE1]);

  if(firstsucc)
  {
    PUSHSIZE(UintConst(1));
  } else
  {
    TOPSIZE += 1;
  }

  return 0;
}

static Sint processbranch(State *state,BOOL firstsucc, Nodeinfo *father,
			  Uint depth, Virtualtree *virtualtree)
{
  Uint           numberofchildren,
                 currentchild;
  Sint           retval;
  Uint i;

  /*
    pop last number from size stack
    pop first tree from tree stack
    while that number is not reached
      pop next tree from tree stack
      report current tree against new tree
      merge current tree with new tree
    if firstsuccessor then put 1 on size stack else stacktop++
  */

  DEBUG3(4,"processbranch: firstsucc=%s, depth=%lu, fatherdepth=%lu\n",
            SHOWBOOL(firstsucc),
            (Showuint) depth,
	    (Showuint) father->depth);

  /* if depth is already too small, do nothing */
  if(depth < state->searchlength)
  {
    return 0;
  } 
 
  numberofchildren = POPSIZE;

  POPLEAFLISTDEEP(state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE1]);

  for(currentchild = UintConst(1); currentchild < numberofchildren; currentchild++)
  {
    POPLEAFLISTDEEP(state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE2]);
    (void) reportmaximalpairs(state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE1],
		       state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE2],
		       depth, state->output, state->outinfo, state->bounds,
		       virtualtree, &state->memrepos);
    retval = mergeleafblocktrees(
			&state->memrepos.leafblocktree[OUTPUTLEAFBLOCKTREE],
			state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE1],
			state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE2],
			virtualtree, &state->memrepos);
    if(retval != 0)
    {
      ERROR0("Merging leafblocktrees failed.\n");
    }
    // the merge result is now saved in leafblocktree[OUTPUTLEAFBLOCKTREE]
    XCHGTEMPLEAFBLOCKTREES(INPUTLEAFBLOCKTREE1, OUTPUTLEAFBLOCKTREE);
  }

  /* if depth is ok, but fatherdepth is not, throw away results */
  if(father->depth < state->searchlength)
  {
    destroyleafblocktree(state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE1]);
    return 0;
  }
	 
  /* else */

  PUSHLEAFLISTDEEP(state->memrepos.leafblocktree[INPUTLEAFBLOCKTREE1]);

  if(firstsucc)
  {
    PUSHSIZE(UintConst(1));
  } else
  {
    TOPSIZE += 1;
  }
  
  return 0;
}

/*----------------------- Include code for traversing tree -------------------*/
 
#define WITHSUFFIX
#define WITHLEFTCHAR
typedef Uint Previoussuffixtype;
#include "vdfstrav.c"

/*-------------------------------- Main function -----------------------------*/
 
/* 
   depth first traversal of the tree.
   push branch node on stack when visited first.
   suppose there is an edge \(father -> son\) such that son is the root of
   a subtree which has been processed. 
   If son is leaf then call processleafedge(father,son). 
   If son is not a leaf, then call processbranch(father,son)
*/

Sint vmatmaxoutwithgaps(Virtualtree *virtualtree,
                        Uint numberofprocessors,
                        Uint searchlength,
                        void *repeatgapspecinfo,
                        void *outinfo,
                        Outputfunction output)
{
  State          state;
  Uint           maxlevel,
                 stacksize;
  Uint           rootchildren;
  Repeatgapspec  *repeatgapspec = (Repeatgapspec *) repeatgapspecinfo;
  Bounds         localbounds;

  /* extract information from Repeatgapspec structure */

  /* Remark: If the bound function pointers are not null,
     the referenced functions are used for the gap bounds.
     If they are null, the constants [lower/upper]gaplength
     are used. If uppergapdefined is False, the upper bound
     is set to the sequence length which corresponds to no
     upper bound on the gap length.
  */

  if(repeatgapspec->lowergapdefined)
  {
    localbounds.lowerboundfun = repeatgapspec->lowerboundfun;   
    localbounds.lowerinfo = repeatgapspec->lowerinfo; 
    localbounds.lower = repeatgapspec->lowergaplength;
  } else
  {
    ERROR0("lowergap is not defined");
    return (Sint) -2;
  }
  if(repeatgapspec->uppergapdefined)
  {
    localbounds.upperboundfun = repeatgapspec->upperboundfun;  
    localbounds.upperinfo = repeatgapspec->upperinfo;    
    localbounds.upper = repeatgapspec->uppergaplength;
  } else
  {
    localbounds.upper = (Sint) virtualtree->multiseq.totallength;
    localbounds.upperboundfun = NULL;  
    localbounds.upperinfo = NULL;
  }

  /* extract skiplist parameters */

  /* Remark: If the skiplistparameters 'p' and 'maxsize' are set to
     zero in 'repeatgapspec', they are set to sensible values internally.
  */

  if(repeatgapspec->skiplistprobability == 0.0)
  {
    state.p = 0.125;
  } else
  {
    state.p = repeatgapspec->skiplistprobability;
  }
  if(repeatgapspec->skiplistmaxsize == 0)
  {
    state.maxsize = UintConst(100);    
  } else
  {
    state.maxsize = repeatgapspec->skiplistmaxsize;
  }

  /* set constants for memory management */
  maxlevel = recommendedmaxlevel(state.p, state.maxsize);
  state.maxlevel = maxlevel;
  stacksize = UintConst(512);

  state.memrepos.initialskiplists          = stacksize * 2;
  state.memrepos.skiplistincrement         = stacksize * 2;
  state.memrepos.initialskipnodes          = 0;
  state.memrepos.skipnodeincrement         = 0;
  state.memrepos.initialskipnodeptrs       = stacksize * 2 * maxlevel;
  state.memrepos.skipnodeptrincrement      = stacksize * 2 * maxlevel;
  state.memrepos.initialskipnodeptrptrs    = stacksize * 2 * maxlevel;
  state.memrepos.skipnodeptrptrincrement   = stacksize * 2 * maxlevel;
  state.memrepos.initialleafblocktrees     = stacksize;
  state.memrepos.leafblocktreeincrement    = stacksize;
  state.sizestackincrement                 = stacksize / 3;

  DEBUG1(2,"searchlength=%lu\n",(Showuint) searchlength);

  if(virtualtree->alpha.mapsize == 0)
  {
    ERROR0("alphabet transformation required");
    return (Sint) -1;
  }
  state.alphabetsize = virtualtree->alpha.mapsize-1;
  state.searchlength = searchlength;
  state.output = output;
  state.outinfo = outinfo;

  state.bounds = &localbounds;
  state.initialized = False;
  state.depth = 0;
  INITARRAY(&state.sizestack,Uint);
  leafblocktreememreposinit(&state.memrepos, maxlevel, 
			    virtualtree->multiseq.totallength);
  
  if(depthfirstvstree(&state,numberofprocessors,virtualtree) != 0)
  {
    return (Sint) -3;
  }

  /* process root */
  if(state.sizestack.nextfreeUint > 0)
  {
    if(state.sizestack.spaceUint == NULL)
    {
      ERROR0("Sizestack space pointer in NULL.\n");
      return (Sint) -1;
    }
    rootchildren = state.sizestack.spaceUint[--state.sizestack.nextfreeUint];
    DEBUG1(4,"Root has %lu children.\n", (Showuint) rootchildren);
    for(;rootchildren > 0; rootchildren--)
    {
      // process rootchildren
    }
  }
  leafblocktreememreposfree(&state.memrepos);

#ifdef WITHBENCHMARK
  printf("vmatbgfind\t%lu\t%lu\t%ld\t%ld\t%.2f\t%lu\t%lu\t",
	 (Showuint) virtualtree->multiseq.totallength, 
	 (Showuint) searchlength, 
	 (Showsint) state.bounds->lower, (Showsint) state.bounds->upper,
	 state.p, (Showuint) state.maxsize, (Showuint) maxlevel);
#endif

#ifdef SHOWMEMREPOS
  DEBUG2(1,"Sizestack          : %lu (%lu)\n",
	 (Showuint) state.sizestack.nextfreeUint,
	 (Showuint) state.sizestack.allocatedUint);
#endif

  FREEARRAY(&state.sizestack,Uint); 

  return 0;
}
