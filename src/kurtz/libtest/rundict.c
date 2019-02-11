#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "debugdef.h"
#include "types.h"
#include "arraydef.h"
#include "qsortdef.h"
#include "alphadef.h"
#include "genfile.h"
#include "redblackdef.h"

#include "remdups.pr"
#include "redblack.pr"
#include "dictmaxsize.pr"
#include "alphabet.pr"

#define READINT(VAR,ARGNUM)\
        if(sscanf(argv[ARGNUM],"%ld",(Scaninteger *) &readint) != 1 ||\
           readint <= 0)\
        {\
          fprintf(stderr,"%s: argument %s is not a positive integer\n",\
                  argv[0],argv[ARGNUM]);\
          return EXIT_FAILURE;\
        }\
        VAR = (Uint) readint

#define DIVISOR     10
#define DICTSIZE(K) ((K)/DIVISOR)

typedef struct
{
  Uint dictkey;
  char *value;
} Dictelem;

DECLAREARRAYSTRUCT(Dictelem);

static Sint storeDictelem (const Keytype key,
                           const VISIT which,
                           /*@unused@*/ const Uint depth,
                           void *storeinfo)
{
  if(which == postorder || which == leaf)
  {
    Dictelem *datap = (Dictelem *) key;
    ArrayDictelem *outlist = (ArrayDictelem *) storeinfo;
    outlist->spaceDictelem[outlist->nextfreeDictelem++].dictkey 
      = datap->dictkey;
  }
  return 0;
}


static Sint cmpDictelem(const Keytype keya, 
                        const Keytype keyb,
                        /*@unused@*/ void *cmpinfo)
{
  Dictelem *dicta = (Dictelem *) keya,
           *dictb = (Dictelem *) keyb;

  DEBUG2(3,"compare %lu and %lu => result = ",
           (Showuint) dicta->dictkey,
           (Showuint) dictb->dictkey);
  if(dicta->dictkey < dictb->dictkey)
  {
    DEBUG0(3,"-1\n");
    return -1;
  }
  if(dicta->dictkey > dictb->dictkey)
  {
    DEBUG0(3,"1\n");
    return 1;
  }
  DEBUG0(3,"0\n");
  return 0;
}

static Sint cmpUint(const Keytype keya, 
                    const Keytype keyb,
                    /*@unused@*/ void *cmpinfo)
{
  DEBUG2(3,"compare %lu and %lu => result = ",
           (Showuint) *((Uint *) keya),
           (Showuint) *((Uint *) keyb));
  if(*((Uint *) keya) < *((Uint *) keyb))
  {
    DEBUG0(3,"-1\n");
    return -1;
  } else
  {
    if(*((Uint *) keya) > *((Uint *) keyb))
    {
      DEBUG0(3,"1\n");
      return 1;
    }
  }
  DEBUG0(3,"0\n");
  return 0;
}

static Qsortcomparereturntype qsortcmpDictelem(const Keytype keya, 
                                               const Keytype keyb)
{
  return (Qsortcomparereturntype) cmpDictelem(keya,keyb,NULL);
}

static void showDictelem(const Keytype key, 
                         /*@unused@*/ void *showinfo)
{
  Dictelem *dictelem = (Dictelem *) key;
  printf("%lu",(Showuint) dictelem->dictkey);
}

static void remdupsDictelem(ArrayDictelem *ar)
{
  Dictelem *storeptr, *readptr;

  if(ar->nextfreeDictelem > 0)
  {
    for(storeptr = ar->spaceDictelem, readptr = ar->spaceDictelem+1;
        readptr < ar->spaceDictelem + ar->nextfreeDictelem;
        readptr++)
    {
      if(storeptr->dictkey != readptr->dictkey)
      {
        storeptr++;
        *storeptr = *readptr;
      }
    }
    ar->nextfreeDictelem = (Uint) (storeptr - ar->spaceDictelem + 1);
  }
}

