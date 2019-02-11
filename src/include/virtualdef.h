//\IgnoreLatex{

#ifndef VIRTUALDEF_H
#define VIRTUALDEF_H

#include <limits.h>
#include <stdio.h>
#include "types.h"
#include "alphadef.h"
#include "multidef.h"

/*
  This file defines the datatype \texttt{virtualtree} which stores
  a virtual suffix tree. 
*/

/*
  Each of the following bits specifies a table 
  of the virtual suffix tree. Some of these tables refer
  to multiple sequences. Hence this file is also included
  in multiseq-adv.c
*/

typedef enum
{
  TISTABNUM = 0, // transformed input sequence
  OISTABNUM,     // original input sequence
  DESTABNUM,     // the fasta descriptions
  SSPTABNUM,     // the sequence separator positions
  BWTTABNUM,     // the bwt-table
  SUFTABNUM,     // the suf-table
  LCPTABNUM,     // the lcp-table
  BCKTABNUM,     // the bck-table
  STI1TABNUM,    // the small inverse of the suf-table
  SKPTABNUM,     // the skip-table

  // The following tables are mainly for experimental alorithms.

  CLDTABNUM,  // the child table
  CLD1TABNUM, // the small child table
  ISOTABNUM,  // the isomorphic depth table
  STITABNUM,  // the inverse of the suf-table
  CFRTABNUM,  // the cfr table
  CRFTABNUM,  // the crf table
  LSFTABNUM,  // the lsf table
  NUMBEROFVTABS
} Vtablenum;

/*
  The following macro defines a shift of 1 by a given integer.
*/

#define MAKETABBIT(TAB) (UintConst(1) << TAB)

#define TISTAB  UintConst(1)            
#define OISTAB  MAKETABBIT(OISTABNUM)     
#define DESTAB  MAKETABBIT(DESTABNUM)
#define SSPTAB  MAKETABBIT(SSPTABNUM)
#define BWTTAB  MAKETABBIT(BWTTABNUM)    
#define SUFTAB  MAKETABBIT(SUFTABNUM)   
#define LCPTAB  MAKETABBIT(LCPTABNUM)
#define BCKTAB  MAKETABBIT(BCKTABNUM)
#define STI1TAB MAKETABBIT(STI1TABNUM)
#define SKPTAB  MAKETABBIT(SKPTABNUM)

/*
  The following are the ORed flags for the tables storing the
  plain sequences and sequence descriptions.
*/

#define SEQUENCETABS (TISTAB | OISTAB | DESTAB | SSPTAB)

/*
  The following are the ORed flags for the basic tables of the 
  virtual suffix tree.
*/

#define BASICTABS (SEQUENCETABS |\
                   BWTTAB | SUFTAB | LCPTAB | BCKTAB | STI1TAB | SKPTAB)

/*
  Here are some bits for the extra table.
*/

#define CLDTAB  MAKETABBIT(CLDTABNUM)
#define CLD1TAB MAKETABBIT(CLD1TABNUM)
#define ISOTAB  MAKETABBIT(ISOTABNUM)
#define STITAB  MAKETABBIT(STITABNUM)
#define CFRTAB  MAKETABBIT(CFRTABNUM)
#define CRFTAB  MAKETABBIT(CRFTABNUM)
#define LSFTAB  MAKETABBIT(LSFTABNUM)

/*
  These are ORed two.
*/

#define EXTRATABS (CLDTAB | CLD1TAB | ISOTAB |\
                   STITAB | CFRTAB | CRFTAB | LSFTAB)

/*
  The following defines the size of an entry in table bcktab.
*/

#define SIZEOFBCKENTRY (UintConst(2) * (Uint) sizeof(Uint))

/*
  This is the maximal length of an index file of a virtual suffix tree.
*/

#define VIRTUALFILENAMEMAX (PATH_MAX+32)

