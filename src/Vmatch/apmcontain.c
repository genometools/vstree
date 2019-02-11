
#include "debugdef.h"
#include "errordef.h"
#include "matchinfo.h"
#include "mparms.h"
#include "divmodmul.h"
#include "fhandledef.h"

#include "detmatch.pr"
#include "echomatch.pr"

#define CHECKCONTAINMENTWITHTOLERANCE(M1,M2)\
        ((M1)->Storeposition1 <= (M2)->Storeposition1 &&\
         (M2)->Storeposition1 + (M2)->Storelength1 <= \
         (M1)->Storeposition1 + (M2)->Storelength1)

static BOOL checkifsortedbyseqnum2(ArrayStoreMatch *matchtab)
{
  StoreMatch *mptr;

  if(matchtab->nextfreeStoreMatch == 0)
  {
    return True;
  }
  for(mptr = matchtab->spaceStoreMatch+1; 
      mptr < matchtab->spaceStoreMatch+matchtab->nextfreeStoreMatch;
      mptr++)
  {
    if((mptr-1)->Storeseqnum2 > mptr->Storeseqnum2)
    {
      ERROR4("match %lu: seqnum2 = %lu > %lu = seqnum2 of match %lu\n",
             (Showuint) ((mptr-1) - matchtab->spaceStoreMatch),
             (Showuint) (mptr-1)->Storeseqnum2,
             (Showuint) mptr->Storeseqnum2,
             (Showuint) (mptr - matchtab->spaceStoreMatch));
      return False;
    }
  }
  printf("# %lu matches checked for correct order\n",
         (Showuint) matchtab->nextfreeStoreMatch);
  return True;
}

static Qsortcomparereturntype ordermatchpos1(StoreMatch *p,StoreMatch *q)
{
  if(p->Storeseqnum2 != q->Storeseqnum2)
  {
    fprintf(stderr,"Can only sort matches with identical seqnum2\n");
    exit(EXIT_FAILURE);
  }
  if(p->Storeposition1 < q->Storeposition2)
  {
    return (Qsortcomparereturntype) -1;
  }
  if(p->Storeposition1 > q->Storeposition2)
  {
    return 0;
  }
  return 0;
}

static void sortgroupsbypos1(ArrayStoreMatch *matchtab)
{
  StoreMatch *mptr, 
             *startofgroup = matchtab->spaceStoreMatch;
  Uint sizeofgroup, countgroups = 0;

  if(matchtab->nextfreeStoreMatch == 0)
  {
    return;
  }
  for(mptr = matchtab->spaceStoreMatch+1; 
      mptr < matchtab->spaceStoreMatch+matchtab->nextfreeStoreMatch;
      mptr++)
  {
    if(mptr->Storeseqnum2 != startofgroup->Storeseqnum2)
    {
      countgroups++;
      sizeofgroup = (Uint) ((mptr-1) - startofgroup);
      if(sizeofgroup > UintConst(1))
      {
        qsort(startofgroup,
              (size_t) sizeofgroup,
              sizeof(StoreMatch),
              (Qsortcomparefunction) ordermatchpos1);
      }
      startofgroup = mptr;
    }
  }
  countgroups++;
  sizeofgroup = (Uint) ((mptr-1) - startofgroup);
  if(sizeofgroup > UintConst(1))
  {
    qsort(startofgroup,
          (size_t) sizeofgroup,
          sizeof(StoreMatch),
          (Qsortcomparefunction) ordermatchpos1);
  }
  printf("# %lu groups detected\n",(Showuint) countgroups);
}

/*@null@*/ static StoreMatch *getleftgroupbound(Uint seqnum2,
                                                StoreMatch *leftptr,
                                                StoreMatch *rightptr)
{
  Uint len;
  StoreMatch *midptr;

  if(seqnum2 == leftptr->Storeseqnum2)
  {
    return leftptr;
  }
  if(seqnum2 > rightptr->Storeseqnum2)
  {
    return NULL;
  }
  while(leftptr + 1 < rightptr)
  {
    len = (Uint) (rightptr-leftptr);
    midptr = leftptr + DIV2(len);
    if(seqnum2 <= midptr->Storeseqnum2)
    {
      rightptr = midptr;
    } else
    {
      leftptr = midptr;
    }
  }
  return rightptr;
}

/*@null@*/ static StoreMatch *getrightgroupbound(Uint seqnum2,
                                                 StoreMatch *leftptr,
                                                 StoreMatch *rightptr)
{
  Uint len;
  StoreMatch *midptr;

  if(seqnum2 == rightptr->Storeseqnum2)
  {
    return rightptr;
  }
  if(seqnum2 < leftptr->Storeseqnum2)
  {
    return NULL;
  }
  while(leftptr + 1 < rightptr)
  {
    len = (Uint) (rightptr-leftptr);
    midptr = leftptr + DIV2(len);
    if(seqnum2 >= midptr->Storeseqnum2)
    {
      leftptr = midptr;
    } else
    {
      rightptr = midptr;
    }
  }
  return leftptr;
}

