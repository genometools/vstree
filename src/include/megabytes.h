//\IgnoreLatex{

#ifndef MEGABYTES_H
#define MEGABYTES_H
#include "types.h"

//}

/*
  The following macro transforms bytes into megabytes.
*/

#define MEGABYTES(V)  ((double) (V)/((UintConst(1) << 20) - 1))

//\IgnoreLatex{

#endif

//}
