/*EE 
  Datastructure $Leafblocktree$ to be used in algorithm
  for finding maximal pairs with bounded gap as in     
  J.Disc.Alg., Vol.1, 2000, pp.77-104
  The datastructure combines the leaflisttree and
  blockstarttree described in the paper.                  
  Author: Henning Stehr, h.stehr@tuhh.de               
  Date: 28/Oct/2004                                    
*/

#ifndef LEAFBLOCKTREE_H
#define LEAFBLOCKTREE_H

#include "types.h"      // Types Uint, Sint, BOOL, etc.
#include "skiplist.h"   // Type Skiplist, Macros: NEWGENERIC, FREEGENERIC
#include "virtualdef.h" // Type Outputfunction
#include "arraydef.h"   // Macros DECLAREARRAYSTRUCT, etc.
#include "boundedgaps.h"   // Macros DECLAREARRAYSTRUCT, etc.

/*-------------------------------- Debug options -----------------------------*/

#define SHOWMEMREPOS  // show content of memrepos on free
#define ALLOCKEYS     // explicitely alloc/free space for skipkeys (recommended)
//#define CHECKLBTREE // verify leafblocktree before and after every treemerge
//#define WITHCOMPAREFUNCTIONS // use comparefunctions instead of macros (n.r.)

/*------------------------------ Type definitions ----------------------------*/

/*
  the leafblocktree type definition
*/
typedef struct 
{
  Skiplist* leaflisttree;       // the leaflisttree 
  Skiplist* blockstarttree;     // the blockstarttree
} Leafblocktree;                // \Typedef{Leafblocktree}

typedef struct
{
  Boundfunction  lowerboundfun, // user specified lower bound function or NULL
                 upperboundfun; // user specified upper bound function or NULL
  void           *lowerinfo,    // info record for lower bound function
                 *upperinfo;    // info record for upper bound function
  Sint           lower,         // constant lower bound (used if boundfun==NULL)
                 upper;         // constant upper bound (used if boundfun==NULL) 
} Bounds; // \Typedef{Bounds}

/*------------------------ Dynamic memory managment --------------------------*/
 
/*
  declaration of dynamic array structures
*/
DECLAREARRAYSTRUCT(Skiplist);
DECLAREARRAYSTRUCT(Skipnode);
DECLAREARRAYSTRUCT(Skipnodeptr);
DECLAREARRAYSTRUCT(Skipnodeptrptr);
DECLAREARRAYSTRUCT(Leafblocktree);
/*
  Static space for skipfingers used in function 'leafblockmerge'
*/
#define TEMPORARYSKIPFINGERS UintConst(4)

/*
  Static space for temporary forwardpointers used in function 'leafblockmerge'
*/
#define TEMPORARYFORWARDPOINTERS UintConst(4)

/*
  Static space for leafblocktrees used in functions 'processleafedge'
  and 'processbranch' in file 'vmatbgfind.c'
*/
#define TEMPORARYLEAFBLOCKTREES UintConst(3)

#define TEMPORARYSKIPLISTS UintConst(6)

/*
  The three temporary leafblocktrees contain six temporary skiplists
  To access the trees transparently, the following enum constants are
  defined.
*/
typedef enum
{
  INPUTLEAFBLOCKTREE1,
  INPUTLEAFBLOCKTREE2,
  OUTPUTLEAFBLOCKTREE
} Temporaryleafblocktreeenum;

typedef struct
{
  /* dynamic arrays for persistent data */

#ifndef ALLOCKEYS
  Uint                 *keys;
#endif
  ArraySkiplist        skiplistarray;
  ArraySkipnode        skipnodearray;
  ArraySkipnodeptr     skipnodeptrarray;
  ArraySkipnodeptrptr  skipnodeptrptrarray;
  ArrayLeafblocktree   leafblocktreearray;
  Uint                 initialskiplists,
                       skiplistincrement,
                       initialskipnodes,
                       skipnodeincrement,
                       initialskipnodeptrs,
                       skipnodeptrincrement,
                       initialskipnodeptrptrs,
                       skipnodeptrptrincrement,
                       initialleafblocktrees,
                       leafblocktreeincrement;

  /* temporary space used in a single functions call */

  Leafblocktree        *leafblocktree[TEMPORARYLEAFBLOCKTREES],
                       *temp;
  Skiplist             *skiplist[TEMPORARYSKIPLISTS],
                       *tempskiplist;
  Skipnode             **forwardpointers[TEMPORARYFORWARDPOINTERS],
                       **headpointers[TEMPORARYSKIPLISTS],
                       **tempheadpointers,
                       ***tracepointers[TEMPORARYSKIPLISTS],
                       ***temptracepointers;
  Skipfinger           skipfinger[TEMPORARYSKIPFINGERS];
} Memrepos;            // \Typedef{Memrepos}

