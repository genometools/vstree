
/*EE 
  SkipList Datastructure to be used in algorithm
  for finding maximal pairs with bounded gap as in     
  J.Disc.Alg., Vol.1, 2000, pp.77-104                  
  Code for insert, delete and find based on implementation by Shane Saunders
  Author: Henning Stehr, h.stehr@tuhh.de              
  Date: 22/Oct/2004                                    
*/

/*
  The use of search fingers:
  A search finger is maintained in the list datastructure. It is used by
  the functions 'skiplistfingernextkey' and 'skiplistfingernextequalkey'.
  The search finger is initialized (i.e. set to the beginning of the list)
  on creation of the list or by calling the 'skiplistinitfinger' function.
  Other functions may also update the search finger. The effect of each
  function on the finger and currentelement pointers (see below) are
  given in the respective function description.

  If more than one search finger is needed, external search fingers can
  be used. They are initialized by calling 'skiplistinitexternalfinger'
  and are used equivalently to the internal finger by the functions
  'skiplistexternalfingernextkey' and 'skiplistexternalfingernextequalkey'.

  For traversing the list the 'currentelement' pointer is maintained as well.
  It can be advanced by 'skiplistcurrentnext' and is initialized by calling
  'skiplistminimumkey'. 
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "types.h"     // Types Uint, Sint, BOOL, etc.
#include "debugdef.h"  // Macros: DEBUG0, DEBUG1, etc.
#include "errordef.h"  // Macros: ERROR0, ERROR1, etc.
#include "spacedef.h"  // Macros: ALLOCSPACE, FREESPACE
#include "megabytes.h" // Macro: MEGABYTES
#include "drand48.h"
/* 
  libraries above are (C) by Stefan Kurtz, kurtz@zbh.uni-hamburg.de
*/
#include "skiplist.h" 

