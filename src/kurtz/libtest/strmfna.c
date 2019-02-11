#include <stdio.h>
#include "args.h"
#include "types.h"
#include "alphadef.h"
#include "spacedef.h"
#include "genfile.h"
#include "debugdef.h"
#include "errordef.h"
#include "fhandledef.h"
#include "multidef.h"

#include "fastastream.pr"
#include "filehandle.pr"
#include "multiseq-adv.pr"

#define USAGE "linewidth filename"

static Sint fastaformat(void *processinfo,
                        const Uchar *seq,
                        Uint seqlen,
                        const Uchar *desc,
                        Uint desclen)
{
  Uint *linewidth = (Uint *) processinfo;

  (void) putchar('>');
  if(fwrite(desc,sizeof(Uchar),(size_t) desclen,stdout) != (size_t) desclen)
  {
    return (Sint) -1;
  }
  if(formatseq(stdout,
               0,
               NULL,
               0,
               *linewidth,
               seq,
               seqlen) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

MAINFUNCTION
{
  FILE *inputstream;
  Scaninteger readint;
  Uint linewidth;

  CHECKARGNUM(3,USAGE);
  if(sscanf(argv[1],"%ld",&readint) != 1)
  {
    fprintf(stderr,"%s: %s\n",argv[0],USAGE);
    return EXIT_FAILURE;
  }
  linewidth = (Uint) readint;
  inputstream = CREATEFILEHANDLE(argv[2],READMODE);
  if(inputstream == NULL)
  {
    STANDARDMESSAGE;
  }
  if(getfastastream(fastaformat,
                    &linewidth,
                    inputstream) != 0)
  {
    STANDARDMESSAGE;
  }
  if(DELETEFILEHANDLE(inputstream) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
