//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "logbase.h"

//}

/*EE
  The following function computes \(\lceil\log_{2}v\rceil\).
*/

Uint ceillog2(Uint v)
{
  if(v <= UintConst(2))
  {
    return UintConst(1);
  } 
  return (Uint) ceil(LOG2((double) v));
}
