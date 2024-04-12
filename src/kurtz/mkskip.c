//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "arraydef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "failures.h"

//}

/*EE
  This file implements a function to compute the skip table from
  the lcp table of the virtual suffix tree.
*/

/*
  We implement the stack by a dynamic array of the type 
  \texttt{Stackelem}, which stores the depth of a node of the virtual
  suffix tree and the step in which the node was created.
*/

typedef struct
{
  Uint depth,    // the depth of a node
       step;     // the step in the which the node was created
} Stackelem;

DECLAREARRAYSTRUCT(Stackelem);

/*
  The top and the push operation for the stack.
*/

#define TOPMKSKIP (stack.spaceStackelem[stack.nextfreeStackelem-1])
#define PUSHMKSKIP(D,S)\
        CHECKARRAYSPACE(&stack,Stackelem,128);\
        DEBUG2(3,"push(%lu,%lu)\n",(Showuint) (D),(Showuint) (S));\
        stack.spaceStackelem[stack.nextfreeStackelem].depth = D;\
        stack.spaceStackelem[stack.nextfreeStackelem].step = S;\
        stack.nextfreeStackelem++

#define COUNTSMALL(V)\
        smallvalue = (V) - TOPMKSKIP.step;\
        if(smallvalue < UCHAR_MAX)\
        { \
          sumsmall+=smallvalue;\
          countsmall++;\
        }

void makeskiptable(Uint *skiptable,Uchar *lcptab,ArrayPairUint *largelcps,
                   Uint totallength)
{
  register Uint i, currentlcp;
  Uint exception = 0;
  ArrayStackelem stack;
  Uint smallvalue;
  __attribute__ ((unused)) Uint sumsmall = 0, countsmall = 0;

  INITARRAY(&stack,Stackelem);
  PUSHMKSKIP((Uint) 0,(Uint) 0);
  for(i=UintConst(1); i <= totallength; i++)
  {
    if((currentlcp = (Uint) lcptab[i]) == UCHAR_MAX)
    {
      currentlcp = largelcps->spacePairUint[exception++].uint1;
    } 
    while(currentlcp < TOPMKSKIP.depth)
    {
      skiptable[TOPMKSKIP.step] = i-1;
      COUNTSMALL(i-1);
      stack.nextfreeStackelem--;
    }
    PUSHMKSKIP(currentlcp,i);
  }
  while(stack.nextfreeStackelem > 0)
  {
    skiptable[TOPMKSKIP.step] = totallength;
    COUNTSMALL(totallength);
    stack.nextfreeStackelem--;
  }
  FREEARRAY(&stack,Stackelem);
/*
  printf("countsmall=%lu(%.2f)\n",
             (Showuint) countsmall,(double) countsmall/totallength);
  printf("sumsmall=%.2f\n",(double) sumsmall/totallength);
  printf("averagesmall=%.2f\n",(double) sumsmall/totallength);
*/
}
