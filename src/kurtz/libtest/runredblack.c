#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "redblackdef.h"

#include "redblack.pr"

#define SEED 0

#define PASSES 100

#define SIZE 100

/* protoypes taken from man initstate */
long random(void);
#ifndef NOINITSTATE
char *initstate( unsigned seed, char *state, size_t n);
#endif

typedef enum
{
  Ascending,
  Descending,
  Randomorder
} SearchOrder;

typedef enum
{
  Build,
  Build_and_del,
  Delete,
  Find
} DoAction;

/* Set to 1 if a test is flunked.  */
static Sint      error = 0;

/* The keys we add to the tree.  */
static Sint      x[SIZE];

/*
 * Pointers into the key array, possibly permutated, to define an order for
 * insertion/removal.
 */
static Sint      y[SIZE];

/* Flags set for each element visited during a tree walk.  */
static Sint      z[SIZE];

/*
 * Depths for all the elements, to check that the depth is constant for all
 * three visits.
 */
static Uint      depths[SIZE];

/* Maximum depth during a tree walk.  */
static Uint      max_depth;

/* Compare two keys.  */
static Sint cmp_fn(const Keytype a, const Keytype b, /*@unused@*/ void *info)
{
  return *(Sint *) a - *(Sint *) b;
}

/* Permute an array of integers.  */
static void memfry(Sint *string)
{
  Sint             i;

  for (i = 0; i < SIZE; ++i)
  {
    Sint         j, c;

    j = (Sint) (random() % SIZE);

    c = string[i];
    string[i] = string[j];
    string[j] = c;
  }
}

static Sint walk_action(const Keytype nodekey, 
                        const VISIT which, 
                        const Uint depth, 
                        /*@unused@*/ void *actinfo)
{
  Sint             key = *(Sint *) nodekey;

  if (depth > max_depth)
    max_depth = depth;
  if (which == leaf || which == preorder)
  {
    ++z[key];
    depths[key] = depth;
  } else
  {
    if (depths[key] != depth)
    {
      (void) fputs("Depth for one element is not constant during tree walk.\n",
	    stdout);
    }
  }
  return 0;
}

static void walk_tree(void *root, Sint expected_count)
{
  Sint             i;

  memset(z, 0, sizeof z);
  max_depth = 0;

  if(redblacktreewalk(root, walk_action,NULL) != 0)
  {
    (void) fputs("walk failed\n", stdout);
    error = 1;
  }
  for (i = 0; i < expected_count; ++i)
    if (z[i] != 1)
    {
      (void) fputs("Node was not visited.\n", stdout);
      error = 1;
    }
  if (max_depth > (Uint) (log((double) expected_count) * 2.0 + 2.0))
  {
    (void) fputs("Depth too large during tree walk.\n", stdout);
    error = 1;
  }
}

/* Perform an operation on a tree.  */
static void mangle_tree(SearchOrder how, DoAction what, void **root, 
                        Sint lag)
{
  Sint             i;
  BOOL nodecreated;

  if (how == Randomorder)
  {
    for (i = 0; i < SIZE; ++i)
      y[i] = i;
    memfry(y);
  }
  for (i = 0; i < SIZE + lag; ++i)
  {
    void           *elem;
    Sint             j, k;

    switch (how)
    {
    case Randomorder:
      if (i >= lag)
	k = y[i - lag];
      else
	k = y[SIZE - i - 1 + lag];
      j = y[i];
      break;

    case Ascending:
      k = i - lag;
      j = i;
      break;

    case Descending:
      k = SIZE - i - 1 + lag;
      j = SIZE - i - 1;
      break;

    default:
      /*
       * This never should happen, but gcc isn't smart enough to recognize it.
       */
      abort();
    }

    switch (what)
    {
    case Build_and_del:
    case Build:
      if (i < SIZE)
      {
	if (redblacktreefind(x + j, (void **) root, cmp_fn, NULL) != NULL)
	{
	  (void) fputs("Found element which is not in tree yet.\n", stdout);
	  error = 1;
	}
	elem = redblacktreesearch(x + j, &nodecreated, root, cmp_fn, NULL);
	if (elem == NULL
	    || redblacktreefind(x + j, (void **) root, cmp_fn, NULL) == NULL)
	{
	  (void) fputs("Couldn't find element after it was added.\n",
		stdout);
	  error = 1;
	}
      }
      if (what == Build || i < lag)
	break;

      j = k;
      /* fall through */

    case Delete:
      elem = redblacktreefind(x + j,  (void **) root, cmp_fn, NULL);
      if (elem == NULL || 
          redblacktreedelete(x + j, root, cmp_fn, NULL) != 0)
      {
	(void) fputs("Error deleting element.\n", stdout);
	error = 1;
      }
      break;

    case Find:
      if (redblacktreefind(x + j, (void **) root, cmp_fn, NULL) == NULL)
      {
	(void) fputs("Couldn't find element after it was added.\n", stdout);
	error = 1;
      }
      break;

    }
  }
}

