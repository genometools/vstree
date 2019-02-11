#ifndef GALIGNDEF_H
#define GALIGNDEF_H

#include "frontdef.h"
#include "alignment.h"

#define STATICFRONTCELLS 10
#define INCFRONTCELLS    4

typedef struct
{
  Frontspec fspecspace[STATICFRONTCELLS];
  Frontvalue frontspace[STATICFRONTCELLS*STATICFRONTCELLS];
  ArrayFrontspec frontspecreservoir;
  ArrayFrontvalue frontvaluereservoir;
  ArrayEditoperation alignment;
} Greedyalignreservoir;

void initgreedyalignreservoir(Greedyalignreservoir *);
void wraptgreedyalignreservoir(Greedyalignreservoir *);

#endif
