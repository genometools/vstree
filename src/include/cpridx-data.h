//\IgnoreLatex{

#ifndef CPRIDX_DATA_H
#define CPRIDX_DATA_H

#include "virtualdef.h"
#include "match.h"

//}

/*
  This file defines the data structure allowing to exchange information
  with an external shared object performing pattern matching tasks. 
*/

typedef struct
{
  Uint includedemand,        // output: to be set by the adddemand function
       excludedemand,        // output: to be set by the adddemand function
       seqnum2,
       plen;
  Uchar *pattern;
  Virtualtree *virtualtree;  // reference to virtual tree
  char *progname,            // name of the calling programming
       *indexname;           // name of the index
  void *voidMatchstate,    // reference to matchstate
       *globalspaceptr;    // reference to structure needed in vplugin-functions
  Processfinalfunction processfinal; // the function called for every match
  Showverbose showverbose; // function to show verbose messages
} Cpridxpatsearchdata;

//\IgnoreLatex{

#endif

//}
