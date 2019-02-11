//\IgnoreLatex{

#ifndef DPBITVEC_H
#define DPBITVEC_H

#include <stdlib.h>
#include <limits.h>
#include "types.h"

//}

/*
  This file defines some macros related to bitvectors used for dynamic
  programming.
*/

/*
  The following macro check if the pattern length exceeds
  \texttt{DPWORDSIZE}.
*/

#define CHECKWORDSIZE(S,PLEN)\
        if(ISLARGEPATTERN(PLEN))\
        {\
          fprintf(stderr,"%s length = %lu must be <= %lu = wordlength\n",\
                          S,(Showuint) (PLEN),(Showuint) DPWORDSIZE);\
          exit(EXIT_FAILURE);\
        }

#define UPDATECOLUMNSCORE(SC)\
        if (Ph & Ebit)\
        {\
          (SC)++;\
        } else\
        {\
          if (Mh & Ebit)\
          {\
            (SC)--;\
          }\
        }

#ifdef __cplusplus
}
#endif

//\IgnoreLatex{

#endif

//}
