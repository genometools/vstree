
//\Ignore{

#ifndef MPARMS
#define MPARMS

#include "types.h"
#include "xdropdef.h"
#include "multidef.h"
#include "evaluedef.h"
#include "absdef.h"
#include "qualint.h"
#include "boundedgaps.h"

//}

/*
  The following macros copy records of type \texttt{Queryinfo} and 
  \texttt{Matchparam}.
*/

/*
  We take a large double to filter Evalues
*/

#define UNDEFMAXIMUMEVALUE ((Evaluetype) (3.40282347e+38F))

#define MPARMEXACTMATCH(MXDIST)   ((MXDIST)->distkind == Exactmatch)
#define MPARMHAMMINGMATCH(MXDIST) ((MXDIST)->distkind == Hammingmatch)
#define MPARMEDISTMATCH(MXDIST)   ((MXDIST)->distkind == Edistmatch)

#define COPYQUERYINFO(DEST,SOURCE)\
        *(DEST) = *(SOURCE)

#define COPYMATCHPARAM(DEST,SOURCE)\
        *(DEST) = *(SOURCE)

#define DEFAULTMATCHPARAM(MP)\
        (MP)->repeatgapspec.lowergaplength = 0;\
        (MP)->repeatgapspec.uppergaplength = 0;\
        (MP)->repeatgapspec.lowergapdefined = False;\
        (MP)->repeatgapspec.uppergapdefined = False;\
        (MP)->repeatgapspec.lowerboundfun = NULL;\
        (MP)->repeatgapspec.upperboundfun = NULL;\
        (MP)->repeatgapspec.lowerinfo = NULL;\
        (MP)->repeatgapspec.upperinfo = NULL;\
        (MP)->repeatgapspec.skiplistprobability = 0.0;\
        (MP)->repeatgapspec.skiplistmaxsize = 0;\
        (MP)->numberofprocessors = UintConst(1);\
        (MP)->userdefinedleastlength = 0;\
        (MP)->seedlength = 0;\
        (MP)->identity = 0;\
        (MP)->queryspeedup = UintConst(2);\
        (MP)->maxdist.distkind = Exactmatch;\
        (MP)->maxdist.distvalue = 0;\
        (MP)->xdropbelowscore = UNDEFXDROPBELOWSCORE;\
        (MP)->xdropleastscore = UNDEFXDROPLEASTSCORE;\
        (MP)->maximumevalue = UNDEFMAXIMUMEVALUE;\
        (MP)->completematchremoveredundancy = False;\
        (MP)->transnum = NOTRANSLATIONSCHEME;\
        (MP)->dnasymbolmapdnavsprot = NULL

/*
  The absolute value of \texttt{xdropbelowscore}.
*/

#define SETFLAGXDROP(MPX)   ((Uchar) (ABS(MPX)))

/*
  The following structure store information about a query sequence.
*/

typedef struct
{
  Multiseq *multiseq;   // multiple sequence for queries
  Uint querysubstringoffset, // offset when matching a subsequence, default=0
       numofcodes,      // number of codes = numofchars^prefixlen
       prefixlength,    // the prefixlength for the bucket boundaries
       *suftab,         // of length multiseq.totallength + 1
       *bcktab;         // of length 2 * numofchars^prefixlen
  Uchar *lcptab;        // of length multiseq.totallength + 1
  BOOL extended;        // true if and only if matchvirtagainstvirt:
                        // this flag is not necessary at the moment
} Queryinfo;            // \Typedef{Queryinfo}

typedef enum
{
  Exactmatch,
  Hammingmatch,
  Edistmatch
} Distancekind;          // \Typedef{Distancekind}

typedef struct
{
  Distancekind distkind;        // the kind of distance
  Qualificationtag distinterpretation;  // is distance an absolute 
                                        // or percentage value
  Uint distvalue;               // the distance value
} Distancevalue;        // \Typedef{Distancevalue}

/*
  The most important parameters for the matching process are 
  collected in the following type.
*/

typedef struct
{
  Uint userdefinedleastlength,  // the value of the -l parameter
       seedlength,              // value >= userdefinedleastlength/(maxdist+1)
       identity,                // percentage
       queryspeedup,            // speedup level: 0=straight, 1=with iso
                                // 2=with iso and sti, only works for query
                                // matches, default is 0
       numberofprocessors,      // number of processors in parallel call
       transnum;                // number of translation table
  char *dnasymbolmapdnavsprot;  // symbolmap for matching protein vs DNA
  Repeatgapspec repeatgapspec;  // specify required gaps between repeat instance
  Distancevalue maxdist;        // maximal distance value allowed
  Xdropscore xdropbelowscore,   // the maximal score below max score allowed
             xdropleastscore;   // the score a match must at least have
  Evaluetype maximumevalue;     // maximal Evalue allowed
  BOOL completematchremoveredundancy;  // remove redundancy in edist complete
} Matchparam;                   // \Typedef{Matchparam}

//\Ignore{

#endif

//}
