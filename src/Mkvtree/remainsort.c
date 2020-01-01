#include "types.h"
#include "arraydef.h"
#include "divmodmul.h"
#include "failures.h"
#include "spacedef.h"
#include "chardef.h"
#include "minmax.h"
#include "gqueue-if.h"
#include "gqueueproc.h"

#include "mkvaux.h"

#include "genericqueue.pr"

typedef struct
{
  Uint key,
       suffixstart;
} Itventry;

typedef struct
{
  Suffixptr *left, 
            *right;
  Uint depth;
} Pairsuffixptr;

typedef struct
{
  Uint *inversesuftab;
  Suffixptr *sortedsuffixes;
  Uint countovermaxdepthsingle;
  Genericqueue *rangestobesorted;
  DefinedUint previousdepth;
  Uint totallength;
  Uchar *sequence;
} Rmnsufinfo;

/*@ignore@*/
Rmnsufinfo *initRmnsufinfo(Suffixptr *sortedsuffixes,Uint totallength,
                           Uchar *sequence)
{
  Rmnsufinfo *rmnsufinfo;

  ALLOCASSIGNSPACE(rmnsufinfo,NULL,Rmnsufinfo,UintConst(1));
  NOTSUPPOSEDTOBENULL(rmnsufinfo);
  rmnsufinfo->sortedsuffixes = sortedsuffixes;
  rmnsufinfo->countovermaxdepthsingle = 0;
  rmnsufinfo->rangestobesorted = emptyqueuegeneric();
  rmnsufinfo->previousdepth.defined = False;
  rmnsufinfo->previousdepth.uintvalue = 0;
  rmnsufinfo->totallength = totallength;
  rmnsufinfo->sequence = sequence;
  return rmnsufinfo;
}
 /*@end@*/

#undef CHECKOUT
#ifdef CHECKOUT

static void showUinttable(const char *comment,const Uint *tab,Uint len)
{
  Uint i;

  for(i=0; i<len; i++)
  {
    printf("%s[%u] = %u\n",comment,i,tab[i]);
  }
}

#endif

static Sint compareprefix(Uint depth,const Uchar *sequence,Uint start1,
                          Uint start2,Uint totallength)
{
  Uchar cc1, cc2;
  Uint pos1, pos2, end1, end2;

  end1 = end2 = totallength;
  if(depth > 0)
  {
    if(start1 + depth < end1)
    {
      end1 = start1 + depth;
    }
    if(start2 + depth < end2)
    {
      end2 = start2 + depth;
    }
  }
  for(pos1=start1, pos2=start2; pos1 < end1 && pos2 < end2; pos1++, pos2++)
  {
    cc1 = sequence[pos1];
    cc2 = sequence[pos2];
    if(ISSPECIAL(cc1))
    {
      if(ISSPECIAL(cc2))
      {
        if(pos1 < pos2)
        {
          return (Sint) -1; // a < b
        }
        if(pos1 > pos2)
        {
          return (Sint) 1; // a > b
        }
        return 0; // a = b
      }
      return (Sint) (Sint) 1; // a > b
    } else
    {
      if(ISSPECIAL(cc2))
      {
        return (Sint) -1; // a < b
      } 
      if(cc1 < cc2)
      {
        return (Sint) -1;
      }
      if(cc1 > cc2)
      {
        return (Sint) 1;
      }
    }
  }
  return 0;
}

static void showlocalsuffix(FILE *fpout,const Uchar *sequence,
                            Uint start,Uint end,Uint totallength)
{
  Uint i;
 
  for(i = start; i <= end; i++)
  {
    if(end == totallength || ISSPECIAL(sequence[i]))
    {
      (void) putc('~',fpout);
      break;
    }
    (void) putc((Fputcfirstargtype) "acgt"[(Sint) sequence[i]],fpout);
  }
}

static void checksuffixorder(const Rmnsufinfo *rmnsufinfo,Uint depth,
                             const Suffixptr *left,
                             const Suffixptr *right)
{
  const Suffixptr *ptr;
  Uint end1, end2;
  Sint cmp;

#ifdef CHECKOUT
  showUinttable("suftab",rmnsufinfo->sortedsuffixes,
                rmnsufinfo->totallength+1);
  printf("checksuffixorder(%lu,%lu,%lu)\n",
         (Showuint) depth,
         (Showuint) (left-rmnsufinfo->sortedsuffixes),
         (Showuint) (right-rmnsufinfo->sortedsuffixes));
#endif
  for(ptr = left + 1; ptr <= right; ptr++)
  {
    cmp = compareprefix(depth,
                        rmnsufinfo->sequence,
                        *(ptr-1),
                        *ptr,
                        rmnsufinfo->totallength);
    if(cmp > 0)
    {
      fprintf(stderr,"compareprefix(%lu=\"",
                       (Showuint) *(ptr-1));
      if(depth == 0)
      {
        end1 = rmnsufinfo->totallength;
        end2 = rmnsufinfo->totallength;
      } else
      {
        end1 = MIN(rmnsufinfo->totallength,*(ptr-1)+depth-1);
        end2 = MIN(rmnsufinfo->totallength,*ptr+depth-1);
      }
      showlocalsuffix(stderr,rmnsufinfo->sequence,*(ptr-1),end1,
                      rmnsufinfo->totallength);
      fprintf(stderr,"\",");
      showlocalsuffix(stderr,rmnsufinfo->sequence,*ptr,end2,
                      rmnsufinfo->totallength);
      fprintf(stderr,"\"=%lu)=%ld\n",(Showuint) *ptr,(Showsint) cmp);
      exit(EXIT_FAILURE);
    }
  }
}

