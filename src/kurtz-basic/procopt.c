//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "optdesc.h"
#include "errordef.h"
#include "debugdef.h"
#include "spacedef.h"

#include "dstrdup.pr"
#include "getbasename.pr"

//}

/*
  This file implements functions for processing options according to
  some option declarations.
*/

/*EE
  The following function initializes the table \texttt{options} storeing the
  information for all options. \texttt{numofoptions} is the number of options.
*/

void initoptions(OptionDescription *options,Uint numofoptions)
{
  Uint i;

  DEBUG1(3,"option[0...%lu] is undeclared\n",(Showuint) (numofoptions-1));
  for(i=0; i<numofoptions; i++)
  {
    options[i].declared = False;
  }
}

/*EE
  The following function adds an option to table 
  \texttt{options}. \texttt{numofoptions} is the number of options.
  \texttt{optnum} is the number of the option declared. \texttt{optname}
  is the option string to be used when calling the program. Finally,
  \texttt{optdesc} is the option description, i.e.\ a short explanation of
  the option. The function returns 0 if everything went fine. In 
  case of an error, a negative error code is returned. The
  function should be called via the macro \texttt{ADDOPTION} as defined
  in \texttt{optdesc.h}.
*/

Sint addoption(OptionDescription *options,Uint numofoptions,
               Uint optnum,char *optname,char *optdesc)
{
  Uint i;

  DEBUG2(3,"add option number %ld: %s\n",(Showsint) optnum,optname);
  if(optnum>=numofoptions)
  {
    ERROR2("option number %ld out of range [0,%lu]",
            (Showsint) optnum,
            (Showuint) (numofoptions-1));
    return (Sint) -1;
  }
  if(optnum > 0 && options[optnum].declared)
  {
    ERROR1("option %lu already declared",(Showuint) optnum);
    return (Sint) -2;
  }
  options[optnum].optname = optname;
  options[optnum].description = optdesc;
  options[optnum].optval = optnum;
  options[optnum].isalreadyset = False;
  options[optnum].declared = True;
  DEBUG1(3,"option %ld is declared\n",(Showsint) optnum);
  if(optnum == numofoptions-1)
  {
    for(i=0; i<numofoptions; i++)
    {
      if(!options[i].declared)
      {
        ERROR1("option %lu not declared",(Showuint) i);
        return (Sint) -3;
      }
    }
  }
  return 0;
}

/*EE
  The following function looks up a string \texttt{optstring} in a table of
  option descriptions. If it has found an option description whose
  \texttt{optname} is identical to \texttt{optstring}, it is checked if 
  this option has already been set. If so, then an error message is 
  thrown and the function returns with an error code.
  If the option was not set before, then the index of the option is returned. 
  If optstring is not found in the table, then this is an error and 
  the program throws an error message and returns with an exit code.
*/ 

Optionnumbertype procoption(OptionDescription *opt,
                            Uint numofopt,
                            const char *optstring)
{
  Uint i;

  for(i=0; i<numofopt; i++)
  {
    if(strcmp(optstring,opt[i].optname) == 0)
    {
      if(opt[i].isalreadyset)
      {
        ERROR1("option \"%s\" already set",optstring);
        return -1;
      }
      opt[i].isalreadyset = True;
      return (Optionnumbertype) i;
    }
  }
  ERROR1("illegal option \"%s\"",optstring);
  return -2;
}

/*
  The following function checks if the number \texttt{i} occurrs in
  \texttt{list}.
*/

static BOOL occursinlist(Uint i,Optionnumbertype *list)
{
  Uint j;

  for(j=0; list[j] != STOPSUBTABLE; j++)
  {
    if(((Optionnumbertype) i) == list[j])
    {
      return True;
    }
  }
  return False;
}

/*
  The following function computes the maximal length of all options
  in the option description \texttt{opt}. If \texttt{excludetab} is not
  \texttt{NULL}, then the option numbers in \texttt{excludetab} are
  ignored.
*/

