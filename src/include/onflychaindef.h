#ifndef ONFLYCHAIN_H
#define ONFLYCHAIN_H

#include "types.h"
#include "arraydef.h"
#include "gqueue-if.h"

#define UNDEFPREVIOUS           NULL

typedef Sint Chainscoretype;

typedef struct _fragment
{
  Uint startpos[2], 
       fraglength,
       lengthofchain;
  struct _fragment *previousinchain,
                   *firstinchain,
                   *bestchainend;
  Chainscoretype score;
  BOOL islast;
#ifdef DEBUG
  Uint identity;
#endif
} Onflyfragment;

DECLAREARRAYSTRUCT(Onflyfragment);

typedef Onflyfragment * Onflyfragmentptr;

DECLAREARRAYSTRUCT(Onflyfragmentptr);

typedef struct
{
  Genericqueue *lastelements;
  ArrayOnflyfragmentptr readyforoutput;
  void *dictroot;
  BOOL overqueue;
#ifdef DEBUG
  Uint currentidentity;
#endif
} Maintainedfragments;

typedef Sint (*Outflychain)(Onflyfragment *,void *info);

#endif
