
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "errordef.h"
#include "megabytes.h"
#include "fhandledef.h"
#include "optdesc.h"
#include "debugdef.h"
#include "matchinfo.h"
#include "chaindef.h"
#include "qualint.h"
#include "chaincall.h"

#include "dstrdup.pr"
#include "prsqualint.pr"
#include "procopt.pr"
#include "safescpy.pr"

#define MAXHELPLINE 512

#define DROPCHAIN 1

/*
  The following string is used to trigger the usage of gap costs
  for global chaining.
*/

#define GAPCOSTSWITCH        "gc"

/*
  The following string is used to trigger the use of a chaining algorithm
  allowing for overlaps between the hits.
*/

#define OVERLAPSWITCH        "ov"

#define CHECKHELPLINELENGTH\
        if(helplinelength >= MAXHELPLINE)\
        {\
          fprintf(stderr,"help line muster be shorter than %lu characters\n",\
                  (Showuint) MAXHELPLINE);\
          exit(EXIT_FAILURE);\
        }

#define CHECKINTANDSET(STR,VAR)\
        if((VAR) > 0)\
        {\
          ERROR3("%s-parameter of option %s already set %lu",STR,\
                  option,(Showuint) (VAR));\
          return (Sint) -1;\
        }\
        VAR = (Uint) readint

typedef enum
{
  OPTGLOBALCHAINING = 0,
  OPTLOCALCHAINING,
  OPTWEIGHTFACTOR,
  OPTMAXGAPWIDTH,
  OPTCHAINOUTPREFIX,
  OPTWITHINBORDERS,
  OPTCHAINTHREAD,
  OPTSILENT,
  OPTVERBOSE,
  OPTVMATCHVERSION,
  OPTHELP,
  NUMOFOPTIONS
} Optionnumber;

static Sint parseglobalchainingparameter(Chainmode *chainmode,
                                         const char *option,
                                         const char *gparam)
{
  if(strcmp(gparam,GAPCOSTSWITCH) == 0)
  {
    chainmode->chainkind = GLOBALCHAININGWITHGAPCOST;
    return (Sint) 0;
  }
  if(strcmp(gparam,OVERLAPSWITCH) == 0)
  {
    chainmode->chainkind = GLOBALCHAININGWITHOVERLAPS;
    return (Sint) 0;
  }
  ERROR3("illegal argument \"%s\" for option %s: please use %s or none",
         gparam,
         option,
         GAPCOSTSWITCH);
  return (Sint) -1;
}

Sint parselocalchainingparameter(Chainmode *chainmode,
                                 const char *option,
                                 const char *lparam)
{
  Qualifiedinteger qualint;

  if(parsequalifiedinteger(&qualint,option,lparam) != 0)
  {
    return (Sint) -1;
  }
  switch(qualint.qualtag)
  {
    case Qualbestof: 
      chainmode->chainkind = LOCALCHAININGBEST;
      chainmode->howmanybest = qualint.integervalue;
      break;
    case Qualpercentaway:
      chainmode->chainkind = LOCALCHAININGPERCENTAWAY;
      chainmode->percentawayfrombest = qualint.integervalue;
      break;
    case Qualabsolute:
      chainmode->chainkind = LOCALCHAININGTHRESHOLD;
      chainmode->minimumscore = (Chainscoretype) qualint.integervalue;
      break;
  }
  return 0;
}

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

static void generateglobalchainhelpline(char *helpline)
{
  Sprintfreturntype helplinelength;

  helplinelength = sprintf(helpline,
                   "global chaining\n"
                   "optional parameter \"%s\" switches\n"
                   "on gap costs (according to L1-model)\n"
                   "optional parameter \"%s\" means\n"
                   "that overlaps between matches are allowed",
                   GAPCOSTSWITCH,
                   OVERLAPSWITCH);
  CHECKHELPLINELENGTH;
}

static void generatelocalchainhelpline(char *helpline)
{
  Sprintfreturntype helplinelength;

  helplinelength = sprintf(helpline,
            "compute local chains (according to L1-model).\n"
            "If no parameter is given, compute local chains with\n"
            "maximums score.\n"
            "If parameter is given, this must be a positive number\n"
            "optionally followed by the character b or p.\n"
            "If only the number, say k, is given, this is the minimum\n"
            "score of the chains output.\n"
            "If a number is followed by character b, then output all\n"
            "chains with the largest k scores.\n"
            "If a number is followed by character p, then output all\n"
            "chains with scores at most k percent away\n"
            "from the best score.");
  CHECKHELPLINELENGTH;
}

static void generateweightfactorhelpline(char *helpline)
{
  Sprintfreturntype helplinelength;

  helplinelength = sprintf(helpline,
                   "specify weight factor > 0.0 "
                   "to obtain the score of a fragment\n"
                   "requires one of the options\n"
                   "-local,\n"
                   "-global %s,\n"
                   "-global %s",
                   GAPCOSTSWITCH,
                   OVERLAPSWITCH);
  CHECKHELPLINELENGTH;
}

