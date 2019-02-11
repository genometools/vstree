//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <limits.h>
#include "spacedef.h"
#include "debugdef.h"
#include "match.h"
#include "redblackdef.h"
#include "failures.h"

#include "redblack.pr"
#include "dictmaxsize.pr"

//}
/*
  This file implements the datatype \texttt{BestMatchlist}.
  The matches themselves are stored in a array of size \texttt{bestnumber}, 
  while references to the matches are stored in a red-black tree w.r.t.\ 
  the order defined by \texttt{cmpBestMatch}.
*/

/*
  The following function compares matches lexicographically, i.e.\ they
  are ordered w.r.t.\ \((smaller Evalue,larger length1,smaller position1)\).
*/

typedef size_t Integerkey;

static Sint cmpBestMatch(const Keytype bminfo1,
                         const Keytype bminfo2,
                         void *cmpinfo)
{
  StoreMatch *bmptr1, *bmptr2;
  
  bmptr1 = ((StoreMatch *) cmpinfo) + (Integerkey) bminfo1;
  bmptr2 = ((StoreMatch *) cmpinfo) + (Integerkey) bminfo2;
  if(bmptr1 == bmptr2)
  {
    return 0;
  }
  if(bmptr1->StoreEvalue < bmptr2->StoreEvalue)
  {
    return (Sint) 1;
  }
  if(bmptr1->StoreEvalue > bmptr2->StoreEvalue)
  {
    return (Sint) -1;
  }
  if(bmptr1->Storelength1 < bmptr2->Storelength1)
  {
    return (Sint) -1;
  }
  if(bmptr1->Storelength1 > bmptr2->Storelength1)
  {
    return (Sint) 1;
  }
  if(bmptr1->Storeposition1 < bmptr2->Storeposition1)
  {
    return (Sint) 1;
  }
  if(bmptr1->Storeposition1 > bmptr2->Storeposition1)
  {
    return (Sint) -1;
  }
  if(bmptr1->Storelength2 < bmptr2->Storelength2)
  {
    return (Sint) -1;
  }
  if(bmptr1->Storelength2 > bmptr2->Storelength2)
  {
    return (Sint) 1;
  }
  if(bmptr1->Storeposition2 < bmptr2->Storeposition2)
  {
    return (Sint) 1;
  }
  if(bmptr1->Storeposition2 > bmptr2->Storeposition2)
  {
    return (Sint) -1;
  }
  if((bmptr1->Storeflag & FLAGPALINDROMIC) ==
     (bmptr2->Storeflag & FLAGPALINDROMIC))
  {
    DEBUG4(1,"# equal matches: (%lu,%lu,%lu,%lu) = ",
            (Showuint) bmptr1->Storelength1,
            (Showuint) bmptr1->Storeposition1,
            (Showuint) bmptr1->Storelength2,
            (Showuint) bmptr1->Storeposition2);
    DEBUG4(1,"(%lu,%lu,%lu,%lu)\n",
            (Showuint) bmptr2->Storelength1,
            (Showuint) bmptr2->Storeposition1,
            (Showuint) bmptr2->Storelength2,
            (Showuint) bmptr2->Storeposition2);
    return 0;
  }
  if(bmptr2->Storeflag & FLAGPALINDROMIC)
  {
    return (Sint) 1;
  }
  if(bmptr1->Storeflag & FLAGPALINDROMIC)
  {
    return (Sint) -1;
  }
  DEBUG4(1,"# equal matches: (%lu,%lu,%lu,%lu) = ",
          (Showuint) bmptr1->Storelength1,
          (Showuint) bmptr1->Storeposition1,
          (Showuint) bmptr1->Storelength2,
          (Showuint) bmptr1->Storeposition2);
  DEBUG4(1,"(%lu,%lu,%lu,%lu)\n",
          (Showuint) bmptr2->Storelength1,
          (Showuint) bmptr2->Storeposition1,
          (Showuint) bmptr2->Storelength2,
          (Showuint) bmptr2->Storeposition2);
  return 0;
}

void showStoreMatch(const Keytype bminfo,void *showinfo)
{
  StoreMatch *bmptr;
  
  bmptr = ((StoreMatch *) showinfo) + (Integerkey) bminfo;
  printf("match=(Evalue=%.2e,length1=%lu,position1=%lu\n",
         bmptr->StoreEvalue, 
         (Showuint) bmptr->Storelength1, 
         (Showuint) bmptr->Storeposition2);
}

/*EE
  The following function initializes the \emph{bestmatch}-list.
*/

