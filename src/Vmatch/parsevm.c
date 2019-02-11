
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#ifdef VMATCHDB
#include "dlopen.h"
#endif
#include "debugdef.h"
#include "codondef.h"
#include "errordef.h"
#include "maxfiles.h"
#include "spacedef.h"
#include "optdesc.h"
#include "absdef.h"
#include "alignment.h"
#include "matchtask.h"
#include "visible.h"
#include "multidef.h"

#include "dstrdup.pr"
#include "checkonoff.pr"
#include "matsort.pr"
#include "procopt.pr"
#include "prsqualint.pr"
#include "safescpy.pr"
#include "codon.pr"

#ifdef VMATCHDB
#include "parsedbms.pr"
#endif
#include "opensel.pr"
#include "procargs.pr"
#include "optstring.pr"
#include "xmlfunc.pr"
#include "vplugin-open.pr"
#include "parsepp.pr"
#include "parsequery.pr"
#include "parsedbcl.pr"
#include "keepflags.pr"

#define SIZEOFEXCLUDETAB     ((Uint) sizeof(excludecombinetable)/\
                              (Uint) sizeof(excludecombinetable[0]))

#define VMOTIFWORKPREFIX          "vmotif"
#define CPRIDXPATSEARCHWORKPREFIX "cpridxps"
#define MUMCANDIDATEFLAG          "cand"

#define READPOSITIVEINT(VAR)\
        READINTGENERIC(options[(Uint) optval].optname,VAR,argc-1,<=,"positive")

#define ASSIGNMAXLENGTH(SKIPPREF)\
        if(maxlength == 0)\
        {\
          matchcallinfo->outinfo.showdesc.untilfirstblank = True;\
        }\
        matchcallinfo->outinfo.showdesc.skipprefix = (Uint) (SKIPPREF);\
        matchcallinfo->outinfo.showdesc.maxlength = (Uint) maxlength

#define HAMMINGEDISTBUFLEN 1024   // size of space reservoir for help message 
                                 // for dbnomatch and dbmaskmatch

#define ERRORDESCSPEC\
        ERROR2("incorrect argument \"%s\" to option %s: must be either "\
               "single number or pair (skipprefix,maxlength) of "\
               "non-negative integers",\
               argv[argnum],options[(Uint) optval].optname)

#define CHECKKEEPARG(OPT,MARK,KEEP)\
        if(ISSET(OPTQUERY) && ISSET(OPT) && \
          !matchcallinfo->nomatch.markfields.MARK)\
        {\
          ERROR3("argument \"%s\" to option %s not allowed "\
                 "if option %s is used",\
                 KEEP,\
                 options[OPT].optname,\
                 options[OPTQUERY].optname);\
          return (Sint) -28;\
        }

#define DEFAULTMARKFIELDS(MF)\
        (MF).markleft = True;\
        (MF).markright = True;\
        (MF).markleftifdifferentsequence = True;\
        (MF).markrightifdifferentsequence = True

#define COMPLETEREMOVEREDUNDANT\
        "remred"

#define EHOPTNAME (ISSET(OPTHDIST) ? options[OPTHDIST].optname\
                                   : options[OPTEDIST].optname)

#define NOPPCOMBINE(B)\
        if(ISSET(OPTPOSTPROCESS) && ISSET(B))\
        {\
          ERROR2("option %s can only be combined with option %s if "\
                 "chains are threaded to form supermatches",\
                  options[B].optname,\
                  options[OPTPOSTPROCESS].optname);\
          return (Sint) -1;\
        }

typedef enum 
{
  OPTQUERY = 0,        // input parameter
  OPTDNAVSPROT,        // input parameter

  OPTTANDEM,           // matchkind; self cmp
  OPTSUPER,            // matchkind; self cmp
  OPTMAXIMALUNIQUE,    // matchkind; self cmp and query cmp
  OPTCOMPLETE,         // matchkind; query cmp mode

  OPTDBNOMATCH,        // postprocessing; self cmp and query cmp
  OPTQUERYNOMATCH,     // postprocessing; query cmp
  OPTDBMASKMATCH,      // postprocessing; self cmp and query cmp
  OPTQUERYMASKMATCH,   // postprocessing; self cmp and query cmp
  OPTDBCLUSTER,        // postprocessing; self cmp
  OPTNONREDUNDANT,     // postprocessing; self cmp
  OPTPOSTPROCESS,      // postprocessing; self cmp and query cmp
  OPTSELFUN,           // postprocessing; self cmp and query cmp
#ifdef VMATCHDB
  OPTDBMS,             // postprocessing; self cmp and query cmp
#endif

  OPTONLINE,           // algorithm
  OPTQUERYSPEEDUP,     // algorithm

  OPTDIRECT,           // direction; preselect
  OPTPALINDROMIC,      // direction; preselect

  OPTLEASTLENGTH,      // constraint (preselect)
  OPTHDIST,            // constraint; degeneration parameter (preselect)
  OPTEDIST,            // constraint; degeneration parameter (preselect)
  OPTALLMAX,           // constraint; degeneration parameter
  OPTSEEDLENGTH,       // constraint; degeneration parameter
  OPTHXDROP,           // constraint; degeneration parameter (preselect)
  OPTEXDROP,           // constraint; degeneration parameter (preselect)
  OPTLEASTSCORE,       // constraint; (postselect)
  OPTEVALUE,           // constraint; (postselect)
  OPTIDENTITY,         // constraint; (postselect)

  OPTSORT,             // output mode
  OPTBEST,             // output mode
  OPTPREINFO,          // output mode

  OPTSTRING,           // output format
  OPTDESC,             // output format
  OPTFILE,             // output format
  OPTABSOLUTEPOS,      // output format
  OPTNODIST,           // output format
  OPTNOEVALUE,         // output format
  OPTNOSCORE,          // output format
  OPTNOIDENTITY,       // output format
#ifdef DISTRIBUTEDDFS
  OPTNUMBEROFPROCESSORS,  // number of processors to be used in parallel version
#endif
  OPTVERBOSE,          
  OPTVMATCHVERSION,
  OPTHELP,
  OPTHELPPLUS,
  NUMOFOPTIONS
} Optionnumber;

/*
  The following table declares the options which exclude each other.
  Each block consists of a maximal substring not beginning with -1, but
  ending  with -1. Let \(i_{0}, i_{1}, \ldots, l_{r}\) be a block.
  Then for \(j\in[1,r]\), \(i_{0}\) and \(i_{j}\) exclude each other.
  After parsing the option the function \texttt{checkexlude} verifies,
  if any of the excluding option pairs is used. if so, then this function
  return an error code and stores an error message in messagespace().
*/

