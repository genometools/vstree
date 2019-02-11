/* 
   This module is derived from the file tsearch.c implementing
   redblack trees. Some bugs and inconsistencies 
   have been fixed by Stefan Kurtz, <kurtz@zbh.uni-hamburg.de> and 
   some functions have been added.

   Copyright (C) 1995, 1996, 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Bernd Schmidt <crux@Pool.Informatik.RWTH-Aachen.DE>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  

   Tree search for red/black trees. The algorithm for adding nodes is taken
   from one of the many "Algorithms" books by Robert Sedgewick, although the
   implementation differs. The algorithm for deleting nodes can probably be
   found in a book named "Introduction to Algorithms" by
   Cormen/Leiserson/Rivest.  At least that's the book that my professor took
   most algorithms from during the "Data Structures" course...

   Red/black trees are binary trees in which the edges are colored either 
   red through or black. They have the following properties: 
   1. The number of black edges on every path from the root to 
      a leaf is constant. 
   2. No two red edges are adjacent. Therefore there is an upper bound 
      on the length of every path, it's O(log n) where n is the number 
      of nodes in the tree. No path can be longer than 1+2*P where P 
      is the length of the shortest path in the tree. Useful for the 
      implementation: 
   3. If one of the children of a node is NULL, then the other one 
      is red (if it exists).
   
   In the implementation, not the edges are colored, but the nodes.
   The color interpreted as the color of the edge leading to this node.
   The color is meaningless for the root node, but we color the root 
   node black for convenience. All added nodes are red initially.
   
   Adding to a red/black tree is rather easy. The right place is searched 
   with a usual binary tree search. Additionally, whenever a node N is 
   reached that has two red successors, the successors are colored black 
   and the node itself colored red. This moves red edges up the tree 
   where they pose less of a problem once we get to really insert the 
   new node.  Changing N's color to red may violate rule 2, however, so 
   rotations may become necessary to restore the invariants. 
   Adding a new red leaf may violate the same rule, so
   afterwards an additional check is run and the tree possibly rotated.
   
   Deleting is hairy. There are mainly two nodes involved: the node to be 
   deleted * (n1), and another node that is to be unchained from the tree 
   (n2). If n1 has a successor (the node with a smallest key that is larger 
   than n1), then the
   successor becomes n2 and its contents are copied into n1, otherwise n1
   becomes n2. Unchaining a node may violate rule 1: if n2 is black, one
   subtree is missing one black edge afterwards.  The algorithm must try to
   move this error upwards towards the root, so that the subtree that 
   does not have enough black edges becomes the whole tree. 
   Once that happens, the error has disappeared. It may not be necessary 
   to go all the way up, since it is possible that rotations and 
   recoloring can fix the error before that.
   
   Although the deletion algorithm must walk upwards through the tree, 
   we do not store parent pointers in the nodes. Instead, delete allocates 
   a small array of parent pointers and fills it while descending the tree. 
   Since we know that the length of a path is O(log n), where n is the 
   number of nodes, this is likely to use less memory.

   Tree rotations look like this:
      A                C
     / \              / \
    B   C            A   G
   / \ / \  -->     / \
   D E F G         B   F
                  / \
                 D   E

   In this case, A has been rotated left. 
   This preserves the ordering of the binary tree.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define NDEBUG
#include <assert.h>

#include "redblackdef.h"

typedef struct node_t
{
  Keytype key;
  struct node_t *left;
  struct node_t *right;
  BOOL red;
} RBTnode;

static Uint redblacktreespace = 0, 
            redblacktreespacepeak = 0;

#define ADDREDBLACKTREESPACE(TYPE)\
        redblacktreespace += ((Uint) sizeof(TYPE));\
        if(redblacktreespacepeak < redblacktreespace)\
        {\
          redblacktreespacepeak = redblacktreespace;\
        }

#define SUBTRACTREDBLACKTREESPACE(TYPE)\
        redblacktreespace -= ((Uint) sizeof(TYPE))

#ifndef NDEBUG

/* 
   Routines to check tree invariants.  
*/

