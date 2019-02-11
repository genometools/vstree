//\IgnoreLatex{

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "gqueueproc.h"
#include "types.h"

//}

typedef struct _Genericelementofqueue
{
  void *contents;
  struct _Genericelementofqueue *previous, *next;
} Genericelementofqueue;

typedef struct
{
  Genericelementofqueue *head, *tail;
  Uint numberofelements;
} Genericqueue;

/*
  This file defines a generic type for queues.
*/

/*@ignore@*/
Genericqueue *emptyqueuegeneric(void)
{
  Genericqueue *q;
  
  q = malloc(sizeof(Genericqueue));
  if(q == NULL)
  {
    fprintf(stderr,"malloc(emptyqueuegeneric) failed\n");
    exit(EXIT_FAILURE);
  }
  assert(q != NULL);
  q->numberofelements = 0;
  q->head = NULL;
  q->tail = NULL;
  assert(q != NULL);
  return q;
}
/*@end@*/

Uint sizeofgenericqueue(const Genericqueue *q)
{
  return q->numberofelements;
}

BOOL queueisemptygeneric(const Genericqueue *q)
{
  if(q->numberofelements == 0)
  {
    return True;
  } 
  return False;
}

void enqueuegeneric(Genericqueue *q,void *contents)
{
  Genericelementofqueue *newqueueelem;

  newqueueelem = malloc(sizeof(Genericelementofqueue));
  if(newqueueelem == NULL)
  {
    fprintf(stderr,"malloc(Genericelementofqueue) failed\n");
    exit(EXIT_FAILURE);
  }
  newqueueelem->contents = contents;
  newqueueelem->previous = NULL;
  newqueueelem->next = q->tail;
  if(q->numberofelements == 0)
  {
    q->head = newqueueelem;
  } else
  {
    q->tail->previous = newqueueelem;
  }
  q->tail = newqueueelem;
  q->numberofelements++;
}

/*@null@*/ void *dequeuegeneric(Genericqueue *q)
{
  void *contents;
  Genericelementofqueue *oldheadptr;

  if(q->numberofelements == 0)
  {
    return NULL;
  } 
  oldheadptr = q->head;
  q->head = q->head->previous;
  if(q->head == NULL)
  {
    q->tail = NULL;
  } else
  {
    q->head->next = NULL;
  }
  contents = oldheadptr->contents;
  free(oldheadptr);
  q->numberofelements--;
  return contents;
}

/*@null@*/ void *headofqueuegeneric(const Genericqueue *q)
{
  if(q->numberofelements == 0)
  {
    return NULL;
  }
  return q->head->contents;
}

Sint overallqueuelementsgeneric(Genericqueue *q,
                                GenericQueueprocessor queueprocessor,
                                void *info)
{
  Genericelementofqueue *current;

  if(q->numberofelements > 0)
  {
    for(current = q->head; 
        current != NULL; 
        current = current->previous)
    {
      if(queueprocessor(current->contents,info) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  return 0;
}

void wrapqueuegeneric(BOOL freecontents,Genericqueue **q)
{
  while(!queueisemptygeneric(*q))
  {
    if(freecontents)
    {
      free((*q)->head->contents);
    }
    (void) dequeuegeneric(*q);
  }
  free(*q);
}