/*--------------------------- private functions ------------------------------*/
/*
  Return the recommended maximum node level for the given probability and size.
*/
Uint recommendedmaxlevel(double p, Uint maxsize)
{
  Uint result;
  double temp;
  temp = (-1) * log((double) maxsize);
  result = (Uint) rint( temp / log(p)  );
  if(result == 0)
  {
    return (Uint) 1;
  }
  return result;
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
/*--------------------------- public functions -------------------------------*/

/*EE
  Create a new empty skiplist. If doalloc = True, allocate space for the
  new skiplist and set 'newskiplist' to point to the new space. If
  doalloc = False, newskiplist has to point to allocated space for a
  skiplist including space for the headpointers and tracepointers.
  If no error occured 0 is returned.
  Currentelement: set to NULL
  Finger: set to NULL
*/
Sint skiplistinit(Skiplist **newskiplist, Uint maxsize,
		  double p, BOOL doalloc)
{
  Uint i, maxnodelevel;
  Skipnode **headpointers, ***tracepointers;
  Skiplist *skiplist;

  /* allocate space for skiplist datastructure */
  if(doalloc == True)
  {
    ALLOCASSIGNSPACE(skiplist, NULL, Skiplist, (Uint) 1);
    if(skiplist == NULL)
    {
      ERROR0("Not possible to allocate memory for new skiplist\n");
      return (Sint) -1;
    }
    *newskiplist = skiplist;
  } else
  {
    if(*newskiplist == NULL)
    {
      ERROR0("Space for new skiplist is not allocated.\n");
      return (Sint) -2;
    }
    skiplist = *newskiplist;
  }

  /* fill datastructure with inital values */
  maxnodelevel = recommendedmaxlevel(p, maxsize); // optimal node size
  skiplist->p = p;
  skiplist->maxlistsize = maxsize;
  skiplist->maxnodelevel = maxnodelevel;
  skiplist->currentlistsize = 0;
  skiplist->currentelement = NULL;  
  
  /* initialize headpointers */
  if(doalloc == True)
  {
    ALLOCASSIGNSPACE(headpointers, NULL, Skipnode*, maxnodelevel);
    if(headpointers == NULL) 
    {
      ERROR0("Unable to allocate memory for headpointers of new skiplist.\n");
      return (Sint) -3;
    }
    (*newskiplist)->headpointers = headpointers;
  } else
  {
    if((*newskiplist)->headpointers == NULL)
    {
      ERROR0("Space for headpointers of new skiplist is not allocated.\n");
      return (Sint) -4;
    }
    headpointers = (*newskiplist)->headpointers;
  }

  /* initialize trace pointers */
  if(doalloc == True)
  {
    ALLOCASSIGNSPACE(tracepointers, NULL, Skipnode**, maxnodelevel);
    if(tracepointers == NULL)
    {
      ERROR0("Unable to allocate memory for tracepointers of new skiplist.\n");
      return (Sint) -5;
    }
    (*newskiplist)->tracepointers = tracepointers;
  } else
  {
    if((*newskiplist)->tracepointers == NULL)
    {
      ERROR0("Space for tracepointers of new skiplist is not allocated.\n");
      return (Sint) -6;
    }
    tracepointers = (*newskiplist)->tracepointers;
  }

  /* fill pointers with values */
  for(i = 0; i < maxnodelevel; i++)
  {
    headpointers[i] = NULL;
    tracepointers[i] = &headpointers[i];
  }

  return 0;
}

/*EE
  Create a new skiplist containing one key 'key'. If doalloc = True,
  allocate space for the new skiplist and set 'newskiplist' to point to
  the new space. If doalloc = False, newskiplist has to point to allocated
  space for a skiplist including space for the headpointers and tracepointers.
  If no error occured 0 is returned.
  Currentelement: set to 'key'
  Finger: set to 'key'
*/
Sint skiplistinitwithkey(Skiplist **newskiplist, Uint maxsize,
			 double p, BOOL doalloc, Skipkey key)
{
  Uint i, maxnodelevel, level;
  Skipnode **headpointers;
  Skipnode ***tracepointers, **forward, *newnode;
  Skiplist *skiplist;  

  /* allocate space for skiplist datastructure */
  if(doalloc == True)
  {
    ALLOCASSIGNSPACE(skiplist, NULL, Skiplist, (Uint) 1);
    if(skiplist == NULL)
    {
      ERROR0("Not possible to allocate memory for new skiplist\n");
      return (Sint) -1;
    }
    *newskiplist = skiplist;
  } else
  {
    if(*newskiplist == NULL)
    {
      ERROR0("Space for new skiplist is not allocated.\n");
      return (Sint) -2;
    }
    skiplist = *newskiplist;
  }

  /* fill datastructure with inital values */
  maxnodelevel = recommendedmaxlevel(p, maxsize); // optimal node size
  skiplist->p = p;
  skiplist->maxlistsize = maxsize;
  skiplist->maxnodelevel = maxnodelevel;  
  skiplist->currentlistsize = (Uint) 1;

  /* initialize headpointers */
  if(doalloc == True)
  {
    ALLOCASSIGNSPACE(headpointers, NULL, Skipnode*, maxnodelevel);
    if(headpointers == NULL) 
    {
      ERROR0("Unable to allocate memory for headpointers of new skiplist.\n");
      return (Sint) -3;
    }
    (*newskiplist)->headpointers = headpointers;
  } else
  {
    if((*newskiplist)->headpointers == NULL)
    {
      ERROR0("Space for headpointers of new skiplist is not allocated.\n");
      return (Sint) -4;
    }
    headpointers = (*newskiplist)->headpointers;
  }
  
  //skiplist->headpointers = headpointers;

  /* initialize trace pointers */
  if(doalloc == True)
  {
    ALLOCASSIGNSPACE(tracepointers, NULL, Skipnode**, maxnodelevel);
    if(tracepointers == NULL)
    {
      ERROR0("Unable to allocate memory for tracepointers of new skiplist.\n");
      return (Sint) -5;
    }
    (*newskiplist)->tracepointers = tracepointers;
  } else
  {
    if((*newskiplist)->tracepointers == NULL)
    {
      ERROR0("Space for tracepointers of new skiplist is not allocated.\n");
      return (Sint) -6;
    }
    tracepointers = (*newskiplist)->tracepointers;
  }

  //skiplist->tracepointers = tracepointers; 

  /* allocate a new node of a random size */
  ALLOCASSIGNSPACE(newnode, NULL, Skipnode, (Uint) 1);
  if(newnode == NULL)
  {
    ERROR0("Unable to allocate memory for initial node of new skiplist\n");
    return (Sint) -7;
  }  

  level = skiplistrandlevel(skiplist->p, maxnodelevel);
  newnode->nodelevel = level;
  ALLOCASSIGNSPACE(forward, NULL, Skipnode*, level);
  newnode->forward = forward;
  if(forward == NULL)
  {
    ERROR0("Unable to allocate memory for forward pointers of new node\n");
    return (Sint) -8;
  }

  newnode->key = key;
  skiplist->currentelement = newnode; // update currentelement

  /* Set forward pointers */
  for(i = 0; i < level; i++) {
    forward[i] = NULL;
    headpointers[i] = newnode;
    tracepointers[i] = &headpointers[i];
  }  

  /* set headpointers */  
  for(i = level; i < maxnodelevel; i++)
  {
    headpointers[i] = NULL;
    tracepointers[i] = &headpointers[i];
  }

  return 0;
}

/*EE
  Skiplist merge operation as in 'A skiplist cookbook' by William Pugh, 1990:
  Create a new list containing the union of the elements of src1 and src2.
  Destroys src1 and src2. If an element appears in both lists, use value
  field from list2. If an error occurs return NULL.
  Properties of new list:
  - newlist.maxsize = max(list1.maxsize, list2.maxsize)
  - newlist.maxnodelevel = max(list1.maxnodelevel, list2.maxnodelevel)
  - newlist.p = list2.p
  - Currentelement: set to first element (or NULL if list is empty)
  - Finger : set to first element (or NULL if list is empty)
*/

Sint skiplistmerge(Skiplist **destlist, Skiplist* list1, Skiplist* list2,
		       Skipcomparefunction cmpfun, void* cmpinfo, BOOL doalloc)
{
  Uint         s1, s2,            // current size of list1, list2
               l1, l2,            // maxnodelevel of list1, list2
               m1, m2,            // maxsize of list1, list2
               newlistsize,       // size of merged list
               newmaxlistsize;    // maxsize of merged list
  Sint         i, lvl,            // loop indeces
               retval,            // error value of skiplistinit function 
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

  /* check input parameters */
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
  if(cmpfun == NULL)
  {
    ERROR0("Could not merge lists, handle to comparefunction is NULL.\n");
    return (Sint) -3;
  }
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

  /* init new list */
  s1 = list1->currentlistsize;
  s2 = list2->currentlistsize;
  l1 = list1->maxnodelevel;
  l2 = list2->maxnodelevel;
  m1 = list1->maxlistsize;
  m2 = list2->maxlistsize;
  if(l1 >= l2) maxlevel = (Sint) l1; else maxlevel = (Sint) l2; //=max(l1,l2)
  if(m1 >= m2) newmaxlistsize = m1; else newmaxlistsize = m2;   //=max(m1,m2);
  newlistsize = s1 + s2;
  newp = list2->p; // take p from second list

  /* initalize new list */
  if(doalloc == True)
  {
    retval = skiplistinit(&newlist, newmaxlistsize, newp, True);
    if(retval != 0)
    {
      ERROR0("Unable to initialize new skip list for merge result\n");
      return (Sint) -6;
    }
    *destlist = newlist;
  } else
  {
    retval = skiplistinit(destlist, newmaxlistsize, newp, False);
    if(retval != 0)
    {
      ERROR0("Unable to initialize new skip list for merge result\n");
      return (Sint) -7;
    }
    newlist = *destlist;
  }
  if(newlist->headpointers == NULL)
  {
    ERROR0("Unable to initialize headpointers for new list.\n");
    return (Sint) -8;
  }

  newlist->currentlistsize = newlistsize;

  /* merge lists */

  /* Create temporary node: This node will temporarily store the headpointer
     information of the new list by using a Skipnode type variable the
     headpointers of the list can be accessed with the same interface as a
     simple list node.
  */
  ALLOCASSIGNSPACE(header.forward, NULL, Skipnode*, (Uint) maxlevel);
  ALLOCASSIGNSPACE(update, NULL, Skipnode*, (Uint) maxlevel);
  
  for(i = 0; i < maxlevel; i++)
  {
    update[i] = &header;
  }

  while((list1->headpointers[0] != NULL) && (list2->headpointers[0] != NULL))
  {
    key1 = list1->headpointers[0]->key;
    key2 = list2->headpointers[0]->key;

    /* assume w.l.g. that key1 <= key2, otherwise 
       xchg(key1, key2) and xchg(list1, list2)
    */
    if(cmpfun(key1, key2, cmpinfo) > 0)
    {
      tempkey = key1; key1 = key2; key2 = tempkey; // XCHG(key1, key2);
      templst = list1; list1 = list2; list2 = templst; // XCHG(list1, list2);
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
            (cmpfun(list1->headpointers[lvl]->key, key2, cmpinfo) <= 0));
    lvl--;
    
    /* for each level attached to output list, find endpoint at that level
       i.e. the last element with key <= key2
    */
    x = list1->headpointers[lvl];
    for(i = lvl; i >= 0; i--)
    {
      while((x->forward[i] != NULL) && 
            (cmpfun(((Skipnode*)(x->forward[i]))->key, key2, cmpinfo) <= 0)) 
        x = (Skipnode*) x->forward[i];
        /* now x->key <= key2 < x->forward[i]->key */

      update[i] = x;
      list1->headpointers[i] = (Skipnode*) x->forward[i];
    }
    /* now x = last element moved to output list */

    /* if the element at the front of list2 is a duplicate of an element
       already moved to output list, eliminate it
    */
    if(cmpfun(key2, x->key, cmpinfo) == 0)
    {
      if(unflipped) x->key = list2->headpointers[0]->key; // #danger: space leak
      y = list2->headpointers[0];
      for(i = 0; (Uint) i < y->nodelevel; i++) 
        list2->headpointers[i] = (Skipnode*) y->forward[i];
      FREESPACE(y->forward);
      FREESPACE(y);
      newlist->currentlistsize--; // don't count duplicates
    }
  } /* end of main while loop */

  if(list2->headpointers[0] == NULL) leftover = list1; else leftover = list2;
  for(i = 0; (Uint) i < leftover->maxnodelevel; i++) 
    update[i]->forward[i] = leftover->headpointers[i];
  for(i = (Sint) leftover->maxnodelevel; i < maxlevel; i++) 
    update[i]->forward[i] = NULL;

  /* copy header content to headpointers of new list */
  for(i = 0; i < maxlevel; i++) {
    newlist->headpointers[i] = header.forward[i];
    newlist->tracepointers[i] = &(newlist->headpointers[i]);
  }

  newlist->currentlistsize = newlistsize;
  newlist->maxlistsize = newmaxlistsize;
  newlist->maxnodelevel = (Uint) maxlevel;
  newlist->p = newp;
  newlist->currentelement = newlist->headpointers[0];

  /* free space for local variables and input lists */
  FREESPACE(header.forward);
  FREESPACE(update);

  if(doalloc == True)
  {
    FREESPACE(list1->tracepointers);
    FREESPACE(list1->headpointers);
    FREESPACE(list1);
    FREESPACE(list2->tracepointers);
    FREESPACE(list2->headpointers);
    FREESPACE(list2);
  }
  return 0;
}

/*EE 
  Insert an item according the the value of its key. The key of an item in
  the skip list must be unique among items in the list.  If an item with the
  same key already exists in the list, a pointer to that item is returned.
  Otherwise, NULL is returned, indicating insertion was successful.
  Currentelement: set to new element if insertion was successfull,
                  otherwise not changed
  Finger: Set to new element or to duplicate element if it exists.
*/
/*@null@*/ Skipkey skiplistinsert(Skiplist *skiplist, Skipkey key, 
                       Skipcomparefunction cmpfun, void* cmpinfo)
{
    Skipnode *new_node, **forward, ***update;
    Sint cmp_result = 0;
    Sint i, l;
    Uint max_level;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not insert into skiplist, handle for list is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not insert into skiplist, headpointers for list is NULL.\n");
      return NULL;
    }
    if(skiplist->tracepointers == NULL)
    {
      ERROR0("Could not insert into skiplist, tracepointers for list is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not insert into skiplist, key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not insert into skiplist, comparefunction is NULL.\n");
      return NULL;
    }

    update = skiplist->tracepointers;
    max_level = skiplist->maxnodelevel;
    forward = skiplist->headpointers;

    /* Locate insertion position. */
    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(forward[i] == NULL) {
	    update[i] = &forward[i];
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun(forward[i]->key, key, cmpinfo)) >= 0) {
	    update[i] = &forward[i];
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	forward = (Skipnode**) forward[i]->forward;
    }
  end_find_loop:

    /* Check that the item being inserted does not have the same key as an item
       already in the list.
    */
    if(forward[0] && cmp_result == 0) return forward[0]->key;

    /* Allocate a new node of a random size. */
    ALLOCASSIGNSPACE(new_node, NULL, Skipnode, (Uint) 1);
    l = (Sint) skiplistrandlevel(skiplist->p, max_level);
    new_node->nodelevel = (Uint) l;
    ALLOCASSIGNSPACE(forward, NULL, Skipnode*, (Uint) l); 
    new_node->forward = forward;
    new_node->key = key;
    skiplist->currentelement = new_node; // update currentelement

    /* Update pointers in the list. */
    for(i = 0; i < l; i++) {
	forward[i] = *update[i];
	*update[i] = new_node;
    }

    skiplist->currentlistsize++;
    
    return NULL;  /* Insertion successful. */
}

