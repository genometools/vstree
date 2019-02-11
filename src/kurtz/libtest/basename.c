#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include "types.h"
#include "args.h"

MAINFUNCTION
{
  char *s1, *s2;

  CHECKARGNUM(2,"filename");
  s1 = vm_getbasename(argv[1]);
  s2 = basename(argv[1]);
  if(strcmp(s1,s2) != 0)
  {
    fprintf(stderr,"Different basenames: \"%s\" != \"%s\"\n",s1,s2);
    return EXIT_FAILURE;
  }
  free(s1);
  s1 = getdirname(argv[1]);
  s2 = dirname(argv[1]);
  if(strcmp(s1,s2) != 0)
  {
    fprintf(stderr,"Different dirnames: \"%s\" != \"%s\"\n",s1,s2);
    return EXIT_FAILURE;
  }
  free(s1);
  return EXIT_SUCCESS;
}
