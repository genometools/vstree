

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include "types.h"
#include "dpbitvec48.h"
#include "failures.h"
#include "chardef.h"

#define GETEQS      getEqs4
#define GETEQSREV   getEqsrev4
#define DPBITVECTOR DPbitvector4

#include "getEqs.gen"

#undef GETEQS
#undef GETEQSREV
#undef DPBITVECTOR

#define GETEQS      getEqs8
#define GETEQSREV   getEqsrev8
#define DPBITVECTOR DPbitvector8

#include "getEqs.gen"