/*EE 
  Delete the first item found with the given key. Return a pointer to the
  deleted item, or NULL if no item was found.
  Currentelement: not changed
  Finger: set to smallest element with key > 'key' or NULL if it doesn't exist.
*/
/*@null@*/ Skipkey skiplistdelete(Skiplist *skiplist, Skipkey key, 
                       Skipcomparefunction cmpfun, void* cmpinfo)
{
    Skipnode *remove_node, **forward, ***update;
    Sint cmp_result = 0;
    Sint i, l;
    Uint max_level;
    Skipkey return_item;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not delete from skiplist, handle for list is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not delete from skiplist, headpointers for list is NULL.\n");
      return NULL;
    }
    if(skiplist->tracepointers == NULL)
    {
      ERROR0("Could not delete from skiplist, tracepointers for list is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not delete from skiplist, key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not delete from skiplist, comparefunction is NULL.\n");
      return NULL;
    }

    update = skiplist->tracepointers;
    max_level = skiplist->maxnodelevel;
    forward = skiplist->headpointers;

    /* Locate deletion position. */
    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(forward[i] == NULL) {
	    update[i] = &forward[i];
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun(forward[i]->key, key, cmpinfo)) >= 0) {
	    update[i] = &forward[i];
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	forward = (Skipnode**) forward[i]->forward;
    }
  end_find_loop:

    remove_node = forward[0];
    
    /* A matching item may not have been found. */
    if(remove_node == NULL || cmp_result != 0) return NULL;
    /* else: */

    /* Item was found.  Update pointers. */
    l = (Sint) remove_node->nodelevel;
    forward = (Skipnode**) remove_node->forward;
    for(i = 0; i < l; i++) {
        *update[i] = forward[i];
    }

    /* Free space and return the deleted item. */
    return_item = remove_node->key;
    FREESPACE(forward);
    FREESPACE(remove_node);

    skiplist->currentlistsize--;
    
    return return_item;
}

