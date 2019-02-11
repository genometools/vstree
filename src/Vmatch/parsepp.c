
#include <string.h>
#include "types.h"
#include "spacedef.h"
#include "debugdef.h"
#include "optdesc.h"
#include "chaincall.h"
#include "mcldef.h"

#include "chncallparse.pr"
#include "parsemcl.pr"

#define DUMMYINDEX   "indexname"

#define CHECKSTRBUFOVERFLOW\
        if(strbufoffset + addedbytes > totstrlen)\
        {\
          ERROR3("buffer overflow: strbufoffset + addedbytes "\
                 "= %lu+%lu > %lu = totstrlen",\
                 (Showuint) strbufoffset,\
                 (Showuint) addedbytes,\
                 (Showuint) totstrlen);\
          return (Sint) -1;\
        }

typedef struct
{
  char **argv;
  Argctype argc;
  char *strbuf;
} Transformedargs;

static Sint filltransformedargs(Transformedargs *transformedargs,
                                BOOL (*checkargument)(const char *),
                                const char * const*argv,
                                Uint argnum,
                                Argctype argc,
                                const char *progname)
{
  Uint strbufoffset, addedbytes, i, j, totstrlen = 0;

  for(i=argnum+1; i<(Uint) (argc-1); i++)
  {
    if(ISOPTION(argv[i]))
    {
      break;
    }
    totstrlen += (Uint) (strlen(argv[i]) + 1 + 1);
  }
  transformedargs->argc = (Argctype) (i-argnum+1);
  DEBUG2(1,"# with transformedargs->argc = %ld and totstrlen=%lu\n",
          (Showsint) transformedargs->argc,(Showuint) totstrlen);
  ALLOCASSIGNSPACE(transformedargs->argv,
                   NULL,char *,(Uint) transformedargs->argc);
  totstrlen += (Uint) (strlen(progname) + 1);
  totstrlen += (Uint) (strlen(DUMMYINDEX) + 1);
  ALLOCASSIGNSPACE(transformedargs->strbuf,NULL,char,totstrlen);
  strbufoffset = 0;
  addedbytes = (Uint) sprintf(transformedargs->strbuf,"%s",progname);
  CHECKSTRBUFOVERFLOW;
  transformedargs->argv[0] = transformedargs->strbuf;
  strbufoffset += (addedbytes+1);
  for(i=argnum+1, j=UintConst(1); i<(Uint) (argc-1); i++, j++)
  {
    if(ISOPTION(argv[i]))
    {
      break;
    }
    if(checkargument(argv[i]))
    {
      addedbytes = (Uint) sprintf(transformedargs->strbuf+strbufoffset,
                                  "-%s",argv[i]);
    } else
    {
      addedbytes = (Uint) sprintf(transformedargs->strbuf+strbufoffset,
                                  "%s",argv[i]);
    }
    CHECKSTRBUFOVERFLOW;
    transformedargs->argv[j] = transformedargs->strbuf + strbufoffset;
    strbufoffset+= (addedbytes+1);
  }
  addedbytes = (Uint) sprintf(transformedargs->strbuf+strbufoffset,
                              "%s",DUMMYINDEX);
  CHECKSTRBUFOVERFLOW;
  transformedargs->argv[j] = transformedargs->strbuf + strbufoffset;
  DEBUG0(1,"# ");
  for(i=0; i<(Uint) transformedargs->argc; i++)
  {
    DEBUG2(1,"%s%c",transformedargs->argv[i],
                    (i<(Uint) (transformedargs->argc-1)) ? ' ' : '\n');
  }
  return 0;
}

static BOOL checkchainargument(const char *argument)
{
  if(strcmp(argument,"global") == 0 ||
     strcmp(argument,"local") == 0 ||
     strcmp(argument,"maxgap") == 0 ||
     strcmp(argument,"outprefix") == 0 ||
     strcmp(argument,"silent") == 0 ||
     strcmp(argument,"thread") == 0 ||
     strcmp(argument,"wf") == 0 ||
     strcmp(argument,"withinborders") == 0)
  {
    return True;
  }
  return False;
}

static BOOL checkmatchclusterarguments(const char *argument)
{
  if(strcmp(argument,"erate") == 0 ||
     strcmp(argument,"gapsize") == 0 ||
     strcmp(argument,"overlap") == 0 ||
     strcmp(argument,"outprefix") == 0)
  {
    return True;
  }
  return False;
}
   
Sint parsegenericpostprocessing(Chaincallinfo *chaincallinfo,
                                Matchclustercallinfo *matchclustercallinfo,
                                const char *progname,
                                const char * const*argv,
                                Uint argnum,
				Argctype argc,
				const char *optname)
{
  if(argnum == (Uint) (argc-1))
  {
    ERROR1("missing argument for option %s",optname);
    return (Sint) -1;
  }
  if(strcmp(argv[argnum],CHAINPREFIX) == 0)
  {
    Transformedargs chainargs;

    if(filltransformedargs(&chainargs,
                           checkchainargument,
                           argv,
                           argnum,
                           argc,
                           progname) != 0)
    {
      return (Sint) -1;
    }
    if(parsechain2dim(True,
                      chaincallinfo,
                      (const char * const*) chainargs.argv,
                      chainargs.argc) != 0)
    {
      return (Sint) -2;
    }
    FREESPACE(chainargs.strbuf);
    FREESPACE(chainargs.argv);
    return (Sint) (chainargs.argc-1);
  }
  if(strcmp(argv[argnum],MATCHCLUSTERNAME) == 0)
  {
    Transformedargs matchclusterargs;

    if(filltransformedargs(&matchclusterargs,
                           checkmatchclusterarguments,
                           argv,
                           argnum,
                           argc,
                           progname) != 0)
    {
      return (Sint) -1;
    }
    if(parsematchcluster(True,
                         matchclustercallinfo,
                         (const char * const*) matchclusterargs.argv,
                         matchclusterargs.argc) != 0)
    {
      return (Sint) -2;
    }
    FREESPACE(matchclusterargs.strbuf);
    FREESPACE(matchclusterargs.argv);
    return (Sint) (matchclusterargs.argc-1);
  }
  ERROR1("illegal postprocessing mode \"%s\"",argv[argnum]);
  return (Sint) -4;
}