static Sint parsethreadarguments(const char * const*argv,const char *option,
                                 Uint argnum,Argctype argc,
                                 Chaincallinfo *chaincallinfo)
{
  Scaninteger readint;

  while(MOREARGOPTSWITHOUTLAST(argnum))
  {
    if(strcmp(argv[argnum],"minlen1") == 0)
    {
      READINTGENERIC(option,readint,argc-1,<=,"positive");
      CHECKINTANDSET("minlen1",chaincallinfo->userdefinedminthreadlength1);
    } else
    {
      if(strcmp(argv[argnum],"maxerror1") == 0)
      {
        READPERCENTAGE(option,readint,"positive");
        CHECKINTANDSET("maxerror1",
                       chaincallinfo->userdefinedminthreaderrorpercentage1);
      } else
      {
        if(strcmp(argv[argnum],"minlen2") == 0)
        {
          READINTGENERIC(option,readint,argc-1,<=,"positive");
          CHECKINTANDSET("minlen2",chaincallinfo->userdefinedminthreadlength2);
        } else
        {
          if(strcmp(argv[argnum],"maxerror2") == 0)
          {
            READPERCENTAGE(option,readint,"positive");
            CHECKINTANDSET("maxerror2",
                           chaincallinfo->userdefinedminthreaderrorpercentage2);
          } else
          {
            ERROR2("illegal identifier \"%s\" for option \"%s\"",
                    argv[argnum],
                    option);
            return (Sint) -1;
          }
        }
      }
    }
    argnum++;
  }
  return (Sint) (argnum-1);
}