/*EE 
  Find the first element in the list that matches 'key'. Returns the matching
  key, or NULL if no element was found.
  Currentelement: unchanged if no element found, otherwise set to 
                  matching element
  Finger: Set to smallest element >= 'key' or NULL if it doesn't exist
*/
/*@null@*/ Skipkey skiplistfind(Skiplist *skiplist, Skipkey key, 
                     Skipcomparefunction cmpfun, void* cmpinfo)
{
    Skipnode **forward, ***update;
    Sint cmp_result = 0;
    Sint i;
    Uint max_level;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not search in skiplist, handle for list is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not search in skiplist, headpointers for list is NULL.\n");
      return NULL;
    }
    if(skiplist->tracepointers == NULL)
    {
      ERROR0("Could not search in skiplist, tracepointers for list is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not search in skiplist, key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not search in skiplist, comparefunction is NULL.\n");
      return NULL;
    }

    update = skiplist->tracepointers;
    max_level = skiplist->maxnodelevel;
    forward = skiplist->headpointers;

    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(forward[i] == NULL) {
	    update[i] = &forward[i];
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun(forward[i]->key, key, cmpinfo)) >= 0) {
	    update[i] = &forward[i];
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	forward = (Skipnode**) forward[i]->forward;
    }
  end_find_loop:
    
    /* Check if a matching item was found. */
    if(forward[0] && cmp_result == 0) {
      skiplist->currentelement = forward[0]; // update currentelement
      return forward[0]->key;
    }

    /* If this point is reached, a matching item was not found.
     */
    return NULL;
}

/*EE
  Return the minimum key in the list
  or NULL if list is empty or an error occured.
  Currentelement: set to minimal element or NULL if list is empty
  Finger: not changed
*/
/*@null@*/ Skipkey skiplistminimumkey(Skiplist* skiplist)
{
  Skipnode *minelem;

  /* check input parameters */
  if(skiplist == NULL)
  {
    ERROR0("Could not return min key from skiplist. List handle is NULL.\n");
    return NULL;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not return min key from skiplist. Headpointer is NULL.\n");
    return NULL;
  }

  minelem = skiplist->headpointers[0];
  if(minelem != NULL) {
    skiplist->currentelement = minelem;
    return minelem->key;
  } else
  {
    skiplist->currentelement = NULL;
    return NULL;
  }
}

