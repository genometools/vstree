
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "spacedef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "failures.h"

//}

/*EE
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

void mkcldtabnextlIndex(Childinfo *cldtab,Uchar *lcptab,
                        ArrayPairUint *largelcpvalues,Uint totallength)
{
  ArrayStackelem stack;
  Uint i, currentlcp, exception = 0, value;
  DEBUGDECL(Uint countlarge = 0);
  DEBUGDECL(Uint countundef = 0);

  for(i=0; i <= totallength; i++)
  {
    cldtab[i].nextlIndex = UNDEFCHILDVALUE;
  }
  INITARRAY(&stack,Stackelem);
  PUSHMKSKIP((Uint) 0,(Uint) 0);
  NOTSUPPOSEDTOBENULL(largelcpvalues->spacePairUint);
  for(i=UintConst(1); i <= totallength; i++)
  {
    SEQUENTIALEVALLCPVALUEGENERIC(currentlcp,lcptab,largelcpvalues,i,exception);
    while(currentlcp < TOPMKSKIP.depth)
    {
      stack.nextfreeStackelem--;
    }
    if(currentlcp == TOPMKSKIP.depth)
    {
      value = i - TOPMKSKIP.step;
      if(value >= LARGECHILDVALUE)
      {
        cldtab[TOPMKSKIP.step].nextlIndex = LARGECHILDVALUE;
#ifdef DEBUG
        countlarge++;
#endif
        DEBUG1(3,"cldtab[%lu].nextlIndex=LARGECHILDVALUE\n",
                   (Showuint) TOPMKSKIP.step);
      } else
      {
        cldtab[TOPMKSKIP.step].nextlIndex = (Uchar) value;
        DEBUG2(3,"cldtab[%lu].nextlIndex=%lu\n",
                   (Showuint) TOPMKSKIP.step,(Showuint) value);
      }
    } 
    PUSHMKSKIP(currentlcp,i);
  }
  FREEARRAY(&stack,Stackelem);
#ifdef DEBUG
  countundef = 0;
  for(i=0; i<= totallength; i++)
  {
    if(cldtab[i].nextlIndex == UNDEFCHILDVALUE)
    {
      countundef++;
    }
  }
  printf("large(nextlIndex)=%lu(%.6f)\n",(Showuint) countlarge,
                            (double) countlarge/totallength);
  printf("undef(nextlIndex)=%lu(%.6f)\n",(Showuint) countundef,
                            (double) countundef/totallength);
  printf("large/defined(nextlIndex)=(%.6f)\n",
                            (double) countlarge/(totallength-countundef));
#endif
}

void mkcldtabupdown(Childinfo *cldtab,Uchar *lcptab,
                    ArrayPairUint *largelcpvalues,Uint totallength)
{
  ArrayStackelem stack;
  Uint i, currentlcp, exception = 0, value;
  Stackelem lastIndex;
  DEBUGDECL(Uint countlargedown = 0);
  DEBUGDECL(Uint countlargeup = 0);
  DEBUGDECL(Uint countundef = 0);

  for(i=0; i <= totallength; i++)
  {
    cldtab[i].up = UNDEFCHILDVALUE;
    cldtab[i].down = UNDEFCHILDVALUE;
  }
  lastIndex.step = UNDEFCHILDVALUE;
  INITARRAY(&stack,Stackelem);
  PUSHMKSKIP((Uint) 0,(Uint) 0);
  for(i=UintConst(1); i <= totallength; i++)
  {
    SEQUENTIALEVALLCPVALUEGENERIC(currentlcp,lcptab,largelcpvalues,i,exception);
    while(currentlcp < TOPMKSKIP.depth)
    {
      lastIndex.step = TOPMKSKIP.step;
      lastIndex.depth = TOPMKSKIP.depth;
      stack.nextfreeStackelem--;
      if(currentlcp <= TOPMKSKIP.depth && TOPMKSKIP.depth != lastIndex.depth)
      {
        value = lastIndex.step - TOPMKSKIP.step;
        if(value >= LARGECHILDVALUE)
        {
          cldtab[TOPMKSKIP.step].down = LARGECHILDVALUE;
#ifdef DEBUG
          countlargedown++;
#endif
          DEBUG1(3,"cldtab[%lu].down=LARGECHILDVALUE\n",
                   (Showuint) TOPMKSKIP.step);
        } else
        {
          cldtab[TOPMKSKIP.step].down = (Uchar) value;
          DEBUG2(3,"cldtab[%lu].down=%lu\n",
                   (Showuint) TOPMKSKIP.step,(Showuint) value);
        }
      }
    }
    if(currentlcp >= TOPMKSKIP.depth)
    {
      if(lastIndex.step != UNDEFCHILDVALUE)
      {
        value = i - lastIndex.step;
        if(value >= LARGECHILDVALUE)
        {
          cldtab[i].up = LARGECHILDVALUE;
#ifdef DEBUG
          countlargeup++;
#endif
          DEBUG1(3,"cldtab[%lu].up=LARGECHILDVALUE\n",
                   (Showuint) i);
        } else
        {
          cldtab[i].up = (Uchar) value;
          DEBUG2(3,"cldtab[%lu]=%lu\n",
                   (Showuint) i,
                   (Showuint) value);
        }
        lastIndex.step = UNDEFCHILDVALUE;
      }
    } 
    PUSHMKSKIP(currentlcp,i);
  }
  FREEARRAY(&stack,Stackelem);
#ifdef DEBUG
  countundef = 0;
  for(i=0; i<= totallength; i++)
  {
    if(cldtab[i].down == UNDEFCHILDVALUE)
    {
      countundef++;
    }
    if(i>0 && cldtab[i].up != UNDEFCHILDVALUE)
    {
      if(cldtab[i-1].nextlIndex != UNDEFCHILDVALUE)
      {
        fprintf(stderr,"cannot store cldtab[%lu].up = %lu in ",
                        (Showuint) i,(Showuint) cldtab[i].up);
        fprintf(stderr,"cldtab[%lu].nextlIndex=%lu\n",
                        (Showuint) (i-1),
                        (Showuint) cldtab[i-1].nextlIndex);
        exit(EXIT_FAILURE);
      }
    }
  }
  printf("large(down)=%lu(%.6f)\n",(Showuint) countlargedown,
                                   (double) countlargedown/totallength);
  printf("undef(down)=%lu(%.6f)\n",(Showuint) countundef,
                            (double) countundef/totallength);
  printf("large/defined(down)=(%.6f)\n",
                            (double) countlargedown/(totallength-countundef));
  printf("large(up)=%lu(%.6f)\n",(Showuint) countlargeup,
                            (double) countlargeup/totallength);
  printf("undef(up)=%lu(%.6f)\n",(Showuint) countundef,
                            (double) countundef/totallength);
  printf("large/defined(up)=(%.6f)\n",
                            (double) countlargeup/(totallength-countundef));
#endif
}

#define DECODENEXTLINDEX(IND)\
        (IND) + virtualtree->cldtab[IND].nextlIndex

#define DECODEUP(IND)\
        (IND) - virtualtree->cldtab[IND].up

#define DECODEDOWN(IND)\
        (IND) + virtualtree->cldtab[IND].down

/*
  The following function compresses the childtab into one byte for each entry.

  Input: A pointer to pre-allocated empty cprcldtab (compressed child tab)
  Input: virtualtree
  Output: written into the passed-by-ref comcldtab
*/

