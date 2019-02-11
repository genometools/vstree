
//\Ignore{

#ifndef OUTINFO_H
#define OUTINFO_H

#include <stdio.h>
#include "intbits.h"
#include "select.h"
#include "bestinfo.h"
#include "alphadef.h"

//}

/*
  This file contains some important stuff related to the output format
  of the matches.
*/

/*
  The following bits are set in the variable \texttt{showmode}.
  For the final output, those bits that are marked by ++ have an effect on
  the output. Those not marked by ++ are just for computing the matches.
*/

#define SHOWDIRECT          UintConst(1)          // compute direct match
#define SHOWPALINDROMIC     (UintConst(1) <<  1)  // compute palindromic match
#define SHOWSELFPALINDROMIC (UintConst(1) <<  2)  // compute palindromic self-matches
#define SHOWABSOLUTE        (UintConst(1) <<  3)  // show absolute positions ++
#define SHOWNODIST          (UintConst(1) <<  4)  // do not show distance ++
#define SHOWNOEVALUE        (UintConst(1) <<  5)  // do not show evalue ++
#define SHOWNOSCORE         (UintConst(1) <<  6)  // do not show score ++
#define SHOWNOIDENTITY      (UintConst(1) <<  7)  // do not show evalue ++
#define SHOWFILE            (UintConst(1) <<  8)  // show filename ++
#define SHOWNOSPECIAL       (UintConst(1) <<  9)  // show no special symbols

/*
  The following bits are set in the variable \texttt{showstring}.
*/

#define SHOWPURELEFTSEQ     FIRSTBIT  // show the left instance, no alignment
#define SHOWPURERIGHTSEQ    SECONDBIT // show the right instance, no alignment
#define SHOWALIGNABBREV     THIRDBIT  // show the matching substrings
                                      // (for mismatches of a against c use
                                      // [ac], no alignment
#define SHOWALIGNABBREVIUB  FOURTHBIT // show the matching substrings
                                      // (for mismatches of a against c use
                                      // IUB-code, no alignment
#define SHOWVMATCHXML       FIFTHBIT  // show xml output

#define MAXLINEWIDTH        1023  // maximal width of a line: 
                                  // must be value $2^i-1$ for some $i>0$

#define MASKTOUPPER         ((Uchar) 256)   // use macro toupper for masking
#define MASKTOLOWER         ((Uchar) 257)   // use macro tolower for masking

#define ARGUMENTLINEPREFIX  "# args="

#define MAXNUMOFARGS       32      // number of args of vmatch,
                                   // this should be enough

#define EXPLAINLENGTH        1024

/*
  The following macro checks if a palindromic match is to be output.
*/

/*
  The following macro calls one of the selection functions.
*/

#define CALLSELECT(MCINFO,ALPHA,VMS,FUN,QMS)\
        if((MCINFO)->outinfo.selectbundle != NULL &&\
           (MCINFO)->outinfo.selectbundle->FUN != NULL)\
        {\
          if((MCINFO)->outinfo.selectbundle->FUN(ALPHA,VMS,QMS) != 0)\
          {\
            return (Sint) -2;\
          }\
        }

/*
  copy the type \texttt{Outinfo}.
*/

#define COPYOUTINFO(DEST,SOURCE)\
        *(DEST) = *(SOURCE)

/*
  Assigndefault values to the digit parameters.
*/

#define ASSIGNDEFAULTDIGITS(DEST)\
        (DEST)->length = UintConst(5);\
        (DEST)->position1 = UintConst(6);\
        (DEST)->position2 = UintConst(6);\
        (DEST)->seqnum1 = UintConst(3);\
        (DEST)->seqnum2 = UintConst(3)

#define SHOWDIGITS(V) printf("%lu %lu %lu %lu %lu\n",\
                             (Showuint) (V)->length,\
                             (Showuint) (V)->position1,\
                             (Showuint) (V)->position2,\
                             (Showuint) (V)->seqnum1,\
                             (Showuint) (V)->seqnum2)

/*
  The following function calls the showverbose function if not NULL
*/

#define SHOWVERBOSE(SPTR,S)\
        if((SPTR)->showverbose != NULL)\
        {\
          (SPTR)->showverbose(S);\
        }

/*
  The following type specifies the width of the column used when showing
  the different numeric components of a match. These are always
  shown right adjusted in a column.
*/

typedef struct
{
  Uint length,       // number of digits to show the length
       position1,    // number of digits to show position1
       position2,    // number of digits to show position2
       seqnum1,      // number of digits to show seqnum1
       seqnum2;      // number of digits to show seqnum2
} Digits;            // \Typedef{Digits}

typedef struct
{
  Showdescinfo showdesc;   // format of description
  Uint showmode,           // mode of showing matches
       showstring,         // if 0 then no showstring, otherwise linewidth of
                           // string output, possible additional bits 
                           // SHOWPURELEFTSEQ, 
                           // SHOWPURERIGHTSEQ, 
                           // SHOWALIGNABBREV
                           // SHOWALIGNABBREVIUB
       sortmode;           // mode to sort the best matches
  Bestinfo bestinfo;       // offset for query number when matching subseq
  Digits digits;           // the width information when show numeric values
  FILE *outfp;             // pointer to output the matches
  BOOL domatchbuffering,   // buffer of matching
       useprecompiledselectbundle;
  SelectBundle *selectbundle;// function to select matches
  ArrayUint matchcounttab;  // table to count matches
  Multiseq *outvms,         // reference to multiseq of virtual tree
           *outqms,         // reference to multiseq with queries
           *outvirtorigdnams;// reference to original DNA sequences for which 
                             // sixframe index was created
  Alphabet *outvmsalpha;    // reference to alphabet for ms outvms points to
} Outinfo;                  // \Typedef{Outinfo}

typedef struct
{
  Uchar *showseqorig,
        *showseqcompare,
        *codon2aminobufferorig, 
        *codon2aminobuffermapped;
  Uint length;
} Showseqinfo;

//\Ignore{

#endif

//}
