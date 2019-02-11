#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "debugdef.h"
#include "redblackdef.h"
#include "intbits.h"
#include "arraydef.h"
#include "errordef.h"

#include "redblack.pr"

static Sint compareregions(const Keytype key,
                           const Keytype treeelem, /*@unused@*/ void *info)
{
  if(((PairUint *) key)->uint0 < ((PairUint *) treeelem)->uint0)
  {
    return (Sint) -1;
  }
  if(((PairUint *) key)->uint0 > ((PairUint *) treeelem)->uint0)
  {
    return (Sint) 1;
  }
  return 0;
}

static Sint checkregioncontainment(void *regiontreeroot,PairUint *region)
{
  PairUint *previouselement;

  NOTSUPPOSEDTOBENULL(regiontreeroot);
  previouselement
    = (PairUint *) redblacktreepreviousequalkey((const Keytype) region,
                                                regiontreeroot,
                                                compareregions,
                                                NULL);
  NOTSUPPOSEDTOBENULL(previouselement);
  if(region->uint0 < previouselement->uint0 ||
     region->uint1 > previouselement->uint1)
  {
    ERROR2("region (%lu,%lu) does not occur in table",
           (Showuint) region->uint0, (Showuint) region->uint1);
    return (Sint) -1;
  }
  return 0;
}

static Sint collectregions (const Keytype key,
                            VISIT which,
                            /*@unused@ */ Uint depth,
                            void *info)
{
  if (which == postorder || which == leaf)
  {
    ArrayPairUint *mergedregions = (ArrayPairUint *) info;
    PairUint *region = (PairUint *) key;

    CHECKARRAYSPACE (mergedregions, PairUint, 128);
    mergedregions->spacePairUint[mergedregions->nextfreePairUint++]
      = *region;
  }
  return 0;
}

static Sint checkforoverlappingregions (ArrayPairUint *regions)
{
  PairUint *pptr;

  pptr = regions->spacePairUint;
  DEBUG2(3,"(%lu,%lu)\n",(Showuint) pptr->uint0, (Showuint) pptr->uint1);
  for (pptr=regions->spacePairUint+1;
       pptr < regions->spacePairUint + 
              regions->nextfreePairUint; 
       pptr++)
  {
    DEBUG2(2,"(%lu,%lu)\n",
            (Showuint) pptr->uint0,
            (Showuint) pptr->uint1);
    if ((pptr-1)->uint1 + 1 >= pptr->uint0)
    {
      ERROR4("(%lu,%lu) overlaps with (%lu,%lu)",
             (Showuint) (pptr-1)->uint0,
             (Showuint) (pptr-1)->uint1,
             (Showuint) pptr->uint0,
             (Showuint) pptr->uint1);
      return (Sint) -1;
    }
  }
  printf("no overlaps for %lu merged regions detected\n",
           (Showuint) regions->nextfreePairUint);
  return 0;
}