/*EE 
  Return the smallest key in the list that is greater than 'key'.
  If no such key exists NULL is returned.
  Currentelement: If key was found, set to matching key, otherwise unchanged.
  Finger: Set to smallest key >= 'key' or NULL if it doesn't exist.
*/
/*@null@*/ Skipkey skiplistnextkey(Skiplist* skiplist, Skipkey key, 
                        Skipcomparefunction cmpfun, void* cmpinfo)
{
    Skipnode **forward, ***update;
    Sint cmp_result = 0;
    Sint i;
    Uint max_level;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, handle for list is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not find nextkey in skiplist. Headpointers is NULL.\n");
      return NULL;
    }
    if(skiplist->tracepointers == NULL)
    {
      ERROR0("Could not find nextkey in skiplist. Tracepointers is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, comparefunction is NULL.\n");
      return NULL;
    }

    update = skiplist->tracepointers;
    max_level = skiplist->maxnodelevel;
    forward = skiplist->headpointers;

    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(forward[i] == NULL) {
	    update[i] = &forward[i];
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun(forward[i]->key, key, cmpinfo)) >= 0) {
	    update[i] = &forward[i];
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	forward = (Skipnode**) forward[i]->forward;
    }
  end_find_loop:
    
    /* If end is reached, no next item exists. */
    if(forward[0] == NULL) return NULL; // no next key found

    /* If item was found, return next item or NULL if next is not available */
    if(cmp_result == 0) {
      forward = (Skipnode**) forward[0]->forward;
      if(forward[0] != NULL) {
        skiplist->currentelement = forward[0]; // update currentelement
        return forward[0]->key;        
      } else
      {
        return NULL;
      }
    } else
    /* otherwise forward[i]->key > key => return current item */  
    {
        skiplist->currentelement = forward[0]; // update currentelement
        return forward[0]->key;        
    }
}

/*EE 
  Return the smallest key in the list that is greater than or equal to 'key'.
  If no such key exists NULL is returned.
  Currentelement: If key was found, set to matching key, otherwise unchanged.
  Finger: Set to smallest key >= 'key' or NULL if it doesn't exist.
*/
/*@null@*/ Skipkey skiplistnextequalkey(Skiplist* skiplist, Skipkey key, 
                             Skipcomparefunction cmpfun, void* cmpinfo)
{
    Skipnode **forward, ***update;
    Sint cmp_result;
    Sint i;
    Uint max_level;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. List handle is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Headpointers is NULL.\n");
      return NULL;
    }
    if(skiplist->tracepointers == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Tracepointers is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Compfun is NULL.\n");
      return NULL;
    }

    update = skiplist->tracepointers; 
    max_level = skiplist->maxnodelevel;
    forward = skiplist->headpointers;

    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(forward[i] == NULL) {
	    update[i] = &forward[i];
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun(forward[i]->key, key, cmpinfo)) >= 0) {
	    update[i] = &forward[i];
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	forward = (Skipnode**) forward[i]->forward;
    }
  end_find_loop:
    
    /* If end is reached, no next item exists. */
    if(forward[0] == NULL) return NULL; // no next key found

    /* Otherwise the item found must be greater of equal than the query */
    skiplist->currentelement = forward[0]; // update currentelement
    return forward[0]->key;        
}

/*EE
  Move the currentelement pointer to the next element in list and return
  the key of that element or NULL if no next element exists.
  The currentelement pointer can be set by the function skiplistminelement
  and skiplistfind.
  Finger: unchanged
*/
/*@null@*/ Skipkey skiplistcurrentnext(Skiplist* skiplist)
{
  /* check input parameters */
  if(skiplist == NULL)
  {
    ERROR0("Could not return next key from skiplist. List handle is NULL.\n");
    return NULL;
  }

  if(skiplist->currentelement == NULL) return NULL;
  skiplist->currentelement = (Skipnode*) skiplist->currentelement->forward[0];
  return skiplist->currentelement != NULL? skiplist->currentelement->key:NULL;
}

/*EE
  Return for the smallest key greater than 'key' starting from
  the element pointed to by the search finger. Return NULL if no element
  was found or the search finger is already past the mathcing element
  (i.e. key of the element pointet to by the search finger > 'key').
  Currentelement: If element found, set to matching element, otherwise unchanged
  Finger: Set to smallest element >= 'key' or NULL if it doesn't exist.
*/

/*@null@*/ Skipkey skiplistfingernextkey(Skiplist* skiplist, 
					 Skipkey key, 
					 Skipcomparefunction cmpfun, 
					 void* cmpinfo)
{
    Skipnode ***update;
    Sint cmp_result = 0;
    Sint i;
    Uint max_level;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, handle for list is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not find nextkey in skiplist. Headpointers is NULL.\n");
      return NULL;
    }
    if(skiplist->tracepointers == NULL)
    {
      ERROR0("Could not find nextkey in skiplist. Tracepointers is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, comparefunction is NULL.\n");
      return NULL;
    }

    max_level = skiplist->maxnodelevel;
    update = skiplist->tracepointers;

    /* check whether end of list is already reached */
    if(*update[0] == NULL) return NULL;

    /* check whether finger is already past key to find */
    if(cmpfun((*update[0])->key, key, cmpinfo) > 0) return NULL;

    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(*update[i] == NULL) {
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun((*update[i])->key, key, cmpinfo)) >= 0) {
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	update[i] = &((*update[i])->forward[i]);
    }
  end_find_loop:
    
    /* If end is reached, no next item exists. */
    if(*update[0] == NULL) return NULL; // no next key found

    /* If item was found, return next item or NULL if next is not available */
    if(cmp_result == 0) {
      update[0] = &((*update[0])->forward[0]);
      if(*update[0] != NULL) {
        skiplist->currentelement = *update[0]; // update currentelement
        return (*update[0])->key;        
      } else
      {
        return NULL;
      }
    } else
    /* otherwise *update[i]->key > key => return current item */  
    {
        skiplist->currentelement = *update[0]; // update currentelement
        return (*update[0])->key;        
    }  
}