#define CHECK_TREE(a) check_tree(a)

static void check_tree_recurse (RBTnode *p,
                                Sint d_sofar,
                                Sint d_total)
{
  if (p == NULL)
  {
    assert (d_sofar == d_total);
    return;
  }
  check_tree_recurse (p->left, d_sofar + (Sint) (p->left && !p->left->red),
                      d_total);
  check_tree_recurse (p->right, 
                      d_sofar + (Sint) (p->right && !p->right->red),
                      d_total);
  if (p->left != NULL)
  {
    assert (!(p->left->red && p->red));
  }
  if (p->right != NULL)
  {
    assert (!(p->right->red && p->red));
  }
}

static void check_tree (RBTnode *root)
{
  Sint cnt = 0;
  RBTnode *p;

  if (root == NULL)
  {
    return;
  }
  root->red = False;
  for (p = root->left; p != NULL; p = p->left)
  {
    if(!p->red)
    {
      cnt++;
    }
  }
  check_tree_recurse (root, 0, cnt);
}

#else

#define CHECK_TREE(a)                  /* Nothing */

#endif    /* NDEBUG */

Uint getredblacktreespacepeak(void)
{
  return redblacktreespacepeak;
}

/*
  Possibly "split" a node with two red successors, 
  and/or fix up two red edges
  in a row.  ROOTP is a pointer to the lowest node we visited, PARENTP and
  GPARENTP pointers to its parent/grandparent.  P_R and GP_R contain the
  comparison values that determined which way was taken in the tree to reach
  ROOTP.  MODE is 1 if we need not do the split, but must check for two red
  edges between GPARENTP and ROOTP.
*/

static void maybe_split_for_insert (RBTnode **rootp,
                                    RBTnode **parentp,
                                    RBTnode **gparentp,
                                    Sint p_r,
                                    Sint gp_r,
                                    Sint mode)
{
  RBTnode *root = *rootp, **rp, **lp;

  rp = &(*rootp)->right;
  lp = &(*rootp)->left;

  /* See if we have to split this node (both successors red).  */
  if (mode == (Sint) 1 ||
      ((*rp) != NULL && (*lp) != NULL && (*rp)->red && (*lp)->red))
  {
    /* This node becomes red, its successors black.  */
    root->red = True;
    if (*rp != NULL)
    {
      (*rp)->red = False;
    }
    if (*lp != NULL)
    {
      (*lp)->red = False;
    }

    /*
     * If the parent of this node is also red, we have to do rotations.
     */
    if (parentp != NULL && (*parentp)->red)
    {
      RBTnode *gp = *gparentp, *p = *parentp;

      /*
       * There are two main cases: 1. The edge types (left or right) of the two
       * red edges differ. 2. Both red edges are of the same type. There exist
       * two symmetries of each case, so there is a total of 4 cases.
       */
      if ((p_r > 0) != (gp_r > 0))
      {
        /*
         * Put the child at the top of the tree, with its parent and
         * grandparent as successors.
         */
        p->red = True;
        gp->red = True;
        root->red = False;
        if (p_r < 0)
        {
          /* Child is left of parent.  */
          p->left = *rp;
          *rp = p;
          gp->right = *lp;
          *lp = gp;
        }
        else
        {
          /* Child is right of parent.  */
          p->right = *lp;
          *lp = p;
          gp->left = *rp;
          *rp = gp;
        }
        *gparentp = root;
      }
      else
      {
        *gparentp = *parentp;
        /*
         * Parent becomes the top of the tree, grandparent and child are its
         * successors.
         */
        p->red = False;
        gp->red = True;
        if (p_r < 0)
        {
          /* Left edges.  */
          gp->left = p->right;
          p->right = gp;
        }
        else
        {
          /* Right edges.  */
          gp->right = p->left;
          p->left = gp;
        }
      }
    }
  }
}

