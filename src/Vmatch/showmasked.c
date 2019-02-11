
#include <stdio.h>
#include <ctype.h>
#include "intbits.h"
#include "debugdef.h"
#include "chardef.h"
#include "inputsymbol.h"
#include "errordef.h"
#include "fhandledef.h"
#include "outinfo.h"

#include "checkonoff.pr"
#include "filehandle.pr"

#define SHOWDESCRIPTION(SEQNUM)\
        if(multiseq->descspace.spaceUchar != NULL)\
        {\
          desclength = DESCRIPTIONLENGTH(multiseq,SEQNUM);\
          if(WRITETOFILEHANDLE(DESCRIPTIONPTR(multiseq,SEQNUM),\
                               (Uint) sizeof(Uchar),\
                               desclength,\
                               outfp) != 0)\
          {\
            return (Sint) -2;\
          }\
        } else\
        {\
          (void) putc('\n',outfp);\
        }

#define SHOWSTARSYMBOL(LU)\
        if(c == '*')\
        {\
          (void) putc('*',outfp);\
        } else\
        {\
          ERROR2("cannot convert character %c to %s case",c,#LU);\
          return (Sint) -4;\
        }

Sint showmaskedseq(BOOL transform,
                   FILE *outfp,
                   Uint linewidth,
                   Uchar *characters,
                   Multiseq *multiseq,
                   Uint *markmatchtable,
                   Uchar maskchar)
{
  Uint totallengthwithoutsep, 
       i, column = 0, seqnum, countmasked = 0, desclength;
  Uchar c;
  Sint retval;
  FILE *outmessage;

  if(multiseq->totallength == 0)
  {
    ERROR0("cannot format empty sequence");
    return (Sint) -1;
  }
  (void) putc(FASTASEPARATOR,outfp);
  SHOWDESCRIPTION(0);
  for(i=0, seqnum = 0; /* Nothing */ ; i++)
  {
    c = multiseq->originalsequence[i];
    if(transform)
    {
      c = characters[c];
    }
    if(c == SEPARATOR)
    {
      if(column > 0)
      {
        (void) putc('\n',outfp);
      }
      (void) putc(FASTASEPARATOR,outfp);
      column=0;
      seqnum++;
      SHOWDESCRIPTION(seqnum);
    } else
    {
      if(ISIBITSET(markmatchtable,i))
      {
        countmasked++;
        if(maskchar == MASKTOUPPER)
        {
          if(islower((Ctypeargumenttype) c))
          {
            (void) putc(toupper((Ctypeargumenttype) c),outfp);
          } else
          {
            SHOWSTARSYMBOL(upper);
          }
        } else
        {
          if(maskchar == MASKTOLOWER)
          {
            if(isupper((Ctypeargumenttype) c))
            {
              (void) putc(tolower((Ctypeargumenttype) c),outfp);
            } else
            {
              SHOWSTARSYMBOL(lower);
            }
          } else
          {
            (void) putc((Fputcfirstargtype) maskchar,outfp);
          }
        }
      } else
      {
        (void) putc((Fputcfirstargtype) c,outfp);
      }
      if(i == multiseq->totallength - 1)
      {
        (void) putc('\n',outfp);
        break;
      }
      column++;
      if(column >= linewidth)
      {
        (void) putc('\n',outfp);
        column = 0;
      } 
    }
  }
  totallengthwithoutsep = multiseq->totallength - (multiseq->numofsequences-1);
  retval = checkenvvaronoff("VMATCHCOMMENTTOSTDOUT");
  if(retval < 0)
  {
    return (Sint) -2;
  }
  if(retval == 1)
  {
    outmessage = stdout;
  } else
  {
    outmessage = stderr;
  }
  fprintf(outmessage,"# sequence length: %lu, number of masked symbols: "
                     "%lu (%.2f percent of the sequences)\n",
                     (Showuint) totallengthwithoutsep,
                     (Showuint) countmasked,
                     100.0 * (double) countmasked / totallengthwithoutsep);
  return 0;
}
