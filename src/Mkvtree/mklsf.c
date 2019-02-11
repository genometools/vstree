#include <stdlib.h>
#include <string.h>
#include "errordef.h"
#include "megabytes.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "chardef.h"
#include "intbits.h"
#include "vnodedef.h"
#include "fhandledef.h"
#include "spacedef.h"

#include "accvirt.pr"
#include "maxpref.pr"
#include "vbfstrav.pr"
#include "readvirt.pr"
#include "outindextab.pr"
#include "filehandle.pr"

#include "qgram2code.c"

#ifdef DEBUG
#define CHECKMMSEARCHRET\
        if(!found)\
        {\
          fprintf(stderr,"mmsearch fails\n");\
          exit(EXIT_FAILURE);\
        }
#else
#define CHECKMMSEARCHRET /* Nothing */
#endif

#define DEFINEUNDEFDROP1 Uint undefdrop1 = virtualtree->multiseq.totallength+1
#define UNDEFDROP1       undefdrop1

//static Uint nodediff = 0;

typedef struct
{
  PairUint *drop1tab;
  Uint *markinsidebucket;
} Drop1info;

#ifdef DEBUG

static void applysuffixlink(Virtualtree *virtualtree,PairUint *drop1tab,
                            Uint offset,Uint left,Uint right,
                            void (*applylink)(Virtualtree *,Uint,Uint,Uint,
                                              Uchar *,Uint,Uint,Uchar *))
{
  Uint homeval, leftlcp, rightlcp;
  Uchar *sptr1, *sptr2;

  sptr1 = virtualtree->multiseq.sequence + virtualtree->suftab[left];
  DELIVERHOME(virtualtree,homeval,leftlcp,rightlcp,left,right);
  sptr2 = virtualtree->multiseq.sequence + 
          virtualtree->suftab[drop1tab[homeval].uint0];
  applylink(virtualtree,offset,left,right,sptr1,
            drop1tab[homeval].uint0,
            drop1tab[homeval].uint1,sptr2);
}

static void showsuffixlink(Virtualtree *virtualtree,Uint offset,Uint left,
                           Uint right,Uchar *sptr1,Uint lleft,Uint lright,
                           Uchar *sptr2)
{
  printf("link(%lu,%lu,%lu,",(Showuint) offset,
                             (Showuint) left,(Showuint) right);
  vm_showsymbolstring(&virtualtree->alpha,sptr1,offset);
  printf(")=(%lu,%lu,%lu,",(Showuint) (offset-1),
                           (Showuint) lleft,
                           (Showuint) lright);
  vm_showsymbolstring(&virtualtree->alpha,sptr2,offset - 1);
  printf(")\n");
}

static void checksuffixlink(/*@unused@*/ Virtualtree *virtualtree,
                            Uint offset,
                            /*@unused@*/ Uint left,
                            /*@unused@*/ Uint right,
                            Uchar *sptr1,
                            /*@unused@*/ Uint lleft,
                            /*@unused@*/ Uint lright,
                            Uchar *sptr2)
{
  Uint i;

  for(i=0; i<offset-1; i++)
  {
    if(sptr1[i+1] != sptr2[i])
    {
      fprintf(stderr,"sptr1[%lu] = %lu != %lu = sptr2[%lu]\n",
                       (Showuint) (i+1),
                       (Showuint) sptr1[i+1],
                       (Showuint) i,
                       (Showuint) sptr2[i]);
      exit(EXIT_FAILURE);
    }
  }
}
#endif

static Sint setdrop1tab(Virtualtree *virtualtree,void *info,
                        Vnode *father,Vnode *child)
{
#ifdef DEBUG
  BOOL found;
#endif
  Vnode vnode;
  Uint hvalchild, leftlcp, rightlcp, hvalfather;
  Drop1info *drop1info = (Drop1info *) info;
#ifdef DEBUG
  Uchar *suffixptr = virtualtree->multiseq.sequence + 
                     virtualtree->suftab[child->left] + 1;
#endif

  DELIVERHOME(virtualtree,hvalchild,leftlcp,rightlcp,child->left,child->right);
  if(father->offset == 0)
  {
    if(child->offset == UintConst(1))
    {
      drop1info->drop1tab[hvalchild].uint0 = 0;
      drop1info->drop1tab[hvalchild].uint1 = virtualtree->multiseq.totallength;
    } else
    {
      vnode = *father;
#ifdef DEBUG
      found = mmsearch(virtualtree->multiseq.sequence,
                       virtualtree->multiseq.totallength,
                       virtualtree->suftab,&vnode,suffixptr,child->offset - 1);
      CHECKMMSEARCHRET;
#endif
      drop1info->drop1tab[hvalchild].uint0 = vnode.left;
      drop1info->drop1tab[hvalchild].uint1 = vnode.right;
    }
  } else
  {
    //nodediff += (child->offset - father->offset);
    vnode.offset = father->offset-1;
    DELIVERHOME(virtualtree,hvalfather,leftlcp,rightlcp,
                father->left,father->right);
    vnode.left = drop1info->drop1tab[hvalfather].uint0;
    vnode.right = drop1info->drop1tab[hvalfather].uint1;
#ifdef DEBUG
    found = mmsearch(virtualtree->multiseq.sequence,
                     virtualtree->multiseq.totallength,
                     virtualtree->suftab,&vnode,suffixptr,child->offset-1);
    CHECKMMSEARCHRET;
#endif
    drop1info->drop1tab[hvalchild].uint0 = vnode.left;
    drop1info->drop1tab[hvalchild].uint1 = vnode.right;
  }
  if(child->offset <= virtualtree->prefixlength)
  {
    // store that the child is inside a bucket
    SETIBIT(drop1info->markinsidebucket,hvalchild);
  }
  DEBUGCODE(3,applysuffixlink(virtualtree,drop1info->drop1tab,child->offset,
                              child->left,child->right,showsuffixlink));
  DEBUGCODE(1,applysuffixlink(virtualtree,drop1info->drop1tab,child->offset,
                              child->left,child->right,checksuffixlink));
  return 0;
}

