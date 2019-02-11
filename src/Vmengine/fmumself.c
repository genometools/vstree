
#include "virtualdef.h"
#include "errordef.h"
#include "chardef.h"
#include "debugdef.h"
#include "genfile.h"

#include "multiseq-adv.pr"

Sint findmaximaluniquematches(Virtualtree *virtualtree,
                              /*@unused@*/ Uint numberofprocessors,
                              Uint searchlength,
                              /*@unused@*/ void *repeatgapspec,
                              void *outinfo,
                              Outputfunction output)
{
  Uint i, exception = 0, temp, firstlcp = 0, 
       secondlcp, thirdlcp, start1, start2, querysepposition;
  Uchar a, b;

  if(!HASINDEXEDQUERIES(&virtualtree->multiseq))
  {
    ERROR0("maximal unique matches search requires at least one query file");
    return (Sint) -1;
  }
  if(virtualtree->multiseq.totallength < UintConst(2))
  {
    ERROR0("search for maximal unique matches requires "
           "at least a table of length 2");
    return (Sint) -2;
  }
  querysepposition = getqueryseppos(&virtualtree->multiseq);
  SEQUENTIALEVALLCPVALUE(secondlcp,1,exception);
  for(i = UintConst(2); i < virtualtree->multiseq.totallength; i++)
  {
    SEQUENTIALEVALLCPVALUE(thirdlcp,i,exception);
    if(secondlcp >= searchlength &&
       firstlcp < secondlcp && thirdlcp < secondlcp)
    {
      start1 = virtualtree->suftab[i-2];
      start2 = virtualtree->suftab[i-1];
      if(start1 > start2)
      {
        temp = start1;
        start1 = start2;
        start2 = temp;
      }
      DEBUG2(3,"suffixes = %lu %lu\n",(Showuint) start1,(Showuint) start2);
      if(start1 < querysepposition && start2 > querysepposition)
      {
        if(start1 == 0 || ISSPECIAL(a = virtualtree->bwttab[i-1]) ||
                          ISSPECIAL(b = virtualtree->bwttab[i-2]) ||
           a != b)
        {
          if(output(outinfo,secondlcp,start1,start2) != 0)
          {
            return (Sint) -3;
          }
        }
      }
    }
    firstlcp = secondlcp;
    secondlcp = thirdlcp;
  }
  return 0;
}
