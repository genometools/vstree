#include "chaincall.h"
#include "match.h"
#include "multidef.h"

typedef struct
{
  Chaincallinfo *chaincallinfo;
  void *voidoutinfo;
  StoreMatch *storematchtab;
  Uint mapsize,
       maxlen,
       allowederrors1,
       allowederrors2,
       stretch1,
       stretch2,
       totalmatchdistance,
       supermatchcounter;
  Multiseq *outvms,
           *outqms;
  Processmatch *finalprocessthread;
} Vmchainthreadinfo;
