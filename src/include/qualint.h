
#ifndef QUALINT_H
#define QUALINT_H

#include "types.h"

#define BESTCHARACTER        'b'
#define PERCENTAWAYCHARACTER 'p'

typedef enum
{
  Qualabsolute,
  Qualpercentaway,
  Qualbestof
} Qualificationtag;

typedef struct
{
  Qualificationtag qualtag;
  Uint integervalue;
} Qualifiedinteger;

#endif