/*EE
  Return for the smallest key greater than or equal to 'key' starting from
  the element pointet to by the search finger. Return NULL if no element
  was found or the search finger is already past the mathcing element
  (i.e. key of the element pointet to by the search finger > 'key').
  Currentelement: If element found, set to matching element, otherwise unchanged
  Finger: Set to smallest element >= 'key' or NULL if it doesn't exist. 
*/
/*@null@*/ Skipkey skiplistfingernextequalkey(Skiplist* skiplist,
                                              Skipkey key,
                                              Skipcomparefunction cmpfun,
                                              void* cmpinfo)
{
    Skipnode ***update;
    Sint cmp_result = 0;
    Sint i;
    Uint max_level;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. List handle is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Headpointers is NULL.\n");
      return NULL;
    }
    if(skiplist->tracepointers == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Tracepointers is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not find nextequalkey in skiplist. Compfun is NULL.\n");
      return NULL;
    }

    update = skiplist->tracepointers; 
    max_level = skiplist->maxnodelevel;

    /* check whether end of list is already reached */
    if(*update[0] == NULL) return NULL;

    /* check whether finger is already past key to find */
    if(cmpfun((*update[0])->key, key, cmpinfo) > 0) return NULL;

    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(*update[i] == NULL) {
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun((*update[i])->key, key, cmpinfo)) >= 0) {
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	update[i] = &((*update[i])->forward[i]);
    }
  end_find_loop:
    
    /* If end is reached, no next item exists. */
    if(*update[0] == NULL) return NULL; // no next key found

    /* Otherwise the item found must be greater of equal than the query */
    skiplist->currentelement = *update[0]; // update currentelement
    return (*update[0])->key;        
}

/*EE
  Return for the smallest key greater than 'key' starting from
  the element pointed to by the external search finger 'finger'.
  An external finger can be initialized by 'skiplistinitexternalfinger'. 
  Return NULL if no element was found or the search finger is already
  past the matching element (i.e. key of the element pointet to by the
  search finger > 'key').
  Currentelement: If element found, set to matching element, otherwise unchanged
  External finger: Set to smallest element >= 'key' or NULL if it doesn't exist.
*/

/*@null@*/ Skipkey skiplistexternalfingernextkey(Skiplist* skiplist, 
					 Skipfinger finger,
					 Skipkey key, 
					 Skipcomparefunction cmpfun, 
					 void* cmpinfo)
{
    Skipnode ***update;
    Sint cmp_result = 0;
    Sint i;
    Uint max_level;

    /* check input parameters */
    if(skiplist == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, handle for list is NULL.\n");
      return NULL;
    }
    if(skiplist->headpointers == NULL)
    {
      ERROR0("Could not find nextkey in skiplist. Headpointers is NULL.\n");
      return NULL;
    }
    if(finger == NULL)
    {
      ERROR0("Could not find nextkey in skiplist. External finger is NULL.\n");
      return NULL;
    }
    if(key == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, key is NULL.\n");
      return NULL;
    }
    if(cmpfun == NULL)
    {
      ERROR0("Could not find nextkey in skiplist, comparefunction is NULL.\n");
      return NULL;
    }

    max_level = skiplist->maxnodelevel;
    update = finger;

    /* check whether end of list is already reached */
    if(*update[0] == NULL) return NULL;

    /* check whether finger is already past key to find */
    if(cmpfun((*update[0])->key, key, cmpinfo) > 0) return NULL;

    i = (Sint) max_level - 1;
    for(;;) {

	/* Ignore NULL pointers at the top of the forward pointer array. */
	while(*update[i] == NULL) {
	    i--;
            if(i < 0) goto end_find_loop;
	}

	/* Don't traverse toward nodes which are not smaller than the
	 * item being searched for.
	 */
        while((cmp_result = cmpfun((*update[i])->key, key, cmpinfo)) >= 0) {
	    i--;
	    if(i < 0) goto end_find_loop;
	}

	update[i] = &((*update[i])->forward[i]);
    }
  end_find_loop:
    
    /* If end is reached, no next item exists. */
    if(*update[0] == NULL) return NULL; // no next key found

    /* If item was found, return next item or NULL if next is not available */
    if(cmp_result == 0) {
      update[0] = &((*update[0])->forward[0]);
      if(*update[0] != NULL) {
        skiplist->currentelement = *update[0]; // update currentelement
        return (*update[0])->key;        
      } else
      {
        return NULL;
      }
    } else
    /* otherwise *update[i]->key > key => return current item */  
    {
        skiplist->currentelement = *update[0]; // update currentelement
        return (*update[0])->key;        
    }  
}

/*EE
  Return for the smallest key greater than or equal to 'key' starting from
  the element pointet to by the external search finger 'finger'. 
  An external finger can be initialized by 'skiplistinitexternalfinger'.
  Return NULL if no element was found or the search finger is already past
  the matching element (i.e. key of the element pointet to by the search
  finger > 'key').
  Currentelement: If element found, set to matching element, otherwise unchanged
  External finger: Set to smallest element >= 'key' or NULL if it doesn't exist. 
*/
/*@null@*/ Skipkey skiplistexternalfingernextequalkey(Skiplist* skiplist,
                                              Skipfinger finger,
                                              Skipkey key,
                                              Skipcomparefunction cmpfun,
                                              void* cmpinfo)
#define INTERNALKEYCOMPARE(K1,K2)\
        cmpfun(K1, K2, cmpinfo)
{
#include "skiplist-inc.gen"
}       

