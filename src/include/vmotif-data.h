
//\IgnoreLatex{

#ifndef VMOTIF_DATA_H
#define VMOTIF_DATA_H

#include "virtualdef.h"
#include "match.h"

//}

/*
  This file defines the data structure allowing to exchange information
  with an external shared object performing pattern matching tasks. 
  For each match these objects call the function processfinal. 
  Hence these are communicated by a corresponding function
  pointer in the structure described below.
*/

typedef struct
{
  Uint includedemand,        // output: to be set by the adddemand function
       excludedemand,        // output: to be set by the addemand function
       numberofqueryfiles;   // input: number of query files
  Virtualtree *virtualtree;  // reference to virtual tree
  BOOL forceonline;          // do online matching
  BOOL optimize;             // optimize query
  char *progname,            // name of the calling programming
       *indexname,           // name of the index
       **queryfiles;         // array of queryfile names
  void *voidMatchstate,
       *globalspaceptr;    // reference to structure needed in vplugin-functions
  Processfinalfunction processfinal; // the function called for every match
  Showverbose showverbose; // function to show verbose messages
} Vmotifdata;

//\IgnoreLatex{

#endif

//}