MAINFUNCTIONWITHUNUSEDARGUMENTS
{
  Sint            total_error = 0;
  static char     state[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  void            *root = NULL;
  Sint             i, j;

  (void) initstate(SEED, &state[0], (size_t) 8);

  for (i = 0; i < SIZE; ++i)
    x[i] = i;

  /*
   * Do this loop several times to get different permutations for the random
   * case.
   */
  (void) fputs("Series I\n", stdout);
  for (i = 0; i < PASSES; ++i)
  {
    printf("Pass %ld... ",(Showsint) (i + 1));
    (void) fflush(stdout);
    error = 0;

    mangle_tree(Ascending, Build, &root, 0);
    mangle_tree(Ascending, Find, &root, 0);
    mangle_tree(Descending, Find, &root, 0);
    mangle_tree(Randomorder, Find, &root, 0);
    walk_tree(root, SIZE);
    mangle_tree(Ascending, Delete, &root, 0);

    mangle_tree(Ascending, Build, &root, 0);
    walk_tree(root, SIZE);
    mangle_tree(Descending, Delete, &root, 0);

    mangle_tree(Ascending, Build, &root, 0);
    walk_tree(root, SIZE);
    mangle_tree(Randomorder, Delete, &root, 0);

    mangle_tree(Descending, Build, &root, 0);
    mangle_tree(Ascending, Find, &root, 0);
    mangle_tree(Descending, Find, &root, 0);
    mangle_tree(Randomorder, Find, &root, 0);
    walk_tree(root, SIZE);
    mangle_tree(Descending, Delete, &root, 0);

    mangle_tree(Descending, Build, &root, 0);
    walk_tree(root, SIZE);
    mangle_tree(Descending, Delete, &root, 0);

    mangle_tree(Descending, Build, &root, 0);
    walk_tree(root, SIZE);
    mangle_tree(Randomorder, Delete, &root, 0);

    mangle_tree(Randomorder, Build, &root, 0);
    mangle_tree(Ascending, Find, &root, 0);
    mangle_tree(Descending, Find, &root, 0);
    mangle_tree(Randomorder, Find, &root, 0);
    walk_tree(root, SIZE);
    mangle_tree(Randomorder, Delete, &root, 0);

    for (j = 1; j < SIZE; j *= 2)
    {
      mangle_tree(Randomorder, Build_and_del, &root, j);
    }

    (void) fputs(error ? " failed!\n" : " ok.\n", stdout);
    total_error |= error;
  }

  (void) fputs("Series II\n", stdout);
  for (i = 1; i < SIZE; i *= 2)
  {
    fprintf(stdout, "For size %ld... ", (Showsint) i);
    (void) fflush(stdout);
    error = 0;

    mangle_tree(Ascending, Build_and_del, &root, i);
    mangle_tree(Descending, Build_and_del, &root, i);
    mangle_tree(Ascending, Build_and_del, &root, i);
    mangle_tree(Descending, Build_and_del, &root, i);
    mangle_tree(Ascending, Build_and_del, &root, i);
    mangle_tree(Descending, Build_and_del, &root, i);
    mangle_tree(Ascending, Build_and_del, &root, i);
    mangle_tree(Descending, Build_and_del, &root, i);

    (void) fputs(error ? " failed!\n" : " ok.\n", stdout);
    total_error |= error;
  }

  return total_error;
}
