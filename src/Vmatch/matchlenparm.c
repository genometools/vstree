
#include "mparms.h"
#include "debugdef.h"

#define DEFAULTSEEDLENGTH    UintConst(30)

Sint determinematchlengthparam(Uint numberofqueryfiles,
                               Matchparam *matchparam,
                               Showverbose showverbose)
{
  if(matchparam->xdropbelowscore == UNDEFXDROPBELOWSCORE)
  {
    if(matchparam->userdefinedleastlength > 0)
    {
      Uint len;

      len = matchparam->userdefinedleastlength/
            (matchparam->maxdist.distvalue+1);
      if(matchparam->seedlength < len)
      {
        matchparam->seedlength = len;
      }
      if(!MPARMEXACTMATCH(&matchparam->maxdist) && showverbose != NULL)
      {
        char sbuf[128+1];
        sprintf(sbuf,"search with minimal length %lu, maximal distance %lu, "
                     " and seed length %lu",
               (Showuint) matchparam->userdefinedleastlength,
               (Showuint) matchparam->maxdist.distvalue,
               (Showuint) matchparam->seedlength);
        showverbose(sbuf);
      }
      if(numberofqueryfiles > 0 && matchparam->repeatgapspec.lowergapdefined)
      {
        ERROR0("specification of gapwidth in option -l cannot be combined "
               "with option -q");
        return (Sint) -1;
      }
    }
  } else
  {
    if(matchparam->seedlength == 0)
    {
      matchparam->seedlength = DEFAULTSEEDLENGTH;
    }
  }
  DEBUG1(2,"seedlength=%lu\n",(Showuint) matchparam->seedlength);
  return 0;
}
