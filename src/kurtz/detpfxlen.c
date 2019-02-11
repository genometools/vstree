#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "types.h"
#include "intbits.h"
#include "errordef.h"
#include "debugdef.h"

/*
  We need \texttt{prefixlenbits} bits to store the length of
  a matching prefix. So we can store the following maximal value 
  in the remaining bits.
*/

#define MAXREMAININGAFTERPREFIXLEN(PFXLENBITS)\
        ((UintConst(1) << (INTWORDSIZE-(PFXLENBITS))) - 1)

/*
  We allow to choose the prefixlength \(l\) in such a way that the size of
  table bcktab (which is $8\cdot k^{l}$ never exceeds the 
  $\texttt{MAXMULTIPLIEROFTOTALLENGTH} \cdot n$, where \(k\) is the size
  of the alphabet and \(n\) is the total length of the input sequence.
*/

#define MAXMULTIPLIEROFTOTALLENGTH 4  

static Uint logalphasize(Uint numofchars,double value)
{
  Uint retval;
  double logtmp1, logtmp2;

  if(value <= (double) numofchars)
  {
    return UintConst(1);
  }
  logtmp1 = log(value);
  logtmp2 = log((double) numofchars);
  DEBUG2(3,"value=%.20f, numofchars=%lu\n",value,(Showuint) numofchars);
  DEBUG2(3,"logtmp1=%.20f, logtmp2=%.20f\n",logtmp1,logtmp2);
  DEBUG1(3,"logtmp1/logtmp2=%.20f\n",logtmp1/logtmp2);
  retval = (Uint) floor(logtmp1/logtmp2);
  DEBUG3(2,"logalphabetsize(%lu,%.20f)=%lu\n",(Showuint) numofchars,
                                              value,
                                              (Showuint) retval);
  return retval;
}

Uint vm_recommendedprefixlength(Uint numofchars,
                             Uint totallength,
                             Uint sizeofbckentry)
{
  Uint prefixlength;

  prefixlength = logalphasize(numofchars,
                              (double) totallength/sizeofbckentry);
  return (prefixlength == 0) ? UintConst(1) : prefixlength;
}

Uint vm_whatisthemaximalprefixlength(Uint numofchars,
                                  Uint totallength,
                                  Uint sizeofbckentry,
                                  Uint prefixlenbits)
{
  Uint maxprefixlen, tmplength;

  maxprefixlen = logalphasize(numofchars,
                           (double) totallength/
                                (sizeofbckentry/MAXMULTIPLIEROFTOTALLENGTH));
  if(prefixlenbits > 0)
  {
    tmplength = logalphasize(numofchars,
                             (double) 
                             MAXREMAININGAFTERPREFIXLEN(prefixlenbits));
    if(maxprefixlen > tmplength)
    {
      maxprefixlen = tmplength;
    }
    tmplength = MAXVALUEWITHBITS(prefixlenbits);
    if(maxprefixlen > tmplength)
    {
      maxprefixlen = tmplength;
    }
  }
  return maxprefixlen;
}

Sint vm_checkprefixlength(Uint maxprefixlen,Uint prefixlength)
{
  if(maxprefixlen < prefixlength)
  {
    ERROR2("prefix length %lu is too large, maximal prefix length "
           "for this input size and alphabet size is %lu",
           (Showuint) prefixlength,
           (Showuint) maxprefixlen);
    return (Sint) -1;
  }
  return 0;
}

void vm_showmaximalprefixlength(Showverbose showverbose,Uint maxprefixlen,
                             Uint recommended)
{
  if(showverbose != NULL)
  {
    char verbosebuf[256];
    sprintf(&verbosebuf[0],
            "for this input size and alphabet size, the maximal prefixlength\n"
            "(argument of option -pl) is %lu,\n"
            "the recommended prefixlength is %lu",
                            (Showuint) maxprefixlen,
                            (Showuint) recommended);
    showverbose(&verbosebuf[0]);
  }
}