static Optionnumbertype excludecombinetable[] = 
{
  (Optionnumbertype) OPTDNAVSPROT,         
      (Optionnumbertype) OPTSUPER,         // super=>no query=>no transnum
      (Optionnumbertype) OPTTANDEM,        // tandem=>no query=>no transnum
      (Optionnumbertype) OPTDBCLUSTER,     // clustering=>no query=>no transnum
      (Optionnumbertype) OPTNONREDUNDANT,  // clustering=>no query=>no transnum
#ifdef DISTRIBUTEDDFS
      (Optionnumbertype) OPTNUMBEROFPROCESSORS, // numofproc=>no query=>no trans
#endif
      STOPSUBTABLE,
  
  (Optionnumbertype) OPTONLINE,         
      (Optionnumbertype) OPTSUPER,         // super=>selfmatch=>not online
      (Optionnumbertype) OPTTANDEM,        // tandem=>selfmatch=>not online
      (Optionnumbertype) OPTDBCLUSTER,     // clustering=>selfmatch=>not online
      (Optionnumbertype) OPTNONREDUNDANT,  // clustering=>selfmatch=>not online
      STOPSUBTABLE,

  (Optionnumbertype) OPTLEASTLENGTH,    
      (Optionnumbertype) OPTCOMPLETE,      // complete=>fixed pattern length
      STOPSUBTABLE,

  (Optionnumbertype) OPTQUERY,          
      (Optionnumbertype) OPTSUPER,         // super=>selfmatch=>no query
      (Optionnumbertype) OPTTANDEM,        // tandem=>selfmatch=>no query
      (Optionnumbertype) OPTDBCLUSTER,     // dbcluster=>selfmatch=>no query
      (Optionnumbertype) OPTNONREDUNDANT,  // nonredundant=>selfmatch=>no query
      STOPSUBTABLE,

  (Optionnumbertype) OPTCOMPLETE,       
      (Optionnumbertype) OPTDBCLUSTER,     // dbcluster=>selfmatch=>subst match
      (Optionnumbertype) OPTNONREDUNDANT,  // redundant=>selfmatch=>no complete
      (Optionnumbertype) OPTALLMAX,        // allmax=>subst match
      (Optionnumbertype) OPTMAXIMALUNIQUE, // mum=>subst match
      (Optionnumbertype) OPTSUPER,         // super=>subst match
      (Optionnumbertype) OPTTANDEM,        // tandem=>subst match
      (Optionnumbertype) OPTSEEDLENGTH,    // seedlength=>subst match
      (Optionnumbertype) OPTHXDROP,        // hxdrop=>subst match
      (Optionnumbertype) OPTEXDROP,        // exdrop=>subst match
      (Optionnumbertype) OPTQUERYSPEEDUP,  // qspeedup=>subst match
      STOPSUBTABLE,

  (Optionnumbertype) OPTMAXIMALUNIQUE,            
      (Optionnumbertype) OPTSUPER,         // tandem=>no mum
      (Optionnumbertype) OPTTANDEM,        // tandem=>no mum
      STOPSUBTABLE,

  (Optionnumbertype) OPTSUPER,          
      (Optionnumbertype) OPTTANDEM,        // tandem=>no supermax
      (Optionnumbertype) OPTPALINDROMIC,   // palindromic=>querymatch=>no super
      (Optionnumbertype) OPTQUERYSPEEDUP,  // qspeedup=>querymatch=>no selfmatch
      STOPSUBTABLE,

  (Optionnumbertype) OPTTANDEM,         
      (Optionnumbertype) OPTALLMAX,        // allmax=>extension=>makes
                                           // makes no sense for tandems
      (Optionnumbertype) OPTHDIST,         // hdist=>extension=>not for tandems
      (Optionnumbertype) OPTEDIST,         // edist=>extension=>not for tandems
      (Optionnumbertype) OPTHXDROP,        // hxdrop=>extension=>not for tandems
      (Optionnumbertype) OPTEXDROP,        // exdrop=>extension=>not for tandems
      (Optionnumbertype) OPTQUERYNOMATCH,  // qnotmatch=>querymatch=>no tandem
      (Optionnumbertype) OPTQUERYMASKMATCH,// qmaskmatch=>querymatch=>no tandem
      (Optionnumbertype) OPTPALINDROMIC,   // tandem=>bottom up trav.=>no pali
      (Optionnumbertype) OPTQUERYSPEEDUP,  // qspeedup=>querymatch=>no selfmatch
      (Optionnumbertype) OPTDBCLUSTER,     // matchs follow each other=>no clust
      (Optionnumbertype) OPTNONREDUNDANT,  // matchs follow each other=>no clust
      STOPSUBTABLE,

  (Optionnumbertype) OPTPREINFO,        
      (Optionnumbertype) OPTDBCLUSTER,     // dbcluster=>no counts=>no preinfo
      (Optionnumbertype) OPTNONREDUNDANT,  // nonredundant=>no count=>no preinfo
      (Optionnumbertype) OPTPOSTPROCESS,   // nonredundant=>no count=>no preinfo
      (Optionnumbertype) OPTSORT,          // sort=>output match=>no preinfo
      (Optionnumbertype) OPTDESC,          // desc=>output match=>no preinfo
      (Optionnumbertype) OPTABSOLUTEPOS,   // absolute=>output match=>no preinfo
      (Optionnumbertype) OPTFILE,          // file=>output match=>no preinfo
      (Optionnumbertype) OPTDBNOMATCH,     // dbnomatch=>mark=>no preinfo
      (Optionnumbertype) OPTDBMASKMATCH,   // dbmaskmatch=>mask=>no preinfo
      (Optionnumbertype) OPTQUERYMASKMATCH,// querymaskmatch=>mask=>no preinfo 
      (Optionnumbertype) OPTQUERYNOMATCH,  // querynomatch=>mask=>no preinfo 
      (Optionnumbertype) OPTNODIST,        // nodist=>output match=>no preinfo
      (Optionnumbertype) OPTNOEVALUE,      // noevalue=>output match=>no preinfo
      (Optionnumbertype) OPTNOSCORE,       // noscore=>output match=>no preinfo
      (Optionnumbertype) OPTNOIDENTITY,    // noident=>output match=>no preinfo
      (Optionnumbertype) OPTSTRING,        // noident=>output match=>no preinfo
      STOPSUBTABLE,

  (Optionnumbertype) OPTHDIST,          
      (Optionnumbertype) OPTEDIST,         // hamming=>no edit distance
      (Optionnumbertype) OPTHXDROP,        // hamming=>no hamming xdrop
      (Optionnumbertype) OPTEXDROP,        // hamming=>no edit xdrop
      STOPSUBTABLE,

  (Optionnumbertype) OPTEDIST,          
      (Optionnumbertype) OPTHXDROP,        // edist=>no hamming
      (Optionnumbertype) OPTEXDROP,        // edist=>no edit xdrop
      STOPSUBTABLE,

  (Optionnumbertype) OPTHXDROP,         
      (Optionnumbertype) OPTEXDROP,        // hamming xdrop=>no edit xdrop
      STOPSUBTABLE,

  (Optionnumbertype) OPTALLMAX,         
      (Optionnumbertype) OPTBEST,          // show allmax=>dont restrict output
      (Optionnumbertype) OPTSORT,          // sort=>do not show allmax
      (Optionnumbertype) OPTHXDROP,        // hxdrop=>all seeds=>not allmax
      (Optionnumbertype) OPTEXDROP,        // exdrop=>all seeds=>not allmax
      (Optionnumbertype) OPTLEASTSCORE,    // leastscore=>all seeds=>not allmax
      (Optionnumbertype) OPTEVALUE,        // evalue=>all seeds=>not allmax
      (Optionnumbertype) OPTIDENTITY,      // identity=>all seeds=>not allmax
      STOPSUBTABLE,

  (Optionnumbertype) OPTDESC,           
      (Optionnumbertype) OPTABSOLUTEPOS,   // description=>no position=>no abs
      STOPSUBTABLE,

  (Optionnumbertype) OPTDBNOMATCH,      
      (Optionnumbertype) OPTNODIST,        // no dist out=>no match output
      (Optionnumbertype) OPTNOEVALUE,      // no evalue out=>no match output
      (Optionnumbertype) OPTNOSCORE,       // no score out=>no match output
      (Optionnumbertype) OPTNOIDENTITY,    // no identity out=>no match out
      (Optionnumbertype) OPTDBCLUSTER,     // cluster=>no no match
      (Optionnumbertype) OPTNONREDUNDANT,  // redundant=>cluster=>no no match
      (Optionnumbertype) OPTDBMASKMATCH,   // dbmaskmatch=>no dbnomatch
      (Optionnumbertype) OPTQUERYMASKMATCH,// querymaskmach=>no dbnomatch
      (Optionnumbertype) OPTQUERYNOMATCH,  // query match=>no dbnomatch
      //(Optionnumbertype) OPTPOSTPROCESS,   // postprocess=>no dbnomatch
      STOPSUBTABLE,

  (Optionnumbertype) OPTQUERYNOMATCH,   
      (Optionnumbertype) OPTNODIST,        // nodist=>match output 
      (Optionnumbertype) OPTNOEVALUE,      // noevalue=>match output 
      (Optionnumbertype) OPTNOSCORE,       // noevalue=>match output 
      (Optionnumbertype) OPTNOIDENTITY,    // noidentity=>match output 
      (Optionnumbertype) OPTDBCLUSTER,     // dbcluster=>no query
      (Optionnumbertype) OPTNONREDUNDANT,  // dbcluster=>redundant=>no query
      //(Optionnumbertype) OPTPOSTPROCESS,   // postprocess=>no query
      STOPSUBTABLE,

  (Optionnumbertype) OPTDBMASKMATCH,    
      (Optionnumbertype) OPTNODIST,        // no dist=>match output
      (Optionnumbertype) OPTNOEVALUE,      // no evalue=>match output
      (Optionnumbertype) OPTNOSCORE,       // no score=>match output
      (Optionnumbertype) OPTNOIDENTITY,    // no identity=>match output
      (Optionnumbertype) OPTDBCLUSTER,     // dbcluster=>no masking
      (Optionnumbertype) OPTNONREDUNDANT,  // redundant=>dbcluster=>no masking
      (Optionnumbertype) OPTQUERYMASKMATCH,// querynomatch=>dbmasknomatch
      //(Optionnumbertype) OPTPOSTPROCESS,   // postprocess=>no dbmask
      STOPSUBTABLE,

  (Optionnumbertype) OPTQUERYMASKMATCH, 
      (Optionnumbertype) OPTNODIST,        // no dist=>match output
      (Optionnumbertype) OPTNOEVALUE,      // no evalue=>match output
      (Optionnumbertype) OPTNOSCORE,       // no score=>match output
      (Optionnumbertype) OPTNOIDENTITY,    // no idendity=>match output
      (Optionnumbertype) OPTDBCLUSTER,     // dbcluster=>no maskmatch
      (Optionnumbertype) OPTNONREDUNDANT,  // redundant=>dbclust=>no maskmatch
      //(Optionnumbertype) OPTPOSTPROCESS,   // postprocess=>no maskmatch
      STOPSUBTABLE,

  (Optionnumbertype) OPTPOSTPROCESS, 
      (Optionnumbertype) OPTSORT,          // sort=>output match=>no preinfo
       STOPSUBTABLE
};

