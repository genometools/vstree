#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "chardef.h"
#include "failures.h"
#include "intbits.h"
#include "divmodmul.h"
#include "debugdef.h"
#include "arraydef.h"
#include "encseq-def.h"

#include "trieins-def.h"

#define ISLEAF(NODE) ((NODE)->firstchild == NULL)
#ifdef DEBUG
#define SHOWNODERELATIONS(NODE)\
        shownoderelations((Uint) __LINE__,#NODE,NODE)
#else
#define SHOWNODERELATIONS(NODE) /* Nothing */
#endif

#define SETFIRSTCHILD(NODE,VALUE)\
        (NODE)->firstchild = VALUE;\
        if((VALUE) != NULL)\
        {\
          (VALUE)->parent = NODE;\
        }

#define SETFIRSTCHILDNULL(NODE)\
        (NODE)->firstchild = NULL

typedef struct
{
  Trienode *previous, 
           *current;
} Nodepair;

static Uchar getfirstedgechar(const Trierep *trierep,
                              const Trienode *node,
                              Uint prevdepth)
{
  Encodedsequence *encseq = trierep->encseqtable + node->suffixinfo.idx;

  if(ISLEAF(node) &&
     node->suffixinfo.startpos + prevdepth >= 
     (Uint) ACCESSSEQUENCELENGTH(encseq))
  {
    return SEPARATOR;
  }
  return ACCESSENCODEDCHAR(encseq,node->suffixinfo.startpos + prevdepth);
}

static Sint comparecharacters(Uchar cc1,Uint idx1,Uchar cc2,Uint idx2)
{
  if(ISSPECIAL(cc1))
  {
    if(ISSPECIAL(cc2))
    {
      if(idx1 <= idx2)
      {
        return (Sint) -1;  // cc1 < cc2
      } else
      {
        return (Sint) 1;  // cc1 > cc2
      }
    } else
    {
      return (Sint) 1; // cc1 > cc2
    }
  } else
  {
    if(ISSPECIAL(cc2))
    {
      return (Sint) -1;  // cc1 < cc2
    } else
    {
      if(cc1 < cc2)
      {
        return (Sint) -1;  // cc1 < cc2
      } else
      {
        if(cc1 > cc2)
        {
          return (Sint) 1;  // cc1 > cc2
        } else
        {
          return 0; // cc1 
        }
      }
    }
  }
}

#ifdef DEBUG
static void showtrie2(Sint debuglevel,
                      const Trierep *trierep,
                      const Uchar *characters,
                      Uint level,
                      const Trienode *node)
{
  Uchar cc = 0;
  Uint pos, endpos;
  Trienode *current;

  for(current = node->firstchild; 
      current != NULL; 
      current = current->rightsibling)
  {
    DEBUG3(debuglevel,"%*.*s",(Fieldwidthtype) (6 * level),
                              (Fieldwidthtype) (6 * level)," ");
    if(ISLEAF(current))
    {
      endpos = (Uint) ACCESSSEQUENCELENGTH(trierep->encseqtable + 
                                            current->suffixinfo.idx);
    } else
    {
      endpos = current->suffixinfo.startpos + current->depth;
    }
    for(pos = current->suffixinfo.startpos + node->depth;
        pos < endpos; pos++)
    {
      cc = ACCESSENCODEDCHAR(trierep->encseqtable + current->suffixinfo.idx,pos);
      if(ISSPECIAL(cc))
      {
        DEBUG0(debuglevel,"#\n");
        break;
      }
      DEBUG1(debuglevel,"%c",characters[(Uint) cc]);
    }
    if(ISLEAF(current))
    {
      if(!ISSPECIAL(cc))
      {
        DEBUG0(debuglevel,"~\n");
      }
    } else
    {
      DEBUG2(debuglevel+1," d=%lu,i=%lu",
            (Showuint) current->depth,
            (Showuint) current->suffixinfo.ident
            );
      DEBUG0(debuglevel,"\n");
      showtrie2(debuglevel,trierep,characters,level+1,current);
    }
  }
}

void showtrie(Sint debuglevel,
              const Trierep *trierep,
              const Uchar *characters)
{
  if(trierep->root != NULL)
  {
    showtrie2(debuglevel,trierep,characters,0,trierep->root);
  }
}

/*
   Check the following:
   (1) for each branching node there exist at least 2 DONE
   (2) for each branching node the list of successors is strictly ordered
       according to the first character of the edge label DONE
   (3) there are no empty edge labels DONE
   (4) there are \(n+1\) leaves and for each leaf there is exactly one
       incoming edge DONE
*/

static void checktrie2(Trierep *trierep,
                        Trienode *node,
                        Trienode *father,
                        Uint *leafused,
                        Uint *numberofbitsset)
{
  Trienode *current, *previous;

  if(ISLEAF(node))
  {
    Uint start = node->suffixinfo.startpos;
    if(ISIBITSET(leafused,start))
    {
      fprintf(stderr,"leaf %lu already found\n",(Showuint) start);
      exit(EXIT_FAILURE);
    }
    SETIBIT(leafused,start);
    (*numberofbitsset)++;
  } else
  {
    if(node->depth > 0 && node->firstchild->rightsibling == NULL)
    {
      fprintf(stderr,"Node has less than two successors\n");
      exit(EXIT_FAILURE);
    }
    if(father != NULL)
    {
      if(ISLEAF(father))
      {
        fprintf(stderr,"father of branching node is a leaf\n");
        exit(EXIT_FAILURE);
      }
      if(father->depth >= node->depth)
      {
        fprintf(stderr,"father.depth = %lu >= %lu = node.depth\n",
                       (Showuint) father->depth,
                       (Showuint) node->depth);
        exit(EXIT_FAILURE);
      }
    }
    previous = NULL;
    for(current = node->firstchild; current != NULL; 
        current = current->rightsibling)
    {
      if(previous != NULL)
      {
        if(comparecharacters(
              getfirstedgechar(trierep,previous,node->depth),
              previous->suffixinfo.idx,
              getfirstedgechar(trierep,current,node->depth),
              current->suffixinfo.idx) >= 0)
        {
          fprintf(stderr,"nodes not correctly ordered\n");
          exit(EXIT_FAILURE);
        }
      }
      checktrie2(trierep,current,node,leafused,numberofbitsset);
      previous = current;
    }
  }
}

void checktrie(Trierep *trierep,Uint numberofleaves)
{
  if(trierep->root != NULL)
  {
    Uint *leafused, numberofbitsset = 0;
    INITBITTAB(leafused,numberofleaves);
    checktrie2(trierep,trierep->root,NULL,leafused,&numberofbitsset);
    if(numberofbitsset != numberofleaves)
    {
      fprintf(stderr,"numberofbitsset = %lu != %lu = numberofleaves\n",
                      (Showuint) numberofbitsset,
                      (Showuint) numberofleaves);
      exit(EXIT_FAILURE);
    }
    FREESPACE(leafused);
  }
}

static void shownode(const Trienode *node)
{
  if(node == NULL)
  {
    DEBUG0(4,"NULL");
  } else
  {
    DEBUG2(4,"%s %lu",ISLEAF(node) ? "leaf" : "branch",
                      (Showuint) node->suffixinfo.ident);
  }
}

static void showsimplenoderelations(const Trienode *node)
{
  shownode(node);
  DEBUG0(4,".firstchild=");
  shownode(node->firstchild);
  DEBUG0(4,"; ");
  shownode(node);
  DEBUG0(4,".rightsibling=");
  shownode(node->rightsibling);
  DEBUG0(4,"\n");
}

static void shownoderelations(Uint line,char *nodestring,const Trienode *node)
{
  DEBUG2(4,"l. %lu: %s: ",(Showuint) line,nodestring);
  showsimplenoderelations(node);
}

void showallnoderelations(const Trienode *node)
{
  Trienode *tmp;

  showsimplenoderelations(node);
  for(tmp = node->firstchild; tmp != NULL; tmp = tmp->rightsibling)
  {
    if(tmp->firstchild == NULL)
    {
      showsimplenoderelations(tmp);
    } else
    {
      showallnoderelations(tmp);
    }
  }
}
#endif

static Trienode *newTrienode(Trierep *trierep)
{
  DEBUG1(3,"# available trie nodes: %lu; ",
          (Showuint) (trierep->allocatedTrienode - trierep->nextfreeTrienode));
  DEBUG1(3,"unused trie nodes: %lu\n",(Showuint) trierep->nextunused);
  if(trierep->nextfreeTrienode >= trierep->allocatedTrienode)
  {
    if(trierep->nextunused == 0)
    {
      fprintf(stderr,"not enough nodes have been allocated\n");
      exit(EXIT_FAILURE);
    }
    trierep->nextunused--;
    return trierep->unusedTrienodes[trierep->nextunused];
  }
  return trierep->nodetable + trierep->nextfreeTrienode++;
}

 /*@ignore@*/
static Trienode *makenewleaf(Trierep *trierep,Suffixinfo *suffixinfo)
{
  Trienode *newleaf;

  DEBUG1(4,"makenewleaf(%lu)\n",(Showuint) suffixinfo->ident);
  newleaf = newTrienode(trierep);
  newleaf->suffixinfo = *suffixinfo;
  SETFIRSTCHILDNULL(newleaf);
  newleaf->rightsibling = NULL;
  SHOWNODERELATIONS(newleaf);
  return newleaf;
}

static Trienode *makeroot(Trierep *trierep,Suffixinfo *suffixinfo)
{
  Trienode *root, *newleaf;

  DEBUG1(4,"makeroot(%lu)\n",(Showuint) suffixinfo->ident);
  root = newTrienode(trierep);
  root->parent = NULL;
  root->suffixinfo = *suffixinfo;
  root->depth = 0;
  root->rightsibling = NULL;
  newleaf = makenewleaf(trierep,suffixinfo);
  SETFIRSTCHILD(root,newleaf);
  SHOWNODERELATIONS(root);
  return root;
}
 /*@end@*/

static void makesuccs(Trienode *newbranch,Trienode *first,Trienode *second)
{
  second->rightsibling = NULL;
  first->rightsibling = second;
  SETFIRSTCHILD(newbranch,first);
  SHOWNODERELATIONS(second);
  SHOWNODERELATIONS(first);
  SHOWNODERELATIONS(newbranch);
}

static Trienode *makenewbranch(Trierep *trierep,
                               Suffixinfo *suffixinfo,
                               Uint currentdepth,
                               Trienode *oldnode)
{
  Trienode *newbranch, *newleaf;
  Uchar cc1, cc2;
  Encodedsequence *encseq = trierep->encseqtable + suffixinfo->idx;

  DEBUG1(4,"makenewbranch(ident=%lu)\n",(Showuint) suffixinfo->ident);
  newbranch = newTrienode(trierep);
  newbranch->suffixinfo = *suffixinfo;
  newbranch->rightsibling = oldnode->rightsibling;
  cc1 = getfirstedgechar(trierep,oldnode,currentdepth);
  if(suffixinfo->startpos + currentdepth >= (Uint) ACCESSSEQUENCELENGTH(encseq))
  {
    cc2 = SEPARATOR;
  } else
  {
    cc2 = ACCESSENCODEDCHAR(encseq,suffixinfo->startpos + currentdepth);
  }
  newleaf = makenewleaf(trierep,suffixinfo);
  if(comparecharacters(cc1,oldnode->suffixinfo.idx,
                       cc2,suffixinfo->idx) <= 0)
  {
    makesuccs(newbranch,oldnode,newleaf);
  } else
  {
    makesuccs(newbranch,newleaf,oldnode);
  }
  newbranch->depth = currentdepth;
  return newbranch;
}

static Uint getlcp(const Encodedsequence *encseq1,Uint start1,Uint end1,
                   const Encodedsequence *encseq2,Uint start2,Uint end2)
{
  Uint i1, i2;
  Uchar cc1;

  for(i1=start1, i2=start2; i1 <= end1 && i2 <= end2; i1++, i2++)
  {
    cc1 = ACCESSENCODEDCHAR(encseq1,i1);
    if(cc1 != ACCESSENCODEDCHAR(encseq2,i2) || ISSPECIAL(cc1))
    {
      break;
    }
  }
  return i1 - start1;
}

static BOOL hassuccessor(const Trierep *trierep,
                         Nodepair *np,
                         Uint prevdepth,
                         const Trienode *node,
                         Uchar cc2,
                         Uint idx2)
{
  Uchar cc1;
  Sint cmpresult;

  for(np->previous = NULL, np->current = node->firstchild; 
      np->current != NULL; 
      np->current = np->current->rightsibling)
  {
    cc1 = getfirstedgechar(trierep,np->current,prevdepth);
    cmpresult = comparecharacters(cc1,np->current->suffixinfo.idx,cc2,idx2);
    if(cmpresult == 1)
    {
      return False;
    }
    if(cmpresult == 0)
    {
      return True;
    }
    np->previous = np->current;
  }
  return False;
}

void insertsuffixintotrie(Trierep *trierep,
                          Trienode *node,
                          Suffixinfo *suffixinfo)
{
  if(trierep->root == NULL)
  {
    trierep->root = makeroot(trierep,suffixinfo);
  } else
  {
    Uint currentdepth, lcpvalue;
    Trienode *currentnode, *newleaf, *newbranch, *succ;
    Nodepair np;
    Uchar cc;
    Encodedsequence *encseq = trierep->encseqtable + suffixinfo->idx;

    if(ISLEAF(node))
    {
      NOTSUPPOSED;
    }
    currentnode = node;
    currentdepth = node->depth;
    while(True)
    {
      if(suffixinfo->startpos + currentdepth >= 
         (Uint) ACCESSSEQUENCELENGTH(encseq))
      {
	cc = SEPARATOR;
      } else
      {
	cc = ACCESSENCODEDCHAR(encseq,suffixinfo->startpos + currentdepth);
      }
      if(currentnode == NULL)
      {
	NOTSUPPOSED;
      } 
      if(ISLEAF(currentnode))
      {
	NOTSUPPOSED;
      }
      if(!hassuccessor(trierep,&np,currentdepth,currentnode,cc,suffixinfo->idx))
      {
	newleaf = makenewleaf(trierep,suffixinfo);
	newleaf->rightsibling = np.current;
	SHOWNODERELATIONS(newleaf);
	if(np.previous == NULL)
	{
          SETFIRSTCHILD(currentnode,newleaf);
	  SHOWNODERELATIONS(currentnode);
	} else
	{
	  np.previous->rightsibling = newleaf;
	  SHOWNODERELATIONS(np.previous);
	}
	return;
      }
      succ = np.current;
      if(ISLEAF(succ))
      {
	lcpvalue = getlcp(trierep->encseqtable + suffixinfo->idx,
                          suffixinfo->startpos + currentdepth + 1,
			  (Uint) ACCESSSEQUENCELENGTH(trierep->encseqtable + 
                                                      suffixinfo->idx) - 1,
			  trierep->encseqtable + succ->suffixinfo.idx,
			  succ->suffixinfo.startpos + currentdepth + 1,
			  (Uint) ACCESSSEQUENCELENGTH(trierep->encseqtable + 
                                                      succ->suffixinfo.idx) - 1);
	newbranch = makenewbranch(trierep,
				  suffixinfo,
				  currentdepth + lcpvalue + 1,
				  succ);
	if(np.previous == NULL)
	{
          SETFIRSTCHILD(currentnode,newbranch);
	  SHOWNODERELATIONS(currentnode);
	} else
	{
	  np.previous->rightsibling = newbranch;
	  SHOWNODERELATIONS(np.previous);
	}
	return;
      }
      lcpvalue = getlcp(trierep->encseqtable + suffixinfo->idx,
                        suffixinfo->startpos + currentdepth + 1,
			(Uint) ACCESSSEQUENCELENGTH(trierep->encseqtable + 
                                                    suffixinfo->idx) - 1,
			trierep->encseqtable + succ->suffixinfo.idx,
			succ->suffixinfo.startpos + currentdepth + 1,
			succ->suffixinfo.startpos + succ->depth - 1);
      if(currentdepth + lcpvalue + 1 < succ->depth)
      {
	newbranch = makenewbranch(trierep,
				  suffixinfo,
				  currentdepth + lcpvalue + 1,
				  succ);
	if(np.previous == NULL)
	{
          SETFIRSTCHILD(currentnode,newbranch);
	  SHOWNODERELATIONS(currentnode);
	} else
	{
	  np.previous->rightsibling = newbranch;
	  SHOWNODERELATIONS(np.previous);
	}
	return;
      }
      currentnode = succ;
      currentdepth = currentnode->depth;
    }
  }
}

Trienode *findsmallestnodeintrie(const Trierep *trierep)
{
  Trienode *node;

  if(trierep->root == NULL)
  {
    NOTSUPPOSED;
  }
  for(node = trierep->root; node->firstchild != NULL; node = node->firstchild)
    /* Nothing */ ;
  return node;
}

void deletesmallestpath(Trienode *smallest,Trierep *trierep)
{
  Trienode *father, *son;

  for(son = smallest; son->parent != NULL; son = son->parent)
  {
    father = son->parent;
    if(son->firstchild == NULL)
    {
      SETFIRSTCHILD(father,son->rightsibling);
      SHOWNODERELATIONS(father);
      son->rightsibling = NULL;
    } else
    {
      if(son->firstchild->rightsibling != NULL)
      {
        break;
      }
      son->firstchild->rightsibling = father->firstchild->rightsibling;
      SETFIRSTCHILD(father,son->firstchild);
      SHOWNODERELATIONS(father);
      son->rightsibling = NULL;
      SETFIRSTCHILDNULL(son);
    }
    DEBUG2(4,"delete %s %lu\n",
             ISLEAF(son) ? "leaf" : "branch",
             (Showuint) son->suffixinfo.ident);
    trierep->unusedTrienodes[trierep->nextunused++] = son;
  }
  if(trierep->root->firstchild == NULL)
  {
    trierep->unusedTrienodes[trierep->nextunused++] = trierep->root;
    trierep->root = NULL;
  }
}

void inittrienodetable(Trierep *trierep,Uint numofsuffixes,Uint numofindexes)
{
  trierep->numofindexes = numofindexes;
  trierep->allocatedTrienode = MULT2(numofsuffixes + 1) + 1;
  ALLOCASSIGNSPACE(trierep->nodetable,NULL,Trienode,trierep->allocatedTrienode);
  trierep->nextfreeTrienode = 0;
  trierep->root = NULL;
  trierep->nextunused = 0;
  ALLOCASSIGNSPACE(trierep->unusedTrienodes,NULL,Trienodeptr,
                   trierep->allocatedTrienode);
}

void freetrierep(Trierep *trierep)
{
  FREESPACE(trierep->nodetable);
  FREESPACE(trierep->unusedTrienodes);
}
