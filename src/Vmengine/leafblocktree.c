/*EE 
  Datastructure $Leafblocktree$ to be used in algorithm
  for finding maximal pairs with bounded gap as in     
  J.Disc.Alg., Vol.1, 2000, pp.77-104
  The datastructure combines the $leaflisttree$ and
  $blockstarttree$ described in the paper.                  
  Author: Henning Stehr, h.stehr@tuhh.de               
  Date: 28/Oct/2004                               

  This implementation uses skiplists for maintaining the
  leaflisttree and blockstarttree.
*/

#include <stdio.h>
#include <stdlib.h> 
#include "types.h"          // Types Uint, Sint, BOOL, etc.
#include "debugdef.h"       // Macros DEBUG0, DEBUG1, etc.
#include "errordef.h"       // Macros ERROR0, ERROR1, etc.
#include "spacedef.h"       // Macros ALLOCSPACE, FREESPACE
#include "chardef.h"        // Macro SEPARATOR
#include "assert.h"
#include "drand48.h"
/* 
  libraries are (C) by Stefan Kurtz, kurtz@zbh.uni-hamburg.de
*/
#include "skiplist.h"       // Datastructure: Skiplist,
                            // Macros: COMPAREINTPTRS, COMPARESKIPNODEPTRS
#include "leafblocktree.h"  // Datastructure: Leafblocktree,
                            // Flags: ALLOCKEYS, CHECKLBTREE, SHOWMEMREPOS

/*------------------ auxiliary functions for implementation ------------------*/
#ifdef WITHCOMPAREFUNCTIONS
#define LBTCOMPAREKEYS(K1,K2)\
        cmpfun(K1, K2, cmpinfo)
#define LBTCOMPAREBLOCKTREEKEYS(K1,K2)\
        blocktreecmpfun(K1,K2, cmpinfo2)
#else
#define LBTCOMPAREKEYS(K1,K2)\
        COMPAREINTPTRS(K1,K2)
#define LBTCOMPAREBLOCKTREEKEYS(K1,K2)\
        COMPARESKIPNODEPTRS(K1,K2)
#endif

/* 
  define Skipcomparefunction 
*/
static Sint comparekeys(const Skipkey k1, const Skipkey k2, /*@unused@*/ void *cmpinfo)
{
  Uint key1, key2;
  key1 = *(Uint*) k1;
  key2 = *(Uint*) k2;

#ifndef WITHCOMPAREFUNCTIONS
  printf("unexpected call of 'comparekeys'.\n");
#endif

  if(key1 < key2) return (Sint) -1; else
  if(key1 > key2) return (Sint) 1; else
  return 0;
}

/* 
  define Skipcomparefunction for blockstarttree
*/
static Sint compareblocktreekeys(const Skipkey k1, const Skipkey k2, /*@unused@*/ void *cmpinfo)
{
  Uint key1, key2;
  Skipnode *node1, *node2;

  node1 = (Skipnode*) k1;
  node2 = (Skipnode*) k2;
  key1 = *(Uint*) node1->key;
  key2 = *(Uint*) node2->key;

#ifndef WITHCOMPAREFUNCTIONS
  printf("unexpected call of 'compareblocktreekeys'.\n");
#endif

  if(key1 < key2) return (Sint) -1; else
  if(key1 > key2) return (Sint) 1; else
  return 0;
}

/* 
  define Skipshowelem 
*/
static void showkey(const Skipkey k, /*@unused@*/ void *showinfo)
{
  Uint key;
  key = *(Uint*) k;
  printf("%lu ", (Showuint) key);
}

/* 
  define Skipshowelem for blockstarttree
*/
static void showblocktreekey(const Skipkey k, /*@unused@*/ void *showinfo)
{
  Uint key;
  Skipnode *node;

  node = (Skipnode*) k;
  key = *(Uint*) (node->key);

  printf("%lu ", (Showuint) key);
}
  
/* 
  traverse the list starting from currentelement until key > to
  and report maximal pairs (p, key, length)
*/
static Sint reportfromtowithblocks(Skiplist *skiplist, 
				   Skiplist *blocklist,
				   Uint pos, Uint to, Uint length,
				   Outputfunction output, 
				   void* outinfo,
				   Virtualtree *virtualtree)
{
  Skipkey       current;
  Uint          currentint,
                currentblockint,
                firstchar,
                secondchar;
  Skipnode      *currentblockkey;

  /*
    when this function is called, currentelement is pointing to  
    the first element to be reported. 
  */

  if(skiplist->currentelement == NULL)
  {
    return (Sint) -1;
  }

  current = skiplist->currentelement->key;
  while(current != NULL)
  {
    currentint = *(Uint*) current;
    if(currentint > to) return (Sint) 1; // interval end
    if(pos == 0)
    {
      firstchar = SEPARATOR;
    } else
    {
      firstchar = virtualtree->multiseq.sequence[pos-1];
    }
    if(currentint == 0)
    {
      secondchar = SEPARATOR;
    } else
    {
      secondchar = virtualtree->multiseq.sequence[currentint-1];
    }
    if(secondchar != firstchar)
    {
      if(pos < currentint) (void) output(outinfo, length, pos, currentint);
                      else (void) output(outinfo, length, currentint, pos);
      current = SKIPLISTCURRENTNEXT(skiplist);
    } else
    {
      // skip to next block
      if(blocklist->currentelement == NULL) return (Sint) 2; // no more block
      currentblockkey = (Skipnode*) blocklist->currentelement->key;
      currentblockint = *(Uint*) currentblockkey->key;
      while(currentblockint <= currentint)
      {
	currentblockkey = (Skipnode*) SKIPLISTCURRENTNEXT(blocklist);
	if(currentblockkey == NULL) return (Sint) 3; // no more block
	currentblockint = *(Uint*) currentblockkey->key;
      }
      skiplist->currentelement = currentblockkey;
      current = currentblockkey->key;
    }
  }
  return 0; // list end
}

