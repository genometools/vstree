//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "spacedef.h"
#include "arraydef.h"
#include "genfile.h"

//}

/*EE
  The following function increments the value for index 
  \texttt{ind} in the distribution \texttt{dist}. If \texttt{ind}
  is larger than any index previously used in   \texttt{dist}, then
  the array \texttt{dist} is enlarged accordingly.
*/

#define ADDAMOUNT 128

void adddistribution(ArrayUint *dist,Uint ind)
{
  Uint i;

  if(ind >= dist->allocatedUint)
  {
    ALLOCASSIGNSPACE(dist->spaceUint,dist->spaceUint,Uint,ind+ADDAMOUNT);
    for(i=dist->allocatedUint; i<ind+ADDAMOUNT; i++)
    {
      dist->spaceUint[i] = 0; 
    }
    dist->allocatedUint = ind+ADDAMOUNT;
  } 
  if(ind + 1 > dist->nextfreeUint)
  {
    dist->nextfreeUint = ind+1;
  }
  dist->spaceUint[ind]++;
}

void adddistributionUint64(ArrayUint64 *dist,Uint ind)
{
  Uint i;

  if(ind >= dist->allocatedUint64)
  {
    ALLOCASSIGNSPACE(dist->spaceUint64,dist->spaceUint64,Uint64,ind+ADDAMOUNT);
    for(i=dist->allocatedUint64; i<ind+ADDAMOUNT; i++)
    {
      dist->spaceUint64[i] = 0; 
    }
    dist->allocatedUint64 = ind+ADDAMOUNT;
  } 
  if(ind + 1 > dist->nextfreeUint64)
  {
    dist->nextfreeUint64 = ind+1;
  }
  dist->spaceUint64[ind]++;
}

void addmultidistribution(ArrayUint *dist,Uint ind,Uint howmany)
{
  if(howmany > 0)
  {
    Uint i;

    if(ind >= dist->allocatedUint)
    {
      ALLOCASSIGNSPACE(dist->spaceUint,dist->spaceUint,Uint,ind+ADDAMOUNT);
      for(i=dist->allocatedUint; i<ind+ADDAMOUNT; i++)
      {
        dist->spaceUint[i] = 0; 
      }
      dist->allocatedUint = ind+ADDAMOUNT;
    } 
    if(ind + 1 > dist->nextfreeUint)
    {
      dist->nextfreeUint = ind+1;
    }
    dist->spaceUint[ind] += howmany;
  }
}

void addmultidistributionUint64(ArrayUint64 *dist,Uint ind,Uint howmany)
{
  if(howmany > 0)
  {
    Uint i;

    if(ind >= dist->allocatedUint64)
    {
      ALLOCASSIGNSPACE(dist->spaceUint64,dist->spaceUint64,Uint64,ind+ADDAMOUNT);
      for(i=dist->allocatedUint64; i<ind+ADDAMOUNT; i++)
      {
        dist->spaceUint64[i] = 0; 
      }
      dist->allocatedUint64 = ind+ADDAMOUNT;
    } 
    if(ind + 1 > dist->nextfreeUint64)
    {
      dist->nextfreeUint64 = ind+1;
    }
    dist->spaceUint64[ind] += howmany;
  }
}

void showdistribution_generic(GENFILE *outfp,ArrayUint *distribution)
{
  Uint sumevents = 0, i;
  double addprob, probsum = 0.0;

  for(i=0; i < distribution->nextfreeUint; i++)
  {
    sumevents += distribution->spaceUint[i];
  }
  printf("# number of events: %lu\n",(Showuint) sumevents);
  for(i=0; i < distribution->nextfreeUint; i++)
  {
    if(distribution->spaceUint[i] > 0)
    {
      addprob = (double) distribution->spaceUint[i] / sumevents;
      probsum += addprob;
      genprintf(outfp,"%lu: %lu (prob=%.4f,cumulative=%.4f)\n",
                (Showuint) i,
                (Showuint) distribution->spaceUint[i],
                addprob,
                probsum);
    }
  }
}

void showdistribution(FILE *outfp,ArrayUint *distribution)
{
  GENFILE genfile;

  genfile.genfilemode  = STANDARD;
  genfile.fileptr.file = outfp;

  showdistribution_generic(&genfile,distribution);
}