void addunsortedrange(Rmnsufinfo *rmnsufinfo,
                      Uint depth,
                      Suffixptr *left,Suffixptr *right)
{
  if(left == right)
  {
    printf("already sorted(%lu,%lu,%lu)\n",
            (Showuint) depth,(Showuint) (left-rmnsufinfo->sortedsuffixes),
                  (Showuint) (right-rmnsufinfo->sortedsuffixes));
    rmnsufinfo->countovermaxdepthsingle++;
  } else
  {
    Pairsuffixptr *pairptr;

    pairptr = malloc(sizeof(Pairsuffixptr));
    if(pairptr == NULL)
    {
      fprintf(stderr,"malloc(Pairsuffixptr) failed\n");
      exit(EXIT_FAILURE);
    }
    if(!rmnsufinfo->previousdepth.defined)
    {
      printf("new level with depth = %lu\n",(Showuint) depth);
      rmnsufinfo->previousdepth.defined = True;
      rmnsufinfo->previousdepth.uintvalue = depth;
    } else
    {
      if(rmnsufinfo->previousdepth.uintvalue < depth)
      {
        //Uint i;
        printf("new level with depth = %lu\n",(Showuint) depth);
        rmnsufinfo->previousdepth.defined = True;
        rmnsufinfo->previousdepth.uintvalue = depth;
        /*
        for(i=0; i<=rmnsufinfo->totallength; i++)
        {
          printf("inverse[%u]=%u\n",i,rmnsufinfo->inversesuftab[i]);
        }
        */
        /* complete last phase */
      } else
      {
        if(rmnsufinfo->previousdepth.uintvalue > depth)
        {
          NOTSUPPOSED;
        }
      }
    }
    pairptr->depth = depth;
    pairptr->left = left;
    pairptr->right = right;
#ifdef CHECKOUT
    printf("enqueue(%u,%u,%u)\n",depth,(Uint) (left-rmnsufinfo->sortedsuffixes),
                                 (Uint) (right-rmnsufinfo->sortedsuffixes));
#endif
    checksuffixorder(rmnsufinfo,depth,left,right);
    enqueuegeneric(rmnsufinfo->rangestobesorted,pairptr);
  }
}

static Qsortcomparereturntype compareitv(const void *a,const void *b)
{
  const Itventry *itva = (const Itventry *) a,
                 *itvb = (const Itventry *) b;

  if(itva->key < itvb->key)
  {
    return (Qsortcomparereturntype) -1;
  }
  if(itva->key > itvb->key)
  {
    return (Qsortcomparereturntype) 1;
  }
  return 0;
}

static void setinversesuftab(Rmnsufinfo *rmnsufinfo,Uint idx,Uint value)
{
  rmnsufinfo->inversesuftab[idx] = value;
#ifdef CHECKOUT
  printf("set inverse[%u]=%u\n",idx,value);
#endif
}

static void inverserange(Rmnsufinfo *rmnsufinfo,
                         Suffixptr *left,Suffixptr *right)
{
  Uint startindex;
  Suffixptr *ptr;

  startindex = (Uint) (left - rmnsufinfo->sortedsuffixes);
  for(ptr=left; ptr<=right; ptr++)
  {
    setinversesuftab(rmnsufinfo,*ptr,startindex);
  }
}