/*
  Randomly generate and return a level for a new skiplist node based on the
  probability 'p' capped by the maximal node level 'maxlevel'.
*/
static Uint skiplistrandlevel(double p, Uint maxlevel)
{
    Uint i; 

    for(i = (Uint) 1; i < maxlevel; i++) {
        if(drand48() > p) break;
    }

    return i;
}

/*EE
  Leafblocktree merge operation. Merges the leaflisttrees and the
  blockstarttrees from the Leafblocktree structure. The leaflisttrees
  are simply merged as described by Pugh. The merging of the block-
  starttrees is more involved. They have to be modified while
  merging to keep the blockstarttree property. The implementation
  is a simplified version of the procedure described in
  J.Disc.Alg., Vol.1, 2000, pp.77-104 adapted to skiplists.

  This function directly manipulates the underlying skiplists.
  For the correct functioning it is assumed that keys are unique,
  as it is the case for the problem of finding bounded repeats.
*/

static Sint leafblockmerge(Skiplist** newlistptr, Skiplist** newblocklistptr,
                        Skiplist* list1, Skiplist* list2,
                        Skiplist* blocklist1, Skiplist* blocklist2, 
                        /*@unused@*/ Skipcomparefunction cmpfun, 
                        /*@unused@*/ void* cmpinfo,
                        /*@unused@*/ Skipcomparefunction blocktreekeycmpfun, 
			/*@unused@*/ void* cmpinfo2,
                        Virtualtree *virtualtree,
			Memrepos *memrepos)
{
  /* varibales for merging leaflists */
  Uint         s1, s2,            // current size of list1, list2
               l1, l2,            // maxnodelevel of list1, list2
               m1, m2,            // maxsize of list1, list2
               newlistsize,       // size of merged list
               newmaxlistsize;    // maxsize of merged list
  Sint         i, lvl,            // loop indeces
               maxlevel;          // maxnodelevel of merged list
  Skiplist     *newlist,          // merged leaflist 
               *templst,          // temp variable for exchanging lists
               *leftover;         // pointer to last remaining list
  double       newp;              // level increase probability of new list
  BOOL         unflipped = True;  // list1 and list2 have been switched?
  Skipkey      key1, key2,        // first keys in remaining list  
               tempkey;           // temp variable for exchanging keys
  Skipnode     *x, *y,            // temporary node pointers
               header,            // temporary node for storing headpointers
               **update;          // local array of Skipnode pointers

  /* variables for merging blockstartlists */
  Uint         bs1, bs2,          // current size of blocklist1, blocklist2
               bl1, bl2,          // maxnodelevel of blocklist1, blocklist2
               bm1, bm2,          // maxsize of blocklist1, blocklist2
               bnewlistsize,      // size of merged blocklist
               bnewmaxlistsize,   // maxsize of merged blocklist
               currentpos,        // current position in sequence
               nextpos,           // next position in sequence  
               currentchar,       // character at current position
               nextchar;          // character at next position
  Sint         retval,            // return value of skiplistinit function
               bmaxlevel;         // maxnodelevel of merged list
  double       bnewp;             // level increase probability of new list
  Skiplist     *newblocklist,     // merged blockstartlist
               *bleftover;        // pointer to last remaining list
  Skipnode     bheader,           // temporary node for storing headpointers
               **bupdate,         // local array of Skipnode pointers
               *newblocktreekey;  // temp variable for newly generated keys
  BOOL         isinblocklist;     // next key is in block list? 


  /* check input parameters for leaflists */
  if(list1 == NULL)
  {
    ERROR0("Could not merge lists, handle for list1 is NULL.\n");
    return (Sint) -1;
  }
  if(list2 == NULL)
  {
    ERROR0("Could not merge lists, handle for list2 is NULL.\n");
    return (Sint) -2;
  }
/*
  if(cmpfun == NULL)
  {
    ERROR0("Could not merge lists, handle to comparefunction is NULL.\n");
    return (Sint) -3;
  }
*/
  if(list1->headpointers == NULL)
  {
    ERROR0("Could not merge lists, headpointers for list1 is NULL.\n");
    return (Sint) -4;
  }
  if(list2->headpointers == NULL)
  {
    ERROR0("Could not merge lists, headpointers for list2 is NULL.\n");
    return (Sint) -5;
  }

  /* check input parameters for blockstartlists */
  if(blocklist1 == NULL)
  {
    ERROR0("Could not merge lists, handle for blocklist1 is NULL.\n");
    return (Sint) -6;
  }
  if(blocklist2 == NULL)
  {
    ERROR0("Could not merge lists, handle for blocklist2 is NULL.\n");
    return (Sint) -7;
  }
/*
  if(blocktreekeycmpfun == NULL)
  {
    ERROR0("Could not merge lists, handle to blockcomparefunction is NULL.\n");
    return (Sint) -8;
  }
*/
  if(blocklist1->headpointers == NULL)
  {
    ERROR0("Could not merge lists, headpointers for blocklist1 is NULL.\n");
    return (Sint) -9;
  }
  if(blocklist2->headpointers == NULL)
  {
    ERROR0("Could not merge lists, headpointers for blocklist2 is NULL.\n");
    return (Sint) -10;
  }

  /* init new list */
  s1 = list1->currentlistsize;
  s2 = list2->currentlistsize;
  l1 = list1->maxnodelevel;
  l2 = list2->maxnodelevel;
  m1 = list1->maxlistsize;
  m2 = list2->maxlistsize;
  if(l1 >= l2) maxlevel = (Sint) l1; else maxlevel = (Sint) l2; //=max(l1,l2)
  /*# possible error: if l1 != l2 then following loops might fail */
  /*# workaround: assert that l1 == l2 */
  assert(l1 == l2);
  if(m1 >= m2) newmaxlistsize = m1; else newmaxlistsize = m2;  //=max(m1,m2)
  newlistsize = s1 + s2;
  newp = list2->p; // take p from second list

  /* init new blocklist */
  bs1 = blocklist1->currentlistsize;
  bs2 = blocklist2->currentlistsize;
  bl1 = blocklist1->maxnodelevel;
  bl2 = blocklist2->maxnodelevel;
  bm1 = blocklist1->maxlistsize;
  bm2 = blocklist2->maxlistsize;
  if(bl1 >= bl2) bmaxlevel = (Sint) bl1; else bmaxlevel = (Sint) bl2;
  assert(bl1 == bl2);
  if(bm1 >= bm2) bnewmaxlistsize = bm1; else bnewmaxlistsize = bm2;
  bnewlistsize = bs1 + bs2;
  bnewp = blocklist2->p; // take p from second list

  /* allocate space for new list */
  retval = skiplistinit(newlistptr, newmaxlistsize, newp, False);
  if(retval != 0)
  {
    ERROR0("Unable to initialize new leaflist for merge result\n");
    return (Sint) -12;
  }
  newlist = *newlistptr;

  if(newlist->headpointers == NULL)
  {
    ERROR0("Unable to initialize headpointers for new leaflist.\n");
    return (Sint) -13;
  }
  newlist->currentlistsize = newlistsize;

  /* allocate space for new blocklist */
  retval = skiplistinit(newblocklistptr, bnewmaxlistsize, bnewp, False);
  if(retval != 0)
  {
    ERROR0("Unable to initialize new blockstartlist for merge result\n");
    return (Sint) -14;
  }
  newblocklist = *newblocklistptr;

  if(newblocklist->headpointers == NULL)
  {
    ERROR0("Unable to initialize headpointers for new blockstartlist.\n");
    return (Sint) -15;
  }
  newblocklist->currentlistsize = bnewlistsize;
  /*# update newblocklist->currentlistsize later */

  /* merge lists */

  /* Create temporary node: This node will temporarily store the headpointer
     information of the new list. By using a Skipnode type variable the
     headpointers of the list can be accessed with the same interface as a
     simple list node.
  */
  header.forward = memrepos->forwardpointers[0];
  update = memrepos->forwardpointers[1];
  
  for(i = 0; i < maxlevel; i++)
  {
    update[i] = &header;
  }

  /* same for blocklists */
  bheader.forward = memrepos->forwardpointers[2];
  bupdate = memrepos->forwardpointers[3];
  
  for(i = 0; i < maxlevel; i++)
  {
    bupdate[i] = &bheader;
  }

  /* main while loop (merge leafslists AND blocklists) */
  while((list1->headpointers[0] != NULL) && (list2->headpointers[0] != NULL))
  {
    key1 = list1->headpointers[0]->key;
    key2 = list2->headpointers[0]->key;

    /* assume w.l.g. that key1 <= key2, otherwise 
       xchg(key1, key2), xchg(list1, list2), xchg(blocklist1, blocklist2)
    */
    if(LBTCOMPAREKEYS(key1, key2) > 0)
    {
      tempkey = key1; key1 = key2; key2 = tempkey;
      templst = list1; list1 = list2; list2 = templst;
      templst = blocklist1; blocklist1 = blocklist2; blocklist2 = templst; 
      unflipped = (BOOL) !unflipped;
    } // now key1 <= key2

    /* merge step: remove from list1 elements with keys <= key2 and put them
       in output list 
    */

    /* for all lvl such that list1->headpointers[lvl]->key <= key2,
       connect output list to list1 
    */
    lvl = 0;
    do
    {
      update[lvl]->forward[lvl] = list1->headpointers[lvl];
      lvl++;
    } while((lvl < maxlevel) && 
            (list1->headpointers[lvl] != NULL) && 
	    (LBTCOMPAREKEYS(list1->headpointers[lvl]->key, key2) <= 0));
    lvl--;
    
    /* for each level attached to output list, find endpoint at that level
       i.e. the last element with key <= key2
    */
    x = list1->headpointers[lvl];
    for(i = lvl; i >= 0; i--)
    {
      while((x->forward[i] != NULL) && 
            (LBTCOMPAREKEYS(((Skipnode*)(x->forward[i]))->key, key2) <= 0)) 
        x = (Skipnode*) x->forward[i];
        /* now x->key <= key2 < x->forward[i]->key */

      update[i] = x;
      list1->headpointers[i] = (Skipnode*) x->forward[i];
    }
    /* now x = last element moved to output list */

    /* if the element at the front of list2 is a duplicate of an element
       already moved to output list, eliminate it
    */
    if(LBTCOMPAREKEYS(key2, x->key) == 0)
    {
      if(unflipped) x->key = list2->headpointers[0]->key; // #danger: space leak
      y = list2->headpointers[0];
      for(i = 0; (Uint) i < y->nodelevel; i++) 
        list2->headpointers[i] = (Skipnode*) y->forward[i];
      FREESPACE(y->forward);
      FREESPACE(y);
      newlist->currentlistsize--; // don't count duplicates
    }
 
    /* now for blocklists */
    lvl = 0;
    while((lvl < bmaxlevel) && 
          (blocklist1->headpointers[lvl] != NULL) && 
(LBTCOMPAREKEYS(((Skipnode*) blocklist1->headpointers[lvl]->key)->key, key2) 
  < 0)) 
    {
      bupdate[lvl]->forward[lvl] = blocklist1->headpointers[lvl];
      lvl++;
    }
    lvl--; // warning: lvl might become negative
    
    /* for each level attached to output list, find endpoint at that level
       i.e. the last element with key <= key2
    */
    if(lvl >= 0) x = blocklist1->headpointers[lvl];
    for(i = lvl; i >= 0; i--)
    {
      while((x->forward[i] != NULL) && 
(LBTCOMPAREKEYS(((Skipnode*)((Skipnode*)(x->forward[i]))->key)->key, key2) < 0)) 
        x = (Skipnode*) x->forward[i];
        /* now x->key < key2 <= x->forward[i]->key */

      bupdate[i] = x;
      blocklist1->headpointers[i] = (Skipnode*) x->forward[i];
    }

    assert(update[0] != NULL);
    currentpos = *(Uint*) update[0]->key;
    nextpos = *(Uint*) key2;
    if(virtualtree != NULL && currentpos > 0)
    {
    currentchar = virtualtree->multiseq.sequence[currentpos-1];
    nextchar = virtualtree->multiseq.sequence[nextpos-1];
    isinblocklist = (blocklist2->headpointers[0] != NULL &&
    LBTCOMPAREKEYS(((Skipnode*)blocklist2->headpointers[0]->key)->key, key2) == 0) 
                     ? True : False;
    DEBUG5(2,"Comparing %lu and %lu at %lu and %lu, nextin=%s\n", 
	   (Showuint) currentchar,
	   (Showuint) nextchar, 
	   (Showuint) currentpos, 
	   (Showuint) nextpos,
	   SHOWBOOL(isinblocklist));

    /* delete next node */
    if(isinblocklist == True && currentchar == nextchar)
    {
      DEBUG1(2,"Deleting blockstartelement with key %lu\n", *(Showuint*) key2);
      y = blocklist2->headpointers[0]; // node to be deleted
      for(i = 0; (Uint) i < y->nodelevel; i++) 
        blocklist2->headpointers[i] = (Skipnode*) y->forward[i];
      FREESPACE(y->forward);
      FREESPACE(y);
      newblocklist->currentlistsize--;
    } else
    {
    /* insert new next node */
    if(isinblocklist == False && currentchar != nextchar)
    {
      DEBUG1(2,"Inserting new element with key %lu\n", *(Showuint*) key2);
      ALLOCASSIGNSPACE(y, NULL, Skipnode, (Uint) 1); // new node
      lvl = (Sint) skiplistrandlevel(bnewp, (Uint) bmaxlevel);
      y->nodelevel = (Uint) lvl;
      ALLOCASSIGNSPACE(y->forward, NULL, Skipnode*, (Uint) lvl);
      // insert key with value key2 and pointer to list2->headpointers[0];
      newblocktreekey = list2->headpointers[0];
      y->key = (Skipkey) newblocktreekey;
      for(i = 0; i < lvl; i++)
      {
        y->forward[i] = bupdate[i]->forward[i];
        bupdate[i]->forward[i] = y;
        bupdate[i] = y;
      }
      newblocklist->currentlistsize++;
    } /* if(isinblocklist...*/
    } /* if(isinblocklist...else */
    } /* if virtualtree != NULL && currentpos > 0 */
  
  } /* end of main while loop */

  if(list2->headpointers[0] == NULL)
  {
    leftover = list1;
    bleftover = blocklist1;
  } else 
  {
    leftover = list2;
    bleftover = blocklist2;
  }
  for(i = 0; (Uint) i < leftover->maxnodelevel; i++) 
    update[i]->forward[i] = leftover->headpointers[i];
  for(i = (Sint) leftover->maxnodelevel; i < maxlevel; i++) 
    update[i]->forward[i] = NULL;

  /* same for blocklists */
  for(i = 0; (Uint) i < bleftover->maxnodelevel; i++) 
    bupdate[i]->forward[i] = bleftover->headpointers[i];
  for(i = (Sint) bleftover->maxnodelevel; i < bmaxlevel; i++) 
    bupdate[i]->forward[i] = NULL;

  /* copy header content to headpointers of new list */
  for(i = 0; i < maxlevel; i++) {
    newlist->headpointers[i] = header.forward[i];
    newlist->tracepointers[i] = &(newlist->headpointers[i]);
  }

  /* same for blocklists */
  for(i = 0; i < bmaxlevel; i++) {
    newblocklist->headpointers[i] = bheader.forward[i];
    newblocklist->tracepointers[i] = &(newblocklist->headpointers[i]);
  }

  /* write header data to new list */
  newlist->currentlistsize = newlistsize;
  newlist->maxlistsize = newmaxlistsize;
  newlist->maxnodelevel = (Uint) maxlevel;
  newlist->p = newp;
  newlist->currentelement = newlist->headpointers[0];

  /* write header data to new blocklist */
  newblocklist->currentlistsize = bnewlistsize;
  newblocklist->maxlistsize = bnewmaxlistsize;
  newblocklist->maxnodelevel = (Uint) bmaxlevel;
  newblocklist->p = bnewp;
  newblocklist->currentelement = newblocklist->headpointers[0];

  return 0;
}

