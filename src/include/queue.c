//\IgnoreLatex{

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "divmodmul.h"
#include "errordef.h"
#include "debugdef.h"
#include "spacedef.h"

//}

/*
  This file defines a generic type for queues.
*/

typedef struct 
{
 Queueelem *queuespace;  // the space to store the queue elements
 Uint enqueueindex,  // points to entry into which element is to be enqued
       dequeueindex,  // last element of queue
       queuesize,     // size of the queue
       noofelements;  // no ofelements between enqueueindex+1 and dequeindex
#ifdef DEBUG
  void(*showelem)(Queueelem);
  Uint maxqueuesize;
#endif
} Queue;              // \Typedef{Queue}

typedef Sint (*Queueprocessor)(Queueelem *,void *info);

#ifdef __cplusplus
extern "C" {
#endif

BOOL queueisempty(const Queue *q);
void enqueue(Queue *q,Queueelem elem);
Queueelem dequeue(Queue *q);
void wrapqueue(Queue *q);
void showqueue(const Queue *q);
Queueelem *headofqueue(const Queue *q);
Queueelem *tailofqueue(const Queue *q);
void deleteheadofqueue(Queue *q);
Sint overallqueuelements(const Queue *q,Queueprocessor queueprocessor,
                         void *info);

#ifdef __cplusplus
}
#endif

/*EE
  The following function delivers an empty queue with a reservoir of
  \texttt{size} elements to be stored in the queue. The 
  reservoir can, if necessary, be enlarged.
*/

static void emptyqueue(Queue *q,Uint queuesize)
{
  if(queuesize == 0)
  {
    fprintf(stderr,"emptyqueue(queuesize=%lu) not allowed\n",
                    (Showuint) queuesize);
    exit(EXIT_FAILURE);
  }
  DEBUG1(3,"#emptyqueue(%lu)\n",(Showuint) queuesize);
#ifdef DEBUG
  q->maxqueuesize = 0;
#endif
  ALLOCASSIGNSPACE(q->queuespace,NULL,Queueelem,queuesize);
  q->noofelements = 0;
  q->queuesize = queuesize;
  q->dequeueindex = q->enqueueindex = queuesize - 1;
}

/*EE
  The following function returns true iff the queue is empty.
*/

BOOL queueisempty(const Queue *q)
{
  if(q->noofelements == 0)
  {
    DEBUG0(3,"#queueisempty=True\n");
    return True;
  } 
  DEBUG0(3,"#queueisempty=False\n");
  return False;
}

/*EE
  The following function resizes the queue by doubling the 
  space reservoir. 
*/

static void resizequeue(Queue *q)
{
  Uint i, j, newsize;
 
  newsize = MULT2(q->queuesize); // double the size
  DEBUG1(3,"#resizequeue to %lu elements\n",(Showuint) newsize);
  ALLOCASSIGNSPACE(q->queuespace,q->queuespace,Queueelem,newsize);
  if(q->enqueueindex < q->dequeueindex)
  {
    j=q->queuesize;
    for(i=q->enqueueindex+1; i<=q->dequeueindex; i++)
    {
      q->queuespace[j++] = q->queuespace[i];
    }
  } else
  {
    j=q->queuesize;
    for(i=q->enqueueindex+1; i<=q->queuesize-1; i++)
    {
      q->queuespace[j++] = q->queuespace[i];
    }
    for(i=0; i<=q->dequeueindex; i++)
    {
      q->queuespace[j++] = q->queuespace[i];
    }
  }
  q->dequeueindex = newsize-1;
  q->enqueueindex = q->queuesize-1;
  q->queuesize = newsize;
}

/*EE
  The following function adds an element \texttt{elem} to the end of 
  the queue.
*/
  
void enqueue(Queue *q,Queueelem elem)
{
  DEBUG0(3,"#enqueue of ");
  DEBUGCODE(3,q->showelem(elem));
  DEBUG0(3,"\n");
  if(q->noofelements == q->queuesize)
  {
    resizequeue(q);
  }
  q->noofelements++;
#ifdef DEBUG
  if(q->noofelements > q->maxqueuesize)
  {
    q->maxqueuesize = q->noofelements;
  }
#endif
  q->queuespace[q->enqueueindex] = elem;
  if(q->enqueueindex > 0)
  {
    q->enqueueindex--;
  } else
  {
    q->enqueueindex = q->queuesize - 1;// dequeuindex smaller than queuesize - 1
  }
}

