//\IgnoreLatex{

#ifndef ALIGNMENT_H
#define ALIGNMENT_H
#include <stdio.h>
#include "types.h"
#include "arraydef.h"
#include "alphadef.h"
#include "genfile.h"

//}

/*
  This file stores some definition useful when computing an
  optimal alignment of two sequences.
*/

/*
  For each entry in the DP-matrix we store a single byte, and 
  use the three rightmost bits to mark which edge in the edit distance
  graph to trace back.
*/

#define REPLACEMENTBIT   ((Uchar) 1)          // replacement
#define DELETIONBIT      (((Uchar) 1) << 1)   // deletion
#define INSERTIONBIT     (((Uchar) 1) << 2)   // insertion
#define LENBIT           (((Uchar) 1) << 3)   // length of identical substring
#define MATCHBIT         (((Uchar) 1) << 4)   // match
#define MISMATCHBIT      (((Uchar) 1) << 5)   // mismatch

/*
  An edit operation is an Ushort value which stores
  in the two most significant bits one of the following edit operation
  or 0 in which case the remaining 14 bits are used for
  representing the length of a pair of identical substrings in the alignment.
  The largest such pair of identical substrings to store is thus of length
  \texttt{MAXIDENTICALLENGTH}.

  An intron is represented as a \texttt{DELETIONEOP} where the remaining 14 bits
  represent the length (01|length).
*/

#define MAXIDENTICALLENGTH ((UintConst(1) << 14) - 1)
#define DELETIONEOP        ((Editoperation) (UintConst(1) << 14)) // 01|00|0^12 
#define INSERTIONEOP       ((Editoperation) (UintConst(1) << 15)) // 10|00|0^12
#define MISMATCHEOP        ((Editoperation) (UintConst(3) << 14)) // 11|00|0^12

#define PRESUBJECT "Sbjct"
#define PREQUERY   "Query"
#define PREEMPTY   "       "   // two longer due to ': '

/*
  To be able to represent DNA/protein alignments we need additional multi edit 
  operations. We use two additional bits for this purpose (the third and 
  fourth most significant ones). 
  In this case the largest pair of identical substrings or introns which can
  be stored in a multi edit operation shrinks to 
  \texttt{MAXIDENTICALLENGTH_PROTEIN}.

  The introns used in DNA alignments (01|00|length) represent introns
  which start after a completely processed codon in DNA/protein alignments. 
  Introns which start after an incompletely processed codon are represented as
  follows:
  \begin{itemize}
  \item Introns which start after two bases of a codon are represented as a
        \texttt{DELETION_WITH_1_GAP_EOP}, because 1 base is left.
        The remaining 12 bits represent the length (01|01|length).
  \item Introns wich start after one base of a codon are represented as a
        \texttt{DELETION_WITH_2_GAPS_EOP}, because 2 bases are left.
        The remaining 12 bits represent the length (01|10|length).
  \end{itemize}
*/

#define MAXIDENTICALLENGTH_PROTEIN ((UintConst(1) << 12) - 1)

#define MISMATCH_WITH_1_GAP_EOP\
        ((Editoperation) (UintConst(13) << 12)) // 11|01|0^12

#define MISMATCH_WITH_2_GAPS_EOP\
        ((Editoperation) (UintConst(14) << 12)) // 11|10|0^12

#define DELETION_WITH_1_GAP_EOP\
        ((Editoperation) (UintConst(5) << 12))  // 01|01|0^12

#define DELETION_WITH_2_GAPS_EOP\
        ((Editoperation) (UintConst(6) << 12))  // 01|10|0^12

/*
  typedefs for the different types of edit operations
*/

typedef enum
{
  EOP_TYPE_MATCH = 0,
  EOP_TYPE_INTRON,
  EOP_TYPE_INTRON_WITH_1_BASE_LEFT,
  EOP_TYPE_INTRON_WITH_2_BASES_LEFT,
  EOP_TYPE_MISMATCH,
  EOP_TYPE_DELETION,
  EOP_TYPE_INSERTION,
  EOP_TYPE_MISMATCH_WITH_1_GAP,
  EOP_TYPE_MISMATCH_WITH_2_GAPS,
  EOP_TYPE_DELETION_WITH_1_GAP,
  EOP_TYPE_DELETION_WITH_2_GAPS,
  NUM_OF_EOP_TYPES
} Eoptype;

