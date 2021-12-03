#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "maxfiles.h"
#include "virtualdef.h"
#include "debugdef.h"
#include "optdesc.h"
#include "errordef.h"
#include "inputdef.h"
#include "mkvaux.h"

#include "dstrdup.pr"
#include "checkonoff.pr"
#include "alphabet.pr"
#include "safescpy.pr"
#include "procopt.pr"
#include "readvirt.pr"
#include "codon.pr"
#include "detpfxlen.pr"
#include "safescpy.pr"

#include "mkvinput.pr"
#include "mkvprocess.pr"
#include "getbasename.pr"

#define USAGEERRORCODE (-99)

#define MAXTABOPTSPACE 256

#define MAKETABHELP(TABNUM,TABEXPLAIN,TABNAME)\
        maketabhelp(taboptionspace[TABNUM],TABEXPLAIN,TABNAME)

typedef enum 
{
  OPTDATABASE = 0,
  OPTQUERY,
  OPTSYMBOLMAP,
  OPTDNAFILE,
  OPTPROTEINFILE,
  OPTTRANSNUM,
  OPTINDEXNAME,
  OPTPREFIXLEN,
  OPTREV,
  OPTCPL,
  OPTTIS,
  OPTOIS,
  OPTSUF,
#ifdef WITHLCP
  OPTSTI1,
#endif
  OPTBWT,
#ifdef WITHLCP
  OPTBCK,
  OPTLCP,
  OPTSKP,
#endif
  OPTALLOUT,
  OPTMAXDEPTH,
  OPTVERBOSE,
  OPTVMATCHVERSION,
  OPTHELP,
  NUMOFOPTIONS
} Optionnumber;

/*
  The following table stores the number of those options which are
  not shown as the help message.
*/


static char *maketabhelp(char *onetaboptionspace,char *tabexplain,
                         char *tabname)
{
  Sprintfreturntype wrotelength;

  wrotelength = sprintf(onetaboptionspace,"output %s (%stab) to file",
                        tabexplain,tabname);
  if(wrotelength >= MAXTABOPTSPACE)
  {
    fprintf(stderr,"maketabhelp: not enough space for taboptionspace\n");
    exit(EXIT_FAILURE);
  }
  return onetaboptionspace;
}

static Optionnumbertype excludeforshow[] =
{
  (Optionnumbertype) OPTQUERY,
  (Optionnumbertype) OPTREV,
  (Optionnumbertype) OPTCPL,
  (Optionnumbertype) OPTTRANSNUM,
  STOPSUBTABLE
};

static Optionnumbertype excludeformkrcidx[] =
{
  (Optionnumbertype) OPTQUERY,
  (Optionnumbertype) OPTREV,
  (Optionnumbertype) OPTSYMBOLMAP,
  (Optionnumbertype) OPTDNAFILE,
  (Optionnumbertype) OPTPROTEINFILE,
  (Optionnumbertype) OPTPREFIXLEN,
  (Optionnumbertype) OPTTIS,
  (Optionnumbertype) OPTOIS,
  (Optionnumbertype) OPTSUF,
#ifdef WITHLCP
  (Optionnumbertype) OPTSTI1,
#endif
  (Optionnumbertype) OPTBWT,
#ifdef WITHLCP
  (Optionnumbertype) OPTBCK,
  (Optionnumbertype) OPTLCP,
  (Optionnumbertype) OPTSKP,
#endif
  (Optionnumbertype) OPTALLOUT,
  (Optionnumbertype) OPTTRANSNUM,
  STOPSUBTABLE
};

static Optionnumbertype excludeformkdna6idx[] =
{
  (Optionnumbertype) OPTQUERY,
  (Optionnumbertype) OPTREV,
  (Optionnumbertype) OPTDNAFILE,
  (Optionnumbertype) OPTPROTEINFILE,
  (Optionnumbertype) OPTPREFIXLEN,
  (Optionnumbertype) OPTSUF,
#ifdef WITHLCP
  (Optionnumbertype) OPTSTI1,
#endif
  (Optionnumbertype) OPTBWT,
#ifdef WITHLCP
  (Optionnumbertype) OPTBCK,
  (Optionnumbertype) OPTLCP,
  (Optionnumbertype) OPTSKP,
#endif
  (Optionnumbertype) OPTALLOUT,
  STOPSUBTABLE
};