/*
  Find or insert datum with given key into search tree. vrootp
  is the address of tree root, cmpfun the ordering function.
*/

/*@null@*/ Keytype redblacktreesearch (const Keytype key,
                                       BOOL *nodecreated,
                                       void **vrootp,
                                       Dictcomparefunction cmpfun,
                                       void *cmpinfo)
{
  RBTnode *newnode, 
          **parentp = NULL, 
          **gparentp = NULL,
          **rootp = (RBTnode **) vrootp, 
          **nextp;
  Sint r = 0, 
       p_r = 0, 
       gp_r = 0; /* No they might not, Mr Compiler. */

  if (rootp == NULL)
  {
    *nodecreated = False;
    return Keytypeerror;
  }

  /* This saves some additional tests below.  */
  if (*rootp != NULL)
  {
    (*rootp)->red = False;
  }

  CHECK_TREE (*rootp);

  nextp = rootp;
  while (*nextp != NULL)
  {
    RBTnode *root = *rootp;

    r = cmpfun (key, root->key, cmpinfo);
    if (r == 0)
    {
      *nodecreated = False;
      return root->key;
    }

    maybe_split_for_insert (rootp, parentp, gparentp, p_r, gp_r, 0);
    /*
     * If that did any rotations, parentp and gparentp are now garbage. That
     * doesn't matter, because the values they contain are never used again in
     * that case.
     */

    nextp = r < 0 ? &root->left : &root->right;
    if (*nextp == NULL)
    {
      break;
    }

    gparentp = parentp;
    parentp = rootp;
    rootp = nextp;

    gp_r = p_r;
    p_r = r;
  }

  newnode = (RBTnode *) malloc (sizeof (RBTnode));
  ADDREDBLACKTREESPACE(RBTnode);
  if(newnode == NULL)
  {
    fprintf(stderr,"file %s, line %lu: malloc(%lu bytes) failed: %s\n",
                   __FILE__,
                   (Showuint) __LINE__,
                   (Showuint) sizeof(RBTnode),
                   strerror(errno));
    exit(EXIT_FAILURE);
  } 
  *nextp = newnode;                        /* link new node to old */
  newnode->key = key;                      /* initialize new node */
  newnode->red = True;
  newnode->left = NULL;
  newnode->right = NULL;
  if (nextp != rootp)
  {
/*
  There may be two red edges in a row now, which we must 
  avoid by rotating the tree.
*/
    maybe_split_for_insert (nextp, rootp, parentp, r, p_r, (Sint) 1);
  }
  *nodecreated = True;
  return newnode->key;
}

/*
  Find datum in search tree. KEY is the key to be located, ROOTP is the
  address of tree root, cmpfun the ordering function.
*/

/*@null@*/ Keytype redblacktreefind (const Keytype key,
                                     void **vrootp,
                                     Dictcomparefunction cmpfun,
                                     void *cmpinfo)
{
  RBTnode **rootp = (RBTnode **) vrootp;

  if (rootp == NULL)
  {
    return Keytypeerror;
  }

  CHECK_TREE (*rootp);

  while (*rootp != NULL)
  {
    RBTnode *root = *rootp;
    Sint r;

    r = cmpfun (key, root->key, cmpinfo);
    if (r == 0)
    {
      return root->key;
    }
    if(r < 0)
    {
      rootp = &root->left;
    } else
    {
      rootp = &root->right;
    }
  }
  return Keytypeerror;
}

/*
  Delete node with given key. vrootp is the
  address of the root of tree, cmpfun the comparison function.
*/

#define STATICSTACKSPACE    UintConst(32)
#define STACKSPACEINCREMENT UintConst(16)

#define CHECKNODEREALLOC\
        if(nodestack == NULL)\
        {\
          fprintf(stderr,"file \"%s\", line %lu: realloc(%lu) failed: %s\n",\
                       __FILE__,\
                       (Showuint) __LINE__,\
                       (Showuint) allocsize,\
                       strerror(errno));\
          exit(EXIT_FAILURE);\
        }

