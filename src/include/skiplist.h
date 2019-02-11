/*EE 
  Skiplist datastructure implementation by Henning Stehr (h.stehr@tuhh.de)
  The Skiplist is a probabilistic alternative to balanced search trees.
  For basic definition of skiplists see 'A Skip List Cookbook' by William Pugh.
  Code for insert, delete and find based on implementation by Shane Saunders
  Author: Henning Stehr, h.stehr@tuhh.de              
  Date: 29/Oct/2004                                    
*/

#ifndef SKIPLIST_H
#define SKIPLIST_H

#include "types.h"              // Types Uint, Sint, BOOL, etc.

/*------------------------- types --------------------------------------------*/
 
typedef void* Skipkey;

typedef Sint (*Skipcomparefunction)(const Skipkey,const Skipkey,void *);
typedef void (*Skipshowelem)(const Skipkey,void *);
typedef void (*Skipfreekey)(const Skipkey,void *);
typedef Sint (*Skipaction)(const Skipkey, Uint, void *);

typedef struct SkipnodeS
{
  struct SkipnodeS **forward;   // array of forward pointers
  Skipkey key;                  // pointer to comparable key type
  Uint nodelevel;               // number of forward pointers
} Skipnode;                     // \Typedef{Skipnode}

typedef Skipnode*  Skipnodeptr;
typedef Skipnode** Skipnodeptrptr;
typedef Skipnode*** Skipfinger; // search finger into skiplist;

typedef struct
{
  Uint currentlistsize;         // current number of elements in list
  Uint maxlistsize;             // maximum number of list elements
  Uint maxnodelevel;            // maximum level of node
  double p;                     // probability of node of size t being size t+1
  Skipnode *currentelement;     // pointer to current element for list traversal
  Skipnode **headpointers;      // array of head pointers
  Skipnode ***tracepointers;    // search finger 
} Skiplist;                     // \Typedef{Skiplist}

/*-------------------------- macros ------------------------------------------*/
/* Specify specialized instances of 'skiplistexternalfingernextequalkey' for  */
/* different kinds of skipkeys                                                */

/* for pointers to integers */
#define COMPAREINTPTRS(K1,K2)\
        (*(Uint*)(K1) < *(Uint*)(K2) ? (Sint) -1 :\
        *(Uint*)(K1) == *(Uint*)(K2) ? 0 : (Sint) 1)

/* for pointers to skipnodes */
#define COMPARESKIPNODEPTRS(K1,K2)\
        (*(Uint*)(((Skipnode*)(K1))->key) < *(Uint*)(((Skipnode*)(K2))->key)\
        ? (Sint) -1 :\
        *(Uint*)(((Skipnode*)(K1))->key) == *(Uint*)(((Skipnode*)(K2))->key)\
        ? 0 : (Sint) 1) 

/* assign integer compare to COMPAREKEYS (used in skiplist.c) */
#define COMPAREKEYS(K1,K2)\
        COMPAREINTPTRS(K1,K2)

/* assign skipnode pointer compare to COMPAREKEYS2 (used in skiplist.c) */
#define COMPAREKEYS2(K1,K2)\
        COMPARESKIPNODEPTRS(K1,K2)

/* Instead of the function of the same name, this macro can be used to advance
   the current element pointer of the skiplist 'S'. The macro is optimized for
   speed and does not check for null pointers. 
*/
#define SKIPLISTCURRENTNEXT(S)\
  (((S)->currentelement = (Skipnode*) (S)->currentelement->forward[0])\
  != NULL ? (S)->currentelement->key : NULL)
 
/*------------------------- functions ----------------------------------------*/
 
//\IgnoreLatex{

#ifdef __cplusplus
extern "C" {
#endif
Sint skiplistinit(Skiplist **newskiplist, Uint maxsize,
		           double p, BOOL doalloc);

Sint skiplistinitwithkey(Skiplist **newskiplist, Uint maxsize,
			   double p, BOOL doalloc, Skipkey key);

Sint skiplistmerge(Skiplist **destlist, Skiplist* list1, Skiplist* list2,
		           Skipcomparefunction cmpfun, void* cmpinfo, 
		           BOOL doalloc);

/*@null@*/ Skipkey skiplistinsert(Skiplist *skiplist, Skipkey key,
                           Skipcomparefunction cmpfun, void* cmpinfo);

/*@null@*/ Skipkey skiplistdelete(Skiplist *skiplist, Skipkey key,
                           Skipcomparefunction cmpfun, void* cmpinfo);

/*@null@*/ Skipkey skiplistfind(Skiplist *skiplist, Skipkey key,
                           Skipcomparefunction cmpfun, void* cmpinfo);

/*@null@*/ Skipkey skiplistminimumkey(Skiplist* skiplist);

/*@null@*/ Skipkey skiplistnextkey(Skiplist* skiplist, Skipkey key,
                           Skipcomparefunction cmpfun, void* cmpinfo);

/*@null@*/ Skipkey skiplistnextequalkey(Skiplist* skiplist, Skipkey key,
                           Skipcomparefunction cmpfun, void* cmpinfo);

/*@null@*/ Skipkey skiplistcurrentnext(Skiplist* skiplist);

/*@null@*/ Skipkey skiplistfingernextkey(Skiplist* skiplist, 
			   Skipkey key, 
			   Skipcomparefunction cmpfun, 
			   void* cmpinfo);

/*@null@*/ Skipkey skiplistfingernextequalkey(Skiplist* skiplist,
			   Skipkey key, 
			   Skipcomparefunction cmpfun, 
			   void* cmpinfo);

/*@null@*/ Skipkey skiplistexternalfingernextkey(Skiplist* skiplist, 
			   Skipfinger finger,
			   Skipkey key, 
			   Skipcomparefunction cmpfun, 
			   void* cmpinfo);

/*@null@*/ Skipkey skiplistexternalfingernextequalkey(Skiplist* skiplist,
			   Skipfinger finger, 
			   Skipkey key, 
			   Skipcomparefunction cmpfun, 
			   void* cmpinfo);

/*@null@*/ Skipkey skiplistexternalfingernextequalkeywithmacro(
			   Skiplist* skiplist,
                           Skipfinger finger,
                           Skipkey key,
                           Skipcomparefunction cmpfun,
                           void* cmpinfo);

/*@null@*/ Skipkey skiplistexternalfingernextequalkeywithmacro2(
			   Skiplist* skiplist,
                           Skipfinger finger,
                           Skipkey key,
                           Skipcomparefunction cmpfun,
                           void* cmpinfo);

/*@null@*/ Skipfinger skiplistinitexternalfinger(Skiplist *skiplist);

Sint skiplistinitfinger(Skiplist *skiplist);

Uint recommendedmaxlevel(double p, Uint maxsize);

void skiplistfreeexternalfinger(Skipfinger finger);

void skiplistwalk(Skiplist *skiplist, Skipaction action, 
                           void* actinfo);

void skiplistprint(Skiplist* skiplist, Skipshowelem showfun, 
                           void* showinfo);

void skiplistprintvalues(Skiplist* skiplist,
                           Skipshowelem showfun, 
                           void* showinfo);

void skipliststatistics(Skiplist* skiplist);

void skiplistdestroy(Skiplist *skiplist, BOOL dofreeskiplist, 
                           BOOL dofreekeys);

#ifdef __cplusplus
}
#endif

#endif

//}