/*
  The following macro assign to the variable \texttt{VAR}
  the value of \(\mathsf{lcptab}[IND]\) for a
  given index \texttt{IND}. If this
  value equals 255, then we have to resort to the next value of
  the exception table \(\mathsf{llvtab}\) via the variable
  \texttt{EXC}.
*/

#define SEQUENTIALEVALLCPVALUEGENERIC(VAR,LCPTAB,LARGELCPS,IND,EXC)\
        if((VAR = LCPTAB[IND]) == UCHAR_MAX)\
        {\
          VAR = (LARGELCPS)->spacePairUint[(EXC)++].uint1;\
        } 

/*
  The following macro is a special version of the previous macro.
*/

#define SEQUENTIALEVALLCPVALUE(VAR,IND,EXC)\
        SEQUENTIALEVALLCPVALUEGENERIC(VAR,\
                                      virtualtree->lcptab,\
                                      &virtualtree->largelcpvalues,\
                                      IND,\
                                      EXC)

/*
  If suffix \(aw\) is the \(i\)th suffix in the lexicographic order
  of all suffixes, then \texttt{RANKOFNEXTLEAF(virtualtree,i)} delivers
  the rank of suffix \(w\).
*/

#define RANKOFNEXTLEAF(VIRT,LEAFRANK)\
        (VIRT)->stitab[(VIRT)->suftab[LEAFRANK]+1]

/*
  The following macro checks if the number of arguments is exactly
  \texttt{N}. Otherwise, an error message is thrown.
*/

#define VSTREECHECKARGNUM(N,S)\
        if (argc != (N))\
        {\
          fprintf(stderr,"Usage: %s %s\n",argv[0],S);\
          fprintf(stderr,"see Vmatch manual at http:%c%cwww.vmatch.de "\
                         " for more information\n",'/','/');\
          exit(EXIT_FAILURE);\
        }

/*
  The following constants are for special cases in \texttt{cldtab}.
*/

#define UNDEFCHILDVALUE 0          // $cldtab[i].value = undef$
#define LARGECHILDVALUE UCHAR_MAX  // maximal value

/*
  The following stores the information for determining the child
  of an lcp-interval in constant time. The values are relative to
  the index they are stored at.
*/

typedef struct
{
  Uchar up,         // smaller than i: up[i] = i - cldtab[i].up
        down,       // larger than i:  down[i] = i + clddtab[i].up
        nextlIndex; // larger than i:  nexlIndex[i] = i + cldtab[i].nextlIndex
} Childinfo;        // \Typedef{Childinfo}

/*
  The following is the central data structure to represent 
  enhanced suffix arrays.
*/

typedef struct
{
  Uchar *bwttab;      // of length multiseq.totallength + 1
  Uint  *suftab,      // of length multiseq.totallength + 1
        *skiptab,     // of length multiseq.totallength + 1
        *bcktab,      // of length 2 * alphasize^prefixlen
        *stitab,      // of length multiseq.totallength + 1
        *afxtab;      // of length multiseq.totallength
  Uchar *stitab1,     // of length multiseq.totallength + 1
        *lcptab,      // of length multiseq.totallength + 1
        *isodepthtab, // of length multiseq.totallength
        *cldtab1,     // of length multiseq.totallength + 1
        *lsftab;      // of length 2 * (multiseq.totallength + 1)
  Childinfo *cldtab;  // of length multiseq.totallength + 1
  DefinedUint longest; // index of longest suffix in suftab
  Alphabet alpha;    // alpha.mapsize == 0, then no alphabet transformation
  Multiseq multiseq; // input sequence over which index is constructed
  ArrayPairUint largelcpvalues; // the lcp values >= 255
  PairUint *llvcachemin, 
           *llvcachemax; // store address of previously used entry
#ifdef COUNT
  Uint llvcachehit,
       callgetexception;
#endif
  BOOL specialsymbols,  // do the input files contain special symbols?
       rcmindex;        // index is reverse complemented index
  Uint sixframeindex,   // six frame index: either NOTRANSLATIONSCHEME or
                        // number of translation scheme
       numofcodes,      // number of codes = alphasize^prefixlen
       maxbranchdepth,  // the maximal depth of a branching node
       mapped,          // status: which table is mapped
       constructed,     // status: which table is constructed
       prefixlength;    // the prefixlength for the bucket boundaries
} Virtualtree;          // \Typedef{Virtualtree}