/*
  The following characters are special characters for switching the mode
  for showing different parts of an alignment.
*/

#define POSITIONCHAR ((Uchar) 248)  // show the position/length info
#define NUMBERCHAR   ((Uchar) 249)  // show a position of the aligned seq
#define HEADERCHAR   ((Uchar) 250)  // \texttt{PRESUBJECT}/\texttt{PREQUERY}
#define ERRORCHAR    ((Uchar) 251)  // mismatches, insertions and deletions
#define NORMALCHAR   ((Uchar) 252)  // match characters

/*
  The following flags are used to specify in which way an alignment is shown
  by the function showalignment:
*/

#define SHOWALIGNMENTFORWARD           UintConst(1)
#define SHOWALIGNMENTFANCY             (UintConst(1) <<  1)
#define SHOWALIGNMENTSELFCOMPARISON    (UintConst(1) <<  2)
#define SHOWALIGNMENTEQUAL             (UintConst(1) <<  3)
#define SHOWALIGNMENTNOSUBJECTANDQUERY (UintConst(1) <<  4)
#define SHOWALIGNMENTTENNERBLOCKS      (UintConst(1) <<  5)
#define SHOWALIGNMENTFORCEUPPER        (UintConst(1) <<  6)
#define SHOWALIGNMENTFORCELOWER        (UintConst(1) <<  7)
#define SHOWALIGNMENTREVERSESUBJECTPOS (UintConst(1) <<  8)
#define SHOWALIGNMENTREVERSEQUERYPOS   (UintConst(1) <<  9)
#define SHOWALIGNMENTWILDCARDIMPLOSION (UintConst(1) << 10)

#define MAKESAFLAG(FLAG,FORWARD,FANCY,SELFCOMPARISON)\
        FLAG = 0;\
        if(FORWARD)\
        {\
          FLAG |= SHOWALIGNMENTFORWARD;\
        }\
        if(FANCY)\
        {\
          FLAG |= SHOWALIGNMENTFANCY;\
        }\
        if(SELFCOMPARISON)\
        {\
          FLAG |= SHOWALIGNMENTSELFCOMPARISON;\
        }

#define CONCRETEINTRONSYMBOL    '.'
#define CONCRETEGAPSYMBOL       '-'
#define ABSTRACTINTRONSYMBOL    (SEPARATOR-4) // the abstract intron symbol
#define ABSTRACTGAPSYMBOL       (SEPARATOR-3) // the abstract gap symbol

/*
  The following structure contains pointers to functions showing 
  different parts of an alignment.
*/

typedef struct
{
  void *info;
  void (*positionchar)(void *,char *,Sint); 
  void (*normalchar)(void *,char *,Sint); 
  void (*errorchar)(void *,char *,Sint); 
  void (*headerchar)(void *,char *,Sint); 
  void (*numberchar)(void *,char *,Sint); 
} Fancydisplay;

typedef Ushort Editoperation;  // \Typedef{Editoperation}

/*
  An array of edit operation is an alignment.
*/

DECLAREARRAYSTRUCT(Editoperation);

/*
  For the greedy computation of the optimal alignment we need a distance
  value and a direction to store from which the maximal/minimal value came.
*/

typedef struct
{
  Sint dptabrow;
  Uchar dptabdirection; // one of the bits REPLACEMENTBIT, 
                        //                 DELETIONBIT, 
                        //                 INSERTIONBIT
} Frontvalue;      // \Typedef{Frontvalue}

DECLAREARRAYSTRUCT(Frontvalue);

/*
  For the display of long introns in short form we need the following
  structure.
*/

typedef struct
{
  Uint start,
       length;
} Shortintroninfo; // \Typedef{Shortintroninfo}

DECLAREARRAYSTRUCT(Shortintroninfo);

//\IgnoreLatex{

#endif

//}
