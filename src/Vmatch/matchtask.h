
//\Ignore{

#ifndef MATCHTASK_H
#define MATCHTASK_H

#include "maxfiles.h"
#include "markinfo.h"
#include "mparms.h"
#include "markinfo.h"
#include "vplugin-interface.h"
#include "vmotif-data.h"
#include "cpridx-data.h"
#include "chaincall.h"
#include "mcldef.h"
#include "vmcldef.h"
#include "fqfinfo.h"
#include "outinfo.h"

#ifdef VMATCHDB
#include "vmdbparms.h"
#endif

//}

/*
  This file contains some definitions for types used when analyzing the
  user given parameters specifying the matching task.
*/

#define TASKONLINE         UintConst(1)        // match is performed online
#define TASKCOMPLETEMATCH  (UintConst(1) << 1) // require complete matches
#define TASKMUM            (UintConst(1) << 2) // require maximal unique matches
#define TASKMUMCANDIDATE   (UintConst(1) << 3) // require mum candidates
#define TASKSUPER          (UintConst(1) << 4) // require maximal unique matches
#define TASKTANDEM         (UintConst(1) << 5) // require maximal unique matches
#define TASKPREINFO        (UintConst(1) << 6) // require to only compute number of exact matches

#define ISTASKONLINE\
        ((matchcallinfo->matchtask & TASKONLINE) ? True: False)

#define DOMATCHBUFFERING(MCINFO)\
        (BOOL) (((MCINFO)->chaincallinfo.defined ||\
                (MCINFO)->matchclustercallinfo.defined))

/*
  The following type aggregates information parsed from the
  argument vector, as provided when calling the vmatch-program.
*/

typedef struct
{
  Uint numberofqueryfiles,// number of query files
       matchtask;      // a combination of the bits TASKONLINE .. TASKPREFINFO
  BOOL ownpostprocessing;             // vmatch does own postprocessing
  Maskmatch maskmatch;
  Nomatch nomatch;
  char progname[PATH_MAX+1],          // program name
       indexormatchfile[PATH_MAX+1],  // name of index or match file
       *mfargs,                       // args for the matchfile
       *queryfiles[MAXNUMBEROFFILES]; // table of query files
  BOOL withindexfile;                 // last argument is indexname
  FQFsubstringinfo fqfsubstringinfo;  // for the first file containing 
                                      // only one sequence
  Matchparam matchparam;              // user specified match parameters
  Outinfo outinfo;                    // information determining the output
  Clusterparms clusterparms;          // parameters required for clustering
  Showverbose showverbose;            // function to show status info
  Markinfo markinfo;                  // for marking matches
  Sequenceclusterinfo seqclinfo;      // sequence clustering
  Chaincallinfo chaincallinfo;
  Matchclustercallinfo matchclustercallinfo;
  Vpluginbundle vmotifbundle, 
                cpridxpatsearchbundle; 
  Vmotifdata vmotifdata;
  Cpridxpatsearchdata cpridxpatsearchdata;
//\Ignore{

#ifdef VMATCHDB
  Databaseparms databaseparms;
#endif

//}

} Matchcallinfo;                          // \Typedef{Matchcallinfo}

//\Ignore{

#endif

//}
