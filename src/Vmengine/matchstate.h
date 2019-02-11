
//\Ignore{

#ifndef MATCHSTATE_H
#define MATCHSTATE_H

#include "types.h"
#include "multidef.h"
#include "arraydef.h"
#include "virtualdef.h"
#include "match.h"
#include "select.h"
#include "alphadef.h"

#include "bestinfo.h"
#include "mparms.h"
#include "currdirect.h"

//}

#define CALLEXTENSIONFUNCTION(SELFMATCH,INFO,ALLMAXIMALMATCHES)\
        if(callgenericextend(SELFMATCH,\
                             matchstate,\
                             &seed,\
                             &amatch,\
                             INFO,\
                             ALLMAXIMALMATCHES) != 0)\
         {\
           return (Sint) -1;\
         }

#define CHECKSHOWPALINDROMIC(MATCHSTATE)\
        ((MATCHSTATE)->currentdirection == DirectionReverse)

/*
  The following structure is used to store global information needed 
  when computing matches. Not all components are required for a 
  tasks, but we keep same all in one record.
*/

typedef struct
{
  Uint currentidnumber,      // the id number of a match; only initialized
                             // in initMstate, and only used in procfinal
       onlinequerynumoffset, // current number of query sequence in online 
                             // computation, otherwise 0, provided as argument
                             // to f-function and only used in procfinal
       transnum;             // number of translation table; provided as
                             // argument to f-function and only used in
                             // procfinal
  Virtualtree *virtualtree;  // the mapped database, used for computing matches
  Queryinfo *queryinfo;      // Information for the Query
  ArrayMatch allmstore,      // store for the matches
             seedmstore;     // store for the seed
  Matchparam matchparam;     // the user given parameters to compute a match
  Bestflag bestflag;         // copy of bestflag from bestinfo
  SelectBundle *selectbundle;// reference to function bundle; provided as
                             // argument to f-function and only used 
                             // in procfinal
  BOOL showselfpalindromic,  // showselfpalindromic set
       shownoevalue,         // show no evalue; provided as
                             // argument to f-function and only used
                             // in procfinal
       domatchbuffering,     // buffer the matches
       revmposorder;         // reverse the order of the match positions
                             // six frame translation of DNA sequence
  Currentdirection currentdirection;
  void *procmultiseq; 
  Processfinalfunction processfinal;
  Evalues *evalues;
} Matchstate;                // \Typedef{Matchstate}

//\Ignore{

#ifdef __cplusplus
extern "C" {
#endif
Sint initMatchstate(Matchstate *matchstate,
                    Virtualtree *virtualtree,
                    void *voidqueryinfo,
                    Matchparam *matchparam,
                    Bestflag bestflag,
                    Uint shownoevalue,
                    Uint showselfpalindromic,
                    SelectBundle *selectbundle,
                    Uint onlinequerynumoffset,
                    void *procmultiseq,
                    Currentdirection currentdirection,
                    BOOL revmposorder,
                    Processfinalfunction processfinalparam,
                    Evalues *evalues,
                    BOOL domatchbuffering);

#ifdef __cplusplus
}
#endif

#endif

//}
