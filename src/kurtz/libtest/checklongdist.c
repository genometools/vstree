#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "args.h"
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "dpbitvec48.h"
#include "spacedef.h"
#include "galigndef.h"

#include "alphabet.pr"
#include "bitvdist.pr"
#include "frontSEP.pr"
#include "distri.pr"
#include "readvirt.pr"

#define DOUKKONEN      UintConst(1)
#define DOMYERS        (UintConst(1) << 1)
#define DOSHOWSTRING   (UintConst(1) << 2)

#define COMPAREALGORITHMS(ALG1,DIST1,ALG2,DIST2)\
        if((mode & (ALG1)) && (mode & (ALG2)))\
        {\
          if((DIST1) != (DIST2))\
          {\
            fprintf(stderr,"%s =%lu != %lu = %s\n",\
                            #DIST1,\
                            (Showuint) (DIST1),\
                            (Showuint) (DIST2),\
                            #DIST2);\
            exit(EXIT_FAILURE);\
          }\
        }

static void getrandomstring(Stringtype *str,Uint totallength,
                            Uint minlength, Uint maxlength)
{
  if(minlength == maxlength)
  {
    str->length = minlength;
  } else
  {
    str->length = (Uint) (minlength +
                         (drand48() * (double) (maxlength-minlength+1)));
  }
  str->start = (Uint) (drand48() * (double) (totallength-str->length));
}

static Uint runcomparison(Uint mode,
                          DPbitvectorreservoir *dpbvres,
                          Alphabet *alpha,
                          Uchar *useq,
                          Uint ulen,
                          Uchar *vseq,
                          Uint vlen)
{
  Uint distukkonen = 0, distmyers = 0;

  if(mode & DOUKKONEN)
  {
    distukkonen = (Uint) unitedistfrontSEPgeneric(False,
                                                  0,
                                                  useq,
                                                  (Sint) ulen,
                                                  vseq,
                                                  (Sint) vlen);
  }
  if(mode & DOMYERS)
  {
    distmyers = distanceofstrings(dpbvres,
                                  alpha->mapsize,
                                  useq,
                                  ulen,
                                  vseq,
                                  vlen);
  }
  if(mode & DOSHOWSTRING)
  {
    printf("\"");
    vm_showsymbolstring(alpha,useq,ulen);
    printf("\" \"");
    vm_showsymbolstring(alpha,vseq,vlen);
    printf("\"\n");
  }
  COMPAREALGORITHMS(DOUKKONEN,distukkonen,DOMYERS,distmyers);
  if(mode & DOUKKONEN)
  {
    return distukkonen;
  }
  if(mode & DOMYERS)
  {
    return distmyers;
  }
  return distukkonen;
}

static Sint dopairwisecomparisons(Uint mode,
                                  Multiseq *multiseq,
                                  Alphabet *alpha,
                                  Uint minlength,
                                  Uint maxlength,
                                  Uint numofcomparisons)
{
  Uint dist, i;
  Stringtype str1, str2;
  ArrayUint lendistrib, distdist;
  DPbitvectorreservoir dpbvres;
  
  if(maxlength < minlength)
  {
    fprintf(stderr,"maxlength =%lu < %lu = maxlength\n",
                     (Showuint) maxlength ,
                     (Showuint) minlength);
    exit(EXIT_FAILURE);
  }
  if(multiseq->totallength <= maxlength)
  {
    fprintf(stderr,"totallength=%lu <= maxlength = %lu\n",
                    (Showuint) multiseq->totallength,
                    (Showuint) maxlength);
    exit(EXIT_FAILURE);
  }
  INITARRAY(&lendistrib,Uint);
  INITARRAY(&distdist,Uint);
  initDPbitvectorreservoir(&dpbvres,alpha->mapsize,maxlength);
  for(i=0; i<numofcomparisons; i++)
  {
    getrandomstring(&str1,multiseq->totallength,minlength,maxlength);
    getrandomstring(&str2,multiseq->totallength,minlength,maxlength);
    adddistribution(&lendistrib,str1.length);
    adddistribution(&lendistrib,str2.length);
    dist = runcomparison(mode,
                         &dpbvres,
                         alpha,
                         multiseq->sequence + str1.start,
                         str1.length,
                         multiseq->sequence + str2.start,
                         str2.length);
    adddistribution(&distdist,dist);
  }
  printf("the distribution of the length\n");
  showdistribution(stdout,&lendistrib);
  printf("the distribution of the distance\n");
  showdistribution(stdout,&distdist);
  freeDPbitvectorreservoir(&dpbvres);
  FREEARRAY(&lendistrib,Uint);
  FREEARRAY(&distdist,Uint);
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  Scaninteger readint;
  Uint mode = 0, minlength, maxlength, numofcomparisons;
  size_t j;

  CHECKARGNUM(6,"[UMS] minlength maxlength numofcomparisons indexname");
  DEBUGLEVELSET;
  
  for(j=0; j<strlen(argv[1]); j++)
  {
    switch(argv[1][j])
    {
      case 'U':
        mode |= DOUKKONEN;
        break;
      case 'M':
        mode |= DOMYERS;
        break;
      case 'S':
        mode |= DOSHOWSTRING;
        break;
      default: 
        fprintf(stderr,"%s: illegal character %c: must be combination of %s\n",
                   argv[0],argv[1][j],"UMS");
        exit(EXIT_FAILURE);
    }
  }
  if(sscanf(argv[2],"%ld",&readint) != 1 || readint < 0)
  {
    fprintf(stderr,"argument 1 must be non-negative integer\n");
    return EXIT_FAILURE;
  }
  minlength = (Uint) readint;
  if(sscanf(argv[3],"%ld",&readint) != 1 || readint < 0)
  {
    fprintf(stderr,"argument 2 must be non-negative integer\n");
    return EXIT_FAILURE;
  }
  maxlength = (Uint) readint;
  if(sscanf(argv[4],"%ld",&readint) != 1 || readint < 0)
  {
    fprintf(stderr,"argument 3 must be non-negative integer\n");
    return EXIT_FAILURE;
  }
  numofcomparisons = (Uint) readint;
  if(mapvirtualtreeifyoucan(&virtualtree,argv[5],TISTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(dopairwisecomparisons(mode,
                           &virtualtree.multiseq,
                           &virtualtree.alpha,
                           minlength,
                           maxlength,
                           numofcomparisons) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
