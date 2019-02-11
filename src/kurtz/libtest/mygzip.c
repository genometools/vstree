#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"

#include "readgzip.pr"
#include "checkgzip.pr"

MAINFUNCTION
{
  Uchar *filecontentptr;
  Uint filecontentlen;

  if (argc != 2)
  {
    fprintf (stderr, "Usage: %s <gzipped file>\n", argv[0]);
    return EXIT_FAILURE;
  }
  if(!checkgzipsuffix(argv[1]))
  {
    fprintf(stderr,"%s: filename '%s' does not end with appropriate suffix",
            argv[0],argv[1]);
    exit(EXIT_FAILURE);
  }
  filecontentptr = readgzippedfile(argv[1],&filecontentlen);
  if(filecontentptr == NULL)
  {
    STANDARDMESSAGE;
  } 
  fprintf(stderr,"totalsize=%lu\n",(Showuint) filecontentlen);
  if(fwrite(filecontentptr,sizeof(Uchar),
            (size_t) filecontentlen,stdout) != 
     (size_t) filecontentlen)
  {
    fprintf (stderr, "%s: cannot write: %lu bytes\n", argv[0], 
             (Showuint) filecontentptr);
    return EXIT_FAILURE;
  }
  FREESPACE(filecontentptr);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
