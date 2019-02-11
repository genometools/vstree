#include "types.h"
#include "evaluedef.h"

#define READINT(N,S)\
        if(sscanf(argv[N],"%ld",&readint) != 1 ||\
           ((N) != 4 && readint < 0))\
        {\
          fprintf(stderr,"%s: argument %lu (%s) must be integer\n",\
                  argv[0],(Showuint) (N+1),S);\
          return EXIT_FAILURE;\
        }

static void gettheEvalueself(Uint asize,Uint ulen,Uint vlen,
                             Sint difference,Uint matchlength)
{
  Evalues hevalues;
  Evaluetype evalue, multiplier;

  inithammingEvalues(&hevalues,1.0/(Evaluetype) asize);
  multiplier = (Evaluetype) ulen * (Evaluetype) vlen;
  evalue = incgetEvalue(&hevalues,
                        multiplier,
                        difference,
                        matchlength);
  printf("asize=%lu length1=%lu length2=%lu distance=%ld matchlength=%lu "
         "=> Evalue = %.2e\n",
         (Showuint) asize,
         (Showuint) ulen,
         (Showuint) vlen,
         (Showsint) difference,
         (Showuint) matchlength,
         evalue);
}

MAINFUNCTION
{
  Uint asize, ulen, vlen, matchlength;
  Sint difference; 
  Scaninteger readint;

  if(argc != 6)
  {
    fprintf(stderr,"Usage: %s <asize> <seqlen1> <seqlen2> "
                   "<difference> <matchlength>\n",argv[0]);
    return EXIT_FAILURE;
  }
  READINT(1,"alphabet size");
  asize = (Uint) readint;
  READINT(2,"length 1");
  ulen = (Uint) readint;
  READINT(3,"length 2");
  vlen = (Uint) readint;
  READINT(4,"difference");
  difference = (Sint) readint;
  READINT(5,"matchlength");
  matchlength = (Uint) readint;
  gettheEvalueself(asize,ulen,vlen,difference,matchlength);
  return EXIT_SUCCESS;
}
