
#include <stdio.h>
#include "outinfo.h"
#include "debugdef.h"
#include "errordef.h"
#include "xdropdef.h"

#define CHECKSTART\
        if(start >= EXPLAINLENGTH)\
        {\
          ERROR0("explanation string is too short");\
          return (Sint) -1;\
        }

#define OUT1(X)   start += sprintf(expstring+start,X);\
                  CHECKSTART

#define OUT2(X,Y) start += sprintf(expstring+start,X,Y);\
                  CHECKSTART

#define PARTINFO(S)\
        OUT2(" l(%s)",S);\
        if(showmode & SHOWFILE)\
        {\
          OUT2(" f(%s)",S);\
        }\
        if(showmode & SHOWABSOLUTE)\
        {\
          OUT2(" p(%s)",S);\
        } else\
        {\
          if(showdesc->defined)\
          {\
            OUT2(" h(%s)",S);\
          } else\
          {\
            OUT2(" n(%s)",S);\
          }\
          OUT2(" r(%s)",S);\
        }

/*
  Chris: call this function as in explainmatchinfofp below, and
  output the string taken as a first argument. Use True as the second
  argument to obtain the explanation about an id-number
*/

Sint explainmatchinfo(char *expstring,
                      BOOL withid,
                      Uint numberofqueryfiles,
                      Uint showmode,
                      Showdescinfo *showdesc,
                      BOOL sixframematch)
{
  Sprintfreturntype start = 0;

  OUT1("# matches are reported in the following way\n#");
  if(withid)
  {
    OUT1(" id ");
  }
  PARTINFO("S");
  OUT1(" t");
  if(numberofqueryfiles == 0)
  {
    PARTINFO("S");
  } else
  {
    PARTINFO("Q");
  }
  if(!(showmode & SHOWNODIST))
  {
    OUT1(" d");
  }
  if(!(showmode & SHOWNOEVALUE))
  {
    OUT1(" e");
  }
  if(!(showmode & SHOWNOSCORE))
  {
    OUT1(" s");
  }
  if(!(showmode & SHOWNOIDENTITY))
  {
    OUT1(" i");
  }
  OUT1("\n# where:\n");
  if(withid)
  {
    OUT1("# id = unique identity number\n");
  }
  OUT1("# l = length\n");
  if(showmode & SHOWFILE)
  {
    OUT1("# f = filename\n");
  }
  if(showmode & SHOWABSOLUTE)
  {
    OUT1("# p = absolute position\n");
  } else
  {
    if(showdesc->defined)
    {
      OUT1("# h = sequence header\n");
    } else
    {
      OUT1("# n = sequence number\n");
    }
    OUT1("# r = relative position\n");
  }
  if(sixframematch)
  {
    OUT1("# t = type (F=fwd/fwd, G=fwd/rev, H=rev/fwd, I=rev/rev)\n");
  } else
  {
    OUT1("# t = type (D=direct, P=palindromic)\n");
  }
  if(!(showmode & SHOWNODIST))
  {
    OUT1("# d = distance value (negative=hamming distance, 0=exact, positive=edit distance)\n");
  }
  if(!(showmode & SHOWNOEVALUE))
  {
    OUT1("# e = E-value\n");
  }
  if(!(showmode & SHOWNOSCORE))
  {
    OUT1("# s = score value (negative=hamming score, positive=edit score)\n");
  }
  if(!(showmode & SHOWNOIDENTITY))
  {
    OUT1("# i = percent identity\n");
  }
  OUT1("# (S) = in Subject\n");
  if(numberofqueryfiles > 0)
  {
    OUT1("# (Q) = in Query\n");
  }
  return 0;
}

Sint explainmatchinfofp(BOOL withid,
                        FILE *outfp,
                        Uint numberofqueryfiles,
                        Uint showmode, 
                        Showdescinfo *showdesc,
                        BOOL sixframematch)
{
  char expstring[EXPLAINLENGTH+1];

  if(explainmatchinfo(&expstring[0],
                      withid,
                      numberofqueryfiles,
                      showmode,
                      showdesc,
                      sixframematch) != 0)
  {
    return (Sint) -1;
  }
  (void) fputs(expstring,outfp);
  return 0;
}
