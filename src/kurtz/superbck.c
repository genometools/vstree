
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "virtualdef.h"
#include "debugdef.h"
#include "divmodmul.h"

 void integercode2string(void (*processstring)(char *),
                         Uint code,
                         Uint numofchars,
                         Uint prefixlen,
                         Uchar *characters);

static Uint findleftmostexception(ArrayPairUint *largelcpvalues,
                                  Uint lb,
                                  Uint rb)
{
  Uint width;
  PairUint *leftptr, *midptr, *rightptr;

  leftptr = largelcpvalues->spacePairUint;
  rightptr = largelcpvalues->spacePairUint +
             largelcpvalues->nextfreePairUint - 1;
  if(leftptr->uint0 > rb || rightptr->uint0 < lb)
  {
    return 0;
  }
  while(rightptr > leftptr + 1)
  {
    width = (Uint) (rightptr - leftptr + 1);
    midptr = leftptr + DIV2(width);
    if(midptr->uint0 >= lb)
    {
      rightptr = midptr;
    } else
    {
      leftptr = midptr;
    }
  }
  if(rightptr->uint0 > rb)
  {
    return 0;
  }
#ifdef DEBUG
  if(rightptr > largelcpvalues->spacePairUint &&
     (rightptr-1)->uint0 >= lb)
  {
    fprintf(stderr,"not the first exception value in this interval\n");
    exit(EXIT_FAILURE);
  }
#endif
  return (Uint) (rightptr - largelcpvalues->spacePairUint);
}

static Sint determinesuperbckvalue(Superbckinterval *superbck,
                                   Virtualtree *virtualtree,
                                   Uint codenumber,
                                   Uint leftbound,
                                   Uint rightbound)
{
  superbck->firstcodenum = codenumber;
  superbck->firstsuftabindex = leftbound;
  superbck->lastsuftabindex = rightbound;
  if(leftbound == 0)
  {
    superbck->firstlcpvalue = superbck->firstexceptionindex = 0;
  } else
  {
    superbck->firstlcpvalue = virtualtree->lcptab[leftbound];
    if(superbck->firstlcpvalue > virtualtree->prefixlength)
    {
      ERROR2("firstlcpvalue = %lu > %lu = prefixlength",
              (Showuint) superbck->firstlcpvalue,
              (Showuint) virtualtree->prefixlength);
      return (Sint) -1;
    }
    if(virtualtree->largelcpvalues.nextfreePairUint == 0)
    {
      superbck->firstexceptionindex = 0;
    } else
    {
      superbck->firstexceptionindex 
        = findleftmostexception(&virtualtree->largelcpvalues,
                                leftbound+1,
                                rightbound);
    }
  }
  return 0;
}

Sint simplebck2superbck(Superbckinterval *superbck,
                        Uint numofsuperbck,
                        Virtualtree *virtualtree)
{
  Uint i,
       remainder, 
       widthofinterval, 
       currentwidth, 
       offset = 0;

  if(virtualtree->numofcodes < numofsuperbck)
  {
    ERROR2("number of buckets %lu is smaller than required "
           "number %lu of superbuckets",
           (Showuint) virtualtree->numofcodes,
           (Showuint) numofsuperbck);
    return (Sint) -1;
  }
  widthofinterval = virtualtree->numofcodes/numofsuperbck;
  remainder = virtualtree->numofcodes % numofsuperbck;
  for(i=0; i < numofsuperbck; i++)
  {
    if(remainder > 0)
    {
      currentwidth = widthofinterval + 1;
      remainder--;
    } else
    {
      currentwidth = widthofinterval;
    }
    if(determinesuperbckvalue(superbck+i,
                              virtualtree,
                              offset >> 1,
                              virtualtree->bcktab[offset],
                              virtualtree->bcktab[offset 
                                                  + (currentwidth << 1)-1] -1)
                              != 0)
    {
      return (Sint) -2;
    }
    offset += (currentwidth << 1);
  }
  return 0;
}

static void simplyshowthestring(char *s)
{
  printf("%s",s);
}

void showsuperbck(Virtualtree *virtualtree,
                  Superbckinterval *superbck,
                  Uint numofsuperbck)
{
  Uint i, idx, width, totalwidth = 0, avgwidth;

  for(i=0; i < numofsuperbck; i++)
  {
    totalwidth += (superbck[i].lastsuftabindex - 
                   superbck[i].firstsuftabindex + 1);
  }
  avgwidth = totalwidth/numofsuperbck;
  printf("# average width = %lu\n",(Showuint) avgwidth);
  for(i=0; i < numofsuperbck; i++)
  {
    idx = superbck[i].firstsuftabindex;
    printf("# %lu: ",(Showuint) i);
    integercode2string(simplyshowthestring,
                       superbck[i].firstcodenum,
                       virtualtree->alpha.mapsize-1,
                       virtualtree->prefixlength,
                       &virtualtree->alpha.characters[0]);
    printf(" (%lu,%lu,%lu,%lu) ",
             (Showuint) superbck[i].firstlcpvalue,
             (Showuint) idx,
             (Showuint) superbck[i].lastsuftabindex,
             (Showuint) superbck[i].firstexceptionindex);
    width = superbck[i].lastsuftabindex - idx + 1;
    printf("width=%lu ",(Showuint) width);
    if(width > avgwidth)
    {
      printf("(+%.2f)\n",100. * (double) (width-avgwidth)/avgwidth);
    } else
    {
      printf("(-%.2f)\n",100. * (double) (avgwidth-width)/avgwidth);
    }
  }
}