static Uint getoptindent(Optionnumbertype *excludetab,
                         OptionDescription *opt,
                         Uint numofopt)
{
  Uint i, slen, indentlevel = 0;

  for(i=0; i<numofopt; i++)
  {
    if(excludetab == NULL || !occursinlist(i,excludetab))
    {
      slen = (Uint) strlen(opt[i].optname);
      if(indentlevel < slen)
      {
        indentlevel = slen;
      }
    }
  }
  return indentlevel + 2;
}

/*
  The following function computes the maximal length of all options
  in the option description \texttt{opt}. Only the options given by option
  numbers in \texttt{devoptiontab} are considered.
*/

static Uint getdevoptindent(Optionnumbertype *devoptiontab,
                            OptionDescription *opt,
                            Uint numofopt)
{
  Uint i, slen, indentlevel = 0;

  for(i=0; i<numofopt; i++)
  {
    if(occursinlist(i,devoptiontab))
    {
      slen = (Uint) strlen(opt[i].optname);
      if(indentlevel < slen)
      {
        indentlevel = slen;
      }
    }
  }
  return indentlevel + 2;
}

/*
  The following function shows the option description.
  An newline symbol in the option description is output with
  \texttt{indentlevel} many blanks.
*/

static void showoptdesc(FILE *fp,Uint indentlevel,const char *desc)
{
  Uint j;

  for(j=0; desc[j] != '\0'; j++)
  {
    if(desc[j] == '\n')
    {
      fprintf(fp,"\n%-*s",(Fieldwidthtype) indentlevel," ");
    } else
    {
      (void) putc(desc[j],fp);
    }
  }
  (void) putc('\n',fp);
}

static void showoptdesclatex(FILE *fp,char *desc)
{
  Uint j;

  for(j=0; desc[j] != '\0' && desc[j] != '\n'; j++)
  {
    if(desc[j] == '>' || desc[j] == '%')
    {
      fprintf(fp,"$\\symbol{%lu}$",(Showuint) desc[j]);
    } else
    {
      (void) putc(desc[j],fp);
    }
  }
  fprintf(fp,"\n");
}

/*
  The following function shows a single option
*/

static void showsingleoption(FILE *fp,char *optname)
{
  Uint offset;

  if(ISOPTION(optname))
  {
    offset = UintConst(1);
  } else
  {
    offset = 0;
  }
  fprintf(fp,"\\Showoption{%s}",optname+offset);
}

static void showoptionsgenericwithoutdevoptions(BOOL latexout,
                                                FILE *outfp,
                                                const char *program,
                                                OptionDescription *opt,
                                                OptionGroup *optiongroups,
                                                Optionnumbertype *devoptiontab,
                                                Uint numofopt)
{
  Uint i, indentlevel, nextgroup = 0;
  FILE *fp;

  if(outfp == NULL)
  {
    fp = stdout;
  } else
  {
    fp = outfp;
  }
  indentlevel = getoptindent(NULL,opt,numofopt);
  if(latexout)
  {
    printf("\\begin{tabular}{ll}\\hline\n");
  }
  for(i=0; i<numofopt; i++)
  {
    if(i != opt[i].optval)
    {
      fprintf(stderr,"%s: optvalue %lu for option \"%s\" must be %lu\n",
                     program,
                     (Showuint) opt[i].optval,
                     opt[i].optname,
                     (Showuint) i);
      exit(EXIT_FAILURE);
    }

    if(devoptiontab == NULL ||
       (devoptiontab != NULL && !occursinlist(i,devoptiontab)))
    {
      if(latexout)
      {
        if(optiongroups != NULL && optiongroups[nextgroup].firstofgroup == i)
        {
          fprintf(fp,"\\Showoptiongroup{%s}\n",
                  optiongroups[nextgroup].groupdescription);
          nextgroup++;
        }
        showsingleoption(fp,opt[i].optname);
        printf("& ");
        showoptdesclatex(fp,opt[i].description);
      } else
      {
        fprintf(fp,"%-*s",(Fieldwidthtype) indentlevel,opt[i].optname);
        showoptdesc(fp,indentlevel,opt[i].description);
      }

      if(latexout)
      {
        fprintf(fp,"\\\\\n");
      }
    }
  }
  if(latexout)
  {
    fprintf(fp,"\\hline\n");
    printf("\\end{tabular}\n");
  }
}

