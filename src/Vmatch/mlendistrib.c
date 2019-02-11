
#include "types.h"
#include "virtualdef.h"
#include "debugdef.h"
#include "genfile.h"
#include "matchtask.h"

#include "distri.pr"

void showmatcount(FILE *outfp,
                  Uint searchlength,
                  ArrayUint *counttab)
{
  Uint i, total = 0;

  for(i=searchlength; i<counttab->nextfreeUint; i++)
  {
    total += counttab->spaceUint[i];
  }
  fprintf(outfp,"# all %lu\n",(Showuint) total);
  if(total > 0)
  {
    // fprintf(outfp,"# allD %lu\n",(Showuint) total);
    for(i=searchlength; i<counttab->nextfreeUint; i++)
    {
      if(counttab->spaceUint[i] > 0)
      {
        fprintf(outfp,"# %lu %lu\n",(Showuint) i,
                                    (Showuint) counttab->spaceUint[i]);
      }
    }
  }
}

Sint immediatelycountmatch(void *showmatchinfo,
                           /*@unused@*/ Multiseq *virtualmultiseq,
                           /*@unused@*/ Multiseq *querymultiseq,
                           StoreMatch *storematch)
{
  Outinfo *outinfo = (Outinfo *) showmatchinfo;

  adddistribution(&outinfo->matchcounttab,storematch->Storelength1);
  return 0;
}