#define CHECKNODESTACKSPACE\
        if (nextfreestack == stacksize)\
        {\
          allocsize = sizeof(RBTnode **) * \
                      (stacksize + STACKSPACEINCREMENT);\
          nodestack = realloc((stacksize == STATICSTACKSPACE) ? NULL :\
                                                                nodestack,\
                              allocsize);\
          CHECKNODEREALLOC;\
          ADDREDBLACKTREESPACE(RBTnode **);\
          if(stacksize == STATICSTACKSPACE)\
          {\
            memcpy(nodestack,&staticstack[0],\
                   sizeof(RBTnode **) * STATICSTACKSPACE);\
          }\
          stacksize += STACKSPACEINCREMENT;\
        }

#define DELETENODESTACKSPACE\
        if(stacksize > STATICSTACKSPACE)\
        {\
          free(nodestack);\
        }

Sint redblacktreedelete (const Keytype key,
                         void **vrootp,
                         Dictcomparefunction cmpfun,
                         void *cmpinfo)
{
  RBTnode *p, 
          *q, 
          *r, 
          **rootp = (RBTnode **) vrootp,
          *root, 
          *unchained, 
          ***nodestack, 
          **staticstack[STATICSTACKSPACE];
  size_t allocsize;
  Sint cmp;
  Uint stacksize = STATICSTACKSPACE, 
       nextfreestack = 0;

  p = *rootp;
  if (p == NULL)
  {
    return (Sint) -1;
  }

  CHECK_TREE (p);

  nodestack = &staticstack[0];
  while ((cmp = cmpfun (key, (*rootp)->key, cmpinfo)) != 0)
  {
    CHECKNODESTACKSPACE;
    nodestack[nextfreestack++] = rootp;
    p = *rootp;
    if(cmp < 0)
    {
      rootp = &(*rootp)->left;
    } else
    {
      rootp = &(*rootp)->right;
    }
    if (*rootp == NULL)
    {
      DELETENODESTACKSPACE;
      return (Sint) -1;
    }
  }

  /*
    We don't unchain the node we want to delete. 
    Instead, we overwrite it with
    its successor and unchain the successor.  If there is no successor, we
    really unchain the node to be deleted.
  */

  root = *rootp;

  r = root->right;
  q = root->left;

  if (q == NULL || r == NULL)
  {
    unchained = root;
  } else
  {
    RBTnode **parent = rootp, 
            **up = &root->right;

    for (;;)
    {
      CHECKNODESTACKSPACE;
      nodestack[nextfreestack++] = parent;
      parent = up;
      if ((*up)->left == NULL)
      {
        break;
      }
      up = &(*up)->left;
    }
    unchained = *up;
  }

  /*
   * We know that either the left or right successor of UNCHAINED is NULL. R
   * becomes the other one, it is chained into the parent of UNCHAINED.
   */
  r = unchained->left;
  if (r == NULL)
  {
    r = unchained->right;
  }
  if (nextfreestack == 0)
  {
    *rootp = r;
  }
  else
  {
    q = *nodestack[nextfreestack - 1];
    if (unchained == q->right)
    {
      q->right = r;
    }
    else
    {
      q->left = r;
    }
  }

  if (unchained != root)
  {
    root->key = unchained->key;
  }
  if (!unchained->red)
  {
    /*
      Now we lost a black edge, which means that the number of 
      black edges on every path is no longer constant.  
      We must balance the tree.
    
      NODESTACK now contains all parents of R.  
      R is likely to be NULL in the
      first iteration.
    
      NULL nodes are considered black throughout - this is necessary for
      correctness.
    */
    while (nextfreestack > 0 && (r == NULL || !r->red))
    {
      RBTnode **pp = nodestack[nextfreestack - 1];

      p = *pp;
      /* Two symmetric cases.  */
      if (r == p->left)
      {
        /*
         * Q is R's brother, P is R's parent.  The subtree with root R has one
         * black edge less than the subtree with root Q.
         */
        q = p->right;
        if (q != NULL && q->red)
        {
          /*
           * If Q is red, we know that P is black. We rotate P left so that Q
           * becomes the top node in the tree, with P below it.  P is colored
           * red, Q is colored black. This action does not change the black
           * edge count for any leaf in the tree, but we will be able to
           * recognize one of the following situations, which all require that
           * Q is black.
           */
          q->red = False;
          p->red = True;
          /* Left rotate p.  */
          p->right = q->left;
          q->left = p;
          *pp = q;
          /*
           * Make sure pp is right if the case below tries to use it.
           */
          CHECKNODESTACKSPACE;  /* this has been added by S.K. */
          nodestack[nextfreestack++] = pp = &q->left;
          q = p->right;
        }
        assert(q != NULL);
        /*
         * We know that Q can't be NULL here.  We also know that Q is black.
         */
        if ((q->left == NULL || !q->left->red) && 
            (q->right == NULL || !q->right->red))
        {
          /*
           * Q has two black successors.  We can simply color Q red. The whole
           * subtree with root P is now missing one black edge. Note that this
           * action can temporarily make the tree invalid (if P is red).  But
           * we will exit the loop in that case and set P black, which both
           * makes the tree valid and also makes the black edge count come out
           * right.  If P is black, we are at least one step closer to the root
           * and we'll try again the next iteration.
           */
          q->red = True;
          r = p;
        }
        else
        {
          /*
           * Q is black, one of Q's successors is red.  We can repair the tree
           * with one operation and will exit the loop afterwards.
           */
          if (q->right == NULL || !q->right->red)
          {
            /*
             * The left one is red.  We perform the same action as in
             * maybe_split_for_insert where two red edges are adjacent but
             * point in different directions: Q's left successor (let's call it
             * Q2) becomes the top of the subtree we are looking at, its parent
             * (Q) and grandparent (P) become its successors. The former
             * successors of Q2 are placed below P and Q. P becomes black, and
             * Q2 gets the color that P had. This changes the black edge count
             * only for node R and its successors.
             */
            RBTnode *q2 = q->left;

            q2->red = p->red;
            p->right = q2->left;
            
            q->left = q2->right;
            q2->right = q;
            q2->left = p;
            *pp = q2;
            p->red = False;
          }
          else
          {
            /*
             * It's the right one.  Rotate P left. P becomes black, and Q gets
             * the color that P had.  Q's right successor also becomes black.
             * This changes the black edge count only for node R and its
             * successors.
             */
            q->red = p->red;
            p->red = False;

            q->right->red = False;

            /* left rotate p */
            p->right = q->left;
            q->left = p;
            *pp = q;
          }

          /* We're done.  */
          nextfreestack = UintConst(1);
          r = NULL;
        }
      }
      else
      {
        /* Comments: see above.  */
        q = p->left;
        if (q != NULL && q->red)
        {
          q->red = False;
          p->red = True;
          p->left = q->right;
          q->right = p;
          *pp = q;
          CHECKNODESTACKSPACE;  /* this has been added by S.K. */
          nodestack[nextfreestack++] = pp = &q->right;
          q = p->left;
        }
        assert(q != NULL);
        if ((q->right == NULL || !q->right->red)
            && (q->left == NULL || !q->left->red))
        {
          q->red = True;
          r = p;
        }
        else
        {
          if (q->left == NULL || !q->left->red)
          {
            RBTnode *q2 = q->right;

            q2->red = p->red;
            p->left = q2->right;
            q->right = q2->left;
            q2->left = q;
            q2->right = p;
            *pp = q2;
            p->red = False;
          }
          else
          {
            q->red = p->red;
            p->red = False;
            q->left->red = False;
            p->left = q->right;
            q->right = p;
            *pp = q;
          }
          nextfreestack = UintConst(1);
          r = NULL;
        }
      }
      --nextfreestack;
    }
    if (r != NULL)
    {
      r->red = False;
    }
  }
  free (unchained);
  SUBTRACTREDBLACKTREESPACE(RBTnode);
  DELETENODESTACKSPACE;
  return 0;
}

