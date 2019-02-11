
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/uio.h>
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <unistd.h>
#include "types.h"
#include "errordef.h"
#include "spacedef.h"
#include "arraydef.h"

/*
  Map a file into the memory. For each line, compute the starting address
  of the line and its length.
*/

Sint file2lines(ArrayStrings *as,char *filename)
{
  Uint i, linecount = 0, lencount, textlen;
  Uchar *tptr, *text;

  text = (Uchar *) CREATEMEMORYMAP(filename,True,&textlen);
  if(text == NULL)
  {
    return (Sint) -1;
  }
  for(tptr = text; tptr < text + textlen; tptr++)
  {
    if(*tptr == '\n')
    {
      linecount++;
    }
  }
  as->allocatedStrings = linecount+1;
  as->nextfreeStrings = 0;
  ALLOCASSIGNSPACE(as->spaceStrings,NULL,Stringtype,linecount+1);
  as->stringbuffer = text;
  as->stringbufferlength = textlen;
  as->spaceStrings[as->nextfreeStrings].start = 0;
  lencount = 0;
  for(i=0; i < textlen; i++)
  {
    if(text[i] == '\n')
    {
      as->spaceStrings[as->nextfreeStrings++].length = lencount;
      as->spaceStrings[as->nextfreeStrings].start = i + 1;
      lencount = 0;
      text[i] = (Uchar) '\0';
    } else
    {
      lencount++;
    }
  }
  return 0;
}