/*EE
  Alternative version of 'skiplistexternalfingernextequalkey'.
  The only difference in functionality is that instead of the comparefunction,
  the macro COMPAREKEYS will be used to compare skipkeys.
  The macro has to be defined in the code before calling this function.

  Use this function instead of 'skiplistexternalfingernextequalkey' if
  key comparison is time critical in the application.
*/
/*@null@*/ Skipkey skiplistexternalfingernextequalkeywithmacro(
			Skiplist* skiplist,
                        Skipfinger finger,
                        Skipkey key,
                        /*@unused@*/ Skipcomparefunction cmpfun,
                        /*@unused@*/ void* cmpinfo)
#ifndef COMPAREKEYS
{
  ERROR0("Macro 'COMPAREKEYS' for comparing keys is not defined.\n");
  return NULL;
}
#else
#undef INTERNALKEYCOMPARE
#define INTERNALKEYCOMPARE(K1,K2)\
        COMPAREKEYS(K1,K2)
{
#include "skiplist-inc.gen"
}
#endif

/*EE
  Alternative version of 'skiplistexternalfingernextequalkey'.
  The only difference in functionality is that instead of the comparefunction,
  the macro COMPAREKEYS2 will be used to compare skipkeys.
  The macro has to be defined in the code before calling this function.
  
  Use this function in addition to 'skiplistexternalfingernextequalkeywithmacro'
  if two different types of keys need to be handled in the same application.
*/
/*@null@*/ Skipkey skiplistexternalfingernextequalkeywithmacro2(
		Skiplist* skiplist,
                Skipfinger finger,
                Skipkey key,
                /*@unused@*/ Skipcomparefunction cmpfun,
                /*@unused@*/ void* cmpinfo)
#ifndef COMPAREKEYS2
{
  ERROR0("Macro 'COMPAREKEYS2' for comparing keys is not defined.\n");
  return NULL;
}
#else
#undef INTERNALKEYCOMPARE
#define INTERNALKEYCOMPARE(K1,K2)\
        COMPAREKEYS2(K1,K2)
{
#include "skiplist-inc.gen"
}
#endif

/*EE
  Allocate space for a new external search finger and return handle to
  the finger or NULL if an error occured.
*/
/*@null@*/ Skipfinger skiplistinitexternalfinger(Skiplist *skiplist)
{
  Skipfinger newfinger;
  Uint i, size;

  /* check input parameters */
  if(skiplist == NULL)
  {
    ERROR0("Could not find nextkey in skiplist, handle for list is NULL.\n");
    return NULL;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not find nextkey in skiplist. Headpointers is NULL.\n");
    return NULL;
  }

  size = skiplist->maxnodelevel;
  ALLOCASSIGNSPACE(newfinger, NULL, Skipnode**, size);
  if(newfinger == NULL)
  {
    ERROR0("Unable to allocate space for new Skipfinger.\n");
    return NULL;
  }
  for(i = 0; i < size; i++)
  {
    newfinger[i] = &skiplist->headpointers[i];
  }
  return newfinger;
}

/*EE
  Set the internal search finger to beginning of list
*/
Sint skiplistinitfinger(Skiplist *skiplist)
{
  Uint i;

  if(skiplist == NULL)
  {
    ERROR0("Could not find nextequalkey in skiplist. List handle is NULL.\n");
    return (Sint) -1;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not find nextequalkey in skiplist. Headpointers is NULL.\n");
    return (Sint) -1;
  }
  if(skiplist->tracepointers == NULL)
  {
    ERROR0("Could not find nextequalkey in skiplist. Tracepointers is NULL.\n");
    return (Sint) -1;
  }

  for(i = 0; i < skiplist->maxnodelevel; i++)
  {
    skiplist->tracepointers[i] = &skiplist->headpointers[i];
  }
  return 0;
}

/*EE
  Free space for external search finger.
*/
void skiplistfreeexternalfinger(Skipfinger finger)
{
  FREESPACE(finger);
}

/*EE
  Traverse the list and call action function for each element in sorted order.
  Provided for compatibility with tree based dictionary structures.
  For each element, action function of type 'dictaction' is called with
  parameters key, position in list, actinfo.
*/
void skiplistwalk(Skiplist *skiplist, Skipaction action, void* actinfo)
{
  Skipnode *current;          // pointer to element in list  
  Uint i = 0;                 // number of element in list

  /* check input parameters */
  if(skiplist == NULL)
  {
    ERROR0("Could not walk through skiplist, handle for list is NULL.\n");
    return;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not walk through skiplist, headpointers for list is NULL.\n");
    return;
  }

  if(action == NULL) {
    ERROR0("Could not walk through skiplist. Action function is NULL\n");
    return;
  }

  current = skiplist->headpointers[0];

  while(current != NULL)
  {
    if(action(current->key, i++, actinfo) < 0)
    {
      ERROR0("Call of action function returned an error.");
    }
    current = (Skipnode*) current->forward[0];
  }
} 

/*EE 
  Print a graphical representation of the list showing the levels of the
  nodes. Not recommended for lists with more than about 10 elements.
*/
void skiplistprint(Skiplist* skiplist, Skipshowelem showfun, void* showinfo)
{
  Skipnode *current;
  Uint i;

  /* check input parameters */
  if(skiplist == NULL)
  {
    ERROR0("Could not print skiplist. List handle is NULL.\n");
    return;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not print skiplist. Headpointers is NULL.\n");
    return;
  }
  if(showfun == NULL)
  {
    ERROR0("Could not print skiplist. Show function is NULL\n");
    return;
  }

  if(skiplist->headpointers[0] == NULL)
  {
    printf("Skiplist is empty\n");
    return;
  }

  for(i = skiplist->maxnodelevel; i > 0; i--)
  {
    current = skiplist->headpointers[0];    
    while(current != NULL)
    {
      if(current->nodelevel >= i) printf("[] "); else printf("   ");
      current = (Skipnode*) current->forward[0];
    }
    printf("\n");
  }

  current = skiplist->headpointers[0];    
  while(current != NULL)
  {
    showfun(current->key, showinfo);
    current = (Skipnode*) current->forward[0];
  }
  printf("\n");
}

