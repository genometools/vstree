//\IgnoreLatex{

#ifndef QUERYEXT_H
#define QUERYEXT_H

/*
  The following type is used for functions extending an interval in the
  enhanced suffix tree to the left and to the right.
*/

#include "types.h"

typedef Sint (*Querymatchextendfunction)(Uint maxintvleft,
                                         Uint maxintvright,
                                         Uint maxlcp,
                                         Uint witness,
                                         Uchar leftchar,
                                         Uint left,
                                         Uint right,
                                         void *info,
                                         Uchar *qsubstring,
                                         Uchar *qseqptr,
                                         Uint qseqlen,
                                         Uint qseqnum);


//\IgnoreLatex{

#endif

//}
