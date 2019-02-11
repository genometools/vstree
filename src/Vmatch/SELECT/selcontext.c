#include "select.h"

Sint selectmatch(Alphabet *alpha,
                 Multiseq *virtualmultiseq,
                 Multiseq *querymultiseq,
                 StoreMatch *storematch)
{
  if(virtualmultiseq->sequence == NULL)
  {
    fprintf(stderr,"selectmatch: sequence is not read: use option -s");
    return -1;
  }
  if(virtualmultiseq->sequence[storematch->Storeposition1] == 0)
  {
    return 1;
  }
  return 0;
}
