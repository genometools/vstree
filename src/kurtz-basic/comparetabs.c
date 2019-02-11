#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "types.h"

#define COMPAREFUNCTION       compareUinttab
#define COMPARETYPE           Uint
#define COMPAREFORMAT(FP,VAL) fprintf(FP,"%lu",(Showuint) (VAL))

#include "cmp-gentab.c"

#undef COMPAREFUNCTION
#undef COMPARETYPE
#undef COMPAREFORMAT

#define COMPAREFUNCTION       compareSinttab
#define COMPARETYPE           Sint
#define COMPAREFORMAT(FP,VAL) fprintf(FP,"%ld",(Showsint) (VAL))

#include "cmp-gentab.c"

#undef COMPAREFUNCTION
#undef COMPARETYPE
#undef COMPAREFORMAT

#define COMPAREFUNCTION       compareUchartab
#define COMPARETYPE           Uchar
#define COMPAREFORMAT(FP,VAL) fprintf(FP,"%lu",(Showuint) (VAL))

#include "cmp-gentab.c"