static void sortitv(Rmnsufinfo *rmnsufinfo,
                    Uint depth,Suffixptr *left,Suffixptr *right)
{
  Itventry *itvinfo;
  Uint i, rangestart, startindex;
  const Uint len = (Uint) (right - left + 1);

  ALLOCASSIGNSPACE(itvinfo,NULL,Itventry,len);
#ifdef CHECKOUT
  printf("current suftab\n");
  showUinttable("suftab",rmnsufinfo->sortedsuffixes,
                rmnsufinfo->totallength+1);
  showUinttable("inversesuftab",rmnsufinfo->inversesuftab,
                rmnsufinfo->totallength+1);
  printf("bucket of length %u\n",len);
#endif
  for(i=0; i<len; i++)
  {
#ifdef CHECKOUT
    printf("bucket %u=%u ",i,left[i]);
#endif
    itvinfo[i].suffixstart = left[i];
    if(left[i]+depth > rmnsufinfo->totallength)
    {
      fprintf(stderr,"left[%lu]+depth=%lu+%lu=%lu>%lu\n",
              (Showuint) i,
              (Showuint) left[i],
              (Showuint) depth,
              (Showuint) (left[i]+depth),
              (Showuint) rmnsufinfo->totallength);
      exit(EXIT_FAILURE);
    }
    itvinfo[i].key = rmnsufinfo->inversesuftab[left[i]+depth];
#ifdef CHECKOUT
    printf("key = %u\n",itvinfo[i].key);
#endif
  }
  qsort(itvinfo,(size_t) len,sizeof(Itventry),compareitv);
#ifdef CHECKOUT
  printf("buckets in %u-sorted order\n",MULT2(depth));
  for(i=0; i<len; i++)
  {
    printf("bucket %u=%u with key %u\n",i,itvinfo[i].suffixstart,
                                          itvinfo[i].key);
  }
#endif
  for(i=0; i<len; i++)
  {
    left[i] = itvinfo[i].suffixstart;
  }
  rangestart = 0;
  startindex = (Uint) (left - rmnsufinfo->sortedsuffixes);
  for(i=UintConst(1); i<len; i++)
  {
    if(itvinfo[i-1].key != itvinfo[i].key)
    {
      if(rangestart + 1 < i)
      {
        addunsortedrange(rmnsufinfo,
                         MULT2(depth),
                         left + rangestart,
                         left + i - 1);
        inverserange(rmnsufinfo,
                     left + rangestart,
                     left + i - 1);
      } else
      {
        setinversesuftab(rmnsufinfo,left[rangestart],startindex+rangestart);
      }
      rangestart = i;
    }
  }
  if(rangestart + 1 < len)
  {
    addunsortedrange(rmnsufinfo,
                     MULT2(depth),
                     left + rangestart,
                     left + len - 1);
    inverserange(rmnsufinfo,
                 left + rangestart,
                 left + len - 1);
  } else
  {
    setinversesuftab(rmnsufinfo,left[rangestart],startindex+rangestart);
  }
  FREESPACE(itvinfo);
}

static Sint putleftbound(void *contents,void *info)
{
  Pairsuffixptr *pairptr = (Pairsuffixptr *) contents; 

  inverserange((Rmnsufinfo *) info,pairptr->left,pairptr->right);
  return 0;
}

static void processRmnsufinfo(Rmnsufinfo *rmnsufinfo,Uint totallength)
{
  Pairsuffixptr *pairptr;
  Uint i;

  printf("# countovermaxdepth=%lu\n",
           (Showuint) sizeofgenericqueue(rmnsufinfo->rangestobesorted));
  printf("# countovermaxdepthsingle=%lu\n",
          (Showuint) rmnsufinfo->countovermaxdepthsingle);
  ALLOCASSIGNSPACE(rmnsufinfo->inversesuftab,NULL,Uint,totallength+1);
  for(i=0; i<= totallength; i++)
  {
    rmnsufinfo->inversesuftab[rmnsufinfo->sortedsuffixes[i]] = i;
  }
  (void) overallqueuelementsgeneric(rmnsufinfo->rangestobesorted,
                                    putleftbound,
                                    rmnsufinfo);
  printf("# countovermaxdepth=%lu\n",
           (Showuint) sizeofgenericqueue(rmnsufinfo->rangestobesorted));
  while(!queueisemptygeneric(rmnsufinfo->rangestobesorted))
  {
    pairptr = dequeuegeneric(rmnsufinfo->rangestobesorted);
    NOTSUPPOSEDTOBENULL(pairptr);
#ifdef CHECKOUT
    printf("dequeue %u %u %u\n",
            pairptr->depth,
            (Uint) (pairptr->left-rmnsufinfo->sortedsuffixes),
            (Uint) (pairptr->right-rmnsufinfo->sortedsuffixes));
#endif
    sortitv(rmnsufinfo,
            pairptr->depth,
            pairptr->left,
            pairptr->right);
    free(pairptr);
  }
  checksuffixorder(rmnsufinfo,0,rmnsufinfo->sortedsuffixes,
                   rmnsufinfo->sortedsuffixes+rmnsufinfo->totallength);
  wrapqueuegeneric(False,&rmnsufinfo->rangestobesorted);
  FREESPACE(rmnsufinfo->inversesuftab);
}

void wrapRmnsufinfo(Rmnsufinfo **rmnsufinfo,Uint totallength)
{
  processRmnsufinfo(*rmnsufinfo,totallength);
  FREESPACE(*rmnsufinfo);
}

/*
  AATAATATATATAAAAAAAAAA
  AAAAATGR
*/
