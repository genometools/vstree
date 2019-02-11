#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "qsortdef.h"
#include "args.h"

#define USAGE         "[LMCS] maxvalue numberofvalues"
#define DOLIBRARY     UintConst(1)
#define DOMYQSORT     (UintConst(1) << 1)
#define DOCHECKSORT   (UintConst(1) << 2)
#define DOSHOWNUMBERS (UintConst(1) << 3)
#define DEREF(V)      (*((const Uint *) (V)))

static void checksortedinteger(Uint *values,Uint nofelements)
{
  Uint i;

  for(i=0; i<nofelements-1; i++)
  {
    if(values[i] > values[i+1])
    {
      fprintf(stderr,"values[%lu] = %lu > %lu = values[%lu]\n",
                     (Showuint) i,
                     (Showuint) values[i],
                     (Showuint) values[i+1],
                     (Showuint) i+1);
      exit(EXIT_FAILURE);
    }
  }
}

static void showvalues(Uint *values,Uint nofelements)
{
  Uint i;

  for(i=0; i<nofelements; i++)
  {
    printf("%lu",(Showuint) values[i]);
    if(i == nofelements-1)
    {
      printf("\n");
    } else
    {
      printf(" ");
    }
  }
}

static Qsortcomparereturntype qsortcmpUint(const void *keya,
                                           const void *keyb)
{
  if(DEREF(keya) < DEREF(keyb))
  {
    return (Sint) -1;
  }
  if(DEREF(keya) > DEREF(keyb))
  {
    return (Sint) 1;
  }
  return 0;
}

static void runsortingalgorithm(Uint mode,
                                Uint *values,
                                Uint nofelements)
{
  if(mode & DOSHOWNUMBERS)
  {
    showvalues(values,nofelements);
  }
  if(mode & DOLIBRARY)
  {
    qsort(values,(size_t) nofelements,sizeof(Uint),
          (Qsortcomparefunction) qsortcmpUint);
  }
  if(mode & DOMYQSORT)
  {
    qsortUint(values,values+nofelements-1);
  }
  if(mode & DOSHOWNUMBERS)
  {
    showvalues(values,nofelements);
  }
  if(mode & DOCHECKSORT)
  {
    checksortedinteger(values,nofelements);
  }
}

MAINFUNCTION
{
  Uint *values, maxvalue, nofelements, i, j, mode = 0;
  Scaninteger readint;

  DEBUGLEVELSET;
  CHECKARGNUM(4,"[LMCS] maxvalue numberofvalues");

  for(j=0; j<(Uint) strlen(argv[1]); j++)
  {
    switch(argv[1][j])
    {
      case 'L':
        mode |= DOLIBRARY;
        break;
      case 'M':
        mode |= DOMYQSORT;
        break;
      case 'C':
        mode |= DOCHECKSORT;
        break;
      case 'S':
        mode |= DOSHOWNUMBERS;
        break;
      default:
        fprintf(stderr,"%s: illegal character %c: must be combination of %s\n",
                   argv[0],argv[1][j],"LMCS");
        exit(EXIT_FAILURE);
    }
  }
  if((mode & DOCHECKSORT))
  {
    if(!(mode & DOMYQSORT) && !(mode & DOLIBRARY))
    {
      fprintf(stderr,"%s: C requires to use either L or M\n",argv[0]);
      return EXIT_FAILURE;
    }
  }
  if((mode & DOMYQSORT) && (mode & DOLIBRARY))
  {
    fprintf(stderr,"%s: cannot use M and L together\n",argv[0]);
    return EXIT_FAILURE;
  }
  if(sscanf (argv[2], "%ld", &readint) != 1 || readint < (Scaninteger) 1)
  {
    fprintf(stderr,"%s: %s\n",argv[0],USAGE);
    return EXIT_FAILURE;
  }
  maxvalue = (Uint) readint;
  if(sscanf (argv[3], "%ld", &readint) != 1 || readint < (Scaninteger) 1)
  {
    fprintf(stderr,"%s: %s\n",argv[0],USAGE);
    return EXIT_FAILURE;
  }
  nofelements = (Uint) readint;
  srand48(42349421);
  ALLOCASSIGNSPACE(values,NULL,Uint,nofelements);
  for (i = 0; i < nofelements; i++)
  {
    values[i] = (Uint) (drand48() * (maxvalue+1));
  }
  runsortingalgorithm(mode,values,nofelements);
  ALLOCASSIGNSPACE(values,NULL,Uint,nofelements);
  return EXIT_SUCCESS;
}
