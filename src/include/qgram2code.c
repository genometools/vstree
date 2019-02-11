#ifndef QGRAM2CODE_C
#define QGRAM2CODE_C
#include "types.h"
#include "divmodmul.h"
#include "chardef.h"

static BOOL qgram2code(Uint *code,Uint numofchars,Uint qvalue,
                       const Uchar *qgram)
{
  Uint i, tmpcode;
  Uchar a;

  a = qgram[0];
  if(ISSPECIAL(a))
  {
    return False;
  }
  tmpcode = (Uint) a;
  for(i=UintConst(1); i < qvalue; i++)
  {
    a = qgram[i];
    if(ISSPECIAL(a))
    {
      return False;
    }
    if(numofchars == DNAALPHASIZE)
    {
      tmpcode = MULT4(tmpcode) | ((Uint) a);
    } else
    {
      tmpcode *= numofchars;
      tmpcode += (Uint) a;
    }
  }
  *code = tmpcode;
  return True;
}
#endif