/*
  Walk the nodes of a tree. ROOT is the root of the tree to be walked, 
  ACTION the function to be called at each node. LEVEL is the level of 
  ROOT in the whole tree.
*/

static Sint mytreerecurse (void *vroot,
                           Dictaction action,
                           Uint level,
                           void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  if (root->left == NULL && root->right == NULL)
  {
    if(action (root->key, leaf, level, actinfo) != 0)
    {
      return (Sint) -1;
    }
  }
  else
  {
    if(action (root->key, preorder, level, actinfo) != 0)
    {
      return (Sint) -2;
    }
    if (root->left != NULL)
    {
      if(mytreerecurse (root->left, action, level + UintConst(1), 
                        actinfo) != 0)
      {
        return (Sint) -3;
      }
    }
    if(action (root->key, postorder, level, actinfo) != 0)
    {
      return (Sint) -4;
    }
    if (root->right != NULL)
    {
      if(mytreerecurse (root->right, action, level + UintConst(1), 
                        actinfo) != 0)
      {
        return (Sint) -5;
      }
    }
    if(action (root->key, endorder, level, actinfo) != 0)
    {
      return (Sint) -6;
    }
  }
  return 0;
}

static Sint mytreerecursewithstop (void *vroot,
                                   Dictaction action,
                                   Uint level,
                                   void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;
  Sint retcode;

  if (root->left == NULL && root->right == NULL)
  {
    retcode = action (root->key, leaf, level, actinfo);
    CHECKREDBLACKRETCODE;
  }
  else
  {
    retcode = action (root->key, preorder, level, actinfo);
    CHECKREDBLACKRETCODE;
    if (root->left != NULL)
    {
      retcode = mytreerecursewithstop (root->left, action, 
                                       level + UintConst(1), 
                                       actinfo);
      CHECKREDBLACKRETCODE;
    }
    retcode = action (root->key, postorder, level, actinfo);
    CHECKREDBLACKRETCODE;
    if (root->right != NULL)
    {
      retcode = mytreerecursewithstop (root->right, action, 
                                       level + UintConst(1), 
                                       actinfo);
      CHECKREDBLACKRETCODE;
    }
    retcode = action (root->key, endorder, level, actinfo);
    CHECKREDBLACKRETCODE;
  }
  return 0;
}