/*--------------------------- public functions -------------------------------*/

/*EE
  create a new leafblocktree with a single leaf element
*/
Sint initleafblocktree(Leafblocktree **desttree, Uint keyindex, double p,
		       Uint maxsize, /*@unused@*/ Memrepos* memrepos)
{
  Skipkey newkey;
  Skipnode *newblocktreekey;
  Skiplist *newlist, *newblockstartlist;
  Sint retval;
  Leafblocktree *newtree;
  Uint *tempkey;

  if(desttree == NULL)
  {
    ERROR0("Space for leafblocktree is not allocated.\n");
    return (Sint) -2;
  }
  newtree = *desttree;

  /* generate a new key for the current index */
#ifdef ALLOCKEYS
  ALLOCASSIGNSPACE(tempkey, NULL, Uint, (Uint) 1);
  newkey = (Skipkey) tempkey;
  *(Uint*) newkey = keyindex;
#else
  newkey = (Skipkey) &memrepos->keys[keyindex];
#endif

  /* init leaflist */
  newlist = newtree->leaflisttree;
  retval = skiplistinitwithkey(&newlist, maxsize, p, False, newkey);
  if(retval != 0)
  {
    ERROR0("Cannot initialize new skiplist.\n");
    return (Sint) -4;
  }
  
  /* new block key: set pointer to first (and only) element in leaflist */
  newblocktreekey = newlist->headpointers[0];

  /* init blockstartlist */
  newblockstartlist = newtree->blockstarttree;
  retval = skiplistinitwithkey(&newblockstartlist, maxsize, p, False,
			       (Skipkey) newblocktreekey);
  if(retval != 0)
  {
    ERROR0("Cannot initialize new skiplist.\n");
    return (Sint) -6;
  }

  return 0;
}