/*
    OPTPOSTPROCESS can be combined with the following option only if chaining
    with threading is on
    - OPTDBNOMATCH
    - OPTQUERYNOMATCH
    - OPTDBMASKMATCH
    - OPTQUERYMASKMATCH
*/

static Optionnumbertype excludeformatchfiletable[] =
{
  (Optionnumbertype) OPTDNAVSPROT, 
  (Optionnumbertype) OPTONLINE, 
  (Optionnumbertype) OPTPREINFO, 
  (Optionnumbertype) OPTCOMPLETE, 
  (Optionnumbertype) OPTMAXIMALUNIQUE, 
  (Optionnumbertype) OPTSUPER, 
  (Optionnumbertype) OPTTANDEM, 
  (Optionnumbertype) OPTQUERY, 
  (Optionnumbertype) OPTSEEDLENGTH, 
  (Optionnumbertype) OPTHXDROP, 
  (Optionnumbertype) OPTEXDROP,
  (Optionnumbertype) OPTHDIST,       // this may be changed later
  (Optionnumbertype) OPTEDIST,       // this may be changed later
  (Optionnumbertype) OPTDIRECT,      // this may be changed later
  (Optionnumbertype) OPTPALINDROMIC, // this may be changed later
  (Optionnumbertype) OPTALLMAX,
  (Optionnumbertype) OPTDBNOMATCH,   // this may be changed later
  (Optionnumbertype) OPTQUERYNOMATCH,// this may be changed later
  (Optionnumbertype) OPTDBMASKMATCH, // this may be changed later
  (Optionnumbertype) OPTQUERYMASKMATCH, // this may be changed later
  (Optionnumbertype) OPTQUERYSPEEDUP,
  (Optionnumbertype) OPTPOSTPROCESS,
  (Optionnumbertype) OPTHELPPLUS,
#ifdef DISTRIBUTEDDFS
  (Optionnumbertype) OPTNUMBEROFPROCESSORS,
#endif
  STOPSUBTABLE
};

static Optionnumbertype extendedoptions[] =
{
  (Optionnumbertype) OPTDNAVSPROT,
  (Optionnumbertype) OPTTANDEM,
  (Optionnumbertype) OPTSUPER,
  (Optionnumbertype) OPTMAXIMALUNIQUE,
  (Optionnumbertype) OPTDBNOMATCH,
  (Optionnumbertype) OPTQUERYNOMATCH,
  (Optionnumbertype) OPTDBMASKMATCH,
  (Optionnumbertype) OPTQUERYMASKMATCH,
  (Optionnumbertype) OPTDBCLUSTER,
  (Optionnumbertype) OPTNONREDUNDANT,
  (Optionnumbertype) OPTPOSTPROCESS,
  (Optionnumbertype) OPTSELFUN,
  (Optionnumbertype) OPTALLMAX,
  (Optionnumbertype) OPTSEEDLENGTH,
  (Optionnumbertype) OPTSORT,
  (Optionnumbertype) OPTBEST,
  (Optionnumbertype) OPTPREINFO,
  (Optionnumbertype) OPTFILE,
  (Optionnumbertype) OPTABSOLUTEPOS,
  (Optionnumbertype) OPTNODIST,
  (Optionnumbertype) OPTNOEVALUE,
  (Optionnumbertype) OPTNOSCORE,
  (Optionnumbertype) OPTNOIDENTITY,
  (Optionnumbertype) OPTQUERYSPEEDUP,
#ifdef VMATCHDB
  (Optionnumbertype) OPTDBMS,
#endif
#ifdef DISTRIBUTEDDFS
  (Optionnumbertype) OPTNUMBEROFPROCESSORS,
#endif
  STOPSUBTABLE
};

static OptionGroup optiongroups[] =
{
  {(Uint) OPTQUERY, "Input~parameter"},
  {(Uint) OPTTANDEM, "Kind~of~matches"},
  {(Uint) OPTDBNOMATCH, "Postprocessing~of~matches"},
  {(Uint) OPTONLINE, "Algorithms"},
  {(Uint) OPTDIRECT, "Direction~of~matches"},
  {(Uint) OPTLEASTLENGTH, "Match~constraints"},
  {(Uint) OPTSORT, "Output~modes"},
  {(Uint) OPTSTRING, "Output~formats"},
  {(Uint) OPTVERBOSE, "Miscellaneous"}
};

