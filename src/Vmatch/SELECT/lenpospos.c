#include "select.h"

Sint selectmatch(Alphabet *alpha,
                 Multiseq *virtualmultiseq,
                 Multiseq *querymultiseq,
                 StoreMatch *storematch)
{
  printf("%lu %lu %lu\n", (Showuint) storematch->Storelength1,
                          (Showuint) storematch->Storeposition1,
                          (Showuint) storematch->Storeposition2);
  return 0;
}
