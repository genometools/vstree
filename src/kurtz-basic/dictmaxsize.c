#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "errordef.h"
#include "debugdef.h"
#include "redblackdef.h"

#include "redblack.pr"

void initDictmaxsize(Dictmaxsize *dict,
                     Uint maxsize)
{
  dict->currentdictsize = 0;
  dict->maxdictsize = maxsize;
  dict->root = NULL;
  dict->lastcallinsertedelem = NULL;
  dict->lastcalldeletedelem = NULL;
  dict->worstelement = NULL;
}

Sint insertDictmaxsize(Dictmaxsize *dict,
                       Dictcomparefunction comparefunction,
                       void *cmpinfo,
                       Dictshowelem showelem,
                       void *showinfo,
                       void *elemin)
{
  BOOL nodecreated;

  DEBUG2(3,"insertDictmaxsize(currentdictsize=%lu,maxdictsize=%lu)\n",
            (Showuint) dict->currentdictsize,(Showuint) dict->maxdictsize);
  DEBUG0(3,"elemin=");
  DEBUGCODE(3,if(showelem != NULL) showelem(elemin,showinfo));
  DEBUG0(3,"\n");
  if(dict->currentdictsize < dict->maxdictsize)
  {
    if(dict->currentdictsize == 0 ||
       comparefunction(elemin,dict->worstelement,cmpinfo) < 0)
    {
      dict->worstelement = elemin;
      DEBUG0(3,"(1) new worstelement=");
      DEBUGCODE(3,if(showelem != NULL) showelem(dict->worstelement,
                                                showinfo));
      DEBUG0(3,"\n");
    }
    (void) redblacktreesearch(elemin,
                              &nodecreated,
                              &dict->root,
                              comparefunction,
                              cmpinfo);
    if(nodecreated)
    {
      dict->currentdictsize++;
      dict->lastcallinsertedelem = elemin;
    } 
  } else
  {
/*
  new element is not as worse as worst element, so insert it and
  and delete the worst element
*/
    DEBUG0(3,"current worstelement=");
    DEBUGCODE(3,if(showelem != NULL) showelem(dict->worstelement,showinfo));
    DEBUG0(3,"\n");
    if(comparefunction(dict->worstelement,elemin,cmpinfo) < 0)
    { 
      (void) redblacktreesearch(elemin,
                                &nodecreated,
                                &dict->root, 
                                comparefunction,
                                cmpinfo);
      if(nodecreated)
      {
        dict->lastcallinsertedelem = elemin;
        if (redblacktreedelete(dict->worstelement,
                               &dict->root, 
                               comparefunction,
                               cmpinfo) != 0)
        {
          ERROR0("insertDictmaxsize: deletion failed\n");
          return (Sint) -2;
        }
        dict->lastcalldeletedelem = dict->worstelement;
        dict->worstelement = redblacktreeminimumkey(dict->root);
        DEBUG0(3,"(2) new worstelement=");
        DEBUGCODE(3,if(showelem != NULL) showelem(dict->worstelement,
                                                  showinfo));
        DEBUG0(3,"\n");
      }
    }
  }
  return 0;
}