void initbml(BestMatchlist *bml,Uint bestnumber)
{
  if(bestnumber <= UintConst(32))
  {
    bml->bmincrementsize = bestnumber+1;
  } else
  {
    bml->bmincrementsize = (Uint) (1 + bestnumber/4);
  }
  INITARRAY(&bml->bmreservoir,StoreMatch);
  initDictmaxsize(&bml->bmdict,bestnumber);
}

/*EE
  The following function inserts an element referred to by 
  \texttt{matchin} into the \emph{bml}-list. The matches are 
  ordered according to the function \texttt{cmpBestMatch}.
*/

Sint insertintobml(BestMatchlist *bml,StoreMatch *matchin)
{
  StoreMatch *bmptr;
  Integerkey indexpos;

  if(bml->bmdict.currentdictsize < bml->bmdict.maxdictsize ||
     bml->bmdict.lastcalldeletedelem == NULL)
  {
    if(bml->bmdict.lastcalldeletedelem == NULL)
    {
      GETNEXTFREEINARRAY(bmptr,&bml->bmreservoir,StoreMatch,1);
    } else
    {
      GETNEXTFREEINARRAY(bmptr,&bml->bmreservoir,StoreMatch,
                         bml->bmincrementsize);
    }
    indexpos = (Integerkey) (bmptr - bml->bmreservoir.spaceStoreMatch);
  } else
  {
    indexpos = (Integerkey) bml->bmdict.lastcalldeletedelem;
    bmptr = bml->bmreservoir.spaceStoreMatch + indexpos;
    bml->bmdict.lastcalldeletedelem = NULL;
  }
  ASSIGNSTOREMATCH(bmptr,matchin);
  if(insertDictmaxsize(&bml->bmdict,
                       cmpBestMatch,
                       bml->bmreservoir.spaceStoreMatch,
                       showStoreMatch,
                       bml->bmreservoir.spaceStoreMatch,
                       (void *) indexpos) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

void freebestmatchlist(BestMatchlist *bml)
{
  FREEARRAY(&bml->bmreservoir,StoreMatch);
  redblacktreedestroy (False,NULL,NULL,bml->bmdict.root);
}

//\Ignore{

/*
  This function does not work for a list of length 1, after
  inserting the second element, which is larger than the first.
*/

/*
void insertintobml(BestMatchlist *bml,Match *matchin,
                   Uint seqnum1,Uint relposition1)
{
  Uint insindex,                     // insertion index
       newindex,                     // index where new elem is stored
       previndex = BMLLISTNIL;       // previous index

  DEBUG1(3,"insert match with evalue %.2e\n",matchin->Evalue);
  if(bml->nextfree == 0)        // list is empty
  {
    bml->space[0].next = BMLLISTNIL;
    bml->space[0].previous = BMLLISTNIL;
    STORENEWMATCH(0);
    bml->nextfree = UintConst(1);
    bml->firstelem = 0;
    bml->lastelem = 0;
  } else
  {
    // not all elements are occupied
    if(bml->nextfree < bml->allocated || 
       // or new match is better than last element
       cmpmatch(matchin,&(bml->space[bml->lastelem].match)) < 0)
    {
      if(bml->nextfree < bml->allocated)
      {
        newindex = bml->nextfree++;
      } else
      {
        newindex = bml->lastelem;
        bml->lastelem = bml->space[bml->lastelem].previous;
        bml->space[bml->lastelem].next = BMLLISTNIL;
      }
      STORENEWMATCH(newindex);    // store new match at newindex
      // find the first match in \emph{bml}-list smaller than new match
      for(insindex = bml->firstelem; 
          insindex != BMLLISTNIL; 
          insindex = bml->space[insindex].next)
      {
        if(cmpmatch(matchin,&(bml->space[insindex].match)) < 0)
        {
          break;
        }
        previndex = insindex;
      }
      // insert the match after previndex
      if(previndex == BMLLISTNIL)    // insindex refers to first element
      {
        bml->space[newindex].next = bml->firstelem;
        bml->space[newindex].previous = BMLLISTNIL;
        bml->space[insindex].previous = bml->firstelem = newindex;
      } else
      {
        if(insindex == BMLLISTNIL)   // new element becomes last in list
        {
          bml->lastelem = newindex;
          bml->space[newindex].next = BMLLISTNIL;
        } else
        {
          bml->space[newindex].next = insindex;
          bml->space[insindex].previous = newindex;
        }
        if(bml->allocated == UintConst(1) && newindex == 0)
        {
          bml->space[0].previous = bml->space[0].next = BMLLISTNIL;
        } else
        {
          bml->space[newindex].previous = previndex;
          bml->space[previndex].next = newindex;
        }
      }
    }
  }
}
*/

//}