static Sint transformdrop1tab(Uchar *smalldrop1tab,
                              Drop1info *drop1info,Virtualtree *virtualtree)
{
  Uint leftbound, indexval, code = 0, i, diffval;
  DEFINEUNDEFDROP1;
  DEBUGDECL(Uint countleftlarge = 0);
  DEBUGDECL(Uint countrightlarge = 0);

  for(i=0, indexval=0; i<=virtualtree->multiseq.totallength; i++, indexval+=2)
  {
    if(drop1info->drop1tab[i].uint0 != UNDEFDROP1 &&
       drop1info->drop1tab[i].uint1 != UNDEFDROP1 &&
       !ISIBITSET(drop1info->markinsidebucket,i))
    {
      if(!qgram2code(&code,virtualtree->alpha.mapsize-1,
                     virtualtree->prefixlength,
                     virtualtree->multiseq.sequence + 
                     virtualtree->suftab[drop1info->drop1tab[i].uint0]))
      {
        ERROR3("qgram2code(%lu,%lu,sequence+%lu)=undefined",
                           (Showuint) (virtualtree->alpha.mapsize-1),
                           (Showuint) virtualtree->prefixlength,
                           (Showuint) virtualtree->suftab[drop1info->drop1tab[i].uint0]);
        return (Sint) -1;
      }
      leftbound = virtualtree->bcktab[MULT2(code)];
      if(leftbound > drop1info->drop1tab[i].uint0)
      {
        ERROR2("leftbound=%lu > %lu not expected",
                        (Showuint) leftbound,
                        (Showuint) drop1info->drop1tab[i].uint0);
        return (Sint) -2;
      }
      if(leftbound > drop1info->drop1tab[i].uint1)
      {
        ERROR2("leftbound=%lu > %lu not expected",
                      (Showuint) leftbound,
                      (Showuint)  drop1info->drop1tab[i].uint1);
        return (Sint) -3;
      }
      diffval = drop1info->drop1tab[i].uint0 - leftbound;
      if(diffval >= UCHAR_MAX)
      {
        smalldrop1tab[indexval] = UCHAR_MAX;
        smalldrop1tab[indexval+1] = UCHAR_MAX;
        DEBUGCODE(1,countleftlarge++);
        DEBUGCODE(1,countrightlarge++);
      } else
      {
        smalldrop1tab[indexval] = (Uchar) diffval;
        diffval = drop1info->drop1tab[i].uint1 - drop1info->drop1tab[i].uint0;
        if(diffval >= UCHAR_MAX)
        {
          smalldrop1tab[indexval+1] = UCHAR_MAX;
          DEBUGCODE(1,countrightlarge++);
        } else
        {
          smalldrop1tab[indexval+1] = (Uchar) diffval;
        }
      }
    } else
    {
      smalldrop1tab[indexval] = UCHAR_MAX;
      smalldrop1tab[indexval+1] = UCHAR_MAX;
    }
  }
  DEBUG1(1,"countleftlarge=%lu\n",(Showuint) countleftlarge);
  DEBUG1(1,"countrightlarge=%lu\n",(Showuint) countrightlarge);
  return 0;
}

static Sint linksuf(Uchar *smalldrop1tab,Virtualtree *virtualtree)
{
  Uint i;
  DEFINEUNDEFDROP1;
  Drop1info drop1info;
  
  ALLOCASSIGNSPACE(drop1info.drop1tab,NULL,PairUint,
                   virtualtree->multiseq.totallength+1);
  INITBITTAB(drop1info.markinsidebucket,virtualtree->multiseq.totallength+1);
  for(i=0;i<=virtualtree->multiseq.totallength; i++)
  {
    drop1info.drop1tab[i].uint0 = drop1info.drop1tab[i].uint1 = UNDEFDROP1;
  }
  if(breadthfirstvstree(virtualtree,setdrop1tab,(void *) &drop1info) != 0)
  {
    return (Sint) -1;
  }
  if(transformdrop1tab(smalldrop1tab,&drop1info,virtualtree) != 0)
  {
    return (Sint) -2;
  }
  FREESPACE(drop1info.drop1tab);
  FREESPACE(drop1info.markinsidebucket);
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  const char *indexname;
  Uchar *smalldrop1tab;

  VSTREECHECKARGNUM(2,"indexname");
  DEBUGLEVELSET;
  
  indexname = argv[1];
  if(mapvirtualtreeifyoucan(&virtualtree,indexname,
                            BCKTAB | SUFTAB | TISTAB | LCPTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  ALLOCASSIGNSPACE(smalldrop1tab,NULL,
                   Uchar,
                   UintConst(2) * (virtualtree.multiseq.totallength+1));
  if(linksuf(smalldrop1tab,&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
  if(outindextab(indexname,"lsf",(void *) smalldrop1tab,
                 (Uint) sizeof(Uchar),
                 UintConst(2) * (virtualtree.multiseq.totallength+1)) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(smalldrop1tab);
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  printf("# overall space peak: main=%.2f MB (%.2f bytes/symbol), "
         "secondary=%.2f MB (%.2f bytes/symbol)\n",
          MEGABYTES(getspacepeak()),
          (double) getspacepeak()/virtualtree.multiseq.totallength,
	  MEGABYTES(mmgetspacepeak()),
	  (double) mmgetspacepeak()/virtualtree.multiseq.totallength);
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