static void showoptionsgeneric(BOOL latexout,FILE *outfp,const char *program,
                               OptionDescription *opt,
                               OptionGroup *optiongroups,
                               Uint numofopt)
{
  showoptionsgenericwithoutdevoptions(latexout,outfp,program,opt,optiongroups,
                                      NULL,numofopt);
}

/*EE
  The following function shows the options as specified by the
  option description \texttt{opt}. For each option, it checks
  if the component \texttt{optval} is identical to the index of the option.
  The option description is output to a file pointer \texttt{outfp} with
  the indentation level delivered by \texttt{getoptindent}.
  \texttt{program} is the program which calls \texttt{showoptions}.
*/

void showoptions(FILE *outfp,const char *program,OptionDescription *opt,
                 Uint numofopt)
{
  showoptionsgeneric(False,outfp,program,opt,NULL,numofopt);
}

void showoptionswithoutdevoptions(FILE *outfp,const char *program,
                                  OptionDescription *opt, 
                                  Optionnumbertype *devoptiontab,
                                  Uint numofopt)
{
  showoptionsgenericwithoutdevoptions(False,outfp,program,opt,NULL,devoptiontab,
                                      numofopt);
}

/*EE
  The following function shows the options as specified by the
  option description \texttt{opt}. The output is in \LaTeX.
*/

void showoptionsinlatex(FILE *outfp,const char *program,
                        OptionDescription *opt,
                        OptionGroup *optiongroups,Uint numofopt)
{
  showoptionsgeneric(True,outfp,program,opt,optiongroups,numofopt);
}

void showoptionsinlatexwithoutdevoptions(FILE *outfp,const char *program,
                                         OptionDescription *opt,
                                         OptionGroup *optiongroups,
                                         Optionnumbertype *devoptiontab,
                                         Uint numofopt)
{
  showoptionsgenericwithoutdevoptions(True,outfp,program,opt,optiongroups,
                                      devoptiontab,numofopt);
}

void showoptionswithoutexcludeandwithoutdevoptions(FILE *outfp,
                                                   const char *program,
                                                   OptionDescription *opt,
                                                   Optionnumbertype
                                                   *excludetab,
                                                   Optionnumbertype 
                                                   *devoptiontab,
                                                   Uint numofopt)
{
  Uint i, indentlevel;
  FILE *fp;

  if(outfp == NULL)
  {
    fp = stdout;
  } else
  {
    fp = outfp;
  }
  indentlevel = getoptindent(excludetab,opt,numofopt);
  for(i=0; i<numofopt; i++)
  {
    if(!occursinlist(i,excludetab) &&
       (devoptiontab == NULL ||
        (devoptiontab != NULL && !occursinlist(i,devoptiontab))))
    {
      if(i != opt[i].optval)
      {
        fprintf(stderr,"%s: optvalue %lu for option \"%s\" must be %lu\n",
                       program,
                       (Showuint) opt[i].optval,
                       opt[i].optname,
                       (Showuint) i);
        exit(EXIT_FAILURE);
      }
      fprintf(fp,"%-*s",(Fieldwidthtype) indentlevel,opt[i].optname);
      showoptdesc(fp,indentlevel,opt[i].description);
    }
  }
}

/*EE
  The following function shows the options as specified by the
  option description \texttt{opt}, except for the
  options whose numbers are specified in list \texttt{exludetab}.
  The option description is output to file pointer \texttt{outfp}.
  \texttt{program} is the program which calls \texttt{showoptions}.
*/

void showoptionswithoutexclude(FILE *outfp,
                               const char *program,
                               OptionDescription *opt,
                               Optionnumbertype *excludetab,
                               Uint numofopt)
{
  showoptionswithoutexcludeandwithoutdevoptions(outfp,program,opt,excludetab,
                                                NULL,numofopt);
}