/*------------------------- memory management macros -------------------------*/

/* generic stack operations */

/*
  PUSHGENERIC/POPGENERIC
  ----------------------
  MSG  : The type as a string for the error message
  PTR  : A pointer to the value to be put on the stack
  A    : A pointer to the stack
  TYPE : The name of the data type to be put on stack
  L    : The number of extra cells to be allocated if stack is full
*/
#define PUSHGENERIC(MSG,PTR,A,TYPE,L)\
        CHECKARRAYSPACE(A,TYPE,L);\
        if((A)->nextfree##TYPE + UintConst(1) > (A)->allocated##TYPE)\
        {\
          ERROR1("pushgeneric: Failed to allocate memory for type %s.\n",\
                 MSG);\
          exit(EXIT_FAILURE);\
        }\
        (A)->space##TYPE[(A)->nextfree##TYPE] = *(PTR);\
        (A)->nextfree##TYPE += UintConst(1)

#define POPGENERIC(PTR,A,TYPE,L)\
        if((A)->nextfree##TYPE < UintConst(1))\
        {\
          ERROR0("Trying to POP from empty stack.\n");\
          exit(EXIT_FAILURE);\
        }\
        (A)->nextfree##TYPE -= UintConst(1);\
        *(PTR) = (A)->space##TYPE[(A)->nextfree##TYPE]

/* elementary stack operations */

#define PUSHLEAFBLOCKTREE(P)\
        PUSHGENERIC("Leafblocktree", P, &state->memrepos.leafblocktreearray,\
                     Leafblocktree, state->memrepos.leafblocktreeincrement)
#define PUSHSKIPLIST(P)\
        PUSHGENERIC("Skiplist", P, &state->memrepos.skiplistarray,\
                     Skiplist, state->memrepos.skiplistincrement)
#define PUSHSKIPNODE(P)\
        PUSHGENERIC("Skipnode", P, &state->memrepos.skipnodearray,\
                     Skipnode, state->memrepos.skipnodeincrement)
#define PUSHSKIPNODEPTR(P)\
        PUSHGENERIC("Skipnodeptr", P, &state->memrepos.skipnodeptrarray,\
                     Skipnodeptr, state->memrepos.skipnodeptrincrement)
#define PUSHSKIPNODEPTRPTR(P)\
        PUSHGENERIC("Skipnodeptrptr", P, &state->memrepos.skipnodeptrptrarray,\
                     Skipnodeptrptr, state->memrepos.skipnodeptrptrincrement)

#define POPLEAFBLOCKTREE(P)\
        POPGENERIC(P, &state->memrepos.leafblocktreearray, Leafblocktree, 1)
#define POPSKIPLIST(P)\
        POPGENERIC(P, &state->memrepos.skiplistarray, Skiplist, 1)
#define POPSKIPNODE(P)\
        POPGENERIC(P, &state->memrepos.skipnodearray, Skipnode, 1)
#define POPSKIPNODEPTR(P)\
        POPGENERIC(P, &state->memrepos.skipnodeptrarray, Skipnodeptr, 1)
#define POPSKIPNODEPTRPTR(P)\
        POPGENERIC(P, &state->memrepos.skipnodeptrptrarray, Skipnodeptrptr, 1)

/* structured stack operations */

/*
  Note for the structured stack operations
  ----------------------------------------
  PUSHLEAFBLOCKTREEDEEP does not put on the stack the components of the
  leafblocktree, pointing to the skiplists.
  They have to be set to sensible values (i.e. allocated blocks
  of memory for holding a skiplist) in advance. 
  POPSKIPLIST recreates the original (i.e. those stored in LIST
  before the pop operation) pointers to the array
  'headpointers' and 'tracepointers'. Otherwise they would still
  point to the position in memory where the old tree, which was
  put on the stack, was storing the headpointers and tracepointers.
*/

#define PUSHHEADPOINTERS(PTR, MAXLEVEL)\
        for(i = UintConst(0); i < MAXLEVEL; i++)\
        {\
          PUSHSKIPNODEPTR(&(PTR)[i]);\
        }
#define PUSHTRACEPOINTERS(PTR, MAXLEVEL)\
        for(i = UintConst(0); i < MAXLEVEL; i++)\
        {\
          PUSHSKIPNODEPTRPTR(&(PTR)[i]);\
        }
#define POPHEADPOINTERS(PTR, MAXLEVEL)\
        for(i = UintConst(1); i <= MAXLEVEL; i++)\
        {\
          POPSKIPNODEPTR(&(PTR)[(MAXLEVEL) - i]);\
        }
#define POPTRACEPOINTERS(PTR, MAXLEVEL)\
        for(i = UintConst(1); i <= MAXLEVEL; i++)\
        {\
          POPSKIPNODEPTRPTR(&(PTR)[(MAXLEVEL) - i]);\
        }
#define PUSHSKIPLISTDEEP(LIST, MAXLEVEL)\
        PUSHSKIPLIST(LIST);\
        PUSHHEADPOINTERS(LIST->headpointers, MAXLEVEL);\
        PUSHTRACEPOINTERS(LIST->tracepointers, MAXLEVEL)
#define POPSKIPLISTDEEP(LIST, MAXLEVEL)\
        POPTRACEPOINTERS(LIST->tracepointers, MAXLEVEL);\
        POPHEADPOINTERS(LIST->headpointers, MAXLEVEL);\
        state->memrepos.tempheadpointers = LIST->headpointers;\
        state->memrepos.temptracepointers = LIST->tracepointers;\
        POPSKIPLIST(LIST);\
        LIST->headpointers = state->memrepos.tempheadpointers;\
        LIST->tracepointers = state->memrepos.temptracepointers
#define PUSHLEAFBLOCKTREEDEEP(TREE, MAXLEVEL)\
        PUSHSKIPLISTDEEP(TREE->leaflisttree, MAXLEVEL);\
        PUSHSKIPLISTDEEP(TREE->blockstarttree, MAXLEVEL)
#define POPLEAFBLOCKTREEDEEP(TREE, MAXLEVEL)\
        POPSKIPLISTDEEP(TREE->blockstarttree, MAXLEVEL);\
        POPSKIPLISTDEEP(TREE->leaflisttree, MAXLEVEL)

/* auxiliary macros */

#define XCHGTEMPLEAFBLOCKTREES(A,B)\
        state->memrepos.temp = state->memrepos.leafblocktree[A];\
        state->memrepos.leafblocktree[A] = state->memrepos.leafblocktree[B];\
        state->memrepos.leafblocktree[B] = state->memrepos.temp

/*--------------------------- public functions -------------------------------*/

//\IgnoreLatex{

#ifdef __cplusplus
extern "C" {
#endif
Sint initleafblocktree(Leafblocktree **desttree, Uint keyindex, double p,
		       Uint maxsize, Memrepos* memrepos);

Sint mergeleafblocktrees(Leafblocktree **desttree,
			 Leafblocktree *lbt1,
			 Leafblocktree *lbt2,
			 Virtualtree *virtualtree,
			 Memrepos* memrepos);

Sint reportmaximalpairs(Leafblocktree *lbt1, Leafblocktree *lbt2,
			Uint length,
			Outputfunction output, void *outinfo,
			Bounds *bounds,
			Virtualtree *virtualtree,
			Memrepos* memrepos);

BOOL checkleafblocktree(Leafblocktree *lbtree, Virtualtree *virtualtree);

void leafblocktreememreposinit(Memrepos *memrepos, Uint maxlevel, Uint seqsize);

void leafblocktreememreposfree(Memrepos *memrepos);

void showSortedList(Leafblocktree *lbtree);

void destroyleafblocktree(Leafblocktree *lbtree);

#ifdef __cplusplus
}
#endif

#endif

//}
