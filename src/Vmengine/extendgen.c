
#include "debugdef.h"

#include "matchstate.h"

#include "mcontain.pr"
#include "extendED.pr"
#include "extendHD.pr"

typedef Sint (*Extensionfunction)(BOOL,
                                  BOOL,
                                  Evalues *,
                                  Uint,
                                  Uint,
                                  Uint,
                                  Match *,
                                  Multiseq *,
                                  Multiseq *,
                                  Match *,
                                  void *,
                                  Processfinalfunction,
                                  Processfinalfunction);

Sint callgenericextend(BOOL selfmatch,
                       Matchstate *matchstate,
                       Match *seed,
                       Match *amatch,
                       void *info,
                       BOOL allmaximalmatches)
{
  Processfinalfunction processallmaxfunction, 
                       processfinalfunction;
  Extensionfunction extensionfunction;
  Multiseq *queryms;
  BOOL rcmode, querycompare;

  if(allmaximalmatches)
  {
    processallmaxfunction = matchcontainer;
    processfinalfunction = NULL;
  } else
  {
    processallmaxfunction = NULL;
    processfinalfunction = matchstate->processfinal;
  }
  if(MPARMEDISTMATCH(&matchstate->matchparam.maxdist))
  {
    extensionfunction = editextend;
  } else
  {
    extensionfunction = hammingextend;
  }
  if(selfmatch)
  {
    queryms = &matchstate->virtualtree->multiseq;
    rcmode = False;
    querycompare = False;
  } else
  {
    queryms = matchstate->queryinfo->multiseq;
    if(CHECKSHOWPALINDROMIC(matchstate))
    {
      rcmode = True;
    } else
    {
      rcmode = False;
    }
    querycompare = True;
  }
  if(extensionfunction(rcmode,
                       querycompare,
                       matchstate->evalues,
                       matchstate->matchparam.maxdist.distvalue,
                       matchstate->matchparam.userdefinedleastlength,
                       matchstate->matchparam.seedlength,
                       seed,
                       &matchstate->virtualtree->multiseq,
                       queryms,
                       amatch,
                       info,
                       processallmaxfunction,
	               processfinalfunction) != 0)
  {
    return (Sint) -1;
  } 
  return 0;
}
