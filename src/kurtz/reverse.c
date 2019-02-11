
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "errordef.h"
#include "debugdef.h"
#include "chardef.h"

#define ASSIGNTRANSFORMEDCOMPLEMENT(VAL,CC)\
        if(ISSPECIAL(CC))\
        {\
          VAL = CC;\
        } else\
        {\
          if((CC) > (Uchar) 3)\
          {\
            ERROR1("complement of %lu undefined",(Showuint) CC);\
            return (Sint) -1;\
          }\
          VAL = ((Uchar) 3) - (CC);\
        }

void reverseinplace(Uchar *s,Uint len)
{
  Uchar *front, *back, tmp;

  for(front = s, back = s + len - 1; front < back; front++, back--)
  {
    tmp = *front;
    *front = *back;
    *back = tmp;
  }
}

Sint reversecomplementinplace(Uchar *s,Uint len)
{
  Uchar *front, *back, cc, tmp = 0;

  for(front = s, back = s + len - 1; front <= back; front++, back--)
  {
    cc = *front;
    ASSIGNTRANSFORMEDCOMPLEMENT(tmp,cc);
    cc = *back;
    ASSIGNTRANSFORMEDCOMPLEMENT(*front,cc);
    *back = tmp;
  }
  return 0;
}

Sint onlycomplement(Uchar *s,Uint len)
{
  Uchar cc, *sptr;

  for(sptr = s; sptr < s + len; sptr++)
  {
    cc = *sptr;
    if(ISNOTSPECIAL(cc))
    {
      if(cc > (Uchar) 3)
      {
        ERROR1("complement of %lu undefined",(Showuint) cc);
        return (Sint) -1;
      }
      *sptr = ((Uchar) 3) - cc; 
    }
  }
  return 0;
}

void reversestring(char *s, Uint m, char *sreverse)
{
  char *sp;

  for(sreverse += m-1, *(sreverse+1) = '\0', sp = s; 
      *sp != '\0'; 
      *sreverse-- = *sp++)
    /*NOTHING*/;
  sreverse++;
}

Sint reversecomplement(Uchar *dest,Uchar *src,Uint len)
{
  Uchar *front, *back, cc;

  for(front = dest, back = src + len - 1; front < dest + len; front++, back--)
  {
    cc = *back;
    ASSIGNTRANSFORMEDCOMPLEMENT(*front,cc);
  }
  return 0;
}
