#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minmax.h"
#include "types.h"
#include "debugdef.h"
#include "args.h"
#include "errordef.h"
#include "alignment.h"
#include "spacedef.h"
#include "simpleopt.h"
#include "galigndef.h"
#include "dpbitvec48.h"
#include "scoredef.h"

#include "showalign.pr"
#include "galign.pr"
#include "frontSEP.pr"
#include "bitvdist.pr"
#include "cmpmaxd.pr"

#define FWRITE(SEQ,LEN)\
        if(fwrite(SEQ,sizeof(Uchar),(size_t) (LEN),stdout) != (size_t) (LEN))\
        {\
          fprintf(stderr,"Cannot write %lu itmes of type Uchar",\
                  (Showuint) (LEN));\
          exit(EXIT_FAILURE);\
        }

static Sint checkgreededistyalign(BOOL withmaxdist,
                                  Sint maxdist,
                                  Uchar *useq,
                                  Uint ulen,
                                  Uchar *vseq,
                                  Uint vlen)
{
  Uint saflag;
  Sint realdistance;

  Greedyalignreservoir greedyalignreservoir;
  initgreedyalignreservoir(&greedyalignreservoir);
  realdistance = greedyedistalign(&greedyalignreservoir,
                                  withmaxdist,
                                  maxdist,
                                  useq,
                                  (Sint) ulen,
                                  vseq,
                                  (Sint) vlen);
  if(realdistance < 0)
  {
    return (Sint) -1;
  }
  verifyedistalignment(__FILE__,(Uint) __LINE__,
                       greedyalignreservoir.alignment.spaceEditoperation,
                       greedyalignreservoir.alignment.nextfreeEditoperation,
                       ulen,
                       vlen,
                       realdistance);
  MAKESAFLAG(saflag,True,False,False);
  showalignment(saflag,
                stdout,
                UintConst(60),
                greedyalignreservoir.alignment.spaceEditoperation,
                greedyalignreservoir.alignment.nextfreeEditoperation,
                (Uint) realdistance,
                useq,
                useq,
                ulen,
                vseq,
                vseq,
                vlen,
                0, 
                0);
  (void) putchar('\n');
  wraptgreedyalignreservoir(&greedyalignreservoir);
  return 0;
}

static Sint checkhammingalign(Sint edist1,
                              Uchar *useq,
                              Uint ulen,
                              Uchar *vseq,
                              Uint vlen)
{
  Uint saflag;
  Greedyalignreservoir greedyalignreservoir;

  initgreedyalignreservoir(&greedyalignreservoir);
  edist1 = (Sint) hammingalignment(&greedyalignreservoir.alignment,
				   useq,
				   ulen,
				   vseq);
  verifyedistalignment(__FILE__,(Uint) __LINE__,
		       greedyalignreservoir.alignment.spaceEditoperation,
		       greedyalignreservoir.alignment.nextfreeEditoperation,
		       ulen,
		       vlen,
		       edist1);
  MAKESAFLAG(saflag,True,False,False);
  showalignment(saflag,
		stdout,
		UintConst(60),
		greedyalignreservoir.alignment.spaceEditoperation,
		greedyalignreservoir.alignment.nextfreeEditoperation,
		(Uint) edist1,
		useq,
		useq,
		ulen,
		vseq,
		vseq,
		vlen,
		0,
		0);
  (void) putchar('\n');
  wraptgreedyalignreservoir(&greedyalignreservoir);
  return 0;
}

static Sint checkfront(/*@unused@*/ BOOL forward,
                       Uchar *useq,
                       Uint ulen,
                       Uchar *vseq,
                       Uint vlen)
{
  Sint edist1;
  Uint edist2, mdist;
  DPbitvectorreservoir dpbvres;

  DEBUG0(1,"\"");
  DEBUGCODE(1,FWRITE(useq,ulen));
  DEBUG0(1,"\" \"");
  DEBUGCODE(1,FWRITE(vseq,vlen));
  DEBUG0(1,"\"\n");
  edist1 = unitedistfrontSEPgeneric(False,0,useq,(Sint) ulen,vseq,(Sint) vlen);
  printf("edist=%ld\n",(Showsint) edist1);
  initDPbitvectorreservoir(&dpbvres,(Uint) EQSSIZE,MIN(ulen,vlen));
  edist2 = distanceofstrings(&dpbvres,(Uint) EQSSIZE,useq,ulen,vseq,vlen);
  if((Uint) edist1 != edist2)
  {
    fprintf(stderr,"different distances: edist1=%ld != %lu = edist2\n",
            (Showsint) edist1,(Showuint) edist2);
    exit(EXIT_FAILURE); 
  }
  if(checkgreededistyalign(True,
                           edist1,
                           useq,
                           ulen,
                           vseq,
                           vlen) != 0)
  {
    return (Sint) -1;
  }
  if(checkgreededistyalign(False,
                           0,
                           useq,
                           ulen,
                           vseq,
                           vlen) != 0)
  {
    return (Sint) -2;
  }
  if(ulen == vlen)
  {
    if(checkhammingalign(edist1,
                         useq,
                         ulen,
                         vseq,
                         vlen) != 0)
    {
      return (Sint) -3;
    }
  }
  mdist = cmpmaxdist(UintConst(4),useq,ulen,vseq,vlen,edist2);
  printf("mdist = %lu\n",(Showuint) mdist); 
  if(mdist != edist2)
  {
    fprintf(stderr,"different distances: mdist=%lu != %lu = edist2\n",
            (Showuint) mdist,(Showuint) edist2);
    exit(EXIT_FAILURE);
  }
  freeDPbitvectorreservoir(&dpbvres);
  return 0;
}

MAINFUNCTION
{
  DEBUGLEVELSET;
  if(applycheckfunctiontosimpleoptions(checkfront,argc,argv) != 0)
  {
    STANDARDMESSAGE;
  }
  return EXIT_SUCCESS;
}
