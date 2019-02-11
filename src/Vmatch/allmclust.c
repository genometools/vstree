
#include "errordef.h"
#include "multidef.h"
#include "debugdef.h"
#include "mcldef.h"

#include "cluedist.pr"
#include "clpos.pr"

Sint genericmatchclustering(Matchclustercallinfo *matchclustercallinfo,
                            Multiseq *outvms,
                            Alphabet *outvmsalpha,
                            Multiseq *outqms,
                            ArrayStoreMatch *matchtab,
                            char *mfargs)
{
  if(matchclustercallinfo->matchclustertype == SimilarityMCL)
  {
    if(uedistcluster(outvms,
                     outvmsalpha,
                     outqms,
                     matchtab,
                     matchclustercallinfo->outprefix,
                     mfargs,
                     matchclustercallinfo->errorrate) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(matchclustercallinfo->matchclustertype == GapMCL)
    {
      if(gapclustermatches(outvms,
                           outvmsalpha,
                           outqms,
                           matchtab,
                           matchclustercallinfo->outprefix,
                           mfargs,
                           matchclustercallinfo->maxgapsize) != 0)
      {
        return (Sint) -2;
      }
    } else
    {
      if(matchclustercallinfo->matchclustertype == OverlapMCL)
      {
        if(overlapclustermatches(outvms,
                                 outvmsalpha,
                                 outqms,
                                 matchtab,
                                 matchclustercallinfo->outprefix,
                                 mfargs,
                                 matchclustercallinfo->minpercentoverlap) != 0)
        {
          return (Sint) -3;
        }
      } else
      {
        ERROR0("unkown matchclustertype");
        return (Sint) -3;
      }
    }
  }
  return 0;
}