Sint parsechain2dim(BOOL fromvmatch,
                    Chaincallinfo *chaincallinfo,
                    const char * const*argv,
                    Argctype argc)
{
  Optionnumbertype optval;
  Uint argnum;
  OptionDescription options[NUMOFOPTIONS];
  Scaninteger readint;
  double readdouble;
  Sint retcode;
  char lchelpline[MAXHELPLINE+1], 
       gchelpline[MAXHELPLINE+1], 
       wfhelpline[MAXHELPLINE+1];

  chaincallinfo->defined = True;
  chaincallinfo->silent = False;
  chaincallinfo->dothreading = False;
  chaincallinfo->withinborders = False;
  chaincallinfo->chainmode.chainkind = GLOBALCHAINING;
  chaincallinfo->chainmode.maxgapwidth = 0;
  chaincallinfo->weightfactor = 1.0;
  chaincallinfo->outprefix = NULL;
  chaincallinfo->showverbose = NULL;
  chaincallinfo->userdefinedminthreadlength1 = 0;
  chaincallinfo->userdefinedminthreaderrorpercentage1 = 0;
  chaincallinfo->userdefinedminthreadlength2 = 0;
  chaincallinfo->userdefinedminthreaderrorpercentage2 = 0;

  initoptions(&options[0],(Uint) NUMOFOPTIONS);
  generatelocalchainhelpline(&lchelpline[0]);
  ADDOPTION(OPTLOCALCHAINING,"-local",&lchelpline[0]);
  generateglobalchainhelpline(&gchelpline[0]);
  ADDOPTION(OPTGLOBALCHAINING,"-global",&gchelpline[0]);
  generateweightfactorhelpline(&wfhelpline[0]);
  ADDOPTION(OPTWITHINBORDERS,"-withinborders",
            "only compute chains which do not cross sequence borders\n"
            "not possible for matches in open format");
  ADDOPTION(OPTWEIGHTFACTOR,"-wf",&wfhelpline[0]);
  ADDOPTION(OPTMAXGAPWIDTH,"-maxgap","maximal width of gap in chain");
  ADDOPTION(OPTCHAINOUTPREFIX,"-outprefix",
            "specify prefix of files to output chains");
  ADDOPTION(OPTCHAINTHREAD,"-thread",
                           "thread the chains, i.e. close the gaps\n"
                           "optional list of keywords\n"
                           "minlen1 minlen2 maxerror1 maxerror2\n"
                           "each followed by a number specifies the minimum\n"
                           "length and the maximum error rate of thread\n"
                           "1 refers to match instance in indexed sequence\n"
                           "2 refers to matching instance in query");
  ADDOPTION(OPTSILENT,"-silent","do not output the chains\n"
                                "and only report "
                                "their lengths and scores");
  ADDOPTION(OPTVERBOSE,"-v","be verbose");
  ADDOPTVMATCHVERSION;
  ADDOPTION(OPTHELP,"-help","this option");
  if(argc == 1)
  {
    if(fromvmatch)
    {
      ERROR1("missing argument for option -%s",CHAINPREFIX);
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
      case OPTVERBOSE:
        chaincallinfo->showverbose = showonstdout; 
        break;
      case OPTSILENT:
        chaincallinfo->silent = True; 
        break;
      case OPTCHAINTHREAD:
        chaincallinfo->dothreading = True;
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          retcode = parsethreadarguments(argv,options[OPTCHAINTHREAD].optname,
                                         argnum+1,argc,chaincallinfo);
          if(retcode < 0)
          {
             return (Sint) -1;
          }
          argnum = (Uint) retcode;
        }
        break;
      case OPTLOCALCHAINING:
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
          if(parselocalchainingparameter(&chaincallinfo->chainmode,
                                         options[(Uint) optval].optname +
                                         (fromvmatch ? DROPCHAIN : 0),
                                         argv[argnum]) != 0)
          {
            return (Sint) -3;
          }
        } else
        {
          chaincallinfo->chainmode.chainkind = LOCALCHAININGMAX; 
        }
        break;
      case OPTGLOBALCHAINING:
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
          if(parseglobalchainingparameter(&chaincallinfo->chainmode,
                                          options[(Uint) optval].optname,
                                          argv[argnum]) != 0)
          {
            return (Sint) -4;
          }
        } else
        {
          chaincallinfo->chainmode.chainkind = GLOBALCHAINING;
        }
        break;
      case OPTWEIGHTFACTOR:
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          READDOUBLEGENERIC(options[(Uint) optval].optname,
                            readdouble,argc-1,<=,"positive");
          chaincallinfo->weightfactor = readdouble;
        } else
        {
          ERROR1("missing argument for option %s",
                  options[(Uint) optval].optname +
                     (fromvmatch ? DROPCHAIN : 0));
          return (Sint) -5;
        }
        break;
      case OPTMAXGAPWIDTH:
        READINTGENERIC(options[(Uint) optval].optname,
                       readint,argc-1,<=,"positive");
        chaincallinfo->chainmode.maxgapwidth = (Uint) readint;
        break;
      case OPTWITHINBORDERS:
        chaincallinfo->withinborders = True;
        break;
      case OPTCHAINOUTPREFIX:
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
          ASSIGNDYNAMICSTRDUP(chaincallinfo->outprefix,argv[argnum]);
        } else
        {
          ERROR1("missing argument for option %s",
                  options[(Uint) optval].optname +
                     (fromvmatch ? DROPCHAIN : 0));
          return (Sint) -6;
        }
        break;
    }
  }
  if(argnum < (Uint) (argc-1))
  {
    ERROR1("superfluous file argument \"%s\"",argv[argc-1]);
    return (Sint) -4;
  }
  if(!fromvmatch)
  {
    if(argnum >= (Uint) argc)
    {
      ERROR0("missing matchfile");
      return (Sint) -5;
    }
  }
  if(!ISSET(OPTLOCALCHAINING) && !ISSET(OPTGLOBALCHAINING))
  {
    if(fromvmatch)
    {
      ERROR2("use either argument \"%s\" or \"%s\"",
             options[OPTLOCALCHAINING].optname+DROPCHAIN,
             options[OPTGLOBALCHAINING].optname+DROPCHAIN);
    } else
    {
      ERROR2("use either option %s or option %s",
             options[OPTLOCALCHAINING].optname,
             options[OPTGLOBALCHAINING].optname);
    }
    return (Sint) -6;
  }
  if(ISSET(OPTWEIGHTFACTOR))
  {
    if(!ISSET(OPTLOCALCHAINING) && 
       chaincallinfo->chainmode.chainkind != GLOBALCHAININGWITHGAPCOST &&
       chaincallinfo->chainmode.chainkind != GLOBALCHAININGWITHOVERLAPS)
    {
      if(fromvmatch)
      {
        ERROR6("use of argument \"%s\" requires either argument \"%s\""
               "or \"%s %s\" or \"%s %s\"",
                options[OPTWEIGHTFACTOR].optname+1,
                options[OPTLOCALCHAINING].optname+DROPCHAIN,
                options[OPTGLOBALCHAINING].optname+DROPCHAIN,
                GAPCOSTSWITCH+1,
                options[OPTGLOBALCHAINING].optname+DROPCHAIN,
                OVERLAPSWITCH+1);
      } else
      {
        ERROR5("option %s requires either option %s or option %s with "
               "argument %s or %s",
                options[OPTWEIGHTFACTOR].optname,
                options[OPTLOCALCHAINING].optname,
                options[OPTGLOBALCHAINING].optname,
                GAPCOSTSWITCH,
                OVERLAPSWITCH);
      }
      return (Sint) -7;
    }
  }
  OPTIONEXCLUDE(OPTSILENT,OPTCHAINOUTPREFIX);
  if(!fromvmatch)
  {
    if(safestringcopy(&chaincallinfo->matchfile[0],argv[argnum],
                      PATH_MAX) != 0)
    {
      return (Sint) -8;
    }
  }
  return 0;
}
