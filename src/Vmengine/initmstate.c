#include "matchstate.h"
#include "mparms.h"
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"

Sint initMatchstate(Matchstate *matchstate,
                    Virtualtree *virtualtree,
                    void *voidqueryinfo,
                    Matchparam *matchparam,
                    Bestflag bestflag,
                    Uint shownoevalue,
                    Uint showselfpalindromic,
                    SelectBundle *selectbundle,
                    Uint onlinequerynumoffset,
                    void *procmultiseq,
                    Currentdirection currentdirection,
                    BOOL revmposorder,
                    Processfinalfunction processfinalparam,
                    Evalues *evalues,
                    BOOL domatchbuffering)
{
  COPYMATCHPARAM(&matchstate->matchparam,matchparam);
  matchstate->queryinfo = (Queryinfo *) voidqueryinfo;
  matchstate->virtualtree = virtualtree;
  matchstate->currentidnumber = 0;
  if(bestflag == Allmaximalmatches && !MPARMEXACTMATCH(&matchparam->maxdist))
  {
    INITARRAY(&matchstate->allmstore,Match);
    INITARRAY(&matchstate->seedmstore,Match);
  }
  matchstate->shownoevalue = shownoevalue;
  matchstate->showselfpalindromic = showselfpalindromic;
  matchstate->bestflag = bestflag;
  matchstate->selectbundle = selectbundle;
  matchstate->procmultiseq = procmultiseq;
  matchstate->processfinal = processfinalparam;
  matchstate->onlinequerynumoffset = onlinequerynumoffset;
  matchstate->transnum = matchparam->transnum;
  matchstate->evalues = evalues;
  matchstate->domatchbuffering = domatchbuffering;
  matchstate->currentdirection = currentdirection;
  matchstate->revmposorder = revmposorder;
  return 0;
}