static void checkoutput(ArrayDictelem *inlist,ArrayDictelem *outlist)
{
  Sint i;
  Uint j;

  printf("size of inlist = %lu\n",(Showuint) inlist->nextfreeDictelem);
  printf("size of outlist = %lu\n",(Showuint) outlist->nextfreeDictelem);
  qsort(inlist->spaceDictelem,
        (size_t) inlist->nextfreeDictelem,
        sizeof(Dictelem),
        (Qsortcomparefunction) qsortcmpDictelem);
  remdupsDictelem(inlist);
  if(inlist->nextfreeDictelem < outlist->nextfreeDictelem)
  {
    fprintf(stderr,"size of inlist = %lu != %lu\n",
           (Showuint) inlist->nextfreeDictelem,
           (Showuint) outlist->nextfreeDictelem);
    exit(EXIT_FAILURE);
  }
  for(i=(Sint) (outlist->nextfreeDictelem-1),
      j = inlist->nextfreeDictelem-1;
      i>=0;
      i--, j--)
  {
    if(inlist->spaceDictelem[j].dictkey != 
       outlist->spaceDictelem[i].dictkey)
    {
      fprintf(stderr,"inlist[%lu]= %lu != %lu = outlist[%ld]\n",
             (Showuint) j,
             (Showuint) inlist->spaceDictelem[j].dictkey,
             (Showuint) outlist->spaceDictelem[i].dictkey,
             (Showsint) i);
      exit(EXIT_FAILURE);
    }
  }
}

static Sint checkdictfunctions(ArrayDictelem *inlist,
                               ArrayDictelem *outlist,
                               Uint *randomnumbers)
{
  Dictmaxsize dict;
  Uint i;

  initDictmaxsize(&dict,DICTSIZE(inlist->nextfreeDictelem));
  ALLOCASSIGNSPACE(inlist->spaceDictelem,NULL,Dictelem,
                   inlist->nextfreeDictelem);
  for(i=0; i<inlist->nextfreeDictelem; i++)
  {
    inlist->spaceDictelem[i].dictkey = randomnumbers[i];
  }
  for(i=0; i<inlist->nextfreeDictelem; i++)
  {
    if(insertDictmaxsize(&dict,
                         cmpDictelem,
                         NULL,
                         showDictelem,
                         NULL,
                         (void *) &inlist->spaceDictelem[i]) != 0)
    {
      return -1;
    }
  }
  outlist->nextfreeDictelem = 0;
  ALLOCASSIGNSPACE(outlist->spaceDictelem,NULL,Dictelem,
                   DICTSIZE(inlist->nextfreeDictelem));
  outlist->allocatedDictelem = DICTSIZE(inlist->nextfreeDictelem);
  if(redblacktreewalk(dict.root,storeDictelem,(void *) outlist) != 0)
  {
    return -2;
  }
  redblacktreedestroy(False,NULL,NULL,dict.root);
  return 0;
}

void treeshape (void *root,Uint level);

