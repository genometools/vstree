#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <math.h>
#include "types.h"

Uint vm_determinenumofcodes(Uint numofchars,Uint prefixlength)
{
  return (Uint) pow((double) numofchars,(double) prefixlength);
}