/*
  The following structure is used for representing super buckets,
  i.e. a sequence of consecutive buckets.
*/

typedef struct
{
  Uint firstcodenum,    // the code of the first suffix in the superbucket 
       firstlcpvalue,   // the first lcp-value of the superbucket
       firstexceptionindex,  // index of first exception value in this interval
       firstsuftabindex,// the first index of the superbucket
       lastsuftabindex; // the last index of the superbucket
} Superbckinterval;     // \Typedef{Superbckinterval}

/*
  The following defines the type of functions applied to an
  lcp-interval.
*/

typedef Sint (*Processmatchinterval)(void *,PairUint *);

/*
  We add some prototypes for the most important functions related
  to virtual suffix trees. For an explanation of these
  function, see the corresponding modules in 
  \texttt{Mkvtree} and \texttt{Vmatch}.
*/

//\IgnoreLatex{

#ifdef __cplusplus
extern "C" {
#endif

//}

Sint callmkvtree(Argctype argc,
                 const char **argv,
                 BOOL storeonfile,
                 Virtualtree *virtualtree,
                 BOOL storedesc,
                 Showverbose showverbose);

Sint wrapvmatch(Virtualtree *,Virtualtree *,Virtualtree *,Virtualtree *);
Sint completevirtualtree(Virtualtree *,Uint,Showverbose);

//\IgnoreLatex{

#ifdef __cplusplus
}
#endif

//}

/*
  The following macro \texttt{DEFINEEVALLCP} defines the function 
  \texttt{evallcp}. For efficiency reason we recommand to instead 
  use the macro \texttt{EVALLCP}.
*/

#define DEFINEEVALLCP\
        static Uint evallcp(Virtualtree *virtualtree,Uint i)\
        {\
          Uint lcpval = virtualtree->lcptab[i];\
          if(lcpval < UCHAR_MAX)\
          {\
            return lcpval;\
          }\
          return (getexception(virtualtree,i))->uint1;\
        }

#define EVALLCP(VIRT,VAR,IND)\
        {\
          VAR = (VIRT)->lcptab[IND];\
          if(VAR >= UCHAR_MAX)\
          {\
            VAR = (getexception(VIRT,IND))->uint1;\
          }\
        }

/*
  The following macro \texttt{DELIVERHOME} evaluates a unique home
  position in the range \([0,n]\), where \([\texttt{LEFT},\texttt{RIGHT}]\)
  in an lcp-interval. The variables \texttt{LCPLEFT} and \texttt{LCPRIGHT}
  are for temporary use. The resulting unique index is stored in variable
  \texttt{VAR}.
*/

#define DELIVERHOME(VIRT,VAR,LCPLEFT,LCPRIGHT,LEFT,RIGHT)\
        {\
          if((LEFT) > 0)\
          {\
            EVALLCP(VIRT,LCPLEFT,LEFT);\
            EVALLCP(VIRT,LCPRIGHT,RIGHT+1);\
            if(LCPLEFT >= LCPRIGHT)\
            {\
              VAR = LEFT;\
            } else\
            {\
              VAR = RIGHT;\
            }\
          } else\
          {\
            VAR = RIGHT;\
          }\
        }

//\IgnoreLatex{

#define MAXVSTACK 32

#ifdef __cplusplus
extern "C" {
#endif

PairUint *getexception(Virtualtree *virtualtree,Uint key);

#ifdef __cplusplus
}
#endif

#endif

//}
