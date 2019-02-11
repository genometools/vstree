#include "errordef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "spacedef.h"
#include "megabytes.h"
#include "fhandledef.h"
#include "vnodedef.h"

#include "vnodes.pr"
#include "accvirt.pr"
#include "outindextab.pr"
#include "readvirt.pr"
#include "filehandle.pr"

#define SMALLISO
#ifdef SMALLISO
#define ISOMAX        UCHAR_MAX
#define SHOWISOVAL(X) printf("%lu",(Showuint) X)
typedef Uchar Isotype;
#else
#define ISOMAX        USHRT_MAX
#define SHOWISOVAL(X) printf("%hu",X)
typedef Ushort Isotype;
#endif

#ifdef DEBUG
static Uint isomorphic = 0;
#endif

#ifdef DEBUG
static void isodepthstat(Isotype *isodepthtab,Uint len)
{
  Uint i, longcount = 0, sumvalues = 0;
  Isotype current, minvalue = ISOMAX, maxvalue = 0;

  for(i=0; i<len; i++)
  {
    current = isodepthtab[i];
    DEBUG2(3,"isodepthtab[%lu]=%lu\n",(Showuint) i,(Showuint) current);
    if(current > maxvalue)
    {
      maxvalue = current;
    }
    if(current < minvalue)
    {
      minvalue = current;
    }
    sumvalues += current;
    if(current == ISOMAX)
    {
      longcount++;
    }
  }
  printf("minvalue = ");
  SHOWISOVAL(minvalue);
  printf(", maxvalue = ");
  SHOWISOVAL(maxvalue);
  printf(", average = %.2f, ",(double) sumvalues/len);
  printf("longcount = %lu (%.2f)\n",(Showuint) longcount,
                                    (double) longcount/len);
}
#endif

static void storeleafunique(void *info,Vleaf *vleaf)
{
  Isotype *leafunique = (Isotype *) info;

  DEBUG2(3,"leafunique[%lu]=%lu\n",(Showuint) vleaf->suffixnum,
                                   (Showuint) vleaf->uniquelength);
  if(vleaf->uniquelength >= ISOMAX)
  {
    leafunique[vleaf->suffixnum] = ISOMAX;
  } else
  {
    leafunique[vleaf->suffixnum] = (Isotype) vleaf->uniquelength;
  }
}

static Sint findminprefixlength(Virtualtree *virtualtree,Uint offset,
                                Uint l,Uint r)
{
  Uint lcpval, minprefixlength;

  if(l == 0)
  {
    minprefixlength = offset;
  } else
  {
    EVALLCP(virtualtree,lcpval,l);
    if(lcpval >= offset)
    {
      return (Sint) -1;  // can extend to the right
    }
    minprefixlength = lcpval+1;
  }
  if(r < virtualtree->multiseq.totallength)
  {
    EVALLCP(virtualtree,lcpval,r+1);
    if(lcpval >= offset)
    {
      return (Sint) -1;
    }
    if(lcpval+1 > minprefixlength)
    {
      minprefixlength = lcpval+1;
    }
  }
  return (Sint) minprefixlength;
}

static Sint processbranch(void *info,Vnode *vnode)
{
  Virtualtree *virtualtree = (Virtualtree *) info;
  Uint i, minprefixlength, rank1, rank2;
  Sint retcode;

  DEBUG3(3,"(%lu,%lu,%lu)->",(Showuint) vnode->offset,
                             (Showuint) vnode->left,
                             (Showuint) vnode->right);
  if(vnode->offset > 0)
  {
    rank1 = RANKOFNEXTLEAF(virtualtree,vnode->left);
    rank2 = RANKOFNEXTLEAF(virtualtree,vnode->right);
    if(rank2 - rank1 > vnode->right - vnode->left)
    {
      return 0;
    }
    retcode = findminprefixlength(virtualtree,vnode->offset-1,rank1,rank2);
    if(retcode < 0)
    {
      return 0;
    }
    minprefixlength = (Uint) retcode;
    DEBUG1(3," with minimal prefix length %lu",
                             (Showuint) minprefixlength);
    DEBUG3(3,", iso[%lu..%lu]->%lu\n",
                             (Showuint) vnode->left,
                             (Showuint) vnode->right,
                             (Showuint) minprefixlength);
    if(minprefixlength > ISOMAX)
    {
      minprefixlength = ISOMAX;
    }
    for(i=vnode->left; i<=vnode->right; i++)
    {
      virtualtree->isodepthtab[i] = (Isotype) minprefixlength;
    }
    DEBUGCODE(1,isomorphic += (vnode->right - vnode->left + 1));
  }
  return 0;
}

static void fillremainleafdepth(Virtualtree *virtualtree)
{
  Uint i;
  Isotype *leafunique;

  ALLOCASSIGNSPACE(leafunique,NULL,Isotype,virtualtree->multiseq.totallength+1);
  (void) enumvleaves(virtualtree,
                     (void *) leafunique,
                     storeleafunique);
  for(i=0; i<virtualtree->multiseq.totallength; i++)
  {
    if(virtualtree->isodepthtab[i] == 0)
    {
      virtualtree->isodepthtab[i] = leafunique[RANKOFNEXTLEAF(virtualtree,i)];
    }
  }
  FREESPACE(leafunique);
}

Sint fillisotab(Virtualtree *virtualtree)
{
  Uint i;

  for(i=0; i<virtualtree->multiseq.totallength; i++)
  {
    virtualtree->isodepthtab[i] = 0;  // initialization
  }
  if(enumvnodes(virtualtree,(void *) virtualtree,processbranch) != 0)
  {
    return (Sint) -1;
  }
  fillremainleafdepth(virtualtree);
  DEBUGCODE(1,isodepthstat(virtualtree->isodepthtab,
                           virtualtree->multiseq.totallength));
  return 0;
}

static Sint makeisotabandoutput(const char *indexname,Virtualtree *virtualtree)
{
  ALLOCASSIGNSPACE(virtualtree->isodepthtab,NULL,Uchar,
                   virtualtree->multiseq.totallength);
  if(fillisotab(virtualtree) != 0)
  {
    return (Sint) -3;
  }
  if(outindextab(indexname,"iso",(void *) virtualtree->isodepthtab,
                 (Uint) sizeof(Uchar),virtualtree->multiseq.totallength) != 0)
  {
    return (Sint) -1;
  }
  FREESPACE(virtualtree->isodepthtab);
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  const char *indexname;

  VSTREECHECKARGNUM(2,"indexname");
  DEBUGLEVELSET;
  
  indexname = argv[1];
  if(mapvirtualtreeifyoucan(&virtualtree,indexname,
                            STITAB | SUFTAB | LCPTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(makeisotabandoutput(indexname,&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
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