/*EE 
  Print the keys stored in the list in sorted order. 
*/
void skiplistprintvalues(Skiplist* skiplist, 
                         Skipshowelem showfun, void* showinfo)
{
  Skipnode *current;

  /* check input parameters */
  if(skiplist == NULL)
  {
    ERROR0("Could not print values in skiplist. List handle is NULL.\n");
    return;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not print values in skiplist. Headpointers is NULL.\n");
    return;
  }
  if(showfun == NULL)
  {
    ERROR0("Could not print values in skiplis. Show function is NULL\n");
    return;
  }

  current = skiplist->headpointers[0];
  if(current == NULL)
  {
    printf("Skiplist is empty\n");
    return;
  }

  while(current != NULL)
  {
    showfun(current->key, showinfo);
    current = (Skipnode*) current->forward[0];
  }
  printf("\n");  
}


/*EE
  Print nodelevel statistics for the given list.
*/

void skipliststatistics(Skiplist* skiplist)
{
  Uint      i,
            nodes = 0,           // count number of nodes
            maxlevel,            // maximal possible node level
            currentlevel,        // current node level
            expected;            // expected number of nodes at current level
  Uint*     levelcount;          // array of node level frequencies
  Skipnode* currentelement;              // pointer to current skipnode
  double p; 
#ifndef NOSPACEBOOKKEEPING
  double spacepeak;                      
#endif
  
  /* check input parameters */
  if(skiplist == NULL)
  {
    ERROR0("Could not print skiplist. List handle is NULL.\n");
    return;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not print skiplist. Headpointers is NULL.\n");
    return;
  }

  maxlevel = skiplist->maxnodelevel;
  ALLOCASSIGNSPACE(levelcount, NULL, Uint, maxlevel);
  if(levelcount == NULL)
  {
    ERROR0("Could not allocate space for 'levelcount' array.\n");
    return;
  }
  
  /* init levelcount */
  for(i = 0; i < maxlevel; i++)
  {
    levelcount[i] = UintConst(0);
  }

  /* traverse list starting from beginning */
  currentelement = skiplist->headpointers[0];
  while(currentelement != NULL)
  {
    currentlevel = currentelement->nodelevel;
    if(currentlevel == 0)
    {
      ERROR0("Nodelevel 0 found.\n");
      return;
    }    
    if(currentlevel > maxlevel)
    {
      ERROR0("Node has level greater than maximal node level.\n");
      return;
    }
    /* now 0 < nodelevel <= maxlevel */

    levelcount[currentlevel-1]++;
    nodes++;

    currentelement = currentelement->forward[0];
  } /* end while */
  
  if(nodes != skiplist->currentlistsize)
  {
    ERROR0("Reported list size differs from actual number of elements.\n");
  }

  /* print statistics */
  printf("-----------------------------------------------------\n");
  printf("Total nodes:   %5lu\n", (Showuint) nodes);
  printf("Max list size: %5lu\n", (Showuint) skiplist->maxlistsize);
  p = 1.0;
  for(i = 0; i < maxlevel; i++)
  {
    p *= skiplist->p;
    expected = (Uint) rint(p * ((double) nodes));
    printf("Level %3lu : %4lu nodes. Expected: %4lu nodes.\n", 
            (Showuint) i+1, (Showuint) levelcount[i], (Showuint) expected);
  }
#ifndef NOSPACEBOOKKEEPING
  spacepeak = MEGABYTES(getspacepeak());
  printf("Spacepeak: %5.1f MB\n", spacepeak);
#endif
  printf("-----------------------------------------------------\n");

  FREESPACE(levelcount);
  
}

/*EE
  Destroy skiplist and free memory for its skipnodes. If 'dofreeskiplist' is
  true, the memory for the skiplist, the headpointers and the tracepointers
  is freed as well. Accordingly, if 'dofreekeys' is true, the memory for
  the skipkeys is freed. Otherwise, freeing the respective memory has to
  be taken care of by the calling program. The memory for the skipnodes
  is always freed.
*/
void skiplistdestroy(Skiplist *skiplist, BOOL dofreeskiplist, BOOL dofreekeys)
{
  Skipnode *next, *remove_node;

  if(skiplist == NULL)
  {
    ERROR0("Could not destroy skiplist. Handle is NULL.\n");
    return;
  }
  if(skiplist->headpointers == NULL)
  {
    ERROR0("Could not free memory for skiplist elements (Headpointer=NULL).\n");
  } else
  { 
    next = skiplist->headpointers[0];
    while(next != NULL)
    {
      remove_node= next;
      next = (Skipnode*) remove_node->forward[0];
      if(dofreekeys == True)
      {
      // free key
        FREESPACE(remove_node->key);
      }
      FREESPACE(remove_node->forward);
      FREESPACE(remove_node);
    }
    if(dofreeskiplist == True)
    {
      FREESPACE(skiplist->headpointers);
    }
  }
  if(dofreeskiplist == True)
  {
    FREESPACE(skiplist->tracepointers);
    FREESPACE(skiplist);
  }
}

