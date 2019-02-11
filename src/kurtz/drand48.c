#include <stdlib.h>
#include "drand48.h"

#ifdef _WIN32
void srand48(long int seedval)
{
  srand(seedval);
}

/* we simulate drand48 with() rand() */
double drand48(void)
{
  return  (double) rand() / RAND_MAX;
}
#endif
