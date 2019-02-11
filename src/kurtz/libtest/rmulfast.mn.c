#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "args.h"
#include "alphadef.h"
#include "multidef.h"
#include "debugdef.h"
#include "errordef.h"
#include "genfile.h"
#include "spacedef.h"

#include "multiseq.pr"
#include "multiseq-adv.pr"
#include "alphabet.pr"

MAINFUNCTION
{
  Alphabet alpha;
  Multiseq multiseq;
  Uchar *text;
  Scaninteger linewidth;
  Uint textlen;
  BOOL withdesc = True, showseqnum;

  CHECKARGNUM(4,"linewidth multiplefastfile showseqnum");
  DEBUGLEVELSET;
  if(sscanf(argv[1],"%ld",&linewidth) != 1)
  {
    fprintf(stderr,"%s: cannot parse integer from argument \"%s\"\n",
                   argv[0],argv[1]);
    return EXIT_FAILURE;
  }
  if(linewidth < 0)
  {
    withdesc = False;
    linewidth *= -1;
  }
  if(strcmp(argv[3],"True") == 0)
  {
    showseqnum = True;
  } else
  {
    if(strcmp(argv[3],"False") == 0)
    {
      showseqnum = False;
    } else
    {
      fprintf(stderr,"Third argument must be true or false\n");
      return EXIT_FAILURE;
    }
  }
  text = (Uchar *) CREATEMEMORYMAP(argv[2],True,&textlen);
  if(text == NULL)
  {
    fprintf(stderr,"%s %s %s: %s",argv[0],argv[1],argv[2],messagespace());
    return EXIT_FAILURE;
  }
  multiseq.originalsequence = NULL;
  multiseq.totalnumoffiles = multiseq.numofqueryfiles = 0;
  assignDNAalphabet(&alpha);
  if(readmultiplefastafile(&alpha,withdesc,&multiseq,text,textlen) != 0)
  {
    fprintf(stderr,"%s %s %s: %s",argv[0],argv[1],argv[2],messagespace());
    return EXIT_FAILURE;
  }
/*
  if(lengthdistribution(stdout,&multiseq) != 0)
  {
    fprintf(stderr,"%s %s %s: %s",argv[0],argv[1],argv[2],messagespace());
    return EXIT_FAILURE;
  }
*/
  if(showmultiplefasta(stdout,False,(Uint) linewidth,
                       &alpha,&multiseq,showseqnum) != 0)
  {
    fprintf(stderr,"%s %s %s: %s",argv[0],argv[1],argv[2],messagespace());
    return EXIT_FAILURE;
  }
  freemultiseq(&multiseq);
  return EXIT_SUCCESS;
}
