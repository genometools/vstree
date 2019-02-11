#include <stdlib.h>
#include "gqueue-if.h"
#include "gqueueproc.h"
#include "errordef.h"
#include "spacedef.h"

#include "genericqueue.pr"

#define SEEDVALUE   42349421

#define USAGE       fprintf(stderr,"Usage: %s <positive number>\n",argv[0]);\
                    return EXIT_FAILURE
  
static Sint showqueuelem(void *elem, /*@unused@*/ void *info)
{
  printf("%lu\n",(Showuint) *((Uint *) elem));
  return 0;
}

MAINFUNCTION
{
  Genericqueue *queue;
  Scaninteger readint;
  Uint i, j, iterations, *numbers;
  void *resultptr;

  if(argc != 2)
  {
    USAGE;
  }
  if(sscanf(argv[1],"%ld",(Scaninteger *) &readint) != 1 || 
     readint < (Scaninteger) 1)
  {
    USAGE;
  }
  iterations = (Uint) readint;
  srand48 (SEEDVALUE);
  queue = emptyqueuegeneric();
  ALLOCASSIGNSPACE(numbers,NULL,Uint,iterations);
  for(i=0; i<iterations; i++)
  {
    numbers[i] = i;
    if(drand48() <= 0.3 && !queueisemptygeneric(queue))
    {
      resultptr = dequeuegeneric(queue);
      if(resultptr == NULL)
      {
        fprintf(stderr,"dequeue failed\n");
        exit(EXIT_FAILURE);
      }
      j = *((Uint *) resultptr);
      printf("dequeue %lu\n",(Showuint) j);
    } else
    {
      printf("enqueue %lu\n",(Showuint) numbers[i]);
      enqueuegeneric(queue,numbers + i);
    }
  }
  if(overallqueuelementsgeneric(queue,showqueuelem,NULL) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(numbers);
  wrapqueuegeneric(False,&queue);
  return EXIT_SUCCESS;
}
