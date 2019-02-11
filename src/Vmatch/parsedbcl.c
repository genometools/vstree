
#include "types.h"
#include "matchtask.h"
#include "debugdef.h"
#include "optdesc.h"

#include "dstrdup.pr"

#define CHECKCLSIZENUM(V,S,OP)\
        if(V OP)\
        {\
          ERROR2("%s number in clustersize specification must not be %s",\
                 S,#OP);\
          return (Sint) -1;\
        }

Sint parsedbclusterparameters(Matchcallinfo *matchcallinfo,
                              const char * const*argv,
                              Argctype argc,
                              Uint argnum,
                              const char *optname)
{
  Scaninteger readint;

  READPERCENTAGE(optname,readint,"first");
  matchcallinfo->clusterparms.dbclpercsmall = (Uchar) readint;
  READPERCENTAGE(optname,readint,"second");
  matchcallinfo->clusterparms.dbclperclarge = (Uchar) readint;
  matchcallinfo->clusterparms.dbclusterdefined = True;
  if(MOREARGOPTSWITHOUTLAST(argnum))
  {
    argnum++;
    if(argv[argnum][0] == '(')
    {
      ERROR0("the specification of minimal and maximal cluster "
             "sizes requires the specification of a file prefix "
             "as third argument");
      return (Sint) -1;
    }
    ASSIGNDYNAMICSTRDUP(matchcallinfo->clusterparms.dbclusterfilenameprefix,
                        argv[argnum]);
    if(MOREARGOPTSWITHOUTLAST(argnum))
    {
      Scaninteger dbclminsize, dbclmaxsize;
      argnum++;
      if(sscanf(argv[argnum],"(%ld,%ld)",
                &dbclminsize,&dbclmaxsize) != 2)
      {
        ERROR2("incorrect fourth argument \"%s\" to option %s: "
               "cluster size specification must be of the form "
               "(dbclminsize,dbclmaxsize)",
               argv[argnum],
               optname);
        return (Sint) -2;
      }
      CHECKCLSIZENUM(dbclminsize,"first",< (Scaninteger) 1);
      CHECKCLSIZENUM(dbclmaxsize,"second",< 0);
      matchcallinfo->clusterparms.dbclminsize = (Uint) dbclminsize;
      if(dbclmaxsize == 0)
      {
        matchcallinfo->clusterparms.dbclmaxsize = DBCLMAXSIZE;
      } else
      {
        if((Uint) dbclmaxsize < matchcallinfo->clusterparms.dbclminsize)
        {
          ERROR0("second number in clustersize specification "
                 "must not be smaller than first number");
          return (Sint) -3;
        }
        matchcallinfo->clusterparms.dbclmaxsize = (Uint) dbclmaxsize;
      }
    }
  }
  return (Sint) argnum;
}
