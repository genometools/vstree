//\IgnoreLatex{


//}

#ifndef MULTIMATDEF_H
#define MULTIMATDEF_H
#include "types.h"
#include "intbits.h"
#include "evaluedef.h"
#include "virtualdef.h"

/*
  The following type is required when searching for matches between
  more than two sequences. The maximal number of sequences 
  is \texttt{MAXSEQUENCENUMBER}. \texttt{length} is the length of
  the match. \texttt{position[i]} is the starting position of the
  match in the $i$th sequence.
*/

#define UNDEFWIDTHUPPERBOUND 0
#define UNDEFMAXEXPECTEDMM   0

#ifndef DYNAMICSEQNUM
#define MAXSEQUENCENUMBER 32
#endif

#ifdef DYNAMICSEQNUM
#define DECLAREdynamicseqnumshared \
BOOL dynamicseqnumshared(void)\
{\
  return True;\
}
#else
#define DECLAREdynamicseqnumshared \
BOOL dynamicseqnumshared(void)\
{\
  return False;\
}
#endif

#define MMPALINDROMICBIT     FIRSTBIT     // bit for palindromic match
#define MMSIXFRAMEBIT        SECONDBIT    // bit for match in 6 reading frames
#define ISMMPALINDROMIC(POS)\
        ((POS) & MMPALINDROMICBIT)        // check if palindromic match
#define ISMMSIXFRAMEMATCH(POS)\
        ((POS) & MMSIXFRAMEBIT)           // check if match in 6 reading frames
#define ISMMSPECIALMATCH(POS)\
        ((POS) & (MMPALINDROMICBIT | MMSIXFRAMEBIT))
#define MMEXTRACTPOSITION(POS)\
        ((POS) & EXCEPTFIRSTTWOBITS)      // extract position

typedef struct
{
  Uint length;                          // length of matching sequence
#ifdef DYNAMICSEQNUM
  Uint *positions;                      // start positions of matches
#else
  Uint positions[MAXSEQUENCENUMBER];    // start positions of matches
#endif
} Multimatch;                           // \Typedef{Multimatch}

typedef struct
{
  Uint length,                          // length of matching sequence
       *sizeofgroups;                   // array of sizes of groups
  ArrayUint orderedpositions;           // array of ordered positions
} Multiexactmatchset;                   // \Typedef{Multiexactmatchset} 

typedef struct
{
  Uint nextfreeEvaluetype,
       allocatedEvaluetype;
  Evaluetype *spaceEvaluetype,
             stepmultiplier;            // $\sigma^{k-1}$
} ArrayEvaluetype;                      // \Typedef{ArrayEvaluetype}

typedef struct
{
  Uint *countrare,
       *maxrare;
} Rareevaluation;                       // \Typedef{Rareevaluation}

typedef enum
{
  RARESEARCHANDOUTPUT,
  ONLYRARESEARCH,
  ONLYRAREOUTPUT
} Rarephase;                       // \Typedef{Rarephase}

/*
  A selection function is a record with three components:
  \begin{itemize}
  \item
  The function \texttt{selectmultimatchHeader} is called once before all 
  matches are output. It is applied to the header string.
  \item
  The function \texttt{selectmultimatchInit} is called once to before 
  all matches are processed.
  \item
  The function \texttt{selectmatchmultimatch} select matches.
  \item
  The function \texttt{selectmultimatchWrap} is called once after all 
  matches are processed.
  \end{itemize}
*/

typedef Sint(*SelectmultimatchHeader)(Argctype,const char **);
typedef Sint(*SelectmultimatchInit)(Virtualtree *,Uint *,Uint);
typedef Sint(*Selectmultimatch)(Virtualtree *,Uint *,Uint,Multimatch *);
typedef Sint(*SelectmultimatchWrap)(Virtualtree *,Uint *,Uint);