static void addindexsuffix(char *indexname,BOOL rev,BOOL cpl)
{
  if(rev)
  {
    if(cpl)
    {
      strcat(indexname,".rcp");
    } else
    {
      strcat(indexname,".rev");
    }
  } else
  {
    if(cpl)
    {
      strcat(indexname,".cpl");
    }
  }
}


/*
  The following function initializes the structure Input and 
  some additional values appropriately.
*/

static Sint mkvparseoptions(Uint *prefixlength,
                            Uint *totalnumoffiles,
                            Uint *numofqueryfiles,
                            Fileinfo *allfiles,
                            Input *input,
                            Argctype argc,
                            const char **argv,
                            BOOL storeonfile,
                            Showverbose showverbose,
                            BOOL useformkrcidx,
                            BOOL useformkdna6idx,
                            Uint *transnum)
{
  Optionnumbertype optval;
  Scaninteger readint;
  Uint i, argnum, dbfileindex = 0, queryfileindex = 0;
  OptionDescription options[NUMOFOPTIONS];
  char taboptionspace[NUMOFOPTIONS][MAXTABOPTSPACE+1],
       transnumbuf[MAXTRANSNAMESPACE+1];

  initoptions(&options[0],(Uint) NUMOFOPTIONS);
  ADDOPTION(OPTDATABASE,"-db","specify database files (mandatory)");
  ADDOPTION(OPTQUERY,"-q","specify query files");
  ADDOPTION(OPTSYMBOLMAP,"-smap","specify file containing a symbol mapping\n"
                                 "this describes the grouping of symbols\n"
                                 "possibly set environment variable\n"
                                 "MKVTREESMAPDIR to path\n"
                                 "where these files can be found");
  ADDOPTION(OPTDNAFILE,"-dna","input is DNA sequence");
  ADDOPTION(OPTPROTEINFILE,"-protein","input is Protein sequence");
  helptransnumorganism((Uint) MAXTRANSNAMESPACE,&transnumbuf[0]);
  ADDOPTION(OPTTRANSNUM,"-transnum",&transnumbuf[0]);
  ADDOPTION(OPTPREFIXLEN,"-pl","specify prefix length for bucket sort\n"
                               "recommendation: use without argument;\n"
                               "then a reasonable prefix length is "
                               "automatically determined");
  ADDOPTION(OPTREV,"-rev","use reverse input sequence");
  ADDOPTION(OPTCPL,"-cpl","use reverse complement of the input sequence");
  ADDOPTION(OPTINDEXNAME,"-indexname","specify name for index to be generated");
  ADDOPTION(OPTTIS,"-tis",MAKETABHELP(OPTTIS,"transformed input sequences","tis"));
  ADDOPTION(OPTOIS,"-ois",MAKETABHELP(OPTOIS,"original input sequences","ois"));
  ADDOPTION(OPTSUF,"-suf",MAKETABHELP(OPTSUF,"suffix array","suf"));
#ifdef WITHLCP
  ADDOPTION(OPTSTI1,"-sti1",MAKETABHELP(OPTSTI1,"reduced inverse suffix array","sti1"));
#endif
  ADDOPTION(OPTBWT,"-bwt",MAKETABHELP(OPTBWT,"Burrows-Wheeler Transformation","bwt"));
#ifdef WITHLCP
  ADDOPTION(OPTBCK,"-bck",MAKETABHELP(OPTBCK,"bucket boundaries","bck"));
  ADDOPTION(OPTLCP,"-lcp",MAKETABHELP(OPTLCP,"longest common prefix lengths","lcp"));
  ADDOPTION(OPTSKP,"-skp",MAKETABHELP(OPTSKP,"skip values","skp"));
#endif
  ADDOPTION(OPTALLOUT,"-allout","output all index tables to files");
  ADDOPTION(OPTMAXDEPTH,"-maxdepth","restrict the sorting to prefixes of the given length");
  ADDOPTION(OPTVERBOSE,"-v","verbose mode");
  ADDOPTVMATCHVERSION;
  ADDOPTION(OPTHELP,"-help","this option");

  if(argc == 1)
  {
    USAGEOUT;
    return (Sint) USAGEERRORCODE;
  }
  input->storeonfile = storeonfile;
  input->showverbose = NULL;
  input->maxdepth.defined = False;
  input->maxdepth.uintvalue = 0;
  input->inputalpha.symbolmapfile = NULL;
  input->inputalpha.proteinfile = False;
  if(useformkrcidx || useformkdna6idx)
  {
    input->inputalpha.dnafile = True;
  } else
  {
    input->inputalpha.dnafile = False;
  }
  input->demand = 0;  // no demand
  input->rev = False;  // forward sequence, so no reverse
  input->cpl = False;  // forward sequence, so no complement
  input->rcm = False;  // forward sequence, so no reverse complement
  input->transnum = 0; // produce index in 6 reading frames;
                       // if not 0, then it stores transnum

  *prefixlength = 0;
  *totalnumoffiles = 0;
  *numofqueryfiles = 0;
  for(argnum = UintConst(1); 
      argnum < (Uint) argc && ISOPTION(argv[argnum]); argnum++)
  {
    if(strcmp("-prefix",argv[argnum]) == 0)
    {
      optval = (Optionnumbertype) OPTINDEXNAME;
      fprintf(stderr,"%s: option -prefix has been renamed to %s",
              argv[0],options[OPTINDEXNAME].optname);
      exit(EXIT_FAILURE);
    }
    optval = procoption(options,(Uint) NUMOFOPTIONS,argv[argnum]);
    if(optval < 0)
    {
      return (Sint) -1;
    }
    switch(optval)
    {
      case OPTHELP:
        if(checkenvvaronoff("VMATCHSHOWOPTIONINLATEX"))
        {
          showoptionsinlatex(stdout,
                             argv[0],
                             options,
                             NULL,
                             (Uint) NUMOFOPTIONS);
        } else
        {
          Optionnumbertype *excludetableptr;

          if(useformkrcidx)
          {
            excludetableptr = excludeformkrcidx;
          } else
          {
            if(useformkdna6idx)
            {
              excludetableptr = excludeformkdna6idx;
            } else
            {
              excludetableptr = excludeforshow;
            }
          }
          showoptionswithoutexclude(stdout,
                                    argv[0],
                                    options,
                                    excludetableptr,
                                    (Uint) NUMOFOPTIONS);
#ifdef SUFFIXPTR
          printf("# compiled with -DSUFFIXPTR\n");
#endif
        }
        return (Sint) 1;
      case OPTVMATCHVERSION:
        return (Sint) 1;
      case OPTSYMBOLMAP:
        argnum++;
        CHECKMISSINGARGUMENT;
        if(!useformkrcidx && !useformkdna6idx)
        {
          ASSIGNDYNAMICSTRDUP(input->inputalpha.symbolmapfile,argv[argnum]);
        }
        break;
      case OPTDNAFILE:
        if(!useformkrcidx && !useformkdna6idx)
        {
          input->inputalpha.dnafile = True;
        }
        break;
      case OPTPROTEINFILE:
        if(!useformkrcidx && !useformkdna6idx)
        {
          input->inputalpha.proteinfile = True;
        }
        break;
      case OPTTRANSNUM:
        if(!useformkdna6idx)
        {
          ERROR1("illegal option \"%s\"",options[OPTTRANSNUM].optname);
          return -2;
        }
        READINTGENERIC(options[(Uint) optval].optname,
                       readint,argc-1,<,"non-negative");
        *transnum = (Uint) readint;
        if(checktransnum(*transnum) != 0)
        {
          return (Sint) -3;
        }
        break;
      case OPTPREFIXLEN:
        if(MOREARGOPTSWITHLAST(argnum))
        {
          READINTGENERIC(options[(Uint) optval].optname,
                         readint,argc,<=,"positive");
          *prefixlength = (Uint) readint;
          if(*prefixlength == AUTOPREFIXLENGTH)
          {
            ERROR2("Illegal value %lu as argument to option %s",
                   (Showuint) *prefixlength,options[OPTPREFIXLEN].optname);
            return (Sint) -2;
          }
        } else
        {
          *prefixlength = AUTOPREFIXLENGTH;
        }
        break;
      case OPTREV:
        input->rev = True;
        break;
      case OPTCPL:
        input->cpl = True;
        break;
      case OPTTIS:
        input->demand |= TISTAB;
        break;
      case OPTOIS:
        input->demand |= OISTAB;
        break;
      case OPTSUF:
        input->demand |= SUFTAB;
        break;
#ifdef WITHLCP
      case OPTSTI1:
        input->demand |= (STI1TAB | LCPTAB | BCKTAB);
        break;
#endif
      case OPTBWT:
        input->demand |= BWTTAB;
        break;
#ifdef WITHLCP
      case OPTBCK:
        input->demand |= BCKTAB;
        break;
      case OPTLCP:
        input->demand |= LCPTAB;
        break;
      case OPTSKP:
        input->demand |= SKPTAB;
        break;
#endif
      case OPTALLOUT:
        break;
      case OPTINDEXNAME:
        argnum++;
        CHECKMISSINGARGUMENT;
        if(safestringcopy(&input->indexname[0],argv[argnum],PATH_MAX) != 0)
        {
          return (Sint) -3;
        }
        break;
      case OPTVERBOSE:
        input->showverbose = showverbose;
        break;
      case OPTMAXDEPTH:
        if(argnum == (Uint) (argc-1) || ISOPTION(argv[argnum+1])) /* No option */
        {
          input->maxdepth.defined = True;
          input->maxdepth.uintvalue = 0;
        } else
        {
          argnum++;
          if(sscanf(argv[argnum],"%ld",&readint) != 1 || readint <= 0)
          {
            ERROR2("argument \"%s\" of option %s must be positive number",
                    argv[argnum],options[OPTMAXDEPTH].optname);
            return (Sint) -1;\
          }
          input->maxdepth.defined = True;
          input->maxdepth.uintvalue = (Uint) readint;
        }
        break;
      case OPTDATABASE:
        argnum++;
        dbfileindex = argnum;
        while(argnum < (Uint) argc)
        {
          if(ISOPTION(argv[argnum]))
          {
            argnum--;
            break;
          }
          (*totalnumoffiles)++;
          argnum++;
        }
        break;
      case OPTQUERY:
        argnum++;
        queryfileindex = argnum;
        while(argnum < (Uint) argc)
        {
          if(ISOPTION(argv[argnum]))
          {
            argnum--;
            break;
          }
          (*totalnumoffiles)++;
          (*numofqueryfiles)++;
          argnum++;
        }
        break;
    }
  }
  if(argc > (Argctype) argnum)
  {
    ERROR1("illegal additional argument \"%s\"",argv[argc-1]);
    return (Sint) -4;
  }
  if(useformkrcidx || useformkdna6idx)
  {
    if(checkforsingleexcludeoption(&options[0],
                                   useformkrcidx ? excludeformkrcidx
                                                 : excludeformkdna6idx) != 0)
    {
      return (Sint) -5;
    }
    input->demand = TISTAB | OISTAB;
  }
  OPTIONIMPLY(OPTALLOUT,OPTPREFIXLEN);
#ifdef WITHLCP
  OPTIONIMPLY(OPTSTI1,OPTPREFIXLEN);
  OPTIONIMPLY(OPTBCK,OPTPREFIXLEN);
  OPTIONIMPLY(OPTSKP,OPTLCP);
  OPTIONIMPLY(OPTSKP,OPTSUF);
#endif
  OPTIONEXCLUDE(OPTDNAFILE,OPTSYMBOLMAP);
  OPTIONEXCLUDE(OPTPROTEINFILE,OPTSYMBOLMAP);
  OPTIONEXCLUDE(OPTDNAFILE,OPTPROTEINFILE);
  if(ISSET(OPTALLOUT))
  {
    if(SOMETHINGOUT(input))
    {
      ERROR1("option %s cannot be combined with any other output option",
              options[OPTALLOUT].optname);            
      return (Sint) -6;
    } 
    input->demand = TISTAB | OISTAB | SUFTAB | STI1TAB | 
                    BWTTAB | BCKTAB | LCPTAB | SKPTAB;
  }
  if(*totalnumoffiles == *numofqueryfiles)
  {
    ERROR1("option %s is mandatory",options[OPTDATABASE].optname);
    return (Sint) -7;
  }
  if(*totalnumoffiles >= (Uint) MAXNUMBEROFFILES)
  {
    ERROR1("too many input files, maximum is %lu",
            (Showuint) MAXNUMBEROFFILES);
    return (Sint) -8;
  }
  for(i=0; i < *totalnumoffiles - *numofqueryfiles; i++)
  {
    ASSIGNDYNAMICSTRDUP(allfiles[i].filenamebuf,argv[dbfileindex+i]);
  }
  if(*numofqueryfiles > 0)
  {
    char *tmpfilename;
    for(i=0; i< *numofqueryfiles; i++)
    {
      ASSIGNDYNAMICSTRDUP(tmpfilename,argv[queryfileindex+i]);
      allfiles[*totalnumoffiles - *numofqueryfiles + i].filenamebuf 
        = tmpfilename;
    }
  }
  if(ISSET(OPTINDEXNAME))
  {
    if(!useformkrcidx && !useformkdna6idx)
    {
      if(!input->storeonfile || !SOMETHINGOUT(input))
      {
        ERROR1("option %s only allowed in connection with output option",
               options[OPTINDEXNAME].optname);            
        return (Sint) -9;
      }
    }
    addindexsuffix(&input->indexname[0],input->rev,input->cpl);
  } else
  {
    if(input->storeonfile && SOMETHINGOUT(input))
    {
      char *basenameptr;

      if(*totalnumoffiles > UintConst(1))
      {
        ERROR1("option %s required for multiple input files",
               options[OPTINDEXNAME].optname);            
        return (Sint) -10;
      }
      basenameptr = vm_getbasename(allfiles[0].filenamebuf);
      if(safestringcopy(&input->indexname[0],basenameptr,PATH_MAX) != 0)
      {
        free(basenameptr);
        return (Sint) -11;
      }
      free(basenameptr);
      addindexsuffix(&input->indexname[0],input->rev,input->cpl);
    }
  }
  return 0;
}

