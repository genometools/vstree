#ifdef DEBUG
#define SETRED(N)           setcolor(True,N)
#define SETBLACK(N)         setcolor(False,N)
#define ISRED(N)            checkredflag((Uint) __LINE__,N)
#else
#define SETRED(N)           (N)->key.STRUCTURECOMPONENTWITHBITS |= REDBIT
#define SETBLACK(N)         (N)->key.STRUCTURECOMPONENTWITHBITS &= ~REDBIT;
#define ISRED(N)            ((N)->key.STRUCTURECOMPONENTWITHBITS & REDBIT)
#endif

#ifdef NOSPACEBOOKKEEPING
#define ADDSPACE(SPACE) /* Nothing */
#define SUBTRACTSPACE(SPACE) /* Nothing */
#else
void addspace(Uint space);
void subtractspace(Uint space);
#define ADDSPACE(SPACE) addspace(SPACE)
#define SUBTRACTSPACE(SPACE) subtractspace(SPACE)
#endif

#define CHECKREDBLACKRETCODE\
        if(retcode < 0 || retcode == SintConst(1))\
        {\
          return retcode;\
        }

static Uint redblacktreespace = 0,
            redblacktreespacepeak = 0;

#ifdef DEBUG
static void setcolor(BOOL red,RBTnode *node)
{
  if(red)
  {
    node->red = True; 
    node->key.STRUCTURECOMPONENTWITHBITS |= REDBIT;
  } else
  {
    node->red = False; 
    node->key.STRUCTURECOMPONENTWITHBITS &= ~REDBIT;
  }
}

static BOOL checkredflag(Uint linenum,const RBTnode *node)
{
  if(node->red && !(node->key.STRUCTURECOMPONENTWITHBITS & REDBIT))
  {
    fprintf(stderr,"%lu: ",(Showuint) linenum);
    fprintf(stderr,"inconsistent color: really red\n");
    exit(EXIT_FAILURE);
  }
  if(!node->red && (node->key.STRUCTURECOMPONENTWITHBITS & REDBIT))
  {
    fprintf(stderr,"%lu: ",(Showuint) linenum);
    fprintf(stderr,"inconsistent color: really black\n");
    exit(EXIT_FAILURE);
  }
  return node->red;
}

/* 
   Routines to check tree invariants.  
*/

#include <assert.h>

#define CHECKTREE(N) checktree(N)

static void checktreerecurse(RBTnode *p,
                             Sint d_sofar,
                             Sint d_total)
{
  if (p == NULL)
  {
    assert (d_sofar == d_total);
    return;
  }
  checktreerecurse(p->left, 
                   d_sofar + (Sint) (p->left && !ISRED(p->left)),
                   d_total);
  checktreerecurse(p->right, 
                   d_sofar + (Sint) (p->right && !ISRED(p->right)),
                   d_total);
  if (p->left != NULL)
  {
    /*@ignore@*/
    assert (!(ISRED(p->left) && ISRED(p)));
    /*@end@*/
  }
  if (p->right != NULL)
  {
    /*@ignore@*/
    assert (!(ISRED(p->right) && ISRED(p)));
    /*@end@*/
  }
}

static void checktree(RBTnode *root)
{
  Sint cnt = 0;
  RBTnode *p;

  if (root == NULL)
  {
    return;
  }
  SETBLACK(root);
  for (p = root->left; p != NULL; p = p->left)
  {
    if(!ISRED(p))
    {
      cnt++;
    }
  }
  checktreerecurse(root, 0, cnt);
}

#else

#define CHECKTREE(N)                  /* Nothing */

#endif    /* DEBUG */

/*
  Possibly "split" a node with two red successors, 
  and/or fix up two red edges
  in a row.  ROOTP is a pointer to the lowest node we visited, PARENTP and
  GPARENTP pointers to its parent/grandparent.  P_R and GP_R contain the
  comparison values that determined which way was taken in the tree to reach
  ROOTP.  MODE is 1 if we need not do the split, but must check for two red
  edges between GPARENTP and ROOTP.
*/

