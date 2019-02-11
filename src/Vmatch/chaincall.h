
//\Ignore{

#ifndef CHAINCALL_H
#define CHAINCALL_H

#include "chaindef.h"

//}

#define CHAINPREFIX  "chain"

typedef struct
{
  BOOL defined, 
       silent,
       dothreading,
       withinborders;
  Chainmode chainmode;
  double weightfactor;
  char matchfile[PATH_MAX+1],
       *outprefix;
  Showverbose showverbose;
  Uint userdefinedminthreadlength1,
       userdefinedminthreaderrorpercentage1,
       userdefinedminthreadlength2,
       userdefinedminthreaderrorpercentage2;
} Chaincallinfo;

//\Ignore{

#endif

//}