/*EE
  Leafblocktree merge operation. Merges the leaflisttrees and the
  blockstarttrees from the Leafblocktree structure. The leaflisttrees
  are simply merged as described by Pugh. The merging of the block-
  starttrees is more involved. The trees have to be modified while
  merging to keep the blockstarttree property. The implementation
  is a simplified version of the procedure described in
  J.Disc.Alg., Vol.1, 2000, pp.77-104 adapted to skiplists.

  Function returns a new Leafblocktree containing the union of the keys
  in lbt1 and lbt2 while maintaining the blockstart property.
  Leafblocktrees lbt1 and lbt2 are destroyed.
*/
Sint mergeleafblocktrees(Leafblocktree **desttree,
			 Leafblocktree *lbt1,
			 Leafblocktree *lbt2,
			 Virtualtree *virtualtree,
			 Memrepos* memrepos)
{
  Sint result;
  Leafblocktree *newtree;

#ifdef CHECKLBTREE
  if(checkleafblocktree(lbt1, NULL) != True)
  {
    ERROR0("Leafblocktree check for tree1 failed before merging.\n");
    return (Sint) -1;
  }
  if(checkleafblocktree(lbt2, NULL) != True)
  {
    ERROR0("Leafblocktree check for tree2 failed before merging.\n");
    return (Sint) -2;
  }
#endif

/* alloc new tree */
  if(desttree == NULL)
  {
    ERROR0("Space for merge result has not been allocated.\n");
    return (Sint) -4;
  }
  newtree = *desttree;

  result = leafblockmerge(&newtree->leaflisttree, 
			  &newtree->blockstarttree,
			  lbt1->leaflisttree, lbt2->leaflisttree,
			  lbt1->blockstarttree, lbt2->blockstarttree,
			  &comparekeys, NULL,
			  &compareblocktreekeys, NULL,
			  virtualtree,
			  memrepos);
  if(result != 0)
  {
    ERROR0("Error while merging two leafblocktrees.\n");
    return (Sint) -5;
  }

#ifdef CHECKLBTREE
  if(checkleafblocktree(newtree, NULL) != True)
  {
    ERROR0("Leafblocktree check failed after merging.\n");
    return (Sint) -6;
  }
#endif

  return 0;
}