static Sint checkpreviousnextfunctions(Uint *randomnumbers,Uint nums)
{
  void *root = NULL;
  Uint tmpkey, i, *prev;
  ArrayUint rnumcopy;
  BOOL nodecreated;

  ALLOCASSIGNSPACE(rnumcopy.spaceUint,NULL,Uint,nums);
  rnumcopy.nextfreeUint = nums;
  for(i=0; i<rnumcopy.nextfreeUint; i++)
  {
    rnumcopy.spaceUint[i] = randomnumbers[i];
    if(redblacktreesearch (randomnumbers + i,
                           &nodecreated,
                           &root,
                           cmpUint,
                           NULL) == NULL)
    {
      ERROR1("redblacktreesearch(%lu)\n",(Showuint) randomnumbers[i]);
      return -1;
    }
  }
  qsortUint(rnumcopy.spaceUint,rnumcopy.spaceUint+rnumcopy.nextfreeUint-1);
  remdups(&rnumcopy);
  for(i=UintConst(1); i<rnumcopy.nextfreeUint; i++)
  {
    prev = redblacktreepreviouskey (rnumcopy.spaceUint + i,
                                    root,
                                    cmpUint,
                                    NULL);
    if(prev == NULL)
    {
      ERROR2("previous = NULL for randomnumber[%lu]=%lu\n",
              (Showuint) i,
              (Showuint) rnumcopy.spaceUint[i]);
      return -2;
    } else
    {
      if(*prev != rnumcopy.spaceUint[i-1])
      {
        ERROR3("previous = %lu but rnumcopy[%lu]=%lu\n",
                 (Showuint) *prev,
                 (Showuint) i-1,
                 (Showuint) rnumcopy.spaceUint[i-1]);
        return -3;
      }
    }
    if(rnumcopy.spaceUint[i] > 0)
    {
      tmpkey = rnumcopy.spaceUint[i] - 1;
      prev = redblacktreepreviousequalkey (&tmpkey,
                                           root,
                                           cmpUint,
                                           NULL);
      if(prev == NULL)
      {
        ERROR1("previousequal = NULL for tmpkey=%lu\n",
                (Showuint) tmpkey);
        return -2;
      }
      if(*prev != rnumcopy.spaceUint[i-1])
      {
        ERROR2("previousequal = %lu but tmpkey=%lu\n",
                 (Showuint) *prev,
                 (Showuint) tmpkey);
        return -3;
      }
    }
    prev = redblacktreepreviousequalkey (rnumcopy.spaceUint + i,
                                         root,
                                         cmpUint,
                                         NULL);
    if(prev == NULL)
    {
      ERROR2("previousequal = NULL for randomnumber[%lu]=%lu\n",
             (Showuint) i,
             (Showuint) rnumcopy.spaceUint[i]);
      return -4;
    }
    if(*prev != rnumcopy.spaceUint[i])
    {
      ERROR3("previousequal = %lu but rnumcopy[%lu]=%lu\n",
             (Showuint) *prev,
             (Showuint) i,
             (Showuint) rnumcopy.spaceUint[i]);
      return -3;
    }
  }
  redblacktreedestroy(False,NULL,NULL,root);
  FREEARRAY(&rnumcopy,Uint);
  return 0;
}

MAINFUNCTION
{
  ArrayDictelem inlist;
  ArrayDictelem outlist;
  Scaninteger readint;
  Uint i, *randomnumbers, maxnumber;

  DEBUGLEVELSET;
  if(argc != 3)
  {
    fprintf(stderr,"Usage: %s <numofrandomelemens> <maxnumber>\n",argv[0]);
    return EXIT_FAILURE;
  }
  READINT(inlist.nextfreeDictelem,1);
  READINT(maxnumber,2);
  
  if(DICTSIZE(inlist.nextfreeDictelem) < UintConst(1))
  {
    fprintf(stderr,"%s: first argument must be >= %lu\n",
            argv[0],(Showuint) DIVISOR);
    return EXIT_FAILURE;
  }
  srand48(42349421);
  printf("maxsize=%lu\n",(Showuint) DICTSIZE(inlist.nextfreeDictelem));
  ALLOCASSIGNSPACE(randomnumbers,NULL,Uint,inlist.nextfreeDictelem);
  for(i=0; i<inlist.nextfreeDictelem; i++)
  {
    randomnumbers[i] =  (Uint) (drand48() * (double) maxnumber);
  }
  if(checkdictfunctions(&inlist,&outlist,randomnumbers) != 0)
  {
    STANDARDMESSAGE;
  }
  checkoutput(&inlist,&outlist);
  FREEARRAY(&outlist,Dictelem);
  FREEARRAY(&inlist,Dictelem);
  if(checkpreviousnextfunctions(randomnumbers,inlist.nextfreeDictelem) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(randomnumbers);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