/*EE
  The following function removes the element \texttt{elem} from the
  start of the queue.
*/

Queueelem dequeue(Queue *q)
{
  Queueelem value;

  if(q->noofelements == 0)
  {
    fprintf(stderr,"queue is empty\n");
    exit(EXIT_FAILURE);
  } 
  q->noofelements--;
  value = q->queuespace[q->dequeueindex];
  if(q->dequeueindex > 0)
  {
    q->dequeueindex--;
  } else
  {
    q->dequeueindex = q->queuesize - 1;  // != enqueueindex, since at least one elem
  }
  DEBUG0(3,"#dequeue of ");
  DEBUGCODE(3,q->showelem(value));
  DEBUG0(3,"\n");
  return value;
}

Queueelem *headofqueue(const Queue *q)
{
  if(q->noofelements == 0)
  {
    fprintf(stderr,"headofqueue failed: queue is empty\n");
    exit(EXIT_FAILURE);
  } 
  return &q->queuespace[q->dequeueindex];
}

Queueelem *tailofqueue(const Queue *q)
{
  if(q->noofelements == 0)
  {
    fprintf(stderr,"tailofqueue failed: queue is empty\n");
    exit(EXIT_FAILURE);
  }
  if(q->enqueueindex == q->queuesize-1)
  {
    return &q->queuespace[0];
  }
  return &q->queuespace[q->enqueueindex+1];
}

void deleteheadofqueue(Queue *q)
{
  if(q->noofelements == 0)
  {
    fprintf(stderr,"queue is empty\n");
    exit(EXIT_FAILURE);
  } 
  q->noofelements--;
  if(q->dequeueindex > 0)
  {
    q->dequeueindex--;
  } else
  {
    q->dequeueindex = q->queuesize - 1; // != enqueueindex, since at least one elem
  }
}

Sint overallqueuelements(const Queue *q,Queueprocessor queueprocessor,
                         void *info)
{
  Uint i;

  if(q->noofelements > 0)
  {
    if(q->enqueueindex < q->dequeueindex)
    {
      for(i=q->enqueueindex+1; i<=q->dequeueindex; i++)
      {
        if(queueprocessor(q->queuespace + i,info) != 0)
        {
          return (Sint) -1;
        }
      }
    } else
    {
      for(i=q->enqueueindex+1; i<=q->queuesize-1; i++)
      {
        if(queueprocessor(q->queuespace + i,info) != 0)
        {
          return (Sint) -1;
        }
      }
      for(i=0; i<=q->dequeueindex; i++)
      {
        if(queueprocessor(q->queuespace + i,info) != 0)
        {
          return (Sint) -1;
        }
      }
    }
  }
  return 0;
}

//}


/*EE
  The following function frees the space required for the queue. 
*/

void wrapqueue(Queue *q)
{
  DEBUG2(3,"#wrapqueue(maxqueuesize=%lu,queuesize=%lu)\n",
            (Showuint) q->maxqueuesize,(Showuint) q->queuesize);
  FREESPACE(q->queuespace);
}

//\IgnoreLatex{

#ifdef DEBUG


#define SHOWQUEUEELEM(X) q->showelem(q->queuespace[X])

void showqueue(const Queue *q)
{
  Uint i;

  printf("queue=");
  if(q->noofelements == 0)
  {
    printf("[]");
  } else
  {
    if(q->enqueueindex < q->dequeueindex)
    {
      for(i=q->enqueueindex+1; i<=q->dequeueindex; i++)
      {
        SHOWQUEUEELEM(i);
      }
    } else
    {
      for(i=q->enqueueindex+1; i<=q->queuesize-1; i++)
      {
        SHOWQUEUEELEM(i);
      }
      for(i=0; i<=q->dequeueindex; i++)
      {
        SHOWQUEUEELEM(i);
      }
    }
  }
  (void) putchar('\n');
}
#endif

//}
