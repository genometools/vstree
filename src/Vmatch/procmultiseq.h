
//\Ignore{

#ifndef PROCMULTISEQ_H
#define PROCMULTISEQ_H

#include "multidef.h"
#include "match.h"
#include "currdirect.h"

typedef enum
{
  Protprotmatchondna,
  Querytranslatedmatch,
  Normalmatch
} Supermatchtask;

typedef struct
{
  Multiseq *constvms,          // reference to multiseq of virtual tree
           *constqms,          // reference to multiseq with queries
           *constq6frms,       // reference to six frame translation of query
           *constvirtorigdnams;// reference to original DNA sequences for which 
                               // sixframeindex was generated
  Supermatchtask supermatchtask;
  Processmatch processmatch;// function to process a match
  BestMatchlist bestmatchlist;// list of best matches
  ArrayStoreMatch matchbuffer;
} Procmultiseq;

#endif