static Sint checkallregionsforcontainment(void *regiontreeroot,
                                          ArrayPairUint *regions)
{
  Uint i;

  for (i = 0; i < regions->nextfreePairUint; i++)
  {
    DEBUG3 (2, "%lu: check region (%lu,%lu)\n", 
            (Showuint) i,
            (Showuint) regions->spacePairUint[i].uint0,
            (Showuint) regions->spacePairUint[i].uint1);
    if (checkregioncontainment (regiontreeroot, 
                                &regions->spacePairUint[i]) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static void markbitsintable(Uint *marktable,Uint left,Uint right)
{
  Uint i;
  for (i=left; i<=right; i++)
  {
    SETIBIT(marktable,i);
  }
}

static void markallregions(Uint *marktable,ArrayPairUint *regions)
{
  PairUint *pptr;

  for (pptr=regions->spacePairUint;
       pptr < regions->spacePairUint + regions->nextfreePairUint; 
       pptr++)
  {
    markbitsintable(marktable,pptr->uint0,pptr->uint1);
  }
}

static void comparemarkedbits(Uint *marktable1,Uint *marktable2,Uint width)
{
  Uint i;
  BOOL ismarked1, ismarked2;

  for(i=0; i<width; i++)
  {
    ismarked1 = ISIBITSET(marktable1,i) ? True : False;
    ismarked2 = ISIBITSET(marktable2,i) ? True : False;
    if(ismarked1 != ismarked2)
    {
      fprintf(stderr,"index %lu: ismarked1=%s != %s=ismarked2\n",
           (Showuint) i,SHOWBOOL(ismarked1),SHOWBOOL(ismarked2));
      exit(EXIT_FAILURE);
    }
  }
  printf("tables of width %lu are identical\n",(Showuint) width);
}

Sint verifyregiontree(void *regiontreeroot,Uint width,
                      ArrayPairUint *originalregions)
{
  ArrayPairUint mergedregions;
  Uint *markmerged, *markorig;

  INITARRAY (&mergedregions, PairUint);
  if (redblacktreewalk (regiontreeroot,
                        collectregions, &mergedregions) != 0)
  {
    return (Sint) -1;
  }
  if(checkforoverlappingregions (&mergedregions) != 0)
  {
    return (Sint) -2;
  }
  if(checkallregionsforcontainment(regiontreeroot,originalregions) != 0)
  {
    return (Sint) -3;
  }
  INITBITTAB(markmerged,width);
  INITBITTAB(markorig,width);
  markallregions(markmerged,&mergedregions);
  markallregions(markorig,originalregions);
  comparemarkedbits(markmerged,markorig,width);
  FREESPACE(markmerged);
  FREESPACE(markorig);
  FREEARRAY (&mergedregions, PairUint);
  return 0;
}

static Sint mergewithonepreviouselement(void **regiontreeroot, 
                                        PairUint **node2storeregion,
                                        PairUint *region)
{
  PairUint *previouselement;

  if(*regiontreeroot == NULL)
  {
    return 0;
  }
  previouselement
    = (PairUint *) redblacktreepreviousequalkey((const Keytype) region,
                                                *regiontreeroot,
                                                compareregions,
                                                NULL);
  if(previouselement != NULL && previouselement->uint1 + 1 >= region->uint0)
  {
    DEBUG4(2,"left merge of previous (%lu,%lu) with (%lu,%lu)\n",
           (Showuint) previouselement->uint0,
           (Showuint) previouselement->uint1,
           (Showuint) region->uint0,
           (Showuint) region->uint1);
    region->uint0 = previouselement->uint0;
    if(previouselement->uint1 > region->uint1)
    {
      region->uint1 = previouselement->uint1;
    }
    *node2storeregion = previouselement;
  } 
  return 0;
}

static Sint mergewithallrightelements(void **regiontreeroot, 
                                      PairUint **node2storeregion,
                                      PairUint *region)
{
  PairUint *nextelement;

  while(True)
  {
    nextelement = (PairUint *) redblacktreenextkey((const Keytype) region,
                                                   *regiontreeroot,
                                                   compareregions,
                                                   NULL);
   
    if(nextelement != NULL && region->uint1 + 1 >= nextelement->uint0)
    {
      DEBUG2(2,"right merge with (%lu,%lu)\n",
             (Showuint) nextelement->uint0,(Showuint) nextelement->uint1);
      if(region->uint1 <= nextelement->uint1)
      {
        region->uint1 = nextelement->uint1;
        if(*node2storeregion == NULL)
        {
          *node2storeregion = nextelement;
        } else
        {
          if(redblacktreedelete((const Keytype) nextelement,
                                regiontreeroot,
                                compareregions, NULL) != 0)
          {
            ERROR2("cannot delete (%lu,%lu)",
                   (Showuint) nextelement->uint0,
                   (Showuint) nextelement->uint1);
            return (Sint) -1;
          }
          free(nextelement);
        }
        break;
      }
      
      if(redblacktreedelete((const Keytype) nextelement,
                            regiontreeroot,
                            compareregions, NULL) != 0)
      {
        ERROR2("cannot delete (%lu,%lu)",
               (Showuint) nextelement->uint0,
               (Showuint) nextelement->uint1);
        return (Sint) -1;
      }
      free(nextelement);
    } else
    {
      break;
    }
  }
  return 0;
}

static Sint dotheregioninsertion(void **regiontreeroot, 
                                 BOOL makedatacopy,
                                 BOOL *nodecreated, 
                                 PairUint *region)
{
  if(makedatacopy)
  {
    PairUint *copyofregion;
    copyofregion = malloc(sizeof(PairUint));
    if(copyofregion == NULL)
    {
      ERROR0("cannot allocate space for copyofregion");
      return (Sint) -1;
    }
    *copyofregion = *region;
    (void) redblacktreesearch((const Keytype) copyofregion,
                              nodecreated,
                              regiontreeroot, compareregions,
                              NULL);
  } else
  {
    (void) redblacktreesearch((const Keytype) region,
                              nodecreated,
                              regiontreeroot, compareregions,
                              NULL);
  }
  return 0;
}


Sint insertnewregion(void **regiontreeroot, 
                     BOOL makedatacopy,
                     BOOL *nodecreated, 
                     PairUint *region)
{
  if(*regiontreeroot == NULL)
  {
    if(dotheregioninsertion(regiontreeroot, 
                            makedatacopy,
                            nodecreated, 
                            region) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    PairUint *node2storeregion = NULL;

    if(mergewithonepreviouselement(regiontreeroot, 
                                   &node2storeregion, region) != 0)
    {
      return (Sint) -2;
    }
    if(mergewithallrightelements(regiontreeroot, 
                                 &node2storeregion, region) != 0)
    {
      return (Sint) -3;
    }
    if(node2storeregion == NULL)
    {
      DEBUG2(2, "insert (%lu,%lu)\n",
                 (Showuint) region->uint0, (Showuint) region->uint1);
      if(dotheregioninsertion(regiontreeroot, 
                              makedatacopy,
                              nodecreated, 
                              region) != 0)
      {
        return (Sint) -4;
      }
      if(!(*nodecreated))
      {
        ERROR2("no node for insert (%lu,%lu) created\n",
                 (Showuint) region->uint0, (Showuint) region->uint1);
        return (Sint) -5;
      }
    } else
    {
      *nodecreated = False;
      *node2storeregion = *region;
    }
  }
  return 0;
}
