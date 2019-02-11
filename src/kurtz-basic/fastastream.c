#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include "types.h"
#include "arraydef.h"
#include "debugdef.h"
#include "errordef.h"
#include "divmodmul.h"
#include "spacedef.h"
#include "inputsymbol.h"
#include "alphadef.h"

Sint getfastastream(Processbioseq processbioseq,
                    void *applyinfo,
                    FILE *inputstream)
{
  Fgetcreturntype currentchar;
  BOOL indesc = False, firstseq = True;
  ArrayUchar sequence, description;

  INITARRAY(&sequence,Uchar);
  INITARRAY(&description,Uchar);
  while((currentchar = fgetc(inputstream)) != EOF)
  {
    if(indesc)
    {
      STOREINARRAY(&description,Uchar,4096,(Uchar) currentchar);
      if(currentchar == NEWLINESYMBOL)
      {
        indesc = False;
      } 
    } else
    {
      if(currentchar == FASTASEPARATOR)
      {
        if(firstseq)
        {
          firstseq = False;
        } else
        {
          if(processbioseq(applyinfo,
                           sequence.spaceUchar,
                           sequence.nextfreeUchar,
                           description.spaceUchar,
                           description.nextfreeUchar) != 0)
          {
            return (Sint) -1;
          }
          sequence.nextfreeUchar = description.nextfreeUchar = 0;
        }
        indesc = True;
      } else
      {
        if(!isspace((Ctypeargumenttype) currentchar))
        {
          STOREINARRAY(&sequence,Uchar,4096,(Uchar) currentchar);
        }
      }
    }
  }
  if(firstseq)
  {
    ERROR0("no sequences in multiple fasta file");
    return (Sint) -2;
  }
  if(processbioseq(applyinfo,
                   sequence.spaceUchar,
                   sequence.nextfreeUchar,
                   description.spaceUchar,
                   description.nextfreeUchar) != 0)
  {
    return (Sint) -3;
  }
  FREEARRAY(&sequence,Uchar);
  FREEARRAY(&description,Uchar);
  return 0;
}