typedef Sint(*Showmultimatchfun)(void *,Multimatch *);
typedef Sint(*Showmultiexactmatchsetfun)(void *,Multiexactmatchset *);
typedef BOOL(*Dynamicseqnum)(void);

typedef struct
{
  SelectmultimatchInit   selectmultimatchInit;
  Selectmultimatch       selectmultimatch;
  SelectmultimatchWrap   selectmultimatchWrap;
  SelectmultimatchHeader selectmultimatchHeader;
  Dynamicseqnum          dynamicseqnumshared;
} SelectmultimatchBundle;

//\IgnoreLatex{

#ifdef __cplusplus
extern "C" {
#endif

//}

Sint selectmultimatchInit(Virtualtree *,Uint *,Uint);
Sint selectmultimatch(Virtualtree *,Uint *,Uint,Multimatch *);
Sint selectmultimatchWrap(Virtualtree *,Uint *,Uint);
Sint selectmultimatchHeader(Argctype,const char **);
BOOL dynamicseqnumshared(void);

Sint findmultimatches(Virtualtree *virtualtree,
                      Virtualtree *dnavirtualtree,
                      Uint transnum,
                      Uint searchlength,
                      Uint maxnummatches,
                      BOOL showpalindromic,
                      BOOL showdirect,
                      BOOL rcmindex,
                      BOOL *maxnumexceeded,
                      void *voidrelbounds,
                      Uint numofrelbounds,
                      PairUint *maxrarespec,
                      Uint numofmaxrarespecs,
                      Uint maxraresetall,
                      SelectmultimatchBundle *selectbundle,
                      ArrayUint *mmlengthdist,
                      Uint numofunits,
                      Uint *unitboundaries,
                      Uint widthupperbound,
                      void *showmultimatchinfo,
                      void *showmultimatchfunvoid,
                      Showverbose showverbose);

Sint findmultimatchesoptimal(Virtualtree *virtualtree,
                             Virtualtree *dnavirtualtree,
                             Uint transnum,
                             Uint searchlength,
                             Uint maxnummatches,
                             BOOL showpalindromic,
                             BOOL showdirect,
                             BOOL rcmindex,
                             BOOL *maxnumexceeded,
                             void *voidrelbounds,
                             Uint numofrelbounds,
                             PairUint *maxrarespec,
                             Uint numofmaxrarespecs,
                             Uint maxraresetall,
                             SelectmultimatchBundle *selectbundle,
                             ArrayUint *mmlengthdist,
                             Uint numofunits,
                             Uint *unitboundaries,
                             Uint widthupperbound,
                             void *showmultimatchinfo,
                             void *showmultimatchfunvoid,
                             Showverbose showverbose);

Sint findmultiexactmatchset(Virtualtree *virtualtree,
                            Virtualtree *dnavirtualtree,
                            Uint transnum,
                            Uint searchlength,
                            Uint maxnummatches,
                            BOOL showpalindromic,
                            BOOL showdirect,
                            BOOL rcmindex,
                            BOOL *maxnumexceeded,
                            void *voidrelbounds,
                            Uint numofrelbounds,
                            /*@unused@*/ PairUint *maxrarespec,
                            /*@unused@*/ Uint numofmaxrarespecs,
                            /*@unused@*/ Uint maxraresetall,
                            /*@unused@*/ SelectmultimatchBundle *selectbundle,
                            ArrayUint *mmlengthdist,
                            Uint numofunits,
                            Uint *unitboundaries,
                            Uint widthupperbound,
                            void *showmultiexactmatchsetinfo,
                            void *showmultiexactmatchsetfunvoid,
                            Showverbose showverbose);

typedef Sint (*Processmultimatch)(void *,
                                  Uint,
                                  Multimatch *);

typedef Sint (*Preprocessformultimatchoutput)(Virtualtree *,
                                              Multiseq *,
                                              Uint,
                                              void *);

typedef Sint (*Postprocessmatches)(void *);

//\IgnoreLatex{

#ifdef __cplusplus
}
#endif

#endif

//}