static void maybesplitforinsert(RBTnode **rootp,
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
      ((*rp) != NULL && (*lp) != NULL && ISRED(*rp) && ISRED(*lp)))
  {
    /* This node becomes red, its successors black.  */
    SETRED(root);
    if (*rp != NULL)
    {
      SETBLACK(*rp);
    }
    if (*lp != NULL)
    {
      SETBLACK(*lp);
    }

    /*
     * If the parent of this node is also red, we have to do rotations.
     */
    if (parentp != NULL && ISRED(*parentp))
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
        SETRED(p);
        SETRED(gp);
        SETBLACK(root);
        if (p_r < 0)
        {
          /* Child is left of parent.  */
          p->left = *rp;
          *rp = p;
          gp->right = *lp;
          *lp = gp;
        } else
        {
          /* Child is right of parent.  */
          p->right = *lp;
          *lp = p;
          gp->left = *rp;
          *rp = gp;
        }
        *gparentp = root;
      } else
      {
        *gparentp = *parentp;
        /*
         * Parent becomes the top of the tree, grandparent and child are its
         * successors.
         */
        SETBLACK(p);
        SETRED(gp);
        if (p_r < 0)
        {
          /* Left edges.  */
          gp->left = p->right;
          p->right = gp;
        } else
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
  is the address of tree root.
*/

/*@null@*/ RBTKEY *RBTKEYSEARCH (const RBTKEY *key,BOOL *nodecreated,
                                 void **vrootp)
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
    return RBTKEYERROR;
  }

  /* This saves some additional tests below.  */
  if (*rootp != NULL)
  {
    SETBLACK(*rootp);
  }
  CHECKTREE (*rootp);

  nextp = rootp;
  while (*nextp != NULL)
  {
    RBTnode *root = *rootp;

    r = RBTKEYCOMPARE (key, &root->key);
    if (r == 0)
    {
      *nodecreated = False;
      return &root->key;
    }

    maybesplitforinsert(rootp, parentp, gparentp, p_r, gp_r, 0);
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
  if(newnode == NULL)
  {
    fprintf(stderr,"file %s, line %lu: malloc(%lu bytes) failed: %s\n",
                   __FILE__,
                   (Showuint) __LINE__,
                   (Showuint) sizeof(RBTnode),
                   strerror(errno));
    exit(EXIT_FAILURE);
  } 
  ADDSPACE((Uint) sizeof(RBTnode));
  redblacktreespace += ((Uint) sizeof(RBTnode));
  if(redblacktreespacepeak < redblacktreespace)
  {
    redblacktreespacepeak = redblacktreespace;
  }
  *nextp = newnode;                        /* link new node to old */
  newnode->key = *key;                     /* initialize new node */
  SETRED(newnode);
  newnode->left = NULL;
  newnode->right = NULL;
  if (nextp != rootp)
  {
/*
  There may be two red edges in a row now, which we must 
  avoid by rotating the tree.
*/
    maybesplitforinsert(nextp, rootp, parentp, r, p_r, (Sint) 1);
  }
  *nodecreated = True;
  return &newnode->key;
}

/*
  Find datum in search tree. KEY is the key to be located, vrootp is the
  address of tree root.
*/

/*@null@*/ RBTKEY *RBTKEYFIND (const RBTKEY *key, void **vrootp)
{
  RBTnode **rootp = (RBTnode **) vrootp;

  if (rootp == NULL)
  {
    return RBTKEYERROR;
  }

  CHECKTREE (*rootp);

  while (*rootp != NULL)
  {
    RBTnode *root = *rootp;
    Sint r;

    r = RBTKEYCOMPARE (key, &root->key);
    if (r == 0)
    {
      return &root->key;
    }
    if(r < 0)
    {
      rootp = &root->left;
    } else
    {
      rootp = &root->right;
    }
  }
  return RBTKEYERROR;
}