/*EE
  report all maximal pairs for current suffix tree node
  using the information from the leaflist and blockstartlist
*/
Sint reportmaximalpairs(Leafblocktree *lbt1, Leafblocktree *lbt2,
			Uint length,
			Outputfunction output, void *outinfo,
			Bounds *bounds,
			Virtualtree *virtualtree,
			Memrepos* memrepos)
{
  Uint           pos,
                 s1, 
                 s2,
                 minleft,
                 minright,
                 maxright,
                 maxleft,
                 i;
  Sint           sminleft,
                 smaxleft,
                 sminright,
                 smaxright;
  Skiplist       *smallerlist,
                 *biggerlist,
                 *biggerblocklist;
  Skipkey        key,
                 result,
                 current;
  Skipfinger     fingerleft,
                 fingerright,
                 blockfingerleft,
                 blockfingerright;
  Skipnode       blockkey;

  s1 = lbt1->leaflisttree->currentlistsize;
  s2 = lbt2->leaflisttree->currentlistsize;
  
  if(s1 > s2) // make sure lbt1 is the smaller tree
  {
    biggerlist = lbt1->leaflisttree;
    smallerlist = lbt2->leaflisttree;
    biggerblocklist = lbt1->blockstarttree;
  } else
  {
    smallerlist = lbt1->leaflisttree;
    biggerlist = lbt2->leaflisttree;
    biggerblocklist = lbt2->blockstarttree;
  }  

  /* initialize four fingers */
  fingerleft = memrepos->skipfinger[0];
  fingerright = memrepos->skipfinger[1];
  blockfingerleft = memrepos->skipfinger[2];
  blockfingerright = memrepos->skipfinger[3];
  for(i = 0; i < biggerlist->maxnodelevel; i++)
  {
    fingerleft[i] = &biggerlist->headpointers[i];
    fingerright[i] = &biggerlist->headpointers[i];
  }
  for(i = 0; i < biggerblocklist->maxnodelevel; i++)
  {
    blockfingerleft[i] = &biggerblocklist->headpointers[i];
    blockfingerright[i] = &biggerblocklist->headpointers[i];
  }

  /* calculate minright */   
  current = skiplistminimumkey(smallerlist);
  if(current == NULL)
  {
    ERROR0("Skiplist is empty.\n");
    return (Sint) -1;
  }
  while(current != NULL)
  {
    pos = *(Uint*) current;
    if(bounds->upperboundfun == NULL)
    {
      sminleft = (Sint) pos - (Sint) length - bounds->upper;      
    } else
    {
      sminleft = (Sint) pos - (Sint) length - 
                 bounds->upperboundfun(length, bounds->upperinfo);
    }
    if(sminleft < 0) sminleft = 0;
    if(bounds->lowerboundfun == NULL)
    {
      smaxleft = (Sint) pos - (Sint) length - bounds->lower;      
    } else
    {
      smaxleft = (Sint) pos - (Sint) length - 
                 bounds->lowerboundfun(length, bounds->lowerinfo);
    }
    if(smaxleft >= (Sint) pos) smaxleft = (Sint) pos - (Sint) 1;
    if(smaxleft >= sminleft)
    {
      minleft = (Uint) sminleft;
      maxleft = (Uint) smaxleft;
      /* process left interval only if right interval bound not negative */
      blockkey.key = (Skipkey) &minleft;
#ifdef WITHCOMPAREFUNCTIONS
      result = skiplistexternalfingernextequalkey(biggerblocklist, 
                              blockfingerleft, (Skipkey) &blockkey, 
			      compareblocktreekeys, NULL); 
#else
      result = skiplistexternalfingernextequalkeywithmacro2(biggerblocklist, 
                              blockfingerleft, (Skipkey) &blockkey, 
			      compareblocktreekeys, NULL); 
#endif     
      key = (Skipkey) &minleft;
#ifdef WITHCOMPAREFUNCTIONS
      result = skiplistexternalfingernextequalkey(biggerlist, 
                              fingerleft, key, comparekeys, NULL);
#else
      result = skiplistexternalfingernextequalkeywithmacro(biggerlist, 
                              fingerleft, key, comparekeys, NULL);
#endif

      if(result != NULL)
      {
        /* now biggerlist.currentelement is set to minleft */
        (void) reportfromtowithblocks(biggerlist, biggerblocklist,
			       pos, maxleft, length,
			       output, outinfo, virtualtree);
      }
    }
    if(bounds->lowerboundfun == NULL)
    {
      sminright = (Sint) pos + (Sint) length + bounds->lower;      
    } else
    {
      sminright = (Sint) pos + (Sint) length + 
                  bounds->lowerboundfun(length, bounds->lowerinfo);
    }
    if(sminright <= (Sint) pos) sminright = (Sint) pos + 1;
    minright = (Uint) sminright;
    if(bounds->upperboundfun == NULL)
    {
      smaxright = (Sint) pos + (Sint) length + bounds->upper;
    } else
    {
      smaxright = (Sint) pos + (Sint) length + 
                  bounds->upperboundfun(length, bounds->upperinfo);
    }
    if(smaxright < 0) smaxright = 0;
    maxright = (Uint) smaxright;
    blockkey.key = (Skipkey) &minright;
#ifdef WITHCOMPAREFUNCTIONS
    result = skiplistexternalfingernextequalkey(biggerblocklist, 
			      blockfingerright, (Skipkey) &blockkey, 
			      compareblocktreekeys, NULL);
#else
    result = skiplistexternalfingernextequalkeywithmacro2(biggerblocklist, 
			      blockfingerright, (Skipkey) &blockkey, 
			      compareblocktreekeys, NULL);
#endif
    key = (Skipkey) &minright;
#ifdef WITHCOMPAREFUNCTIONS
    result = skiplistexternalfingernextequalkey(biggerlist,
                            fingerright, key, comparekeys, NULL);
#else
    result = skiplistexternalfingernextequalkeywithmacro(biggerlist,
                            fingerright, key, comparekeys, NULL);
#endif
    if(result != NULL)
    {
      /* now biggerlist.currentelement is set to minright */
      (void) reportfromtowithblocks(biggerlist, biggerblocklist,
			     pos, maxright, length, 
			     output, outinfo, virtualtree);
    }
    /* advance smallerlist */
    current = SKIPLISTCURRENTNEXT(smallerlist);
  }

  return 0;
} 