static Sint seqnum2togroup(PairUint *bounds,
                           Uint seqnum2,
                           ArrayStoreMatch *matchtab)
{
  StoreMatch *leftptr, *rightptr, *leftmost, *rightmost;

  leftptr = matchtab->spaceStoreMatch;
  rightptr = matchtab->spaceStoreMatch + matchtab->nextfreeStoreMatch - 1;
  leftmost = getleftgroupbound(seqnum2,leftptr,rightptr);
  rightmost = getrightgroupbound(seqnum2,leftptr,rightptr);
  if(leftmost == NULL || rightmost == NULL || leftmost > rightmost)
  {
    ERROR1("cannot find match for sequence number %lu",(Showuint) seqnum2);
    return (Sint) -1;
  }
  bounds->uint0 = (Uint) (leftmost - matchtab->spaceStoreMatch);
  bounds->uint1 = (Uint) (rightmost - matchtab->spaceStoreMatch);
  return 0;
}

static Sint checkifoneiscontainedinother(ArrayStoreMatch *matchtab1,
                                         ArrayStoreMatch *matchtab2,
                                         Multiseq *virtualmultiseq,
                                         Multiseq *querymultiseq)
{
  StoreMatch *mptr, *tmpptr;
  BOOL foundcontained;
  PairUint bounds;
  Uint idx, countnotcontained = 0;

  printf("checkifoneiscontainedinother\n");
  (void) fflush(stdout);
  for(mptr = matchtab1->spaceStoreMatch; 
      mptr < matchtab1->spaceStoreMatch+matchtab1->nextfreeStoreMatch;
      mptr++)
  {
    if(seqnum2togroup(&bounds,
                      mptr->Storeseqnum2,
                      matchtab2) != 0)
    {
      return (Sint) -1;
    }
    if(bounds.uint0 > 0)
    {
      tmpptr = matchtab2->spaceStoreMatch + bounds.uint0 - 1;
      if(tmpptr->Storeseqnum2 >= mptr->Storeseqnum2)
      {
        ERROR3("left bound=%lu but (leftbound-1)->Storeseqnum2 = %lu >= %lu",
             (Showuint) bounds.uint0,
             (Showuint) tmpptr->Storeseqnum2,
             (Showuint) mptr->Storeseqnum2);
        return (Sint) -2;
      }
    }
    if(bounds.uint1 < matchtab2->nextfreeStoreMatch - 1)
    {
      tmpptr = matchtab2->spaceStoreMatch + bounds.uint1 + 1;
      if(tmpptr->Storeseqnum2 <= mptr->Storeseqnum2)
      {
        ERROR3("right bound=%lu but (rightbound+1)->Storeseqnum2 = %lu <= %lu",
             (Showuint) bounds.uint1,
             (Showuint) tmpptr->Storeseqnum2,
             (Showuint) mptr->Storeseqnum2);
        return (Sint) -2;
      }
    }
    foundcontained = False;
    for(idx=bounds.uint0; idx<=bounds.uint1; idx++)
    {
      if(CHECKCONTAINMENTWITHTOLERANCE(matchtab2->spaceStoreMatch + idx,
                                       mptr))
      {
        foundcontained = True;
        break;
      }
    }
    if(!foundcontained)
    {
      countnotcontained++;
      fprintf(stderr,"the following match is not contained in other table\n");
      if(simpleechomatch2file(stderr,virtualmultiseq,querymultiseq,mptr) != 0)
      {
        fprintf(stderr,"%s\n",messagespace());
      }
      exit(EXIT_FAILURE);
    }
  }
  printf("done.\n");
  (void) fflush(stdout);
  return 0;
}

Sint checkifmatchtablesarecontained(ArrayStoreMatch *matchtab1,
                                    ArrayStoreMatch *matchtab2,
                                    Multiseq *virtualmultiseq,
                                    Multiseq *querymultiseq)
{
  if(!checkifsortedbyseqnum2(matchtab1))
  {
    return (Sint) -1;
  }
  if(!checkifsortedbyseqnum2(matchtab2))
  {
    return (Sint) -2;
  }
  sortgroupsbypos1(matchtab1);
  sortgroupsbypos1(matchtab2);
  if(checkifoneiscontainedinother(matchtab1,matchtab2,
                                  virtualmultiseq,querymultiseq) != 0)
  {
    return (Sint) -1;
  }
  if(checkifoneiscontainedinother(matchtab2,matchtab1,
                                  virtualmultiseq,querymultiseq) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

MAINFUNCTION
{
  Matchinfo matchinfo1, matchinfo2;

  DEBUGLEVELSET;
  if(argc != 3)
  {
    fprintf(stderr,"Usage: %s <matchfile1> <matchfile2>\n",argv[0]);
    return EXIT_FAILURE;
  }
  ASSIGNDEFAULTSHOWDESC(&matchinfo1.outinfo.showdesc);
  if(determineMatchinfo(NULL,
                        &matchinfo1,
                        0,
                        NULL,
                        argv[1],
                        NULL,
                        NULL,
                        True,
                        NULL) != 0)
  {
    STANDARDMESSAGE;
  }
  ASSIGNDEFAULTSHOWDESC(&matchinfo2.outinfo.showdesc);
  if(determineMatchinfo(NULL,
                        &matchinfo2,
                        0,
                        NULL,
                        argv[2],
                        NULL,
                        NULL,
                        True,
                        NULL) != 0)
  {
    STANDARDMESSAGE;
  }
  if(checkifmatchtablesarecontained(&matchinfo1.matchtab,
                                    &matchinfo2.matchtab,
                                    matchinfo1.outinfo.outvms,
                                    matchinfo1.outinfo.outqms) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freeMatchinfo(&matchinfo1) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freeMatchinfo(&matchinfo2) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