/*
  Delete node with given key. vrootp is the
  address of the root of tree.
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
          SUBTRACTSPACE(stacksize * sizeof(RBTnode **));\
          ADDSPACE(allocsize);\
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
          SUBTRACTSPACE(stacksize);\
        }

#define COPYCOLOR(P,Q)\
        if(ISRED(P))\
        {\
          SETRED(Q);\
        } else\
        {\
          SETBLACK(Q);\
        }

Sint RBTKEYDELETE (const RBTKEY *key,void **vrootp)
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

  CHECKTREE (p);

  nodestack = &staticstack[0];
  while ((cmp = RBTKEYCOMPARE (key, &((*rootp)->key))) != 0)
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
  if (!ISRED(unchained))
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
    while (nextfreestack > 0 && (r == NULL || !ISRED(r)))
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
        if (q != NULL && ISRED(q))
        {
          /*
           * If Q is red, we know that P is black. We rotate P left so that Q
           * becomes the top node in the tree, with P below it.  P is colored
           * red, Q is colored black. This action does not change the black
           * edge count for any leaf in the tree, but we will be able to
           * recognize one of the following situations, which all require that
           * Q is black.
           */
          SETBLACK(q);
          SETRED(p);
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
        NOTSUPPOSEDTOBENULL(q);
        /*
         * We know that Q can't be NULL here.  We also know that Q is black.
         */
        if ((q->left == NULL || !ISRED(q->left)) && 
            (q->right == NULL || !ISRED(q->right)))
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
          SETRED(q);
          r = p;
        }
        else
        {
          /*
           * Q is black, one of Q's successors is red.  We can repair the tree
           * with one operation and will exit the loop afterwards.
           */
          if (q->right == NULL || !ISRED(q->right))
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

            COPYCOLOR(q2,p);
            p->right = q2->left;
            
            q->left = q2->right;
            q2->right = q;
            q2->left = p;
            *pp = q2;
            SETBLACK(p);
          }
          else
          {
            /*
             * It's the right one.  Rotate P left. P becomes black, and Q gets
             * the color that P had.  Q's right successor also becomes black.
             * This changes the black edge count only for node R and its
             * successors.
             */
            COPYCOLOR(q,p);
            SETBLACK(p);
            SETBLACK(q->right);

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
        if (q != NULL && ISRED(q))
        {
          SETBLACK(q);
          SETRED(p);
          p->left = q->right;
          q->right = p;
          *pp = q;
          CHECKNODESTACKSPACE;  /* this has been added by S.K. */
          nodestack[nextfreestack++] = pp = &q->right;
          q = p->left;
        }
        NOTSUPPOSEDTOBENULL(q);
        if ((q->right == NULL || !ISRED(q->right))
            && (q->left == NULL || !ISRED(q->left)))
        {
          SETRED(q);
          r = p;
        }
        else
        {
          if (q->left == NULL || !ISRED(q->left))
          {
            RBTnode *q2 = q->right;

            COPYCOLOR(q2,p);
            p->left = q2->right;
            q->right = q2->left;
            q2->left = q;
            q2->right = p;
            *pp = q2;
            SETBLACK(p);
          }
          else
          {
            COPYCOLOR(q,p);
            SETBLACK(p);
            SETBLACK(q->left);
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
      SETBLACK(r);
    }
  }
  free (unchained);
  redblacktreespace -= ((Uint) sizeof(RBTnode));
  DELETENODESTACKSPACE;
  return 0;
}

/*
  Walk the nodes of a tree. ROOT is the root of the tree to be walked, 
  ACTION the function to be called at each node. LEVEL is the level of 
  ROOT in the whole tree.
*/

