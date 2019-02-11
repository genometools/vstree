#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "spacedef.h"
#include "chardef.h"
#include "errordef.h"
#include "debugdef.h"
#include "encseq-def.h"

/*@null@*/ Uint64 *encseqtable2seqoffsets(BOOL check32bit,
                                          Uint64 *totallength,
                                          Specialcharinfo *specialcharinfo,
                                          const Encodedsequence *encseqtable,
                                          Uint numofindexes)
{
  Uint idx;
  Uchar lastofprevious, firstofcurrent;
  Uint64 *sequenceoffsettable,
         tmplength,
         tmpspecialcharacters, tmpspecialranges;

  ALLOCASSIGNSPACE(sequenceoffsettable,NULL,Uint64,numofindexes);
  tmpspecialcharacters = (Uint64) (numofindexes-1);
  tmpspecialranges = 0;
  for(idx=0; idx<numofindexes; idx++)
  {
    if(idx == 0)
    {
      tmplength = 0;
      sequenceoffsettable[idx] = 0;
    } else
    {
      tmplength = ACCESSSEQUENCELENGTH(encseqtable + idx - 1);
      sequenceoffsettable[idx] 
	= sequenceoffsettable[idx-1] + tmplength + (Uint64) 1;
    }
    tmpspecialcharacters 
      += (Uint64) encseqtable[idx].specialcharinfo.specialcharacters;
    tmpspecialranges 
      += (Uint64) encseqtable[idx].specialcharinfo.specialranges;
    if(idx > 0)
    {
      lastofprevious = ACCESSENCODEDCHAR64(encseqtable + idx - 1,
                                           tmplength-1);
      firstofcurrent = ACCESSENCODEDCHAR64(encseqtable + idx,UintConst(0));
      if(ISSPECIAL(lastofprevious))
      {
         if(ISSPECIAL(firstofcurrent))
         {
           tmpspecialranges--;
         }
      } else
      {
        if(ISNOTSPECIAL(firstofcurrent))
        {
          tmpspecialranges++;
        }
      }
    }
    if(check32bit)
    {
      Uint64 currentseqlen = sequenceoffsettable[idx] + 
		             (Uint64) ACCESSSEQUENCELENGTH(encseqtable + idx);
      if(currentseqlen > (Uint64) UINT_MAX) // use of UINT_MAX is okay
      {
        /*@ignore@*/
        ERROR3("failed to write 32bit index since overall sequence length "
               FormatUint64 " of indexes [0..%lu] is larger than %lu",
               currentseqlen,
               (Showuint) idx,
               (Showuint) UINT_MAX); // use of UINT_MAX is okay
        /*@end@*/
        FREESPACE(sequenceoffsettable);
        return NULL;
      }
    }
#ifndef SIXTYFOURBITS
    if(tmpspecialcharacters > (Uint64) UINT_MAX)
    {
      /*@ignore@*/
      ERROR2("number of specialcharacters is " FormatUint64 " > %lu",
              tmpspecialcharacters,
              (Showuint) UINT_MAX);
      /*@end@*/
      FREESPACE(sequenceoffsettable);
      return NULL;
    }
#endif
    DEBUG2(3,"# seqlen[%lu] = %lu\n",
           (Showuint) idx,
           (Showuint) ACCESSSEQUENCELENGTH(encseqtable + idx));
  }
  tmplength = ACCESSSEQUENCELENGTH(encseqtable + numofindexes -1);
  *totallength = sequenceoffsettable[numofindexes-1] + tmplength;
  specialcharinfo->specialcharacters = (Uint) tmpspecialcharacters;
  specialcharinfo->specialranges = (Uint) tmpspecialranges;
  specialcharinfo->lengthofspecialprefix =
    encseqtable[0].specialcharinfo.lengthofspecialprefix;
  specialcharinfo->lengthofspecialsuffix =
    encseqtable[idx-1].specialcharinfo.lengthofspecialsuffix;
  return sequenceoffsettable;
}