/*EE
  Check leafblocktree for consistency. Return True if the
  Leafblocktree has the following properties:
  - every element of the blocktree is in the leaflist
  - every element of the blocktree starts a new block
  - every position that starts a new block is in the blocklist
*/
BOOL checkleafblocktree(Leafblocktree *lbtree, Virtualtree *virtualtree)
{
  Skipkey       leafkey,
                result;
  Skipnode      *blockkey,
                tempkey;
  Uint          pos, 
                current,
                last;

  // for every key in blocklist
  blockkey = (Skipnode*) skiplistminimumkey(lbtree->blockstarttree);
  while(blockkey != NULL)
  {
    leafkey = blockkey->key;
    // make sure that every element in blocklist is in leaflist
    result = skiplistfind(lbtree->leaflisttree, leafkey, comparekeys, NULL);
    if(result == NULL)
    {
      ERROR1("Position %lu in blocklist but not in leaflist.\n", 
              *(Showuint*) leafkey);
      return False;
    }
    if(result != blockkey->key)
    {
      ERROR1("Position %lu in blocklist does not point to leaflist.\n",
	     *(Showuint*) leafkey);
      return False;
    }
    blockkey = (Skipnode*) SKIPLISTCURRENTNEXT(lbtree->blockstarttree);
  }

  if(virtualtree != NULL)
  {
    // for first key in blocklist
    blockkey = (Skipnode*) skiplistminimumkey(lbtree->blockstarttree);
    if(blockkey != NULL)
    {
      leafkey = blockkey->key;
      pos = *(Uint*) leafkey;
      last = virtualtree->multiseq.sequence[pos-1];
      if(lbtree->blockstarttree->currentelement != NULL)
      {
        blockkey = (Skipnode*) SKIPLISTCURRENTNEXT(lbtree->blockstarttree); 
      } else blockkey = NULL;
      while(blockkey != NULL)
      {
	// make sure that position starts a new block
	leafkey = blockkey->key;
	pos = *(Uint*) leafkey;
	current = virtualtree->multiseq.sequence[pos-1];
	if(current == last)
	{
	  ERROR1("Position %lu does not start a block but is in blocklist.\n",
		 (Showuint) pos);
	  return False;
	}
	last = current;
        if(lbtree->blockstarttree->currentelement != NULL)
	{
	  blockkey = (Skipnode*) SKIPLISTCURRENTNEXT(lbtree->blockstarttree);
	}
	else blockkey = NULL;
      } /* while(blockkey != NULL */
    } /* if blockkey != NULL */
  } /* if(virtualtree != NULL) */

  // for every key in leaflist
  if(virtualtree != NULL)
  {
    // process first element
    leafkey = skiplistminimumkey(lbtree->leaflisttree);
    if(leafkey != NULL)
    {
      pos = *(Uint*) leafkey;
      // make sure first element is in blocklist
      tempkey.key = (Skipkey) &pos;
      result = skiplistfind(lbtree->blockstarttree, (Skipkey) &tempkey,
			    compareblocktreekeys, NULL);
      if(result == NULL)
      {
	ERROR1("First leaflist element %lu is not in blocklist.\n",
	       (Showuint) pos);
	return False;
      }
      if(pos == 0)
      {
	last = SEPARATOR;
      } else
      {
	last = virtualtree->multiseq.sequence[pos-1];
      }
      leafkey = SKIPLISTCURRENTNEXT(lbtree->leaflisttree);
      // for all other elements
      while(leafkey != NULL)
      {
	// make sure that if position starts a new block it is in blocklist
	pos = *(Uint*) leafkey;
	assert(pos > 0); // second element > first element >= 0
	current = virtualtree->multiseq.sequence[pos-1];
	if(current != last)
	{
	  // new block
	  tempkey.key = (Skipkey) &pos;
	  result = skiplistfind(lbtree->blockstarttree, (Skipkey) &tempkey,
				compareblocktreekeys, NULL);
	  if(result == NULL)
	  {
	    ERROR1("Position %lu starts a new block but is not in blocklist.\n",
		   (Showuint) pos);
	    return False;
	  }	
	} else
	{
	  // no new block
	  tempkey.key = (Skipkey) &pos;
	  result = skiplistfind(lbtree->blockstarttree, (Skipkey) &tempkey,
				compareblocktreekeys, NULL);
	  if(result != NULL)
	  {
	    ERROR1("Position %lu does not start a block but is in blocklist.\n",
		   (Showuint) pos);
	    return False;
	  }
	}	  
	last = current;
        if(lbtree->leaflisttree->currentelement != NULL)
	{
	  leafkey = SKIPLISTCURRENTNEXT(lbtree->leaflisttree);
	} else leafkey = NULL;
      } /* while leafkey != NULL */
    } /* if leafkey != NULL */
  } /* if virtualtree != NULL */
  return True;
}