static Sint mytreerecurse(void *vroot,
                          RBTKEYACTION action,
                          void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  if (root->left == NULL && root->right == NULL)
  {
    if(action (&root->key, actinfo) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if (root->left != NULL)
    {
      if(mytreerecurse(root->left, action, actinfo) != 0)
      {
        return (Sint) -2;
      }
    }
    if(action (&root->key, actinfo) != 0)
    {
      return (Sint) -3;
    }
    if (root->right != NULL)
    {
      if(mytreerecurse(root->right, action, actinfo) != 0)
      {
        return (Sint) -4;
      }
    }
  }
  return 0;
}

/*
  Walk the nodes of a tree. ROOT is the root of the tree to be walked, ACTION
  the function to be called at each node.
*/

Sint RBTKEYWALK (void *vroot,RBTKEYACTION action,void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  if (root != NULL && action != NULL)
  {
    if(mytreerecurse(root, action, actinfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint mytreerecursewithstop(void *vroot,
                                  RBTKEYACTION action,
                                  void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;
  Sint retcode;

  if (root->left == NULL && root->right == NULL)
  {
    retcode = action (&root->key, actinfo);
    CHECKREDBLACKRETCODE;
  }
  else
  {
    if (root->left != NULL)
    {
      retcode = mytreerecursewithstop(root->left, action, actinfo);
      CHECKREDBLACKRETCODE;
    }
    retcode = action (&root->key, actinfo);
    CHECKREDBLACKRETCODE;
    if (root->right != NULL)
    {
      retcode = mytreerecursewithstop(root->right, action, actinfo);
      CHECKREDBLACKRETCODE;
    }
  }
  return 0;
}

Sint RBTKEYWALKSTOP (void *vroot,RBTKEYACTION action,void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  CHECKTREE (root);

  if (root != NULL && action != NULL)
  {
    Sint retcode = mytreerecursewithstop(root, action, actinfo);
    CHECKREDBLACKRETCODE;
  }
  return 0;
}

static Sint mytreerecursereverseorder(void *vroot,
                                      RBTKEYACTION action,
                                      void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  if (root->left == NULL && root->right == NULL)
  {
    if(action (&root->key, actinfo) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if (root->right != NULL)
    {
      if(mytreerecursereverseorder(root->right,action,actinfo) != 0)
      {
        return (Sint) -2;
      }
    }
    if(action (&root->key, actinfo) != 0)
    {
      return (Sint) -3;
    }
    if (root->left != NULL)
    {
      if(mytreerecursereverseorder(root->left,action,actinfo) != 0)
      {
        return (Sint) -4;
      }
    }
  }
  return 0;
}

Sint RBTWALKREVERSE (void *vroot,RBTKEYACTION action,void *actinfo)
{
  RBTnode *root = (RBTnode *) vroot;

  CHECKTREE (root);

  if (root != NULL && action != NULL)
  {
    if(mytreerecursereverseorder(root, action, actinfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

/*
  find minimum key
*/

/*@null@*/ RBTKEY *RBTKEYMINKEY (void *vroot)
{
  RBTnode *root = (RBTnode *) vroot;

  if(root == NULL)
  {
    return RBTKEYERROR;
  }
  while(root->left != NULL)
  {
    root = root->left;
  }
  return &root->key;
}

/*
  find maximum key
*/

/*@null@*/ RBTKEY *RBTKEYMAXKEY (void *vroot)
{
  RBTnode *root = (RBTnode *) vroot;

  if(root == NULL)
  {
    return RBTKEYERROR;
  }
  while(root->right != NULL)
  {
    root = root->right;
  }
  return &root->key;
}

/*
  find largest element strictly smaller than key
*/

/*@null@*/ RBTKEY *RBTKEYPREVIOUS (const RBTKEY *key,void *vroot)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = RBTKEYCOMPARE (key, &current->key);
    if (cmp == 0)
    {
      if (current->left == NULL)
      {
        if(found == NULL)
        {
          return RBTKEYERROR;
        }
        return &found->key;
      } 
      return RBTKEYMAXKEY (current->left);
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
    return RBTKEYERROR;
  }
  return &found->key;
}

/*
  find largest element smaller than or equal to the key
*/


/*@null@*/ RBTKEY *RBTKEYPREVIOUSEQ (const RBTKEY *key, void *vroot)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = RBTKEYCOMPARE (key, &current->key);
    if (cmp == 0)
    {
      return &current->key;
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
    return RBTKEYERROR;
  }
  return &found->key;
}

/*
  find smallest element strictly larger than key
*/

/*@null@*/ RBTKEY *RBTKEYNEXT (const RBTKEY *key,void *vroot)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = RBTKEYCOMPARE (key, &current->key);
    if (cmp == 0)
    {
      if (current->right == NULL)
      {
        if(found == NULL)
        {
          return RBTKEYERROR;
        }
        return &found->key;
      }
      return RBTKEYMINKEY (current->right);
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
    return RBTKEYERROR;
  }
  return &found->key;
}

/*@null@*/ RBTKEY *RBTKEYNEXTEQ (const RBTKEY *key,void *vroot)
{
  Sint cmp;
  RBTnode *current = (RBTnode *) vroot,
          *found = NULL;

  while (current != NULL)
  {
    cmp = RBTKEYCOMPARE (key, &current->key);
    if (cmp == 0)
    {
      return &current->key;
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
    return RBTKEYERROR;
  }
  return &found->key;
}

/*
  The standardized functions miss an important functionality: 
  the tree cannot be removed easily. We provide a function to do this.
  If the boolean freekey is true, then also the key field is freed.
  This only works if key points to a malloced block.
*/

static void redblacktreedestroyrecurse(RBTnode *root)
{
  if (root->left != NULL)
  {
    redblacktreedestroyrecurse(root->left);
  }
  if (root->right != NULL)
  {
    redblacktreedestroyrecurse(root->right);
  }
  /* Free the node itself.  */
  RBTKEYFREE(&root->key);
  free (root);
}

void RBTKEYDESTROY (void *vroot)
{
  RBTnode *root = (RBTnode *) vroot;

  if (root != NULL)
  {
    redblacktreedestroyrecurse(root);
  }
  printf("# redblacktreespacepeak = %.2f MB\n",MEGABYTES(redblacktreespacepeak));
}
