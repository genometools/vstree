
#include <math.h>
#include "types.h"
#include "outinfo.h"

void assignvirtualdigits(Digits *digits,Multiseq *multiseq)
{
  Uint dblen = DATABASELENGTH(multiseq);

  if(dblen < UintConst(1000))
  {
    digits->length = UintConst(2);
  } else
  {
    if(dblen < UintConst(10000))
    {
      digits->length = UintConst(3);
    } else
    {
      if(dblen < UintConst(100000))
      {
        digits->length = UintConst(4);
      } else
      {
        digits->length = UintConst(5);
      }
    }
  }
  digits->position1 = UintConst(1) + (Uint) log10((double) dblen);
  digits->seqnum1
    = UintConst(1) + (Uint) log10((double) NUMOFDATABASESEQUENCES(multiseq));
/*
  Copy the values. If there are query sequences, then position2 and
  seqnum2 are overwritten later.
*/
  digits->position2 = digits->position1;
  digits->seqnum2 = digits->seqnum1;
}

void assignquerydigits(Digits *digits,Multiseq *querymultiseq)
{
  digits->position2 = UintConst(1) + 
                     (Uint) log10((double) querymultiseq->totallength);
  digits->seqnum2 = UintConst(1) + 
                    (Uint) log10((double) querymultiseq->numofsequences);
}
