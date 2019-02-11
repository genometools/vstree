#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"

void kmercode2string(char *buffer,
                     Uint code,
                     Uint numofchars,
                     Uint kmersize,
                     const char *characters)
{
  Sint i;
  Uint cc, tmpcode = code;

  buffer[kmersize] = '\0';
  for(i=(Sint) (kmersize-1); i>=0; i--)
  {
    cc = tmpcode % numofchars;
    buffer[i] = (char) characters[cc];
    tmpcode = (tmpcode - cc) / numofchars;
  }
}