void showdevoptions(FILE *outfp,
                    const char *program,
                    OptionDescription *opt,
                    Optionnumbertype *devoptiontab,
                    Uint numofopt)
{
  Uint i, indentlevel;
  FILE *fp;

  if(outfp == NULL)
  {
    fp = stdout;
  } else
  {
    fp = outfp;
  }
  indentlevel = getdevoptindent(devoptiontab,opt,numofopt);
  for(i=0; i<numofopt; i++)
  {
    if(occursinlist(i,devoptiontab))
    {
      if(i != opt[i].optval)
      {
        fprintf(stderr,"%s: optvalue %lu for option \"%s\" must be %lu\n",
                       program,
                       (Showuint) opt[i].optval,
                       opt[i].optname,
                       (Showuint) i);
        exit(EXIT_FAILURE);
      }
      fprintf(fp,"%-*s",(Fieldwidthtype) indentlevel,opt[i].optname);
      showoptdesc(fp,indentlevel,opt[i].description);
    }
  }
}

/*EE
  The following function checks for all pairs of options
  if they have already been declared to be excluding each other.
*/

Sint checkdoubleexclude(Uint numofopts,
                        OptionDescription *opt,
                        Optionnumbertype *excludetab,
                        Uint len)
{
  Uint i, j;
  Optionnumbertype indi, indj;
  BOOL *excludepairs;

  ALLOCASSIGNSPACE(excludepairs,NULL,BOOL,numofopts*numofopts);
  for(i=0; i < numofopts * numofopts; i++)
  {
    excludepairs[i] = False;
  }
  for(i=0; i < len; i = j+1)
  {
    indi = excludetab[i];
    for(j=i+1; excludetab[j] != STOPSUBTABLE; j++)
    {
      indj = excludetab[j];
      DEBUG2(2,"Exclude %s %s\n",opt[indi].optname,opt[indj].optname);
      if(excludepairs[indi*numofopts+indj])
      {
        ERROR2("option %s and option %s already specified as excluded",
               opt[indi].optname,opt[indj].optname);
        return (Sint) -1;
      }
      excludepairs[indi*numofopts+indj] = True;
      excludepairs[indj*numofopts+indi] = True;
    }
  }
  FREESPACE(excludepairs);
  return 0;
}

/*EE
  To specify pairs of options which exclude each other we
  use an exclude table. An exclude table is just an array of 
  signed integers. Each block of this table consists of a maximal 
  subsequence not beginning with the symbol \texttt{STOPSUBTABLE}, but ending 
  with it. Let \(i_{0}, i_{1}, \ldots, l_{r}\) be a block. Each number is 
  an option number.
  For \(j\in[1,r]\), option number \(i_{0}\) and option number 
  \(i_{j}\) exclude each other.

  The following function takes a table of option descriptions and an
  exclude table of length \texttt{len}. It checks if
  two options which exclude each other are set. If there are no
  such options, then the function returns -1 and throws an error message.
  The function must be called after parsing the options.
*/

Sint checkexclude(OptionDescription *opt,
                  Optionnumbertype *excludetab,
                  Uint len)
{
  Uint i, j;

  for(i=0; i < len; i = j+1)
  {
    for(j=i+1; excludetab[j] != STOPSUBTABLE; j++)
    {
      DEBUG2(2,"Exclude %s %s\n",opt[(Uint) excludetab[i]].optname,
                                 opt[(Uint) excludetab[j]].optname);
      if(opt[(Uint) excludetab[i]].isalreadyset && 
         opt[(Uint) excludetab[j]].isalreadyset)
      {
        ERROR2("option %s and option %s exclude each other",
               opt[(Uint) excludetab[i]].optname,
               opt[(Uint) excludetab[j]].optname);
        return (Sint) -1;
      }
    }
  }
  return 0;
}

/*EE
  The following function checks if any of the options specified in
  \texttt{excludetab} is set. If this is the case, then 
  an error message is thrown and the function returns \texttt{-1}.
  Otherwise, the function returns 0.
*/

Sint checkforsingleexcludeoption(OptionDescription *options,
                                 Optionnumbertype *excludetab)
{
  Uint i;

  for(i=0; excludetab[i] != STOPSUBTABLE; i++)
  {
    if(ISSET(excludetab[i]))
    {
      ERROR1("illegal option %s",options[excludetab[i]].optname);
      return (Sint) -1;
    }
  }
  return 0;
}

