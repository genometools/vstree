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

#define FWRITE(SEQ,LEN)\
        if(fwrite(SEQ,sizeof(Uchar),(size_t) (LEN),stdout) != (size_t) (LEN))\
        {\
          fprintf(stderr,"Cannot write %lu itmes of type Uchar",\
                  (Showuint) (LEN));\
          exit(EXIT_FAILURE);\
        }

#define MAXLEN 100

static Sint checkuedist(Uchar *useq,Sint ulen,Uchar *vseq,Sint vlen)
{
  Sint edist1;
  Uint i, edist2;
  Uchar characters[UCHAR_MAX+1], ali1[MAXLEN], ali2[MAXLEN];

  if(ulen >= MAXLEN)
  {
    fprintf(stderr,"ulen = %lu >= %lu=MAXLEN\n",(Showuint) ulen,
                                                (Showuint) MAXLEN);
    exit(EXIT_FAILURE);
  }
  if(vlen >= MAXLEN)
  {
    fprintf(stderr,"vlen = %lu >= %lu=MAXLEN\n",(Showuint) vlen,
                                                (Showuint) MAXLEN);
    exit(EXIT_FAILURE);
  }
  for(i=0; i<=UCHAR_MAX; i++)
  {
    characters[i] = (Uchar) i;
  }
  DEBUG2(1,"ulen=%lu,vlen=%lu\n",(Showuint) ulen,(Showuint) vlen);
  DEBUGCODE(1,FWRITE(useq,ulen));
  DEBUG0(1," ");
  DEBUGCODE(1,FWRITE(vseq,vlen));
  DEBUG0(1,"\n");
  edist1 = unitedistfront(True,useq,(Sint) ulen,vseq,(Sint) vlen);
  edist2 = alignlinearspace(useq,vseq,(Uint) ulen,(Uint) vlen,
                            &ali1[0],&ali2[0]);
  if(((Uint) edist1) != edist2)
  {
    ERROR2("checkuedist failed: edist1=%ld != %ld=edist2\n",
                    (Showsint) edist1,(Showsint) edist2);
    return (Sint) -1;
  }
  return 0;
}

#undef FROMARG
#undef FROMFILE
#define FROMTEXT

#ifdef FROMFILE
MAINFUNCTION
{
  Uchar *u, *v;
  Uint ulen, vlen;
  
  CHECKARGNUM(3,"file1 file2");

  u = CREATEMEMORYMAP(argv[1],False,&ulen);
  if(u == NULL)
  {
    STANDARDMESSAGE;
  }
  v = CREATEMEMORYMAP(argv[2],False,&vlen);
  if(v == NULL)
  {
    STANDARDMESSAGE;
  }
  
  DEBUGLEVELSET;
  if(checkuedist(u,(Sint) ulen,v,(Sint) vlen) != 0)
  {
    STANDARDMESSAGE;
  }
  if(DELETEMEMORYMAP(u) != 0)
  {
    STANDARDMESSAGE;
  }
  if(DELETEMEMORYMAP(v) != 0)
  {
    STANDARDMESSAGE;
  }
  return EXIT_SUCCESS;
}

#else
#ifdef FROMARG

MAINFUNCTION
{
  Sint i, len;
  
  CHECKARGNUM(2,"text");

  DEBUGLEVELSET;
  
  len = strlen(argv[1]);
  for(i=1; i<=len/2; i++)
  {
    if(checkuedist(argv[1],i,argv[1]+i,len-i) != 0)
    {
      STANDARDMESSAGE;
    }
  }
  return EXIT_SUCCESS;
}
#else
Sint applycheckuedist(Uchar *text,Uint textlen,/*@unused@*/ void *maxval)
{
  Uint i;

  printf("%s\n",(char *) text);
  for(i=0; i<=textlen; i++)
  {
    if(checkuedist(text,(Sint) i,text+i,(Sint) (textlen-i)) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

MAINFUNCTION
{
  Scaninteger textlen;
  char *alphabet;
  
  CHECKARGNUM(3,"alphabet len");

  DEBUGLEVELSET;
  
  alphabet = argv[1];
  if(sscanf(argv[2],"%ld",&textlen) != 1 || textlen <= 0)
  {
    fprintf(stderr,"textlen<=0 not allowed\n");
    exit(EXIT_FAILURE);
  }
  if(applyall(alphabet,(Uint) textlen,NULL,applycheckuedist) != 0)
  {
    STANDARDMESSAGE;
  }
  return EXIT_SUCCESS;
}
#endif
#endif
