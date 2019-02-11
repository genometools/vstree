

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "types.h"
#include "errordef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "failures.h"

#define ELEM                Uint
#define QSORTHEADER         void qsortUint(ELEM *l, ELEM *r)
#define ELEMGREATER(A,B)    (*(A) >  *(B))
#define ELEMGREATEREQ(A,B)  (*(A) >= *(B))

#include "qsort.gen"

#undef ELEM
#undef QSORTHEADER
#undef ELEMGREATER
#undef ELEMGREATEREQ

#define ELEM                ThreeUint
#define QSORTHEADER         void qsortThreeUintwrtuint2(ELEM *l, ELEM *r)
#define ELEMGREATER(A,B)    ((A)->uint2 > (B)->uint2)
#define ELEMGREATEREQ(A,B)  ((A)->uint2 >= (B)->uint2)

#include "qsort.gen"

#undef ELEM
#undef QSORTHEADER
#undef ELEMGREATER
#undef ELEMGREATEREQ

#define ELEM                PairUint
#define QSORTHEADER         void qsortPairUintwrtuint0(ELEM *l, ELEM *r)
#define ELEMGREATER(A,B)    ((A)->uint0 > (B)->uint0)
#define ELEMGREATEREQ(A,B)  ((A)->uint0 >= (B)->uint0)

#include "qsort.gen"

#undef ELEM