/*EE
  Initialize the repository of dynamic arrays. The values
  "initial..." and "...increment" in the datastructure 
  "memrepos" have to be set previously.
*/
void leafblocktreememreposinit(Memrepos *memrepos,
			       Uint maxlevel, /*@unused@*/ Uint seqsize)
{
  Uint i;

#ifndef ALLOCKEYS
  /* init key array */
  ALLOCASSIGNSPACE(memrepos->keys, NULL, Uint, seqsize);
  for(i = 0; i < seqsize; i++)
  {
    memrepos->keys[i] = i;
  }
#endif

  /* init empty arrays */
  INITARRAY(&memrepos->skiplistarray, Skiplist);
  INITARRAY(&memrepos->skipnodearray, Skipnode);
  INITARRAY(&memrepos->skipnodeptrarray, Skipnodeptr);
  INITARRAY(&memrepos->skipnodeptrptrarray, Skipnodeptrptr);
  INITARRAY(&memrepos->leafblocktreearray, Leafblocktree);  

  /* allocate initial space for arrays */
  CHECKARRAYSPACE(&memrepos->leafblocktreearray, Leafblocktree, 
		  memrepos->initialleafblocktrees);
  CHECKARRAYSPACE(&memrepos->skiplistarray, Skiplist, 
		  memrepos->initialskiplists);
  CHECKARRAYSPACE(&memrepos->skipnodearray, Skipnode, 
		  memrepos->initialskipnodes);
  CHECKARRAYSPACE(&memrepos->skipnodeptrarray, Skipnodeptr, 
		  memrepos->initialskipnodeptrs);
  CHECKARRAYSPACE(&memrepos->skipnodeptrptrarray, Skipnodeptrptr, 
		  memrepos->initialskipnodeptrptrs);

  /* alloc space for temporary variables */
  for(i = 0; i < TEMPORARYSKIPFINGERS; i++)
  {
    ALLOCASSIGNSPACE(memrepos->skipfinger[i], NULL, Skipnode**, maxlevel);
  }
  for(i = 0; i < TEMPORARYFORWARDPOINTERS; i++)
  {
    ALLOCASSIGNSPACE(memrepos->forwardpointers[i], NULL, Skipnode*, maxlevel);
  }
  for(i = 0; i < TEMPORARYSKIPLISTS; i++)
  {
    ALLOCASSIGNSPACE(memrepos->skiplist[i], NULL, Skiplist, (Uint) 1);
    ALLOCASSIGNSPACE(memrepos->headpointers[i], NULL, Skipnode*, maxlevel);
    ALLOCASSIGNSPACE(memrepos->tracepointers[i], NULL, Skipnode**, maxlevel);
    /* connect head/tracepointers to list */
    memrepos->skiplist[i]->headpointers = memrepos->headpointers[i];
    memrepos->skiplist[i]->tracepointers = memrepos->tracepointers[i];
  }
  for(i = 0; i < TEMPORARYLEAFBLOCKTREES; i++)
  {
    ALLOCASSIGNSPACE(memrepos->leafblocktree[i], NULL, Leafblocktree, (Uint) 1);
    memrepos->leafblocktree[i]->leaflisttree = memrepos->skiplist[2*i];
    memrepos->leafblocktree[i]->blockstarttree = memrepos->skiplist[2*i+1];
  }
}

