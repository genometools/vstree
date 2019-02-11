//\IgnoreLatex{

#ifndef CMP_TABDEF_H
#define CMP_TABDEF_H
#include "types.h"

#ifdef __cplusplus
  extern "C" {
#endif

//}

void compareUinttab(char *tag,Uint *tab1,Uint *tab2,Uint len);
void compareSinttab(char *tag,Sint *tab1,Sint *tab2,Uint len);
void compareUchartab(char *tag,Uchar *tab1,Uchar *tab2,Uint len);

//\IgnoreLatex{

#ifdef __cplusplus
}
#endif

#endif

//}
