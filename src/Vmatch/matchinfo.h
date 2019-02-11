
//\Ignore{

#ifndef MATCHINFO_H
#define MATCHINFO_H

#include "match.h"
#include "maxfiles.h"
#include "virtualdef.h"
#include "outinfo.h"

/*
  The following type stores information about matches including the
  sequence sets for which the matches are computed. There is some information
  overlapping with the type \texttt{Matchcallinfo}.
*/

typedef struct
{
  Uint numberofqueryfiles, // number of query files
       matchtask,          // a combination of bits TASKONLINE .. TASKPREFINFO
       transnum;
  Xdropscore xdropbelowscore;
  char *largeheading,   // heading shown in large font in Genalyzer
       *smallheading,   // heading shown in small format in Genalyzer
       *mfargs,         // concated arguments of the matchfile
       *dnasymbolmapdnavsprot,
       *queryfiles[MAXNUMBEROFFILES];  // the query files
  ArrayStoreMatch matchtab;            // table storing the matches
  Virtualtree virtualtree,             // the virtual tree for the index
              queryvirtualtree,        // the virtual tree for the query
              sixframeofqueryvirtualtree,
              dnavirtualtree;          // the virtual tree of the original dna 
  Outinfo outinfo;                     // information determining the output
} Matchinfo;                           // \Typedef{Matchinfo}

//\Ignore{

#endif

//}