/*EE
  Free space for dynamic arrays.
*/
void leafblocktreememreposfree(Memrepos *memrepos)
{
  Uint i;

#ifdef SHOWMEMREPOS
  DEBUG0(1,"\n");
  DEBUG0(1,"Current memory repository:\n");
  DEBUG0(1,"--------------------------\n");
  DEBUG2(1,"Skiplist       : %lu (%lu)\n", 
	 (Showuint) memrepos->skiplistarray.nextfreeSkiplist,
	 (Showuint) memrepos->skiplistarray.allocatedSkiplist);
  DEBUG2(1,"Skipnode       : %lu (%lu)\n", 
	 (Showuint) memrepos->skipnodearray.nextfreeSkipnode,
	 (Showuint) memrepos->skipnodearray.allocatedSkipnode);
  DEBUG2(1,"Skipnode*      : %lu (%lu)\n", 
	 (Showuint) memrepos->skipnodeptrarray.nextfreeSkipnodeptr,
	 (Showuint) memrepos->skipnodeptrarray.allocatedSkipnodeptr);
  DEBUG2(1,"Skipnode**     : %lu (%lu)\n", 
	 (Showuint) memrepos->skipnodeptrptrarray.nextfreeSkipnodeptrptr,
	 (Showuint) memrepos->skipnodeptrptrarray.allocatedSkipnodeptrptr);
  DEBUG2(1,"Leafblocktree  : %lu (%lu)\n", 
	 (Showuint) memrepos->leafblocktreearray.nextfreeLeafblocktree,
	 (Showuint) memrepos->leafblocktreearray.allocatedLeafblocktree);
#endif

#ifndef ALLOCKEYS
  /* free key array */
  FREESPACE(memrepos->keys);
#endif

  /* free dynamic arrays */
  FREEARRAY(&memrepos->skiplistarray, Skiplist);
  FREEARRAY(&memrepos->skipnodearray, Skipnode);
  FREEARRAY(&memrepos->skipnodeptrarray, Skipnodeptr);
  FREEARRAY(&memrepos->skipnodeptrptrarray, Skipnodeptrptr);
  FREEARRAY(&memrepos->leafblocktreearray, Leafblocktree);

  /* free temporary variables */
  for(i = 0; i < TEMPORARYSKIPFINGERS; i++)
  {
    FREESPACE(memrepos->skipfinger[i]);
  } 
  for(i = 0; i < TEMPORARYFORWARDPOINTERS; i++)
  {
    FREESPACE(memrepos->forwardpointers[i]);
  }  
  for(i = 0; i < TEMPORARYLEAFBLOCKTREES; i++)
  {
    FREESPACE(memrepos->leafblocktree[i]);
  }
  for(i = 0; i < TEMPORARYSKIPLISTS; i++)
  {
    FREESPACE(memrepos->skiplist[i]);
    FREESPACE(memrepos->headpointers[i]);
    FREESPACE(memrepos->tracepointers[i]);
  } 
}

/*EE
  traverse the tree and output all elements in order
*/
void showSortedList(Leafblocktree *lbtree)
{  
  printf("Leaflisttree:   ");
  skiplistprintvalues(lbtree->leaflisttree, &showkey, NULL);
  printf("Blockstarttree: ");
  skiplistprintvalues(lbtree->blockstarttree, &showblocktreekey, NULL);
}

/*EE 
  destroy leafblocktree and free memory 
*/
void destroyleafblocktree(Leafblocktree *lbtree)
{
  skiplistdestroy(lbtree->blockstarttree, False, False); // free nodes only 
#ifdef ALLOCKEYS
  skiplistdestroy(lbtree->leaflisttree, False, True); // free keys & nodes
#else
  skiplistdestroy(lbtree->leaflisttree, False, False); // free nodes only
#endif
}