static Sint mytreerecursereverseorder (void *vroot,
                                   Dictaction action,
                                   Uint level,
                                   void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  if (root->left == NULL && root->right == NULL)
  {
    if(action (root->key, leaf, level, actinfo) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(action (root->key, preorder, level, actinfo) != 0)
    {
      return (Sint) -2;
    }
    if (root->right != NULL)
    {
      if(mytreerecursereverseorder (root->right, action, level + UintConst(1), 
                                 actinfo) != 0)
      {
        return (Sint) -3;
      }
    }
    if(action (root->key, postorder, level, actinfo) != 0)
    {
      return (Sint) -4;
    }
    if (root->left != NULL)
    {
      if(mytreerecursereverseorder (root->left, action, level + UintConst(1), 
                                 actinfo) != 0)
      {
        return (Sint) -5;
      }
    }
    if(action (root->key, endorder, level, actinfo) != 0)
    {
      return (Sint) -6;
    }
  }
  return 0;
}


/*
  Walk the nodes of a tree. ROOT is the root of the tree to be walked, ACTION
  the function to be called at each node.
*/

Sint redblacktreewalk (void *vroot,
                       Dictaction action,
                       void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  CHECK_TREE (root);

  if (root != NULL && action != NULL)
  {
    if(mytreerecurse (root, action, 0, actinfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

Sint redblacktreewalkwithstop (void *vroot,
                               Dictaction action,
                               void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  CHECK_TREE (root);

  if (root != NULL && action != NULL)
  {
    Sint retcode = mytreerecursewithstop (root, action, 0, actinfo);
    CHECKREDBLACKRETCODE;
  }
  return 0;
}

Sint redblacktreewalkreverseorder (void *vroot,
                                   Dictaction action,
                                   void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  CHECK_TREE (root);

  if (root != NULL && action != NULL)
  {
    if(mytreerecursereverseorder (root, action, 0, actinfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

/*
  find minimum key
*/

/*@null@*/ Keytype redblacktreeminimumkey (void *vroot)
{
  RBTnode *root = (RBTnode *) vroot;

  if(root == NULL)
  {
    return Keytypeerror;
  }
  while(root->left != NULL)
  {
    root = root->left;
  }
  return root->key;
}

/*
  find maximum key
*/

/*@null@*/ Keytype redblacktreemaximumkey (void *vroot)
{
  RBTnode *root = (RBTnode *) vroot;

  if(root == NULL)
  {
    return Keytypeerror;
  }
  while(root->right != NULL)
  {
    root = root->right;
  }
  return root->key;
}

void treeshape (void *root,Uint level)
{
  RBTnode *vroot = (RBTnode *) root;
  
  printf("visit node %lu at level %lu\n",
          (Showuint) *((Uint *) vroot->key),(Showuint) level);
  if(vroot->left != NULL)
  {
    printf("visit left branch\n");
    treeshape(vroot->left,level+1);
  }
  if(vroot->right != NULL)
  {
    printf("visit right branch\n");
    treeshape(vroot->right,level+1);
  }
  printf("bracktrack\n");
}

/*
  find largest element strictly smaller than key
*/

/*@null@*/ Keytype redblacktreepreviouskey (const Keytype key, 
                                            void *vroot, 
                                            Dictcomparefunction cmpfun,
                                            void *cmpinfo)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = cmpfun (key, current->key, cmpinfo);
    if (cmp == 0)
    {
      if (current->left == NULL)
      {
        if(found == NULL)
        {
          return Keytypeerror;
        }
        return found->key;
      } 
      return redblacktreemaximumkey (current->left);
    } else
    {
      if (cmp < 0)
      {
        current = current->left;
      } else
      {
        found = current;
        current = current->right;
      }
    }
  }
  if(found == NULL)
  {
    return Keytypeerror;
  }
  return found->key;
}

/*
  find largest element smaller than or equal to the key
*/


/*@null@*/ Keytype redblacktreepreviousequalkey (const Keytype key, 
                                                 void *vroot, 
                                                 Dictcomparefunction cmpfun,
                                                 void *cmpinfo)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = cmpfun (key, current->key, cmpinfo);
    if (cmp == 0)
    {
      return current->key;
    } else
    {
      if (cmp < 0)
      {
        current = current->left;
      } else
      {
        found = current;
        current = current->right;
      }
    }
  }
  if(found == NULL)
  {
    return Keytypeerror;
  }
  return found->key;
}

/*
  find smallest element strictly larger than key
*/

/*@null@*/ Keytype redblacktreenextkey (const Keytype key, 
                                        void *vroot, 
                                        Dictcomparefunction cmpfun,
                                        void *cmpinfo)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = cmpfun (key, current->key, cmpinfo);
    if (cmp == 0)
    {
      if (current->right == NULL)
      {
        if(found == NULL)
        {
          return Keytypeerror;
        }
        return found->key;
      }
      return redblacktreeminimumkey (current->right);
    } else
    {
      if (cmp < 0)
      {
        found = current;
        current = current->left;
      } else 
      {
        current = current->right;
      }
    }
  }
  if(found == NULL)
  {
    return Keytypeerror;
  }
  return found->key;
}

/*@null@*/ Keytype redblacktreenextequalkey (const Keytype key,
                                             void *vroot,
                                             Dictcomparefunction cmpfun,
                                             void *cmpinfo)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = cmpfun (key, current->key, cmpinfo);
    if (cmp == 0)
    {
      return current->key;
    } else
    {
      if (cmp < 0)
      {
        found = current;
        current = current->left;
      } else
      {
        current = current->right;
      }
    }
  }
  if(found == NULL)
  {
    return Keytypeerror;
  }
  return found->key;
}

/*
  The standardized functions miss an important functionality: 
  the tree cannot be removed easily. We provide a function to do this.
  If the boolean freekey is true, then also the key field is freed.
  This only works if key points to a malloced block.
*/

static void redblacktreedestroyrecurse (BOOL dofreekey,
                                        Freekeyfunction freekey,
                                        void *freeinfo,
                                        RBTnode *root)
{
  if (root->left != NULL)
  {
    redblacktreedestroyrecurse (dofreekey,freekey,freeinfo,root->left);
  }
  if (root->right != NULL)
  {
    redblacktreedestroyrecurse (dofreekey,freekey,freeinfo,root->right);
  }
  if(dofreekey)
  {
    if(freekey == NULL)
    {
      free ((void *) root->key);
    } else
    {
      freekey (root->key,freeinfo);
    }
  }
  /* Free the node itself.  */
  free (root);
  SUBTRACTREDBLACKTREESPACE(RBTnode);
}

void redblacktreedestroy (BOOL dofreekey,
                          Freekeyfunction freekey,
                          void *freeinfo,
                          void *vroot)
{
  RBTnode *root = (RBTnode *) vroot;

  CHECK_TREE (root);

  if (root != NULL)
  {
    redblacktreedestroyrecurse (dofreekey,freekey,freeinfo,root);
  }
}

/*@null@*/ Keytype extractrootkey (void *vroot)
{
  RBTnode *root = (RBTnode *) vroot;

  if(root == NULL)
  {
    return Keytypeerror;
  }
  return root->key;
}

static Sint rangetreerecurse (void *vroot,
                              Dictaction action,
                              Uint level,
                              void *actinfo,
                              Comparewithkey greaterequalleft,
                              Comparewithkey lowerequalright,
                              void *cmpinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  if (root->left == NULL && root->right == NULL)
  {
    if(greaterequalleft(root->key,cmpinfo) && 
       lowerequalright(root->key,cmpinfo))
    {
      if(action (root->key, leaf, level, actinfo) != 0)
      {
        return (Sint) -1;
      }
    }
  } else
  {
    if(root->left != NULL)
    {
      if(greaterequalleft(root->key,cmpinfo))
      {
        if(rangetreerecurse (root->left, action, level + UintConst(1), 
                             actinfo, greaterequalleft, lowerequalright,
                             cmpinfo) != 0)
        {
          return (Sint) -2;
        }
      }
    }
    if(greaterequalleft(root->key,cmpinfo) && 
       lowerequalright(root->key,cmpinfo))
    {
      if(action (root->key, postorder, level, actinfo) != 0)
      {
        return (Sint) -3;
      }
    }
    if(root->right != NULL)
    {
      if(lowerequalright(root->key,cmpinfo))
      {
        if(rangetreerecurse (root->right, action, level + UintConst(1), 
                             actinfo, greaterequalleft, lowerequalright, 
                             cmpinfo) != 0)
        {
          return (Sint) -4;
        }
      }
    }
  }
  return 0;
}

Sint redblacktreewalkrange (void *vroot,
                            Dictaction action,
                            void *actinfo,
                            Comparewithkey greaterequalleft,
                            Comparewithkey lowerequalright,
                            void *cmpinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  CHECK_TREE (root);

  if (root != NULL && action != NULL)
  {
    if(rangetreerecurse (root, action, 0, actinfo, 
                         greaterequalleft, lowerequalright,
                         cmpinfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}
