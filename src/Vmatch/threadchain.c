
#include "debugdef.h"
#include "outinfo.h"
#include "chaindef.h"
#include "chaincall.h"
#include "dpbitvec48.h"
#include "arraydef.h"
#include "errordef.h"
#include "threaddef.h"

#include "bitvdist.pr"
#include "procfinal.pr"

/*
  Add evalue to fillmatch
  Allow for all kinds of matches, e.g. in six reading frames
  check that all matches to chain have the same direction
  add recursive strategy to close gaps by first computing 
  exact matches.
*/

typedef struct
{
  Uint gaplength1, gaplength2, gapid;
  Uchar *gapptr1 , *gapptr2;
} Gapspecification;

DECLAREARRAYSTRUCT(Gapspecification);

#define ASSIGNANDHANDLESTRETCH(NUM)\
        vmchainthreadinfo->stretch##NUM\
          = second->Storeposition##NUM + second->Storelength##NUM\
                                       - first->Storeposition##NUM;\
        if(!vmchainthreadinfo->chaincallinfo->silent)\
        {\
          printf("# stretch%s=%lu %lu (%lu)\n",\
                #NUM,\
                (Showuint) first->Storeposition##NUM,\
                (Showuint) (second->Storeposition##NUM +\
                            second->Storelength##NUM-1),\
                (Showuint) vmchainthreadinfo->stretch##NUM);\
        }\
        if(vmchainthreadinfo->chaincallinfo->\
                              userdefinedminthreadlength##NUM > 0 &&\
           vmchainthreadinfo->stretch##NUM <\
           vmchainthreadinfo->chaincallinfo->userdefinedminthreadlength##NUM)\
        {\
          if(!vmchainthreadinfo->chaincallinfo->silent)\
          {\
            printf("# stretch%s = %lu < %lu = userdefinedminthreadlength%s\n",\
                   #NUM,\
                   (Showuint) vmchainthreadinfo->stretch##NUM,\
                   (Showuint) vmchainthreadinfo->chaincallinfo->\
                                             userdefinedminthreadlength##NUM,\
                   #NUM);\
          }\
          return 0;\
        }

#define CHECKLENGTHCONSTRAINTS(NUM)\
        if(vmchainthreadinfo->chaincallinfo->\
           userdefinedminthreaderrorpercentage##NUM > 0)\
        {\
          vmchainthreadinfo->allowederrors##NUM\
            = ((totalmatchlength##NUM + totalgap##NUM) *\
               vmchainthreadinfo->chaincallinfo->\
                                  userdefinedminthreaderrorpercentage##NUM)\
                   / 100;\
          if(vmchainthreadinfo->totalmatchdistance <=\
             vmchainthreadinfo->allowederrors##NUM)\
          {\
            vmchainthreadinfo->allowederrors##NUM \
              -= vmchainthreadinfo->totalmatchdistance;\
          } else\
          {\
            if(!vmchainthreadinfo->chaincallinfo->silent)\
            {\
              printf("# totalmatchdistance = %lu > %lu = allowederrors##NUM\n",\
                         (Showuint) vmchainthreadinfo->totalmatchdistance,\
                         (Showuint) vmchainthreadinfo->allowederrors##NUM);\
            }\
            return 0;\
          }\
        } else\
        {\
          vmchainthreadinfo->allowederrors##NUM = 0;\
        }

#define CHECKGAPDISTANCE(NUM)\
        if(vmchainthreadinfo->allowederrors##NUM > 0 &&\
           sumgapdistance > vmchainthreadinfo->allowederrors##NUM)\
        {\
          if(!vmchainthreadinfo->chaincallinfo->silent)\
          {\
            printf("# sumgapdistance=%lu > %lu = allowederrors%s "\
                   "(after %lu of %lu gaps)\n",\
                    (Showuint) sumgapdistance,\
                    (Showuint) vmchainthreadinfo->allowederrors##NUM,\
                    #NUM,\
                    (Showuint) (gapspecptr-gaptable.spaceGapspecification),\
                    (Showuint) gaptable.nextfreeGapspecification);\
          }\
          FREEARRAY(&gaptable,Gapspecification);\
          freeDPbitvectorreservoir(&dpbvres);\
          return 0;\
        }

static Sint evaluategaps(ArrayGapspecification *gaptable,
                         Vmchainthreadinfo *vmchainthreadinfo,
                         Chain *chain)
{
  Uint i, 
       totalgaplengthdiff = 0, 
       totalmatchlength1 = 0,
       totalmatchlength2 = 0,
       totalgap1 = 0,
       totalgap2 = 0;
  StoreMatch *first, *second;
  Gapspecification *gapspecptr;

  vmchainthreadinfo->totalmatchdistance = 0;
  if(!vmchainthreadinfo->chaincallinfo->silent)
  {
    printf("# chain of length %lu with score %ld\n",
            (Showuint) chain->chainedfragments.nextfreeUint,
            (Showsint) chain->scoreofchain);
  }
  first = vmchainthreadinfo->storematchtab + 
          chain->chainedfragments.spaceUint[0];
  vmchainthreadinfo->totalmatchdistance = (Uint) ABS(first->Storedistance);
  second = vmchainthreadinfo->storematchtab + 
           chain->chainedfragments.spaceUint[chain->chainedfragments.
                                             nextfreeUint-1];
  ASSIGNANDHANDLESTRETCH(1);
  ASSIGNANDHANDLESTRETCH(2);
  gaptable->allocatedGapspecification = chain->chainedfragments.nextfreeUint-1;
  ALLOCASSIGNSPACE(gaptable->spaceGapspecification,
                   NULL,
                   Gapspecification,
                   gaptable->allocatedGapspecification);
  gapspecptr = gaptable->spaceGapspecification;
  vmchainthreadinfo->maxlen = 0;
  for(i=0; i<chain->chainedfragments.nextfreeUint-1; i++)
  {
    first = vmchainthreadinfo->storematchtab + 
            chain->chainedfragments.spaceUint[i];
    second = vmchainthreadinfo->storematchtab + 
             chain->chainedfragments.spaceUint[i+1];
    vmchainthreadinfo->totalmatchdistance += (Uint) ABS(second->Storedistance);
    if(first->Storeseqnum1 != second->Storeseqnum1)
    {
      ERROR2("chain consists of matches which occur in different sequences "
             "%lu and %lu",(Showuint) first->Storeseqnum1,
                           (Showuint) second->Storeseqnum1);
      return (Sint) -1;
    }
    if(first->Storeseqnum2 != second->Storeseqnum2)
    {
      ERROR2("chain consists of matches which occur in different sequences "
             "%lu and %lu",(Showuint) first->Storeseqnum2,
                           (Showuint) second->Storeseqnum2);
      return (Sint) -1;
    }
    totalmatchlength1 += first->Storelength1;
    totalmatchlength2 += first->Storelength2;
    gapspecptr->gaplength1 = second->Storeposition1 
                             - first->Storeposition1 - first->Storelength1;
    gapspecptr->gaplength2 = second->Storeposition2 
                             - first->Storeposition2 - first->Storelength2;
    if(gapspecptr->gaplength1 < gapspecptr->gaplength2)
    {
      totalgaplengthdiff += (gapspecptr->gaplength2 - gapspecptr->gaplength1);
      if(vmchainthreadinfo->maxlen < gapspecptr->gaplength1)
      {
        vmchainthreadinfo->maxlen = gapspecptr->gaplength1;
      }
    } else
    {
      totalgaplengthdiff += (gapspecptr->gaplength1 - gapspecptr->gaplength2);
      if(vmchainthreadinfo->maxlen < gapspecptr->gaplength2)
      {
        vmchainthreadinfo->maxlen = gapspecptr->gaplength2;
      }
    }
    totalgap1 += gapspecptr->gaplength1;
    totalgap2 += gapspecptr->gaplength2;
    gapspecptr->gapptr1 = vmchainthreadinfo->outvms->sequence +
                          first->Storeposition1 + first->Storelength1;
    /*
      The following only handles the standard case. For 
      non-standard cases, like 6 frame matches, use a generalization
      as in function echomatch. XXX
    */
    if(first->Storeflag & FLAGQUERY)
    {
      gapspecptr->gapptr2 = vmchainthreadinfo->outqms->sequence +
                            first->Storeposition2 + first->Storelength2;
    } else
    {
      gapspecptr->gapptr2 = vmchainthreadinfo->outvms->sequence +
                            first->Storeposition2 + first->Storelength2;
    }
    gapspecptr->gapid = i;
    gapspecptr++;
  }
  gaptable->nextfreeGapspecification = chain->chainedfragments.nextfreeUint-1;
  CHECKLENGTHCONSTRAINTS(1);
  CHECKLENGTHCONSTRAINTS(2);
  if(!vmchainthreadinfo->chaincallinfo->silent)
  {
    printf("# totalmatchdistance = %lu\n",
            (Showuint) vmchainthreadinfo->totalmatchdistance);
    printf("# allowederrors1 = %lu\n",
            (Showuint) vmchainthreadinfo->allowederrors1);
    printf("# allowederrors2 = %lu\n",
            (Showuint) vmchainthreadinfo->allowederrors2);
    printf("# totalmatchlength1 = %lu\n",(Showuint) totalmatchlength1);
    printf("# totalmatchlength2 = %lu\n",(Showuint) totalmatchlength2);
    printf("# totalgap1 = %lu\n",(Showuint) totalgap1);
    printf("# totalgap2 = %lu\n",(Showuint) totalgap2);
    printf("# average gap1 = %.2f\n",
             (double) totalgap1/(chain->chainedfragments.nextfreeUint-1));
    printf("# average gap2 = %.2f\n",
             (double) totalgap2/(chain->chainedfragments.nextfreeUint-1));
    printf("# totalgaplengthdiff = %lu\n",(Showuint) totalgaplengthdiff);
    printf("# errorrate1 = %.2f\n",
            (double) totalgap1/(totalmatchlength1+totalgap1));
    printf("# errorrate2 = %.2f\n",
            (double) totalgap2/(totalmatchlength2+totalgap2));
    printf("# minerrorrate1 = %.5f\n",
            (double) totalgaplengthdiff/(totalmatchlength1+totalgap1));
    printf("# minerrorrate2 = %.5f\n",
            (double) totalgaplengthdiff/(totalmatchlength2+totalgap2));
  }
  return (Sint) 1;
}

static Qsortcomparereturntype comparethegaps(const void *keya,
                                             const void *keyb)
{
  if(((const Gapspecification *) keya)->gaplength1 + 
     ((const Gapspecification *) keya)->gaplength2 <
     ((const Gapspecification *) keyb)->gaplength1 + 
     ((const Gapspecification *) keyb)->gaplength2)
  {
    return (Qsortcomparereturntype) -1;
  }
  if(((const Gapspecification *) keya)->gaplength1 + 
     ((const Gapspecification *) keya)->gaplength2 >
     ((const Gapspecification *) keyb)->gaplength1 + 
     ((const Gapspecification *) keyb)->gaplength2)
  {
    return (Qsortcomparereturntype) 1;
  }
  return 0;
}

static void sortthegaps(ArrayGapspecification *gaptable)
{
  if(gaptable->nextfreeGapspecification > UintConst(1))
  {
    qsort((void *) gaptable->spaceGapspecification,
          (size_t) gaptable->nextfreeGapspecification,
          sizeof(Gapspecification),
          comparethegaps);
  }
}

static Uint gapdistance(DPbitvectorreservoir *dpbvres,
                        Uint mapsize,
                        Gapspecification *gapspecptr)
{
  return distanceofstrings(dpbvres,
                           mapsize,
                           gapspecptr->gapptr1,
                           gapspecptr->gaplength1,
                           gapspecptr->gapptr2,
                           gapspecptr->gaplength2);
}

static BOOL acceptsinglematch(Chaincallinfo *chaincallinfo,
                              StoreMatch *storematch)
{
  if(chaincallinfo->userdefinedminthreadlength1 > 0 &&
     storematch->Storelength1 <
     chaincallinfo->userdefinedminthreadlength1)
  {
    return False;
  }
  if(chaincallinfo->userdefinedminthreadlength2 > 0 &&
     storematch->Storelength2 <
     chaincallinfo->userdefinedminthreadlength2)
  {
    return False;
  }
  if(storematch->Storedistance > 0)
  {
    Uint distvalue = (Uint) ABS(storematch->Storedistance);
    if(chaincallinfo->userdefinedminthreaderrorpercentage1 > 0 &&
        distvalue > storematch->Storelength1 * 
                    chaincallinfo->userdefinedminthreaderrorpercentage1 / 100)
    {
      return False;
    }
    if(chaincallinfo->userdefinedminthreaderrorpercentage2 > 0 &&
        distvalue > storematch->Storelength2 * 
                    chaincallinfo->userdefinedminthreaderrorpercentage2 / 100)
    {
      return False;
    }
  }
  return True;
}

Sint threadchainedfragments(void *info,Chain *chain)
{
  Vmchainthreadinfo *vmchainthreadinfo = (Vmchainthreadinfo *) info;
  ArrayGapspecification gaptable;
  Gapspecification *gapspecptr;
  Uint sumgapdistance = 0;
  Sint retval;
  StoreMatch fillmatch, *firstmatch;
  DPbitvectorreservoir dpbvres;
  Outinfo *outinfo;

  INITARRAY(&gaptable,Gapspecification);
  outinfo = (Outinfo *) vmchainthreadinfo->voidoutinfo;
  if(chain->chainedfragments.nextfreeUint == UintConst(1))
  {
    firstmatch = vmchainthreadinfo->storematchtab +
                 chain->chainedfragments.spaceUint[0];
    //printf("finalprocessthread(fillmatch)=%s\n",
            //vmchainthreadinfo->finalprocessthread->functionname);
    if(acceptsinglematch(vmchainthreadinfo->chaincallinfo,firstmatch))
    {
      if(applymatchfun(vmchainthreadinfo->finalprocessthread,
                       outinfo->outvms,
                       outinfo->outqms,
		       firstmatch) != 0)
      {
        return (Sint) -1;
      }
    }
    return 0;
  }
  retval = evaluategaps(&gaptable,vmchainthreadinfo,chain);
  if(retval < 0)
  {
    return (Sint) -2;
  }
  if(retval == 0)  // chain is not valid
  {
    return 0;
  }
  sortthegaps(&gaptable);
  NOTSUPPOSEDTOBENULL(gaptable.spaceGapspecification);
  initDPbitvectorreservoir(&dpbvres,
                           vmchainthreadinfo->mapsize,
                           vmchainthreadinfo->maxlen);
  for(gapspecptr = gaptable.spaceGapspecification;
      gapspecptr < gaptable.spaceGapspecification +
                   gaptable.nextfreeGapspecification; gapspecptr++)
  {
    sumgapdistance += gapdistance(&dpbvres,
                                  vmchainthreadinfo->mapsize,
                                  gapspecptr);
    CHECKGAPDISTANCE(1);
    CHECKGAPDISTANCE(2);
  }
  freeDPbitvectorreservoir(&dpbvres);
  FREEARRAY(&gaptable,Gapspecification);
  if(!vmchainthreadinfo->chaincallinfo->silent)
  {
    printf("# sumgapdistance=%lu\n",(Showuint) sumgapdistance);
  }
  firstmatch = vmchainthreadinfo->storematchtab +
               chain->chainedfragments.spaceUint[0];
  fillmatch.idnumber = vmchainthreadinfo->supermatchcounter++;
  fillmatch.Storeflag = firstmatch->Storeflag;
  fillmatch.Storedistance = (Sint) (vmchainthreadinfo->totalmatchdistance + 
                                     sumgapdistance);
  fillmatch.Storeposition1 = firstmatch->Storeposition1;
  fillmatch.Storelength1 = vmchainthreadinfo->stretch1;
  fillmatch.Storeposition2 = firstmatch->Storeposition2;
  fillmatch.Storelength2 = vmchainthreadinfo->stretch2;
  fillmatch.Storeseqnum1 = firstmatch->Storeseqnum1;
  fillmatch.Storerelpos1 = firstmatch->Storerelpos1;
  fillmatch.Storeseqnum2 = firstmatch->Storeseqnum2;
  fillmatch.Storerelpos2 = firstmatch->Storerelpos2;
  fillmatch.StoreEvalue = 0.0;
  //printf("finalprocessthread(fillmatch)=%s\n",
          //vmchainthreadinfo->finalprocessthread->functionname);
  if(applymatchfun(vmchainthreadinfo->finalprocessthread,
                   outinfo->outvms,
                   outinfo->outqms,
                   &fillmatch) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}