/*EE
  The following function takes a table of option descriptions and an
  exclude table of length \texttt{len}. It prints the exclude table
  in \LaTeX\ format. We use this function to produce a table in
  the manual of a program showing which options exclude each other.
*/

void showexclude(OptionDescription *opt,
                 Optionnumbertype *excludetab,
                 Uint len)
{
  Uint sumlength, countoutput = 0, i, j;

  printf("\\begin{tabular}{|l|l|}\\hline\n");
  printf("option&cannot be combined with option\\\\\\hline\n");
  for(i=0; i < len; i = j+1)
  {
    if(i>0)
    {
      printf("\\\\\\hline\n");
    }
    showsingleoption(stdout,opt[(Uint) excludetab[i]].optname);
    printf("& ");

    sumlength = 0;
    for(j=i+1; excludetab[j] != STOPSUBTABLE; j++)
    {
      showsingleoption(stdout,opt[(Uint) excludetab[j]].optname);
      countoutput++;
      printf(" ");
      sumlength += strlen(opt[(Uint) excludetab[j]].optname);
      if(sumlength > UintConst(40) && excludetab[j+1] != STOPSUBTABLE)
      {
        printf("\\\\\n&");
        sumlength = 0;
      }
    }
  }
  printf("\\\\\\hline");
  printf("\n\\end{tabular}\n");
  printf("%% number of exclude combinations: %lu\n",(Showuint) countoutput);
}

Sint extractoption(const char *optname,const char **argv,Argctype argc)
{
  Argctype i;

  for(i=1; i<argc; i++)
  {
    if(strcmp(argv[i],optname) == 0)
    {
      return (Sint) i;
    }
  }
  return (Sint) -1;
}

Sint extractoptionwithpossibleargument(BOOL withlast,
                                       BOOL *hasargument,
                                       const char *optstring,
                                       const char **argv,
                                       Argctype argc)
{
  Sint argnum; 
  Argctype firstignore;

  argnum = extractoption(optstring,argv,argc);
  if(argnum <= 0)
  {
    ERROR1("cannot find option \"%s\"",optstring);
    return (Sint) -1;
  }
  if(withlast)
  {
    firstignore = argc;
  } else
  {
    firstignore = argc-1;
  }
  if(MOREARGOPTSGENERIC(argnum,firstignore))
  {
    *hasargument = True;
  } else
  {
    *hasargument = False;
  }
  return argnum;
}

Sint extractindexname(char **indexname,const char **argv,Argctype argc)
{
  Sint argnum;
  BOOL hasargument;
  size_t i;
  char *optionnames[] = {"-indexname","-db"};

  for(i=0; i<sizeof(optionnames)/sizeof(optionnames[0]); i++)
  {
    argnum = extractoptionwithpossibleargument(True,
                                               &hasargument,
                                               optionnames[i],
                                               argv,
                                               argc);
    if(argnum > 0)
    {
      char *basenameptr;

      if(!hasargument)
      {
        ERROR1("missing argument to option %s",optionnames[i]);
        return (Sint) -2;
      }
      basenameptr = vm_getbasename(argv[argnum+1]);
      if(i==0)
      {
        ASSIGNDYNAMICSTRDUP(*indexname,argv[argnum+1]);
      } else
      {
        ASSIGNDYNAMICSTRDUP(*indexname,basenameptr);
      }
      free(basenameptr);
      return 0;
    }
  }
  ERROR0("cannot find indexname");
  return (Sint) -3;
}

Sint optionaddbitmask(Optionargmodedesc *modedesc,
                      Uint numberofentries,
                      Uint *mode,
                      const char *optname,
                      const char *optionargument)
{
  Uint modecount;
  
  for(modecount=0; modecount < numberofentries; modecount++)
  {
    if(strcmp(optionargument,modedesc[modecount].name) == 0)
    {
      if(*mode & modedesc[modecount].bitmask)
      {
        ERROR2("argument \"%s\" to option %s already specified",
                modedesc[modecount].name,optname);
        return (Sint) -1;
      }
      *mode |= modedesc[modecount].bitmask;
      return 0;
    }
  }
  ERROR2("illegal argument \"%s\" to option %s",optionargument,optname);
  return (Sint) -2;
}
