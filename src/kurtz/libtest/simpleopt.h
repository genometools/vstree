#include "types.h"

typedef enum
{
  OPTTWOSTRINGS = 0,
  OPTTWOFILES,
  OPTTEXT,
  OPTALPHALEN,
  OPTHELP,
  NUMOFOPTIONS
} SimpleOptionnumber;

typedef struct
{
  SimpleOptionnumber optnum;
  char *string1,
       *string2,
       *file1,
       *file2,
       *text,
       *charlist;
  Uint lengthofstrings;
} SimpleOptionvalues;

typedef Sint (*Checkalignfuntype)(BOOL,Uchar *,Uint,Uchar *,Uint);

Sint parsesimpleoptions(SimpleOptionvalues *optionvalues,
                        Argctype argc,
                        const char **argv);

void freesimpleoptions(SimpleOptionvalues *optionvalues);

Sint applycheckfunctiontosimpleoptions(Checkalignfuntype checkfunction,
                                       Argctype argc,const char **argv);
