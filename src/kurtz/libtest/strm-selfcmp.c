
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "minmax.h"
#include "virtualdef.h"
#include "megabytes.h"
#include "alphadef.h"
#include "fhandledef.h"
#include "queryext.h"
#include "genfile.h"
#include "esastream.h"
#include "args.h"
#define ESASTREAMACCESS
#include "vmatfind-def.h"

#include "alphabet.pr"
#include "matchsub.pr"
#include "readmulti.pr"
#include "handleesastream.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"

#include "readvirt.pr"

static Sint simpleexactselfmatchoutput(/*@unused@*/ void *info,
                                       Uint len,
                                       Uint pos1,
                                       Uint pos2)
{
  if(pos1 > pos2)
  {
    Uint tmp = pos1;
    pos1 = pos2;
    pos2 = tmp;
  }
  printf("%lu %lu %lu\n",(Showuint) len,
                         (Showuint) pos1,
                         (Showuint) pos2);
  return 0;
}

static Sint findallsubmatches(Esastream *esastream,
                              Alphabet *alphabet,
                              BOOL rcmode,
                              Uint userdefinedleastlength,
                              BOOL verbose)
{
  if(verbose)
  {
    printf("# alphasize = %lu\n",(Showuint) alphabet->mapsize-1);
    printf("# searchlength = %lu\n",(Showuint) userdefinedleastlength);
  }
  if(rcmode)
  {
    fprintf(stderr,"rcmode for selfmatch not implemented yet\n");
    exit(EXIT_FAILURE);
  }
  if(strmvmatmaxoutdynamic(esastream,
                           UintConst(1),
                           userdefinedleastlength,
                           NULL,
                           NULL,
                           simpleexactselfmatchoutput) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

MAINFUNCTION
{
  Esastream esastream;
  Scaninteger readint;
  const char *indexname;
  Uint userdefinedleastlength;

  DEBUGLEVELSET;
  CHECKARGNUM(3,"<searchlength> <indexname>");
  if(sscanf(argv[1],"%ld",&readint) != 1 || readint < (Scaninteger) 1)
  {
    fprintf(stderr,"Usage: %s: first argument must be positive integer\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  userdefinedleastlength = (Uint) readint;
  indexname = argv[2];
  if(initesastream(&esastream,
                   indexname,
                   LCPTABSTREAM | SUFTABSTREAM | BWTTABSTREAM) != 0)
  {
    STANDARDMESSAGE;
  }
  if(findallsubmatches(&esastream,
                       &esastream.alpha,
                       False,
                       userdefinedleastlength,
                       False) != 0)
  {
    STANDARDMESSAGE;
  }
  if(closeesastream(&esastream) != 0)
  {
    STANDARDMESSAGE;
  }
  checkspaceleak();
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