void compresscldtab(Virtualtree *virtualtree,Uchar *cldtab1)
{
  Uint i, next, nextlIndex, up, down, tmplcp1, tmplcp2;

  // Copying The defined nextlIndex to the cldtab1
 for(i=0; i < virtualtree->multiseq.totallength; i++)
 {
   nextlIndex = DECODENEXTLINDEX(i);
   if(nextlIndex>i)
   {
     cldtab1[i] = (Uchar) nextlIndex;
     if((nextlIndex-i)<LARGECHILDVALUE)
     {
	cldtab1[i] = (Uchar) (nextlIndex-i);
      } else
      {
	cldtab1[i] = LARGECHILDVALUE;
      }
    }
  }
 // Copying the non-redundant down fields into the cldtab1
  for(i = 0; i<virtualtree->multiseq.totallength; i++)
  {
    nextlIndex = DECODENEXTLINDEX(i);
    down = DECODEDOWN(i);
    next = nextlIndex;
    if(((next==i)&&(down>i))||(next==virtualtree->multiseq.totallength))
    {
      cldtab1[i] = (Uchar) down;
      if((down-i)<LARGECHILDVALUE)
      {
	cldtab1[i] = (Uchar)(down-i);
      } else
      {
	 cldtab1[i] = LARGECHILDVALUE;
      }
    }
  }
  // Copying the up fields in comcld[index-1]
  for(i=0; i<virtualtree->multiseq.totallength-1; i++)
  {
    EVALLCP(virtualtree,tmplcp1,i);
    EVALLCP(virtualtree,tmplcp2,i+1);
    if(tmplcp1 > tmplcp2)
    {
      up = DECODEUP(i+1);
      down = DECODEDOWN(i+1);
      if(up != down)
      {
	if(((i+1)-up) < LARGECHILDVALUE)
        {
	  cldtab1[i] = (Uchar)((i+1)-up);
	} else
        {
	  cldtab1[i] = LARGECHILDVALUE;
	}
      }
    }
  }
}
