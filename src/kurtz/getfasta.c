

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "types.h"
#include "arraydef.h"
#include "debugdef.h"
#include "errordef.h"

/* 
  read the sequence part from a file in fasta format:
  The first line must begin with the symbol >. If it does, then 
  it is ignored. The remaining lines are scanned for alphanumeric
  characters which make up the new sequence. White spaces are ignored.
  No other characters are allowed.
*/

Sint getfasta(Uchar *seq, Uint seqlen, Uchar *newseq, Uint *newseqlen)
{
  Uchar *seqptr = seq, *newptr = newseq;

  if(*seqptr != '>')
  {
    ERROR1("fasta file does not start with character '%c'",'>');
    return (Sint) -1;
  }
  for(seqptr++; seqptr < seq + seqlen && *seqptr != '\n'; seqptr++)
    /* Nothing */ ;
  while(seqptr < seq + seqlen)
  {
    if(isalpha((Ctypeargumenttype) *seqptr) || *seqptr == '*')
    {
      *newptr++ = *seqptr++;
    } else
    {
      if(isspace((Ctypeargumenttype) *seqptr))
      {
        seqptr++;
      } else
      {
        ERROR1("character '%c' not allowed in sequence",*seqptr);
        return (Sint) -2;
      }
    }
  }
  *newseqlen = (Uint) (newptr - newseq);
  return 0;
}
