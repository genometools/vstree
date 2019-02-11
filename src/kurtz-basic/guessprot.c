#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <ctype.h>
#include "types.h"
#include "inputsymbol.h"
#include "genstream.h"

BOOL guessifproteinsequence(Uchar *input,Uint inputlen)
{
  Uchar current, *inputptr;
  Uint countnonbases = 0, 
       countcharacters = 0, 
       readnumoffirstcharacters,
       leastnumofnonbases;
  BOOL indesc = False;

  if(inputlen < UintConst(1000))
  {
    readnumoffirstcharacters = inputlen;
  } else
  {
    readnumoffirstcharacters = UintConst(1000);
  }
  leastnumofnonbases = readnumoffirstcharacters/10;
  for(inputptr = input; countnonbases < leastnumofnonbases &&
                        countcharacters < readnumoffirstcharacters &&
                        inputptr < input + inputlen; inputptr++)
  {
    current = *inputptr;
    if(indesc)
    {
      if(current == NEWLINESYMBOL)
      {
        indesc = False;
      } 
    } else
    {
      if(current == FASTASEPARATOR)
      {
        indesc = True;
      } else
      {
        if(!isspace((Ctypeargumenttype) current))
        {
          countcharacters++;
          switch(current)
          {
            case 'L':
            case 'I':
            case 'F':
            case 'E':
            case 'Q':
            case 'P':
            case 'X':
            case 'Z':
              countnonbases++;
              break;
            default:
              break;
          }
        }
      }
    }
  }
  if(countnonbases >= leastnumofnonbases)
  {
    return True;
  }
  return False;
}

BOOL vm_guessifproteinsequencestream(Genericstream *inputstream)
{
  Fgetcreturntype currentchar;
  Uint countnonbases = 0, 
       countcharacters = 0; 
  BOOL indesc = False;

  for(;;)
  {
    if(inputstream->isgzippedstream)
    {
      currentchar = gzgetc(inputstream->stream.gzippedstream);
    } else
    {
      currentchar = fgetc(inputstream->stream.fopenstream);
    }
    if(indesc)
    {
      if(currentchar == NEWLINESYMBOL)
      {
        indesc = False;
      } 
    } else
    {
      if(currentchar == FASTASEPARATOR)
      {
        indesc = True;
      } else
      {
        if(!isspace((Ctypeargumenttype) currentchar))
        {
          countcharacters++;
          switch(currentchar)
          {
            case 'L':
            case 'I':
            case 'F':
            case 'E':
            case 'Q':
            case 'P':
            case 'X':
            case 'Z':
              countnonbases++;
              break;
            default:
              break;
          }
        }
      }
    }
    if(countcharacters >= UintConst(1000))
    {
      break;
    }
  }
  if(countnonbases >= countcharacters/10)
  {
    return True;
  }
  return False;
}
