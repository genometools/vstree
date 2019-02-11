#include <stdlib.h>
#include "intbits.h"
#include "debugdef.h"
#include "errordef.h"
#include "arraydef.h"
#include "clusterdef.h"
#include "args.h"

#include "cluster.pr"

static void mkrlist(Uint *rlist,Uint len,Uint range)
{
  Uint r, i = 0, *occurs;

  INITBITTAB(occurs,range);
  srand48(42349421);
  while(i<len)
  {
    r = (Uint) (drand48() * (double) range);
    if(!ISIBITSET(occurs,r))
    {
      SETIBIT(occurs,r);
      rlist[i++] = r;
    }
  }
  FREESPACE(occurs);
}

static Sint checkmod10(void *info,Uint i,Uint j)
{
  Uint *rlist = (Uint *) info;

  /*printf("checkmod10(rlist[%lu]=%lu,rlist[%lu]=%lu)\n",
             (Showuint) i,
             (Showuint) rlist[i],
             (Showuint) j,
             (Showuint) rlist[j]);*/
  if(rlist[i] % 10 != rlist[j] % 10)
  {
    return -1;
  }
  return 0;
}


static void makeedges(ArrayPairUint *cedge,Uint *rlist,Uint len)
{
  Uint i, j;
  PairUint *pair;

  for(i=0; i<len; i++)
  {
    for(j=i+1; j<len; j++)
    {
      if(checkmod10(rlist,i,j) == 0)
      {
        GETNEXTFREEINARRAY(pair,cedge,PairUint,128);
        pair->uint0 = i;
        pair->uint1 = j;
      }
    }
  }
}

static Sint doclustering(ClusterSet *clusterset,Uint len,ArrayPairUint *cedge)
{
  Uint edgenum;

  initClusterSet(clusterset,len);
  for(edgenum=0; edgenum<cedge->nextfreePairUint; edgenum++)
  {
    if(linkcluster(clusterset,cedge->spacePairUint[edgenum].uint0,
                              cedge->spacePairUint[edgenum].uint1) != 0)
    {
      return -1;
    }
  }
  for(edgenum=0; edgenum<cedge->nextfreePairUint; edgenum++)
  {
    if(addClusterEdge(clusterset,cedge->spacePairUint[edgenum].uint0,
                                 cedge->spacePairUint[edgenum].uint1,
                                 edgenum) != 0)
    {
      return -2;
    }
  }
  return 0;
}

static Sint showonstdout1(void *info,Uint i)
{
  Uint *rlist = (Uint *) info;
  printf(" %lu",(Showuint) rlist[i]);
  return 0;
}

#ifdef DEBUG
static Sint showonstdout2(void *info1,void *info2,Uint i)
{
  Uint *rlist = (Uint *) info1;
  PairUint *cedges = (PairUint *) info2;
  printf(" (%lu,%lu)",(Showuint) rlist[cedges[i].uint0],
                      (Showuint) rlist[cedges[i].uint1]);
  return 0;
}

static Sint showclusternum(/*@unused@*/ void *info,Uint i,
                          /*@unused@*/ Uint csize)
{
  printf("%lu: ",(Showuint) i);
  return 0;
}

static Sint showclusternewline(/*@unused@*/ void *info,/*@unused@*/ Uint i)
{
  printf("\n");
  return 0;
}

static void showrlist(Uint *rlist,Uint len)
{
  Uint i;

  printf("rlist=");
  for(i=0; i<len; i++)
  {
    printf(" %lu",(Showuint) rlist[i]);
  }
  printf("\n");
}
#endif

MAINFUNCTION
{
  Scaninteger range, len;
  Uint *rlist;
  ArrayPairUint cedge;
  ClusterSet clusterset;

  DEBUGLEVELSET;
  CHECKARGNUM(3,"range len");
  if(sscanf(argv[1],"%ld",&range) != 1 || range <= 0)
  {
    fprintf(stderr,"%s: first argument %s must be posititive integer\n",
            argv[0],argv[1]);
    return EXIT_FAILURE;
  }
  if(sscanf(argv[2],"%ld",&len) != 1 || len <= 0)
  {
    fprintf(stderr,"%s: first argument %s must be posititive integer\n",argv[0],
                    argv[2]);
    return EXIT_FAILURE;
  }
  if(range < len)
  {
    fprintf(stderr,"%s: range=%ld < %ld=len not allowed\n",
            argv[0],
            (Showsint) range,
            (Showsint) len);
    return EXIT_FAILURE;
  }
  ALLOCASSIGNSPACE(rlist,NULL,Uint,(Uint) len);
  mkrlist(rlist,(Uint) len,(Uint) range);
  DEBUGCODE(2,showrlist(rlist,(Uint) len));
  INITARRAY(&cedge,PairUint);
  makeedges(&cedge,rlist,(Uint) len);
  if(doclustering(&clusterset,(Uint) len,&cedge) != 0)
  {
    STANDARDMESSAGE;
  }
  printf("doclustering okay\n");
  (void) fflush(stdout);
  DEBUGCODE(2,(void) showClusterSet(&clusterset,
                             0,
                             MAXCLUSTERSIZE(&clusterset),
                             NULL,
                             showclusternum,
                             (void *) rlist,
                             showonstdout1,
                             NULL,
                             showclusternewline,
                             (void *) cedge.spacePairUint,
                             showonstdout2));
  printf("singleton:\n");
  if(showSingletonSet(&clusterset,(void *) rlist,showonstdout1) != 0)
  {
    STANDARDMESSAGE;
  }
  if(checkClusterSet(&clusterset,(void *) rlist,checkmod10) != 0)
  {
    STANDARDMESSAGE;
  }
  printf("checkClusterSet okay\n");
  freeClusterSet(&clusterset);
  printf("freeClusterSet okay okay\n");
  FREESPACE(rlist);
  FREEARRAY(&cedge,PairUint);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
