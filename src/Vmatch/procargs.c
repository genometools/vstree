
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "debugdef.h"
#include "spacedef.h"
#include "select.h"
#include "outinfo.h"
#include "optdesc.h"
#include "file.h"

#include "checkonoff.pr"
#include "splitargs.pr"

#define SKIPSOMEARGS(IND)\
        while(basic && (strcmp(argv[IND] ,"-s") == 0 ||\
                        strcmp(argv[IND], "-sort") == 0 ||\
                        strcmp(argv[IND], "-selfun") == 0 ||\
                        strcmp(argv[IND], "-best") == 0 ||\
                        strcmp(argv[IND], "-dbcluster") == 0 ||\
                        strcmp(argv[IND], "-qspeedup") == 0 ||\
                        strcmp(argv[IND], "-pp") == 0 ||\
                        strcmp(argv[IND], "-nonredundant") == 0))\
        {\
          IND++;\
          while((IND) < (Sint) (argc - 1) && !ISOPTION(argv[IND]))\
          {\
            IND++;\
          }\
        }

Sint savethearguments(char **args,BOOL basic,Argctype argc,
                      const char * const*argv,const char *indexormatchfile)
{
  Sint i, j, l, alen, retval;

  DEBUG2(2,"argc=%ld,indexormatchfile=%s\n",
           (Showsint) argc,indexormatchfile);
  alen = (Sint) (argc + strlen(indexormatchfile));
  for(i=(Sint) 1; i<(Sint) (argc-1); i++)
  {
    SKIPSOMEARGS(i);
    alen += strlen(argv[i]);
  }
  ALLOCASSIGNSPACE(*args,NULL,char,(Uint) alen + PATH_MAX);
  for(j=0, i=(Sint) 1; i<(Sint) (argc-1); i++)
  {
    DEBUG2(2,"check argv[%ld]=%s\n",(Showsint) i,argv[i]);
    SKIPSOMEARGS(i);
    if(i >= (Sint) (argc-1))
    {
      break;
    }
    DEBUG2(2,"argv[%ld]=%s\n",(Showsint) i,argv[i]);
    for(l=0; argv[i][l] != '\0'; l++)
    {
      (*args)[j++] = argv[i][l];
    }
    (*args)[j++] = ' ';
  }
  retval = checkenvvaronoff("VMATCHRELATIVEINDEXPATH");
  if(retval < 0)
  {
    return (Sint) -1;
  }
  if(!retval)
  {
    if(indexormatchfile[0] != PATH_SEPARATOR)
    {
      char directoryname[PATH_MAX+1];
      if(getcwd(&directoryname[0],PATH_MAX) == NULL)
      {
        j += sprintf((*args)+j,"."PATH_SEPARATOR_STR);
      } else
      {
        j += sprintf((*args)+j,"%s"PATH_SEPARATOR_STR,&directoryname[0]);
      }
    }
  }
  strcpy((*args)+j,indexormatchfile);
  return 0;
}

Sint showargumentline(SelectmatchHeader paramselectmatchHeader,
                      FILE *outfp,
                      char *argstring,
                      Argctype callargc,
                      const char **callargv)
{
  if(paramselectmatchHeader == NULL)
  {
    fprintf(outfp,"%s%s\n",ARGUMENTLINEPREFIX,argstring);
  } else
  {
    Argctype originalargc;
    char *originalargv[MAXNUMBEROFFILES+MAXNUMOFARGS];
    originalargv[0] = "";
    if(splitargstring(argstring,(Uint) strlen(argstring),
                      (Uint) (MAXNUMOFARGS+MAXNUMBEROFFILES),
                      &originalargc,&originalargv[1]) != 0)
    {
      return (Sint) -1;
    }
    if(callargc == 0)
    {
      if(paramselectmatchHeader(originalargc,
                                (const char * const *) originalargv,
                                originalargc,
                                (const char * const *) originalargv) != 0)
      {
        return (Sint) -2;
      }
    } else
    {
      if(paramselectmatchHeader(originalargc,
                                (const char * const *) originalargv,
                                callargc,
                                callargv) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  return 0;
}
