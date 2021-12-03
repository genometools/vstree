#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "errordef.h"
#include "megabytes.h"
#include "debugdef.h"
#include "spacedef.h"
#include "chardef.h"
#include "divmodmul.h"
#include "codondef.h"
#include "optdesc.h"
#include "multidef.h"
#include "virtualdef.h"
#include "esafileend.h"
#include "fhandledef.h"

#include "dstrdup.pr"
#include "detpfxlen.pr"
#include "sixframe.pr"
#include "accvirt.pr"
#include "procopt.pr"
#include "readvirt.pr"
#include "safescpy.pr"
#include "filehandle.pr"
#include "alphabet.pr"

#include "inputdef.h"

#include "mkvprocess.pr"
#include "mkvtree.pr"

static void showonstdout(char *s)
{
  printf("%s\n",s);
}

static Sint makedna6idx(Virtualtree *virtualtree,
                        Uint transnum,
                        char *indexname,
                        char *smapfile,
                        Showverbose showverbose)
{
  Input input;
  
  input.storeonfile = True;
  input.rev = False;
  input.rcm = False;
  input.transnum = transnum;
  input.demand = SUFTAB | BWTTAB | LCPTAB | TISTAB | OISTAB;
  input.maxdepth.defined = False;
  input.maxdepth.uintvalue = 0;
  input.numofchars = virtualtree->alpha.mapsize - UintConst(1);
  if(safestringcopy(&input.indexname[0],indexname,PATH_MAX) != 0)
  {
    return (Sint) -1;
  }
  strcat(&input.indexname[0],SIXFRAMESUFFIX);
  input.inputalpha.dnafile = False;
  if(smapfile == NULL)
  {
    input.inputalpha.proteinfile = True;
    input.inputalpha.symbolmapfile = NULL;
  } else
  {
    input.inputalpha.proteinfile = False;
    input.inputalpha.symbolmapfile = smapfile;
  }
  input.showverbose = showverbose;
  virtualtree->specialsymbols = True;
  virtualtree->prefixlength 
      = vm_recommendedprefixlength(input.numofchars,
                                   virtualtree->multiseq.totallength,
                                   SIZEOFBCKENTRY);
  if(mkvtreeprocess(&input,virtualtree) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

static Sint extractsmapfile(char **smapfileptr,const char **argv,Argctype argc)
{
  Sint argnum;
  BOOL hasargument;

  argnum = extractoptionwithpossibleargument(True,
                                             &hasargument,
                                             "-smap",
                                             argv,
                                             argc);
  if(argnum < 0)
  {
    *smapfileptr = NULL; 
  } else
  {
    if(!hasargument)
    {
      ERROR1("missing argument to option %s\n","-smap");
      return (Sint) -1;
    }
    ASSIGNDYNAMICSTRDUP(*smapfileptr,argv[argnum+1]);
  }
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree,
              dnasixframevirtualtree;
  char *smapfile, *indexname;
  Sint ret;
  Showverbose showverbose;
  Uint transnum;

  if(extractoption("-v",argv,argc) > 0)
  {
    showverbose = showonstdout;
  } else
  {
    showverbose = NULL;
  }

  DEBUGLEVELSET;
  transnum = STANDARDTRANSLATIONTABLE;  // Standard Scheme
  makeemptyvirtualtree(&virtualtree);
  ret = callmkvtreegeneric(False,
                           True,
                           argc,
                           argv,
                           True,
                           &virtualtree,
                           True,
                           showverbose,
                           &transnum);
  if(ret == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  if(ret == (Sint) -1)
  {
    SIMPLESTANDARDMESSAGE;
  }
  if(ret < 0)
  {
    STANDARDMESSAGE;
  }
  makeemptyvirtualtree(&dnasixframevirtualtree);
  if(extractsmapfile(&smapfile,argv,argc) != 0)
  {
    STANDARDMESSAGE;
  }
  if(smapfile == NULL)
  {
    assignProteinalphabet(&dnasixframevirtualtree.alpha);
  } else
  {
    if(readsymbolmap(&dnasixframevirtualtree.alpha,
                     (Uint) UNDEFCHAR,
                     (Uint) WILDCARD,
                     smapfile) != 0)
    {
      STANDARDMESSAGE;
    }
  }
  if(multisixframetranslateDNA(transnum,
                               True,
                               &dnasixframevirtualtree.multiseq,
                               &virtualtree.multiseq,
                               dnasixframevirtualtree.alpha.symbolmap) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
  if(extractindexname(&indexname,argv,argc) != 0)
  {
    STANDARDMESSAGE;
  }
  if(makedna6idx(&dnasixframevirtualtree,transnum,
                 indexname,smapfile,showverbose) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(indexname);
  if(freevirtualtree(&dnasixframevirtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
  if(smapfile != NULL)
  {
    FREESPACE(smapfile);
  }
#ifndef NOSPACEBOOKKEEPING
  if(extractoption("-v",argv,argc) > 0)
  {
    printf("# overall space peak: main=%.2f MB, "
           " secondary=%.2f MB\n",
           MEGABYTES(getspacepeak()),
	   MEGABYTES(mmgetspacepeak()));
  }
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
