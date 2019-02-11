#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minmax.h"
#include "types.h"
#include "debugdef.h"
#include "args.h"
#include "errordef.h"
#include "alignment.h"
#include "xdropdef.h"
#include "spacedef.h"
#include "scoredef.h"
#include "simpleopt.h"

#include "xdropal2.pr"
#include "showalign.pr"

#define FWRITE(SEQ,LEN)\
        if(fwrite(SEQ,sizeof(Uchar),(size_t) (LEN),stdout) != (size_t) (LEN))\
        {\
          fprintf(stderr,"Cannot write %lu itmes of type Uchar",\
                  (Showuint) (LEN));\
          exit(EXIT_FAILURE);\
        }

void compareAlignments(Editoperation *al1,Uint allen1,
                       Editoperation *al2,Uint allen2)
{
  Uint i;

  if(allen1 != allen2)
  {
    fprintf(stderr,"alignments have different length %lu and %lu\n",
            (Showuint) allen1,(Showuint) allen2);
    exit(EXIT_FAILURE);
  }
  for(i=0; i<allen1; i++)
  {
    if(al1[i] != al2[i])
    {
      fprintf(stderr,"al1[%lu] = ",(Showuint) i);
      showoneeditop(stderr,al1[i]);
      fprintf(stderr," != ");
      showoneeditop(stderr,al2[i]);
      fprintf(stderr," al2[%lu]\n",(Showuint) i);
      exit(EXIT_FAILURE);
    }
  }
}

void checkxalign1(BOOL forward,Uchar *useq,Sint ulen,
                         Uchar *vseq,Sint vlen)
{
  ArrayEditoperation al1;
  Uint saflag;

  DEBUGCODE(1,FWRITE(useq,ulen));
  DEBUG0(1," ");
  DEBUGCODE(1,FWRITE(vseq,vlen));
  DEBUG0(1,"\n");
  INITARRAY(&al1,Editoperation);
  al1.allocatedEditoperation = (Uint) (ulen+vlen);
  ALLOCASSIGNSPACE(al1.spaceEditoperation,NULL,Editoperation,
                   al1.allocatedEditoperation);
  (void) onexdropalignment1(forward,&al1,useq,(Uint) ulen,vseq,(Uint) vlen);
  DEBUGCODE(3,showeditops(al1.spaceEditoperation,
                          al1.nextfreeEditoperation,stdout));
  MAKESAFLAG(saflag,forward,False,False);
  showalignment(saflag,stdout,UintConst(60),al1.spaceEditoperation,
                al1.nextfreeEditoperation,UintConst(100),
                useq,useq,(Uint) ulen,vseq,vseq,(Uint) vlen,0,0);
  FREEARRAY(&al1,Editoperation);
}

static Sint checkxalign(BOOL forward,Uchar *useq,Uint ulen,
                        Uchar *vseq,Uint vlen)
{
  ArrayEditoperation al2;
  Uint saflag;

  DEBUG0(1,"checkxalign \"");
  DEBUGCODE(1,FWRITE(useq,ulen));
  DEBUG0(1,"\" \"");
  DEBUGCODE(1,FWRITE(vseq,vlen));
  DEBUG0(1,"\"\n");
  INITARRAY(&al2,Editoperation);
  (void) onexdropalignment2(forward,&al2,useq,(Sint) ulen,vseq,(Sint) vlen,10);
  DEBUGCODE(3,showeditops(al2.spaceEditoperation,
                          al2.nextfreeEditoperation,stdout));
  MAKESAFLAG(saflag,forward,False,False);
  showalignment(saflag,stdout,UintConst(60),al2.spaceEditoperation,
                al2.nextfreeEditoperation,UintConst(100),
                useq,useq,(Uint) ulen,vseq,vseq,(Uint) vlen,0,0);
  FREEARRAY(&al2,Editoperation);
  return 0;
}

MAINFUNCTION
{
  DEBUGLEVELSET;
  if(applycheckfunctiontosimpleoptions(checkxalign,argc,argv) != 0)
  {
    STANDARDMESSAGE;
  }
  return EXIT_SUCCESS;
}
