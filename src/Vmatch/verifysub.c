
#include "types.h"
#include "multidef.h"
#include "debugdef.h"
#include "fqfinfo.h"

Sint verifysubstringinfo(FQFsubstringinfo *fqfsubstringinfo,
                         Multiseq *querymultiseq)
{
  if(fqfsubstringinfo == NULL)
  {
    ERROR0("fqfsubstringinfo is NULL");
    return (Sint) -1;
  }
  if(fqfsubstringinfo->ffdefined)
  {
    if(querymultiseq->numofsequences != UintConst(1))
    {
      ERROR0("substring specification only allowed for query file "
             "containing one sequence");
      return (Sint) -2;
    }
    // end not specified or to large
    if(fqfsubstringinfo->readseqend == 0 ||
       fqfsubstringinfo->readseqend >= querymultiseq->totallength)
    {
      fqfsubstringinfo->readseqend = querymultiseq->totallength-1;
    } 
    if(fqfsubstringinfo->readseqstart >= fqfsubstringinfo->readseqend)
    {
      ERROR2("illegal substring specification (%lu,%lu): "
             "left boundary must be smaller than right boundary",
             (Showuint) fqfsubstringinfo->readseqstart,
             (Showuint) fqfsubstringinfo->readseqend);
      return (Sint) -3;
    }
  }
  return 0;
}

