//\IgnoreLatex{

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "errordef.h"
#include "debugdef.h"
#include "spacedef.h"

//}

/*
  This file defines a generic type for queues.
*/

typedef struct 
{
  Stackelem *spaceStackelem; // the space to store the stack elements
  Uint nextfreeStackelem,    // points to top+1
       allocatedStackelem;   // size of Stack
#ifdef DEBUG
  void(*showelem)(Stackelem);
  Uint maxstacksize;
#endif
} Genericstack;              // \Typedef{Queue}

/*EE
  The following function delivers an empty queue with a reservoir of
  \texttt{size} elements to be stored in the queue. The 
  reservoir can, if necessary, be enlarged.
*/

void emptygenericStack(Genericstack *stack,Uint stacksize)
{
  if(stacksize == 0)
  {
    fprintf(stderr,"emptygenericStack(stacksize=%lu) not allowed\n",
                    (Showuint) stacksize);
    exit(EXIT_FAILURE);
  }
  DEBUG1(3,"#emptystack(%lu)\n",(Showuint) stacksize);
#ifdef DEBUG
  stack->maxstacksize = 0;
#endif
  ALLOCASSIGNSPACE(stack->spaceStackelem,NULL,Stackelem,stacksize);
  stack->nextfreeStackelem = 0;
  stack->allocatedStackelem = stacksize;
}

/*EE
  The following function returns true iff the queue is empty.
*/

BOOL stackisempty(Genericstack *stack)
{
  if(stack->nextfreeStackelem == 0)
  {
    DEBUG0(3,"# stackisempty=True\n");
    return True;
  } 
  DEBUG0(3,"# stackisempty=False\n");
  return False;
}

/*EE
  The following function adds an element \texttt{elem} to the end of 
  the queue.
*/
  
void pushGenericstack(Genericstack *stack,Stackelem elem)
{
  DEBUG0(3,"# enqueue of ");
  DEBUGCODE(3,stack->showelem(elem));
  DEBUG0(3,"\n");
  if(stack->nextfreeStackelem == stack->allocatedStackelem)
  {
    stack->allocatedStackelem += UintConst(16);
    ALLOCASSIGNSPACE(stack->spaceStackelem,stack->spaceStackelem,
                     Stackelem,stack->allocatedStackelem);
  }
  stack->spaceStackelem[stack->nextfreeStackelem++] = elem;
#ifdef DEBUG
  if(stack->nextfreeStackelem > stack->maxstacksize)
  {
    stack->maxstacksize = stack->nextfreeStackelem;
  }
#endif
}

Stackelem popGenericstack(Genericstack *stack)
{
  Stackelem value;

  if(stack->nextfreeStackelem == 0)
  {
    fprintf(stderr,"stack is empty\n");
    exit(EXIT_FAILURE);
  } 
  stack->nextfreeStackelem--;
  value = stack->spaceStackelem[stack->nextfreeStackelem];
  DEBUG0(3,"# popGenericstack of ");
  DEBUGCODE(3,stack->showelem(value));
  DEBUG0(3,"\n");
  return value;
}

/*EE
  The following function frees the space required for the queue. 
*/

void wrapGenericstack(Genericstack *stack)
{
  DEBUG2(3,"# wrapGenericstack(maxstacksize=%lu,stacksize=%lu)\n",
            (Showuint) stack->maxstacksize,
            (Showuint) stack->nextfreeStackelem);
  FREESPACE(stack->spaceStackelem);
}

//\IgnoreLatex{

#ifdef DEBUG

#define SHOWSTACKELEM(X) stack->showelem(stack->spaceStackelem[X])

void showGenericstack(Genericstack *stack)
{
  Uint i;

  printf("stack=");
  if(stack->nextfreeStackelem == 0)
  {
    printf("[]");
  } else
  {
    for(i=0; i<stack->nextfreeStackelem; i++)
    {
      SHOWSTACKELEM(i);
    }
  }
  (void) putchar('\n');
}
#endif

//}
