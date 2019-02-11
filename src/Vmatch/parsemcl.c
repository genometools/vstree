
#include "types.h"
#include "optdesc.h"
#include "debugdef.h"
#include "errordef.h"
#include "mcldef.h"
#include "galigndef.h"

#include "dstrdup.pr"
#include "procopt.pr"
#include "frontSEP.pr"
#include "safescpy.pr"

#define READNONNEGATIVEINT(VAR)\
        READINTGENERIC(options[(Uint) optval].optname,\
                       VAR,argc-1,<,"non-negative")

typedef enum
{
  OPTMATCHCLUSTERERRORRATE = 0,
  OPTMATCHCLUSTERGAPSIZE,
  OPTMATCHCLUSTERPERCENTOVERLAP,
  OPTMATCHCLUSTEROUTPREFIX,
  OPTVMATCHVERSION,
  OPTHELP,
  NUMOFOPTIONS
} Optionnumber;

Sint parsematchcluster(BOOL fromvmatch,
                       Matchclustercallinfo *matchclustercallinfo,
                       const char * const*argv,
                       Argctype argc)
{
  Optionnumbertype optval;
  Uint argnum, dropminus;
  OptionDescription options[NUMOFOPTIONS];
  Scaninteger readint;

  dropminus = fromvmatch ? UintConst(1) : 0;
  matchclustercallinfo->defined = True;
  matchclustercallinfo->errorrate = 0;
  matchclustercallinfo->maxgapsize = 0;
  matchclustercallinfo->minpercentoverlap = 0;
  matchclustercallinfo->matchclustertype = UndefMCL;
  matchclustercallinfo->outprefix = NULL;
  matchclustercallinfo->matchfile = NULL;
  initoptions(&options[0],(Uint) NUMOFOPTIONS);

  ADDOPTION(OPTMATCHCLUSTERERRORRATE,"-erate",
                                     "specify maximum error rate in "
                                     "range [0,100] for similarity clustering");
  ADDOPTION(OPTMATCHCLUSTERGAPSIZE,"-gapsize",
                                   "specify maximum gap size for gap clustering");
  ADDOPTION(OPTMATCHCLUSTERPERCENTOVERLAP,"-overlap",
                                   "specify minimum percentage of overlap for "
                                   "overlap clustering");
  ADDOPTION(OPTMATCHCLUSTEROUTPREFIX,"-outprefix",
                                     "specify prefix of files to output "
                                     "clusters");
  ADDOPTVMATCHVERSION;
  ADDOPTION(OPTHELP,"-help","this option");
  if(argc == 1)
  {
    if(fromvmatch)
    {
      ERROR1("missing argument for option -%s",MATCHCLUSTERNAME);
    } else
    {
      ERROR1("missing options: %s displays the possible options",
              options[OPTHELP].optname);
    }
    return (Sint) -1;
  }
  for(argnum = UintConst(1);
      argnum < (Uint) argc && ISOPTION(argv[argnum]);
      argnum++)
  {
    optval = procoption(options,(Uint) NUMOFOPTIONS,argv[argnum]);
    if(optval < 0)
    {
      return (Sint) -2;
    }
    switch(optval)
    {
      case OPTHELP:
        showoptions(stdout,argv[0],options,(Uint) NUMOFOPTIONS);
        return (Sint) 1;
      case OPTVMATCHVERSION:
        return (Sint) 1;
      case OPTMATCHCLUSTERERRORRATE:
        READPERCENTAGE(options[(Uint) optval].optname,readint,"");
        matchclustercallinfo->errorrate = (Uint) readint;
        matchclustercallinfo->matchclustertype = SimilarityMCL;
        break;
      case OPTMATCHCLUSTERGAPSIZE:
        READNONNEGATIVEINT(readint);
        matchclustercallinfo->maxgapsize = (Uint) readint;
        matchclustercallinfo->matchclustertype = GapMCL;
        break;
      case OPTMATCHCLUSTERPERCENTOVERLAP:
        READPERCENTAGE(options[(Uint) optval].optname,readint,"");
        matchclustercallinfo->minpercentoverlap = (Uint) readint;
        matchclustercallinfo->matchclustertype = OverlapMCL;
        break;
      case OPTMATCHCLUSTEROUTPREFIX:
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
          ASSIGNDYNAMICSTRDUP(matchclustercallinfo->outprefix,argv[argnum]);
        } else
        {
          ERROR1("missing argument for option %s",
                  options[(Uint) optval].optname + dropminus);
          return (Sint) -3;
        }
        break;
    }
  }
  if(argnum < (Uint) (argc-1))
  {
    ERROR1("superfluous file argument \"%s\"",argv[argc-1]);
    return (Sint) -4;
  }
  ASSIGNDYNAMICSTRDUP(matchclustercallinfo->matchfile,argv[argnum]);
  if(!fromvmatch)
  {
    if(argnum >= (Uint) argc)
    {
      ERROR0("missing matchfile");
      return (Sint) -5;
    }
  }
  if(ISSET(OPTMATCHCLUSTERERRORRATE))
  {
    if(ISSET(OPTMATCHCLUSTERGAPSIZE))
    {
      ERROR2("options %s and %s exclude each other",
             options[OPTMATCHCLUSTERERRORRATE].optname + dropminus,
             options[OPTMATCHCLUSTERGAPSIZE].optname + dropminus);
      return (Sint) -6;
    }
    if(ISSET(OPTMATCHCLUSTERPERCENTOVERLAP))
    {
      ERROR2("options %s and %s exclude each other",
             options[OPTMATCHCLUSTERERRORRATE].optname + dropminus,
             options[OPTMATCHCLUSTERPERCENTOVERLAP].optname + dropminus);
      return (Sint) -6;
    }
  } else
  {
    if(ISSET(OPTMATCHCLUSTERGAPSIZE) && ISSET(OPTMATCHCLUSTERPERCENTOVERLAP))
    {
      ERROR2("options %s and %s exclude each other",
             options[OPTMATCHCLUSTERGAPSIZE].optname + dropminus,
             options[OPTMATCHCLUSTERPERCENTOVERLAP].optname + dropminus);
      return (Sint) -6;
    }
  }
  if(!ISSET(OPTMATCHCLUSTERERRORRATE) && 
     !ISSET(OPTMATCHCLUSTERGAPSIZE) &&
     !ISSET(OPTMATCHCLUSTERPERCENTOVERLAP))
  {
    ERROR3("one of the options %s, %s, or %s must be used",
           options[OPTMATCHCLUSTERERRORRATE].optname + dropminus,
           options[OPTMATCHCLUSTERGAPSIZE].optname + dropminus,
           options[OPTMATCHCLUSTERPERCENTOVERLAP].optname + dropminus);
    return (Sint) -7;
  }
  if(!ISSET(OPTMATCHCLUSTEROUTPREFIX))
  {
    ERROR1("option %s is mandatory",
           options[OPTMATCHCLUSTEROUTPREFIX].optname + dropminus);
    return (Sint) -8;
  }
  if(!fromvmatch)
  {
    if(safestringcopy(&matchclustercallinfo->matchfile[0],argv[argnum],
                      PATH_MAX) != 0)
    {
      return (Sint) -9;
    }
  }
  return 0;
}
