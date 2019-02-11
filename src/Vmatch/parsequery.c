
#include "errordef.h"
#include "matchtask.h"
#include "optdesc.h"
#include "debugdef.h"

#include "dstrdup.pr"

#define CHECKSUBNUM(V)\
        if((V) < 0)\
        {\
          ERROR1("illegal negative number %ld in subequence specification",\
                  (Showsint) (V));\
          return (Sint) -1;\
        }

static Sint parseSubstringspec(const char *argument,
                               Matchcallinfo *matchcallinfo,
                               const char *optname)
{ 
  Scaninteger readseqstart, readseqend; 

  if(matchcallinfo->numberofqueryfiles != UintConst(1))
  {
    ERROR2("incorrect argument \"%s\" to option %s: subequence "
           "specification can only follow first filename",argument,
           optname);
    return (Sint) -1;
  }
  if(sscanf(argument,"(%ld,%ld)",&readseqstart,&readseqend) != 2)
  {
    ERROR2("incorrect argument \"%s\" to option %s: subequence specification "
           " must be of form (seqstart,seqend)",argument,
           optname);
    return (Sint) -2;
  }
  CHECKSUBNUM(readseqstart);
  CHECKSUBNUM(readseqend);
  FQFSETVALUES(&matchcallinfo->fqfsubstringinfo,
               (Uint) readseqstart,
               (Uint) readseqend);
  return 0;
}

Sint parsequeryparameters(Matchcallinfo *matchcallinfo,
                          const char * const*argv,
                          Argctype argc,
                          Uint argnum,
                          const char *optionname)
{
  argnum++;
  CHECKMISSINGARGUMENTGENERIC(optionname,argc-1);
  while(argnum < (Uint) argc)
  {
    if(argnum == (Uint) (argc-1) || ISOPTION(argv[argnum]))
    {
      argnum--;
      break;
    }
    if(argv[argnum][0] == '(')
    {
      if(parseSubstringspec(argv[argnum],
                            matchcallinfo,
                            optionname) != 0)
      {
        return (Sint) -1;
      }
    } else
    {
      if(matchcallinfo->numberofqueryfiles >= (Uint) MAXNUMBEROFFILES)
      {
        ERROR1("maximal number of queryfiles is %lu",
               (Showuint) MAXNUMBEROFFILES);
        return (Sint) -2;
      }
      ASSIGNDYNAMICSTRDUP(matchcallinfo->queryfiles[
                          matchcallinfo->numberofqueryfiles],argv[argnum]);
      matchcallinfo->numberofqueryfiles++;
    }
    argnum++;
  }
  if(matchcallinfo->fqfsubstringinfo.ffdefined && 
     matchcallinfo->numberofqueryfiles != UintConst(1))
  {
    ERROR2("incorrect argument \"%s\" to option %s: "
           "if subequence specification is used, then only one "
           "query sequence is allowed",
           argv[argnum],optionname);
    return (Sint) -3;
  }
  if(matchcallinfo->numberofqueryfiles == 0)
  {
    ERROR1("option %s requires at least one argument",optionname);
    return (Sint) -4;
  }
  return (Sint) argnum;
}