Sint freeMatchcallinfo(Matchcallinfo *matchcallinfo)
{
  Uint i;

  FREESPACE(matchcallinfo->chaincallinfo.outprefix);
  FREESPACE(matchcallinfo->matchparam.dnasymbolmapdnavsprot);
  FREESPACE(matchcallinfo->matchclustercallinfo.matchfile);
  FREESPACE(matchcallinfo->matchclustercallinfo.outprefix)
  for(i=0; i<matchcallinfo->numberofqueryfiles; i++)
  {
    FREESPACE(matchcallinfo->queryfiles[i]);
  }
  FREESPACE(matchcallinfo->mfargs);
  FREEARRAY(&matchcallinfo->outinfo.matchcounttab,Uint);
#ifdef VMATCHDB
  freedbmsparms (&matchcallinfo->databaseparms);
#endif
  if(!matchcallinfo->outinfo.useprecompiledselectbundle)
  {
    if(closeSelectmatch(matchcallinfo->outinfo.selectbundle) != 0)
    {
      return (Sint) -1;
    }
    FREESPACE(matchcallinfo->outinfo.selectbundle);
  }
  if(vpluginclose(&matchcallinfo->vmotifbundle) != 0)
  {
    return (Sint) -2;
  }
  if(vpluginclose(&matchcallinfo->cpridxpatsearchbundle) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

static Sint readqueryspeedupfromenvironment(Uint *queryspeedup)
{
  char *envstring;

  if((envstring = getenv("QUERYSPEEDUP")) != NULL)
  {
    Scaninteger readint;
    if(sscanf(envstring,"%ld",&readint) != 1 || readint  < 0)
    {
      ERROR1("incorrect value \"%s\" of environment variable QUERYSPEEDUP; "
             "must be non-negative integer",
             envstring);
      return (Sint) -1;
    }
    *queryspeedup = (Uint) readint;
  }
  return 0;
}

static Sint fillhelpforoptionhammingedist(char *msgbuf,
                                          char *hammingoredist,
                                          char *error)
{
  Sprintfreturntype start;

  start = sprintf(msgbuf,
                  "specify the allowed %s distance > 0\n"
                  "in combination with option -complete one can switch on\n"
                  "the percentage search mode or the best search mode\n"
                  "for the percentage search mode use an argument\n"
                  "of the form ip where i is a positive integer\n"
                  "this means that up to i*100/m %s are\n"
                  "allowed in a match of a query of length m\n"
                  "for the best search mode use an argument\n"
                  "of the form ib where i is a positive integer\n"
                  "this means that in a first phase the minimum threshold q\n"
                  "is determined such that there is still a match\n"
                  "with q %s. q is in the range 0 to i*100/m",
                  hammingoredist,error,error);
  if(start>=HAMMINGEDISTBUFLEN)
  {
    ERROR1("fillhelpforoptionhammingedist: space buffer is too small, "
           "need at least %ld bytes",
           (Showsint) start);
    return (Sint) -1;
  }
  return 0;
}

static Sint getmaskchar(const char *arg,const char *optstring)
{
  if(strcmp(arg,"toupper") == 0)
  {
    return (Sint) MASKTOUPPER;
  }
  if(strcmp(arg,"tolower") == 0)
  {
    return (Sint) MASKTOLOWER;
  } 
  if(strlen(arg) == (size_t) 1 && !INVISIBLE((Uchar) arg[0]))
  {
    return (Sint) arg[0];
  } 
  ERROR2("illegal argument \"%s\" to option %s: "
         "must be single character or the keywords "
         "\"toupper\" or \"tolower\"",
         arg,optstring);
  return (Sint) -1;
}

static Sint parselowerupperbounds(Matchcallinfo *matchcallinfo,
                                  const char * const*argv,
                                  Argctype argc,
                                  Uint argnum,
                                  char *optionname)
{
  Scaninteger readint;

  if(MOREARGOPTSWITHOUTLAST(argnum))
  {
    argnum++;
    if(sscanf(argv[argnum],"%ld",&readint) != 1)
    {
      ERROR2("optional second argument \"%s\" of option %s must "
                   " be number",argv[argnum],optionname);
      return (Sint) -1;
    }
    if(readint < 0 && 
       -readint > (Scaninteger) matchcallinfo->matchparam.
                                    userdefinedleastlength)
    {
      ERROR0("if second argument is negative, the absolute value must "
             "not be larger than the user defined leastlength");
      return (Sint) -1;
    }
    matchcallinfo->matchparam.repeatgapspec.lowergaplength = (Sint) readint;
    matchcallinfo->matchparam.repeatgapspec.lowergapdefined = True;
    if(MOREARGOPTSWITHOUTLAST(argnum))
    {
      argnum++;
      if(sscanf(argv[argnum],"%ld",&readint) != 1)
      {
        ERROR2("optional third argument \"%s\" of option %s must be a number",
               argv[argnum],optionname);
        return (Sint) -2;
      }
      if(readint < 
         (Scaninteger) matchcallinfo->matchparam.repeatgapspec.lowergaplength)
      {
        ERROR3("optional second argument \"%s\" of option %s must "
               "be greater or equal than first argument \"%s\"",
               argv[argnum],optionname,argv[argnum-1]);
        return (Sint) -3;
      }
      matchcallinfo->matchparam.repeatgapspec.uppergaplength = (Sint) readint;
      matchcallinfo->matchparam.repeatgapspec.uppergapdefined = True;
    }
  }
  return (Sint) argnum;
}

static Sint parsedescparameters(Matchcallinfo *matchcallinfo,
                                const char * const*argv,
                                Argctype argc,
                                Uint argnum,
                                OptionDescription *options,
                                Optionnumbertype optval)
{
  argnum++;
  CHECKMISSINGARGUMENTWITHOUTLAST;
  matchcallinfo->outinfo.showdesc.defined = True;
  matchcallinfo->outinfo.showdesc.replaceblanks = True;
  matchcallinfo->outinfo.showdesc.untilfirstblank = False;
  if(argv[argnum][0] == '(')
  {
    Scaninteger skipprefix, maxlength;

    if(sscanf(argv[argnum],"(%ld,%ld)",&skipprefix,&maxlength) != 2 ||
       skipprefix < 0 || maxlength < 0)
    {
      ERRORDESCSPEC;
      return (Sint) -1;
    }
    ASSIGNMAXLENGTH(skipprefix);
  } else
  {
    Scaninteger maxlength;

    if(sscanf(argv[argnum],"%ld",&maxlength) != 1 || maxlength < 0)
    {
      ERRORDESCSPEC;
      return (Sint) -2;
    }
    ASSIGNMAXLENGTH(0);
  }
  return (Sint) argnum;
}

#ifdef VMATCHDB

static Sint parsevmatchdbparameters(BOOL withindexfile,
                                    Uint *selfunfileindex,
                                    Matchcallinfo *matchcallinfo,
                                    const char **argv,
                                    Argctype argc,
                                    Uint argnum,
                                    OptionDescription *options,
                                    Optionnumbertype optval)
{
  /*
    In case matchdata has to be transferred into database using
    a selfun bundle, parsing of database paramaters can't be done
    here. It will be done in a selection function, because the
    databaseparms-record can't be passed to a selection function.
    Even if we would accept a doubled parsing of database params here
    and in the selection function we then would have to cope with
    actions triggered by the parsing process like prompting for a
    password.
  */
  if(!withindexfile)
  {
    if (parsedbmsopt(&matchcallinfo->databaseparms,
                     argc,argv,&argnum,1) < 0)
    {
      return (Sint) -37;
    }
  } else
  {
    argnum++;
    CHECKMISSINGARGUMENTWITHOUTLAST;
    /*
       keeping -dbms without impact in case an old argument line is going
       to be processed.
    */
    if (strlen(argv[0]) > 0)
    {
      matchcallinfo->databaseparms.usedatabase = True;
      *selfunfileindex = argnum;
      /*
        The vmatchDB needs access to original input sequences
        and descriptions to store them into database. The simpliest
        way to make vmatch map this data into memory is to pretend
        that -s and -showdesc had been given.
      */
      matchcallinfo->outinfo.showstring = DEFAULTLINEWIDTH;
      matchcallinfo->outinfo.showdesc.defined = True;
    }
    while(MOREARGOPTSWITHOUTLAST(argnum))
    {
      argnum++;
    }
  }
  return (Sint) argnum;
}

#endif

static Sint initmatchcallinfo(Matchcallinfo *matchcallinfo,
                              BOOL withindexfile,
                              const char * const*argv,
                              FILE *outfp)
{
  DEFAULTMATCHPARAM(&matchcallinfo->matchparam);
  matchcallinfo->chaincallinfo.defined = False;
  matchcallinfo->chaincallinfo.outprefix = NULL;
  matchcallinfo->matchclustercallinfo.defined = False;
  matchcallinfo->matchclustercallinfo.outprefix = NULL;
  matchcallinfo->matchclustercallinfo.matchfile = NULL;
  matchcallinfo->vmotifbundle.handle = NULL;
  matchcallinfo->cpridxpatsearchbundle.handle = NULL;
  matchcallinfo->numberofqueryfiles = 0;
  matchcallinfo->matchtask = 0;
  matchcallinfo->nomatch.donomatch = False;
  DEFAULTMARKFIELDS(matchcallinfo->nomatch.markfields);
  matchcallinfo->maskmatch.domaskmatch = False;
  DEFAULTMARKFIELDS(matchcallinfo->maskmatch.markfields);
  matchcallinfo->clusterparms.dbclusterdefined = False;
  matchcallinfo->clusterparms.dbclminsize = UintConst(1);
  matchcallinfo->clusterparms.dbclmaxsize = DBCLMAXSIZE;
  matchcallinfo->clusterparms.dbclusterfilenameprefix = NULL;
  matchcallinfo->clusterparms.nonredundantfile = NULL;
  matchcallinfo->mfargs = NULL;
  matchcallinfo->withindexfile = withindexfile;
  ASSIGNDEFAULTDIGITS(&matchcallinfo->outinfo.digits);
  matchcallinfo->outinfo.showmode = 0;
  matchcallinfo->outinfo.showstring = 0;
  matchcallinfo->outinfo.showdesc.defined = False;
  matchcallinfo->outinfo.outfp = outfp;
  matchcallinfo->outinfo.sortmode = undefsortmode();
  matchcallinfo->markinfo.markmatchtable = NULL;
  INITARRAY(&matchcallinfo->outinfo.matchcounttab,Uint);
#ifdef VMATCHDB
  initdbmsparms(&matchcallinfo->databaseparms);
#endif
  if(safestringcopy(&matchcallinfo->progname[0],argv[0],PATH_MAX) != 0)
  {
    return (Sint) -1;
  }
  matchcallinfo->showverbose = NULL;
/*
  In the default case we show all best matches. So we set the 
  bestnumber accordingly.
*/
  matchcallinfo->outinfo.bestinfo.bestflag = Allbestmatches; 
  matchcallinfo->fqfsubstringinfo.ffdefined = False;
  return 0;
}

Sint parsevmatchargs(BOOL withindexfile,
                     Argctype argc,
                     const char * const*argv,
                     Showverbose showverbose,
                     FILE *outfp,
                     SelectBundle *precompiledselectbundle,
                     Matchcallinfo *matchcallinfo)
{
  Uint argnum, selfunfileindex = 0;
  Sint retcode, sortmode;
  Optionnumbertype optval;
  Scaninteger readint;
  char keepflagshelp[KEEPFLAGSHELPSIZE+1],
       hamminghelpbuf[HAMMINGEDISTBUFLEN+1],
       dnavsprothelpbuf[MAXTRANSNAMESPACE+1],
       edisthelpbuf[HAMMINGEDISTBUFLEN+1]; 
  Qualifiedinteger qualint;
  OptionDescription options[NUMOFOPTIONS];

  if(initmatchcallinfo(matchcallinfo,
                       withindexfile,
                       argv,
                       outfp) != 0)
  {
    return (Sint) -1;
  }
  initoptions(&options[0],(Uint) NUMOFOPTIONS);
  ADDOPTION(OPTONLINE,"-online",
                      "run algorithms online without using the index");
  helptransnumorganism((Uint) MAXTRANSNAMESPACE,&dnavsprothelpbuf[0]);
  ADDOPTION(OPTDNAVSPROT,"-dnavsprot",&dnavsprothelpbuf[0]);
  ADDOPTION(OPTLEASTLENGTH,"-l",
                           "specify that match must have the given length\n"
                           "optionally specify minimum and maximum size of\n"
                           "gaps between repeat instances");
  ADDOPTION(OPTPREINFO,"-i",
                       "give information about number of different matches");
  ADDOPTION(OPTCOMPLETE,"-complete",
                        "specify that query sequences must match completely");
  ADDOPTION(OPTMAXIMALUNIQUE,"-mum",
                   "compute maximal unique matches");
  ADDOPTION(OPTSUPER,"-supermax",
                     "compute supermaximal matches");
  ADDOPTION(OPTTANDEM,"-tandem",
                      "compute right branching tandem repeats");
  ADDOPTION(OPTQUERY,"-q",
                     "specify files containing queries to be matched");
  ADDOPTION(OPTQUERYSPEEDUP,"-qspeedup",
                            "specify speedup level when matching queries\n"
                            "(0: fast, 2: faster; default is 2)\n"
                            "beware of time/space tradeoff");
  if(fillhelpforoptionhammingedist(&hamminghelpbuf[0],
                                   "hamming","mismatches") !=0)
  {
    return (Sint) -1;
  }
  ADDOPTION(OPTHDIST,"-h", &hamminghelpbuf[0]);
  if(fillhelpforoptionhammingedist(&edisthelpbuf[0],"edit","differences") != 0)
  {
    return (Sint) -1;
  }
  ADDOPTION(OPTEDIST,"-e",&edisthelpbuf[0]);
  ADDOPTION(OPTHXDROP,"-hxdrop",
                      "specify the xdrop value for hamming distance extension");
  ADDOPTION(OPTEXDROP,"-exdrop",
                      "specify the xdrop value for edit distance extension");
  ADDOPTION(OPTLEASTSCORE,"-leastscore",
                          "specify the minimum score of a match");
  ADDOPTION(OPTEVALUE,"-evalue",
                      "specify the maximum E-value of a match");
  ADDOPTION(OPTIDENTITY,"-identity",
                        "specify minimum identity of match in range [1..100%]");
  ADDOPTION(OPTSEEDLENGTH,"-seedlength",
                          "specify the seed length");
  ADDOPTION(OPTDESC,"-showdesc",
                    "show sequence description of match");
  ADDOPTION(OPTDIRECT,"-d",
                      "compute direct matches (default)");
  ADDOPTION(OPTPALINDROMIC,"-p",
                           "compute palindromic "
                           "(i.e. reverse complemented matches)");
  ADDOPTION(OPTALLMAX,"-allmax",
                      "show all maximal matches in the order "
                      "of their computation");
  ADDOPTION(OPTBEST,"-best",
                    "show the best matches (those with smallest E-values)\n"
                    "default is best 50");
  ADDOPTION(OPTSORT,"-sort",concatsorthelp());
  ADDOPTION(OPTSELFUN,"-selfun",
                      "specify shared object file containing "
                      "selection function");
#ifdef VMATCHDB
  ADDOPTION(OPTDBMS,DBOPTIONNAME,  //-dbms
                    "specify values for connecting to a database");
#endif
  ADDOPTION(OPTSTRING,"-s",
                      "show the alignment of matching sequences");
  ADDOPTION(OPTABSOLUTEPOS,"-absolute",
                           "show absolute positions");
  ADDOPTION(OPTFILE,"-f",
                    "show filename where match occurs");
  ADDOPTION(OPTNODIST,"-nodist",
                      "do not show distance of match");
  ADDOPTION(OPTNOEVALUE,"-noevalue",
                        "do not show E-value of match");
  ADDOPTION(OPTNOSCORE,"-noscore",
                       "do not show score of match");
  ADDOPTION(OPTNOIDENTITY,"-noidentity",
                          "do not show identity of match");
  if(fillkeepflagshelp(keepflagshelp,"show all database substrings not",
                       "keep") != 0)
  {
    return (Sint) -2;
  }
  ADDOPTION(OPTDBNOMATCH,"-dbnomatch",
                         keepflagshelp);
  ADDOPTION(OPTQUERYNOMATCH,"-qnomatch",
                            "show all query substrings "
                            "not containing a match");
  if(fillkeepflagshelp(keepflagshelp,"mask all database substrings",
                       "not mask") != 0)
  {
    return (Sint) -2;
  }
  ADDOPTION(OPTDBMASKMATCH,"-dbmaskmatch",
                           keepflagshelp);
  ADDOPTION(OPTQUERYMASKMATCH,"-qmaskmatch",
                              "mask all query substrings containing a match");
  ADDOPTION(OPTDBCLUSTER,"-dbcluster",
                         "cluster the database sequences\n"
                         "* first argument is percentage of shorter string\n"
                         "  to be included in match,\n"
                         "* second argument is percentage of larger string\n"
                         "  to be included in match,\n"
                         "* third optional argument is filenameprefix,\n"
                         "* fourth optional argument is (minclustersize, "
                         "maxclustersize)");
  ADDOPTION(OPTNONREDUNDANT,"-nonredundant",
                            "generate file with non-redundant set of "
                            "sequences;\n"
                            "only works together with option -dbcluster");
  ADDOPTION(OPTPOSTPROCESS,"-pp","generic postprocessing of matches");
#ifdef DISTRIBUTEDDFS
  ADDOPTION(OPTNUMBEROFPROCESSORS,"-numproc",
                            "specify the number of processors to\n"
                            "be used in parallel version");
#endif
  ADDOPTION(OPTVERBOSE,"-v","verbose mode");
  ADDOPTVMATCHVERSION;
  ADDOPTION(OPTHELP,"-help","show basic options");
  ADDOPTION(OPTHELPPLUS,"-help+","show all options");

  if(argc == 1)
  {
    USAGEOUT;
    return (Sint) -2;
  } 
  for(argnum = UintConst(1); 
      argnum < (Uint) argc && ISOPTION(argv[argnum]);
      argnum++)
  {
    if(strcmp("-bestall",argv[argnum]) == 0)
    {
      ERROR0("option -bestall does not exist any more; "
             "bestall is the default mode now");
      return (Sint) -3;
    }
    optval = procoption(options,(Uint) NUMOFOPTIONS,argv[argnum]);
    if(optval < 0)
    {
      return (Sint) -4;
    }
    switch(optval)
    {
      case OPTHELP:
        showoptionswithoutexclude(matchcallinfo->outinfo.outfp,
                                  argv[0],
                                  options,
                                  withindexfile ? 
                                    extendedoptions :
                                    excludeformatchfiletable,
                                  (Uint) NUMOFOPTIONS);
        return (Sint) 1;
      case OPTVMATCHVERSION:
        return (Sint) 1;
      case OPTHELPPLUS:
        if(checkenvvaronoff("VMATCHSHOWOPTIONINLATEX"))
        {
          showoptionsinlatex(matchcallinfo->outinfo.outfp,
                             argv[0],
                             options,
                             &optiongroups[0],
                             (Uint) NUMOFOPTIONS);
        } else
        {
          if(withindexfile)
          {
            showoptions(matchcallinfo->outinfo.outfp,
                        argv[0],options,(Uint) NUMOFOPTIONS);
          } else
          {
            ERROR1("illegal option \"%s\"",options[OPTHELPPLUS].optname);
            return (Sint) -5;
          }
        }
        return (Sint) 1;
      case OPTLEASTLENGTH:
        READPOSITIVEINT(readint);
        matchcallinfo->matchparam.userdefinedleastlength = (Uint) readint;
        retcode = parselowerupperbounds(matchcallinfo,
                                        argv,
                                        argc,
                                        argnum,
                                        options[(Uint) optval].optname);
        if(retcode < 0)
        {
          return (Sint) -6;
        }
        argnum = (Uint) retcode;
        break;
      case OPTHDIST:
      case OPTEDIST:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        if(parsequalifiedinteger(&qualint,
                                 options[(Uint) optval].optname,
                                 argv[argnum]) != 0)
        {
          return (Sint) -5;
        }
        matchcallinfo->matchparam.maxdist.distinterpretation = qualint.qualtag;
        matchcallinfo->matchparam.maxdist.distvalue = qualint.integervalue;
        if(optval == (Optionnumbertype) OPTHDIST)
        {
          matchcallinfo->matchparam.maxdist.distkind = Hammingmatch;
        } else
        {
          matchcallinfo->matchparam.maxdist.distkind = Edistmatch;
        }
        break;
      case OPTHXDROP:
      case OPTEXDROP:
        READPOSITIVEINT(readint);
        if(readint > (Scaninteger) FLAGXDROP)
        {
          ERROR3("argument \"%s\" to option %s must be in the range [1,%lu]",
                  argv[argnum],
                  options[(Uint) optval].optname,
                  (Showuint) FLAGXDROP);
          return (Sint) -6;
        }
        if(optval == (Optionnumbertype) OPTHXDROP)
        {
          matchcallinfo->matchparam.xdropbelowscore = (Xdropscore) -readint;
        } else
        {
          matchcallinfo->matchparam.xdropbelowscore = (Xdropscore) readint;
        }
        break;
      case OPTLEASTSCORE:
        READPOSITIVEINT(readint);
        matchcallinfo->matchparam.xdropleastscore = (Xdropscore) readint;
        break;
      case OPTEVALUE:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        if(sscanf(argv[argnum],"%lf",
                  &matchcallinfo->matchparam.maximumevalue) != 1 ||
                  (matchcallinfo->matchparam.maximumevalue != 0.0 &&
                   matchcallinfo->matchparam.maximumevalue < SMALLESTEVALUE))
        {
          ERROR3("argument \"%s\" to option %s must be floating point value "
                 "which is either 0.0 or >= %.2e",
                  argv[argnum],
                  options[(Uint) optval].optname,
                  SMALLESTEVALUE);
          return (Sint) -8;
        }
        break;
      case OPTIDENTITY:
        READPOSITIVEINT(readint);
        if(readint > (Scaninteger) 100)
        {
          ERROR2("argument \"%s\" to option %s must be in the range [1,100]",
                  argv[argnum],options[(Uint) optval].optname);
          return (Sint) -9;
        }
        matchcallinfo->matchparam.identity = (Uint) readint;
        break;
      case OPTDBNOMATCH:
      case OPTQUERYNOMATCH:
        READPOSITIVEINT(readint);
        matchcallinfo->nomatch.donomatch = True;
        matchcallinfo->nomatch.nomatchlength = (Uint) readint;
        if(optval == (Optionnumbertype) OPTDBNOMATCH)
        {
          matchcallinfo->nomatch.markfields.markdb = True;
          if(MOREARGOPTSWITHOUTLAST(argnum))
          {
            argnum++;
            if(parsekeepflags(&matchcallinfo->nomatch.markfields,
                              argv[argnum],
                              options[(Uint) optval].optname) != 0)
            {
              return (Sint) -10;
            }
          }
        } else
        {
          matchcallinfo->nomatch.markfields.markdb = False;
        }
        break;
      case OPTDBMASKMATCH:
      case OPTQUERYMASKMATCH:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        retcode = getmaskchar(argv[argnum],options[(Uint) optval].optname);
        if(retcode < 0)
        {
          return (Sint) -11;
        }
        matchcallinfo->maskmatch.maskchar = (Uchar) retcode;
        matchcallinfo->maskmatch.domaskmatch = True;
        if(optval == (Optionnumbertype) OPTDBMASKMATCH)
        {
          matchcallinfo->maskmatch.markfields.markdb = True;
          if(MOREARGOPTSWITHOUTLAST(argnum))
          {
            argnum++;
            if(parsekeepflags(&matchcallinfo->maskmatch.markfields,
                              argv[argnum],
                              options[(Uint) optval].optname) != 0)
            {
              return (Sint) -12;
            }
          }
        } else
        {
          matchcallinfo->maskmatch.markfields.markdb = False;
        }
        break;
      case OPTPOSTPROCESS:
        argnum++;
        retcode 
          = parsegenericpostprocessing(&matchcallinfo->chaincallinfo,
                                       &matchcallinfo->matchclustercallinfo,
                                       matchcallinfo->progname,
                                       argv,
                                       argnum,
                                       argc,
                                       options[(Uint) optval].optname);
        if(retcode <= 0)
        {
          return (Sint) -13;
        }
        argnum += (Uint) (retcode-1);
        break;
      case OPTSEEDLENGTH:
        READPOSITIVEINT(readint);
        matchcallinfo->matchparam.seedlength = (Uint) readint;
        break;
      case OPTBEST:
        READPOSITIVEINT(readint);
        matchcallinfo->outinfo.bestinfo.bestflag = Fixednumberofbest;
        matchcallinfo->outinfo.bestinfo.bestnumber = (Uint) readint;
        break;
      case OPTSORT:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        sortmode = checksortmode(argv[argnum]);
        if(sortmode == (Sint) -1)
        {
          ERROR2("argument \"%s\" to option sort must be one of %s",
                  argv[argnum],concatsortmodes());
          return (Sint) -12;
        }
        matchcallinfo->outinfo.sortmode = (Uint) sortmode;
        break;
      case OPTALLMAX:
        matchcallinfo->outinfo.bestinfo.bestflag = Allmaximalmatches;
        break;
      case OPTQUERY:
        retcode = parsequeryparameters(matchcallinfo,
                                       argv,
                                       argc,
                                       argnum,
                                       options[(Uint) optval].optname);
        if(retcode < 0)
        {
          return (Sint) -13;
        }
        argnum = (Uint) retcode;
        break;
      case OPTQUERYSPEEDUP:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        if(sscanf(argv[argnum],"%ld",&readint) != 1 || readint < 0)
        {
          ERROR2("argument \"%s\" of option %s must be non-negative integer",
                 argv[argnum],options[(Uint) optval].optname);
          return (Sint) -16;
        } 
        matchcallinfo->matchparam.queryspeedup = (Uint) readint;
        break;
      case OPTCOMPLETE:
        matchcallinfo->matchtask |= TASKCOMPLETEMATCH;
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
          if(strcmp(argv[argnum],COMPLETEREMOVEREDUNDANT) == 0)
          {
            matchcallinfo->matchparam.completematchremoveredundancy = True;
          } else
          {
            if(strncmp(argv[argnum],VMOTIFWORKPREFIX,
                       strlen(VMOTIFWORKPREFIX)) == 0)
            {
              if(vpluginopen(argv[argnum],
                             &matchcallinfo->vmotifbundle) != 0)
              {
                return (Sint) -3;
              }
            } else
            {
              if(strncmp(argv[argnum],CPRIDXPATSEARCHWORKPREFIX,
                       strlen(CPRIDXPATSEARCHWORKPREFIX)) == 0)
              {
                if(vpluginopen(argv[argnum],
                               &matchcallinfo->cpridxpatsearchbundle) != 0)
                {
                  return (Sint) -4;
                }
              } else
              {
                ERROR4("argument to option %s must be either the keyword "
                       "\"%s\" or names of shared object files with prefix "
                       "\"%s\" or \"%s\"",options[(Uint) optval].optname,
                       COMPLETEREMOVEREDUNDANT,
                       VMOTIFWORKPREFIX,
                       CPRIDXPATSEARCHWORKPREFIX);
                return (Sint) -5;
              }
            }
          }
        } 
        break;
      case OPTDESC:
        retcode = parsedescparameters(matchcallinfo,
                                      argv,
                                      argc,
                                      argnum,
                                      options,
                                      optval);
        if(retcode < 0)
        {
          return (Sint) -18;
        }
        argnum = (Uint) retcode;
        break;
      case OPTABSOLUTEPOS:
        matchcallinfo->outinfo.showmode |= SHOWABSOLUTE;
        break;
      case OPTFILE:
        matchcallinfo->outinfo.showmode |= SHOWFILE;
        break;
      case OPTNODIST:
        matchcallinfo->outinfo.showmode |= SHOWNODIST;
        break;
      case OPTNOEVALUE:
        matchcallinfo->outinfo.showmode |= SHOWNOEVALUE;
        break;
      case OPTNOSCORE:
        matchcallinfo->outinfo.showmode |= SHOWNOSCORE;
        break;
      case OPTNOIDENTITY:
        matchcallinfo->outinfo.showmode |= SHOWNOIDENTITY;
        break;
      case OPTSELFUN:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        selfunfileindex = argnum;
        while(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
        }
        break;
      case OPTSTRING:
        matchcallinfo->outinfo.showstring
          = parsesequenceoutparms(argc,
                                  argv,
                                  &argnum,
                                  options[(Uint) optval].optname);
        if(matchcallinfo->outinfo.showstring == 0)
        {
          return (Sint) -1;
        }
        break;
      case OPTDIRECT:
        matchcallinfo->outinfo.showmode |= SHOWDIRECT;
        break;
      case OPTPALINDROMIC:
        matchcallinfo->outinfo.showmode |= SHOWPALINDROMIC;
        break;
      case OPTPREINFO:
        matchcallinfo->matchtask |= TASKPREINFO;
        break;
      case OPTMAXIMALUNIQUE:
        matchcallinfo->matchtask |= TASKMUM;
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
          if(strcmp(argv[argnum],MUMCANDIDATEFLAG) == 0)
          {
            matchcallinfo->matchtask |= TASKMUMCANDIDATE;
          } else
          {
            ERROR2("optional argument of option %s must be %s",
                    options[(Uint) optval].optname,
                    MUMCANDIDATEFLAG);
            return (Sint) -1;
          }
        }
        break;
      case OPTSUPER:
        matchcallinfo->matchtask |= TASKSUPER;
        break;
      case OPTTANDEM:
        matchcallinfo->matchtask |= TASKTANDEM;
        break;
      case OPTDBCLUSTER:
        retcode = parsedbclusterparameters(matchcallinfo,
                                           argv,
                                           argc,
                                           argnum,
                                           options[(Uint) optval].optname);
        if(retcode < 0)
        {
          return (Sint) -21;
        }
        argnum = (Uint) retcode;
        break;
      case OPTNONREDUNDANT:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        ASSIGNDYNAMICSTRDUP(matchcallinfo->clusterparms.nonredundantfile,
                            argv[argnum]);
        break;
      case OPTONLINE:
        matchcallinfo->matchtask |= TASKONLINE;
        break;
      case OPTDNAVSPROT:
        READINTGENERIC(options[(Uint) optval].optname,
                       readint,argc-1,<,"non-negative");
        matchcallinfo->matchparam.transnum = (Uint) readint;
        if(checktransnum(matchcallinfo->matchparam.transnum) != 0)
        {
          return (Sint) -22;
        }
        if(MOREARGOPTSWITHOUTLAST(argnum))
        {
          argnum++;
          ASSIGNDYNAMICSTRDUP(matchcallinfo->matchparam.dnasymbolmapdnavsprot,
                              argv[argnum]);
        }
        break;
#ifdef VMATCHDB
      case OPTDBMS:
        retcode = parsevmatchdbparameters(withindexfile,
                                          &selfunfileindex,
                                          matchcallinfo,
                                          argv,
                                          argc,
                                          argnum,
                                          options,
                                          optval);
        if(retcode < 0)
        {
          return (Sint) -22;
        }
        argnum = (Sint) retcode;
        break;
#endif
#ifdef DISTRIBUTEDDFS
      case OPTNUMBEROFPROCESSORS:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        if(sscanf(argv[argnum],"%ld",&readint) != 1 || 
           readint < (Scaninteger) 2)
        {
          ERROR2("argument \"%s\" of option %s must be integer >= 2",
                 argv[argnum],options[(Uint) optval].optname);
          return (Sint) -16;
        }
        matchcallinfo->matchparam.numberofprocessors = (Uint) readint;
        break;
#endif
      case OPTVERBOSE:
        matchcallinfo->showverbose = showverbose;
        break;
    }
  }
#ifdef VMATCHDB
  if (ISSET(OPTSELFUN) && ISSET(OPTDBMS))
  {
    if(withindexfile)
    {
      ERROR2("option %s can't be given together with option %s",
             options[OPTSELFUN].optname,
             DBOPTIONNAME);
      return (Sint) -37;
    } else
    {
      void *handle;
      handle = dlopen(argv[selfunfileindex],RTLD_LAZY);
      if (handle != NULL)
      {
        if (dlsym(handle,"dbmsConnect") != NULL)\
        {
          matchcallinfo->outinfo.showstring = DEFAULTLINEWIDTH;
          matchcallinfo->outinfo.showdesc.defined = True;
        }
      }
    }
  }
#endif
  if(!withindexfile)
  {
    if(checkforsingleexcludeoption(&options[0],
                                   excludeformatchfiletable) 
                                   != 0)
    {
      return (Sint) -24;
    }
  }
  if(argnum < (Uint) (argc-1))
  {
    ERROR1("superfluous file argument \"%s\"",argv[argc-1]);
    return (Sint) -25;
  }
  if(argnum >= (Uint) argc)
  {
    if(withindexfile)
    {
      ERROR0("missing indexname");
    } else
    {
      ERROR0("missing matchfile");
    }
    return (Sint) -26;
  }
  if(matchcallinfo->matchparam.queryspeedup == UintConst(1))
  {
    ERROR0("Algorithm 1 is no longer available, "
           "please use Algorithm 0, or 2; we recommend Algorithm 2");
    return (Sint) -28;
  }
  if(safestringcopy(&matchcallinfo->indexormatchfile[0],argv[argnum],
                    PATH_MAX) != 0)
  {
    return (Sint) -27;
  }
  if(matchcallinfo->outinfo.showstring & SHOWVMATCHXML)
  {
    Optionnumber onum;
    for(onum = OPTNODIST; onum <= OPTNOIDENTITY; onum++)
    {
      if(ISSET(onum))
      {
        ERROR1("option \"%s\" is not possible for xml format",
                options[onum].optname);
        return (Sint) -28;
      }
    }
  }
  OPTIONIMPLYEITHER4(OPTSEEDLENGTH,OPTHDIST,OPTEDIST,OPTHXDROP,OPTEXDROP);
  OPTIONIMPLY(OPTDNAVSPROT,OPTQUERY);
  OPTIONIMPLY(OPTONLINE,OPTQUERY); // This should be modified
  OPTIONIMPLY(OPTMAXIMALUNIQUE,OPTLEASTLENGTH);
  OPTIONIMPLYEITHER2(OPTALLMAX,OPTHDIST,OPTEDIST);
  OPTIONIMPLY(OPTSUPER,OPTLEASTLENGTH);
  OPTIONIMPLY(OPTTANDEM,OPTLEASTLENGTH);
  OPTIONIMPLY(OPTCOMPLETE,OPTQUERY);
  OPTIONIMPLY(OPTNONREDUNDANT,OPTDBCLUSTER);
  OPTIONIMPLYEITHER2(OPTQUERYSPEEDUP,OPTQUERY,OPTPALINDROMIC);
  OPTIONIMPLYEITHER4(OPTHXDROP,OPTLEASTSCORE,OPTIDENTITY,OPTLEASTLENGTH,
                     OPTEVALUE);
  OPTIONIMPLYEITHER4(OPTEXDROP,OPTLEASTSCORE,OPTIDENTITY,OPTLEASTLENGTH,
                     OPTEVALUE);
  if(!matchcallinfo->chaincallinfo.defined ||
     !matchcallinfo->chaincallinfo.dothreading)
  {
    NOPPCOMBINE(OPTDBCLUSTER);
    NOPPCOMBINE(OPTDBNOMATCH);
    NOPPCOMBINE(OPTQUERYNOMATCH);
    NOPPCOMBINE(OPTDBMASKMATCH);
    NOPPCOMBINE(OPTQUERYMASKMATCH);
  }
  if(ISSET(OPTCOMPLETE))
  {
    if(!ISSET(OPTONLINE))
    {
      if(matchcallinfo->matchparam.completematchremoveredundancy)
      {
        ERROR3("argument \"%s\" of option %s requires option %s",
               COMPLETEREMOVEREDUNDANT,
               options[OPTCOMPLETE].optname,
               options[OPTONLINE].optname);
        return (Sint) -17;
      }
    } else
    {
      if(MPARMEXACTMATCH(&matchcallinfo->matchparam.maxdist) &&
         matchcallinfo->matchparam.completematchremoveredundancy)
      {
        ERROR4("argument \"%s\" of option %s requires options %s or %s",
               COMPLETEREMOVEREDUNDANT,
               options[OPTCOMPLETE].optname,
               options[OPTEDIST].optname,
               options[OPTHDIST].optname);
        return (Sint) -29;
      }
    }
  } else
  {
    if(ISSET(OPTHDIST) || ISSET(OPTEDIST))
    {
      if(!ISSET(OPTLEASTLENGTH))
      {
        ERROR3("if option %s is not used, then option %s requires option %s",
               options[OPTCOMPLETE].optname,
               EHOPTNAME,
               options[OPTLEASTLENGTH].optname);
        return (Sint) -29;
      }
      if(matchcallinfo->matchparam.maxdist.distinterpretation == Qualbestof)
      {
        ERROR4("option %s %lu%c is only possible together with option %s",
               EHOPTNAME,
               (Showuint) matchcallinfo->matchparam.maxdist.distvalue,
               BESTCHARACTER,
               options[OPTCOMPLETE].optname);
        return (Sint) -29;
      }
      if(matchcallinfo->matchparam.maxdist.distinterpretation ==
         Qualpercentaway)
      {
        ERROR4("option %s %lu%c is only possible together with option %s",
               EHOPTNAME,
               (Showuint) matchcallinfo->matchparam.maxdist.distvalue,
               PERCENTAWAYCHARACTER,
               options[OPTCOMPLETE].optname);
        return (Sint) -29;
      }
/*
      if(matchcallinfo->matchparam.maxdist.distinterpretation == Qualabsolute &&
         matchcallinfo->matchparam.maxdist.distvalue > (Uint) MAXDIST)
      {
        ERROR2("distance value for option %s must be in the range [1,%lu]",
                EHOPTNAME,(Showuint) MAXDIST);
        return (Sint) -30;
      }
*/
    }
  }
  CHECKKEEPARG(OPTDBNOMATCH,markleft,"keepleft");
  CHECKKEEPARG(OPTDBMASKMATCH,markleft,"keepleft");
  CHECKKEEPARG(OPTDBNOMATCH,markright,"keepright");
  CHECKKEEPARG(OPTDBMASKMATCH,markright,"keepright");
  CHECKKEEPARG(OPTDBNOMATCH,markleftifdifferentsequence,
                            "keepleftifsamesequence");
  CHECKKEEPARG(OPTDBMASKMATCH,markleftifdifferentsequence,
                              "keepleftifsamesequence");
  CHECKKEEPARG(OPTDBNOMATCH,markrightifdifferentsequence,
                            "keeprightifsamesequence");
  CHECKKEEPARG(OPTDBMASKMATCH,markrightifdifferentsequence,
                              "keeprightifsamesequence");
  if(withindexfile)
  {
    if(!ISSET(OPTCOMPLETE) && 
       !ISSET(OPTHXDROP) && 
       !ISSET(OPTEXDROP) && 
       !ISSET(OPTLEASTLENGTH))
    {
      ERROR4("at least one of the options %s, %s, %s, or %s is necessary here",
              options[OPTCOMPLETE].optname,
              options[OPTHXDROP].optname,
              options[OPTEXDROP].optname,
              options[OPTLEASTLENGTH].optname);
      return (Sint) -28;
    }
    OPTIONIMPLY(OPTSORT,OPTBEST);
  }
  if(matchcallinfo->fqfsubstringinfo.ffdefined)
  {
    if(ISSET(OPTONLINE) && !ISSET(OPTCOMPLETE))
    {
      ERROR2("substring specification for substring matches not allowed, "
             "if option %s is used but not option %s",
              options[OPTONLINE].optname,
              options[OPTCOMPLETE].optname);
      return (Sint) -29;
    }
    if(ISSET(OPTDNAVSPROT))
    {
      /* we exclude this since addsubstringoffset incorrectly changes
         the length of the query, which leads to corrupt processing
         of the query */
      ERROR1("substring specification not allowed, "
             "if option %s is used",
              options[OPTDNAVSPROT].optname);
      return (Sint) -29;
    }
  }
  if(matchcallinfo->clusterparms.dbclusterdefined && 
     matchcallinfo->clusterparms.dbclusterfilenameprefix == NULL && 
     ISSET(OPTSTRING))
  {
    ERROR2("the combination of options %s and %s is only possible "
           "if the latter option has a filenameprefix as second argument",
            options[OPTSTRING].optname,options[OPTDBCLUSTER].optname);
    return (Sint) -30;
  }
  if(ISSET(OPTNONREDUNDANT) && 
     (matchcallinfo->clusterparms.dbclusterfilenameprefix != NULL ||
      matchcallinfo->clusterparms.dbclminsize != UintConst(1) ||
      matchcallinfo->clusterparms.dbclmaxsize != DBCLMAXSIZE))
  {
    ERROR2("option %s requires that option %s is used with only two arguments",
           options[OPTNONREDUNDANT].optname,
           options[OPTDBCLUSTER].optname);
    return (Sint) -31;
  }
#ifdef DEBUG
  if(checkdoubleexclude((Uint) NUMOFOPTIONS,
                        options,
                        excludecombinetable,
                        SIZEOFEXCLUDETAB) != 0)
  {
    return (Sint) -32;
  }
#endif
  if(checkexclude(options,
                  excludecombinetable,
                  SIZEOFEXCLUDETAB) != 0)
  {
    return (Sint) -33;
  }
  if(checkenvvaronoff("VMATCHSHOWEXCLUDETAB"))
  {
    showexclude(options,
                excludecombinetable,
                SIZEOFEXCLUDETAB);
    exit(0);
  }
  if(ISSET(OPTDBMASKMATCH) || ISSET(OPTQUERYMASKMATCH))
  {
    if(!ISSET(OPTSTRING))
    {
      matchcallinfo->outinfo.showstring = DEFAULTLINEWIDTH;
    }
    if(!ISSET(OPTDESC))
    {
      ASSIGNDEFAULTSHOWDESC(&matchcallinfo->outinfo.showdesc); 
          // to show the description
    }
  }
  if(matchcallinfo->matchparam.xdropleastscore != UNDEFXDROPLEASTSCORE &&
     ISSET(OPTHXDROP))
  {
    matchcallinfo->matchparam.xdropleastscore 
      = -matchcallinfo->matchparam.xdropleastscore;
  }
  if(!(matchcallinfo->outinfo.showmode & SHOWPALINDROMIC))
  {
    matchcallinfo->outinfo.showmode |= SHOWDIRECT;
  }
  if(selfunfileindex == 0)
  {
    if(precompiledselectbundle == NULL)
    {
      matchcallinfo->outinfo.selectbundle = NULL;
      matchcallinfo->outinfo.useprecompiledselectbundle = False;
    } else
    {
      matchcallinfo->outinfo.selectbundle = precompiledselectbundle;
      matchcallinfo->outinfo.useprecompiledselectbundle = True;
    }
  } else
  {
    if(precompiledselectbundle == NULL)
    {
      ALLOCASSIGNSPACE(matchcallinfo->outinfo.selectbundle,
                       NULL,SelectBundle,
                       UintConst(1));
      matchcallinfo->outinfo.useprecompiledselectbundle = False;
      if(openSelectmatch(argv[selfunfileindex],
                         matchcallinfo->outinfo.selectbundle) != 0)
      {
        return (Sint) -34;
      } 
    } else
    {
      ERROR1("option %s not possible if precompiled selection function "
             "bundle is provided for function callvmatch",
            options[OPTSELFUN].optname);
      return (Sint) -35;
    }
  } 
  if(readqueryspeedupfromenvironment(&matchcallinfo->matchparam.queryspeedup) 
     != 0)
  {
    return (Sint) -35;
  }
  if(matchcallinfo->outinfo.outfp != NULL &&
     (!(matchcallinfo->outinfo.showstring & SHOWVMATCHXML) || 
      (matchcallinfo->outinfo.selectbundle != NULL &&
       matchcallinfo->outinfo.selectbundle->selectmatchHeader != NULL)))
  {
    if(!matchcallinfo->maskmatch.domaskmatch)
    {
      char *rawargs;

      if(savethearguments(&rawargs,False,argc,argv,argv[argc-1]) != 0)
      {
        return (Sint) -36;
      }
      if(showargumentline((matchcallinfo->outinfo.selectbundle == NULL) 
                           ? NULL
                           : matchcallinfo->outinfo.selectbundle->
                             selectmatchHeader,
                          matchcallinfo->outinfo.outfp,
                          rawargs,
                          0,
                          NULL) != 0)
      {
        return (Sint) -37;
      }
      FREESPACE(rawargs);
    }
    if(savethearguments(&matchcallinfo->mfargs,True,argc,argv,argv[argc-1]) 
       != 0)
    {
      return (Sint) -38;
    }
  }
  if(matchcallinfo->outinfo.showstring & SHOWVMATCHXML)
  {
    vmatchxmlheader(matchcallinfo->outinfo.outfp,argc,argv);
  }
  return 0;
}
