#ifndef GEXTENDDEF_H
#define GEXTENDDEF_H

#include "frontdef.h"
#include "arraydef.h"

#define STATICFRONTCELLS 10
#define INCFRONTCELLS    4

typedef Sint Frontvalue;

DECLAREARRAYSTRUCT(Frontvalue);

typedef struct
{
  Frontspec fspecspace[STATICFRONTCELLS];
  Frontvalue frontspace[STATICFRONTCELLS * STATICFRONTCELLS];
  ArrayFrontspec frontspecreservoir;
  ArrayFrontvalue frontvaluereservoir;
} Greedyalignreservoir;

void initgreedyextendreservoir(Greedyalignreservoir *);
void wrapgreedyextendreservoir(Greedyalignreservoir *);

#endif