static Sint mkvalpha(Inputalpha *inputalpha,BOOL *specialsymbols,
                     Alphabet *alpha)
{
  if(inputalpha->symbolmapfile != NULL ||
     inputalpha->dnafile || inputalpha->proteinfile)
  {
    *specialsymbols = True;
    if(determineAlphabet(alpha,inputalpha) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    *specialsymbols = False;
  }
  return 0;
}

#ifdef DEBUG
static void showargumentlist(Argctype argc,const char **argv)
{
  Argctype i;

  printf("callmkvtree(");
  for(i=0; i<argc; i++)
  {
    printf("%s",argv[i]);
    if(i<argc-1)
    {
      printf(" ");
    }
  }
  printf(")\n");
}
#endif

Sint callmkvtreegeneric(BOOL useformkrcidx,
                        BOOL useformkdna6idx,
                        Argctype argc,
                        const char **argv,
                        BOOL storeonfile,
                        Virtualtree *virtualtree,
                        BOOL storedesc,
                        Showverbose showverbose,
                        Uint *transnum)
{
  Input input;
  Sint pret;

#ifdef DEBUG
  showargumentlist(argc,argv);
#endif
  pret = mkvparseoptions(&virtualtree->prefixlength,
                         &virtualtree->multiseq.totalnumoffiles,
                         &virtualtree->multiseq.numofqueryfiles,
                         &virtualtree->multiseq.allfiles[0],
                         &input,
                         argc,
                         argv,
                         storeonfile,
                         showverbose,
                         useformkrcidx,
                         useformkdna6idx,
                         transnum);
  if(pret == (Sint) 1)
  {
    return (Sint) 1;
  }
  if(pret == (Sint) USAGEERRORCODE)
  {
    return (Sint) -1;
  }
  if(pret < 0)
  {
    return (Sint) -2;
  }
  if(mkvalpha(&input.inputalpha,
              &virtualtree->specialsymbols,
              &virtualtree->alpha) != 0)
  {
    return (Sint) -3;
  }
  if(mkvtreeinput(virtualtree->specialsymbols,
                  (input.demand & OISTAB) ? True : False,
                  input.rev,
                  input.cpl,
                  (virtualtree->prefixlength == AUTOPREFIXLENGTH) 
                                                    ? True : False,
                  (virtualtree->prefixlength > 0) ? True : False,
                  &virtualtree->multiseq,
                  storedesc,
                  &virtualtree->alpha,
                  input.showverbose) != 0)
  {
    return (Sint) -4;
  }
  DEBUGCODE(2,showsymbolmap(&virtualtree->alpha));
  input.numofchars = virtualtree->alpha.mapsize - 
                      (virtualtree->specialsymbols ? 1 : 0);
  if(virtualtree->prefixlength == AUTOPREFIXLENGTH)
  {
    virtualtree->prefixlength 
      = vm_recommendedprefixlength(input.numofchars,
                                virtualtree->multiseq.totallength,
                                SIZEOFBCKENTRY);
  }
  if(input.maxdepth.defined)
  {
    if(input.maxdepth.uintvalue == 0)
    {
      if(virtualtree->prefixlength == 0)
      {
        ERROR0("if option maxdepth is used without an argument, "
               "then option prefixlength is required");
        return (Sint) -1;
      }
      input.maxdepth.uintvalue = virtualtree->prefixlength;
    } else
    {
      if(input.maxdepth.uintvalue < virtualtree->prefixlength)
      {
        ERROR2("maxdepth = %lu < %lu = prefixlength",
               (Showuint) input.maxdepth.uintvalue,
               (Showuint) virtualtree->prefixlength);
        return (Sint) -7;
      }
    }
  }
  if(mkvtreeprocess(&input,virtualtree) != 0)
  {
    return (Sint) -6;
  }
  FREESPACE(input.inputalpha.symbolmapfile);
  return 0;
}

Sint callmkvtree(Argctype argc,
                 const char **argv,
                 BOOL storeonfile,
                 Virtualtree *virtualtree,
                 BOOL storedesc,
                 Showverbose showverbose)
{
  Uint transnum;

  return callmkvtreegeneric(False,
                            False,
                            argc,
                            argv,
                            storeonfile,
                            virtualtree,
                            storedesc,
                            showverbose,
                            &transnum);
}

Sint completevirtualtree(Virtualtree *virtualtree,
                         Uint demand,
                         Showverbose showverbose)
{
  Input input;

/*
  On the user side: Initialize 

  virtualtree->prefixlength
  virtualtree->specialsymbols
  virtualtree->multiseq
  virtualtree->alpha
*/

  input.storeonfile = False;
  input.demand = demand;
  input.showverbose = showverbose;
  strcpy(&input.indexname[0],"none");
  input.maxdepth.defined = False;
  input.maxdepth.uintvalue = 0;
  input.numofchars = virtualtree->alpha.mapsize - 
                     (virtualtree->specialsymbols ? 1 : 0);

  if(mkvtreeprocess(&input,virtualtree) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}
