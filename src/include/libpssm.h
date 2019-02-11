#ifndef LIBPSSM_H
#define LIBPSSM_H
#ifndef ENABLE_MATCHER_PSSEARCH
#define ENABLE_MATCHER_PSSEARCH
#endif /* !ENABLE_MATCHER_PSSEARCH */
#ifndef ENABLE_MATCHER_PLASEARCH
#define ENABLE_MATCHER_PLASEARCH
#endif /* !ENABLE_MATCHER_PLASEARCH */
#ifndef ENABLE_MATCHER_PASEARCH
#define ENABLE_MATCHER_PASEARCH
#endif /* !ENABLE_MATCHER_PASEARCH */
#ifndef DEBUG
#ifndef LIBPSSM_FASTMEM
#define LIBPSSM_FASTMEM
#endif /* !LIBPSSM_FASTMEM */
#endif /* !DEBUG */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*
 * libbasics -- a convenience library for libautomata and libpssm
 * Copyright (C) 2000-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

#ifndef MATCHOPTIONSDEF_H
#define MATCHOPTIONSDEF_H

#include "types.h"

/*EE
 For clarity, this type should be used to pass matcher options.
*/
typedef Uint Matchoptions;

/*EE
 Report only the first match per suffix in the virtual suffix tree. If this
 bit is set, then only the first matching suffix is reported as a match and
 all suffixes with the same prefix are skipped (if the suffix can not be
 extended to a longer match).
*/
#define MATCHOPT_FIRSTONLY  UintConst(0x00000001)

/*EE
 If this option is given, then the shortest string matching the pattern will
 be reported, the algorithms will not try to prolong the match.
*/
#define MATCHOPT_SHORTEST   UintConst(0x00000002)

/*EE
 This option prevents the matchers from seeking the first matching position,
 that is they will check if the sequence at the given depth matches the
 expression and only then carry on searching for other matches. If combined
 with \texttt{MATCHOPT\SY{95}FIRSTONLY} (and probably
 \texttt{MATCHOPT\SY{95}SHORTEST}), the matchers simply check if one given
 location in the sequence matches the expression.
*/
#define MATCHOPT_FIXEDPOS   UintConst(0x00000004)

/*EE
 Perform search on the reverse sequence.
*/
#define MATCHOPT_REVERSE    UintConst(0x00000008)

#endif /* !MATCHOPTIONSDEF_H */
/*
 * libbasics -- a convenience library for libautomata and libpssm
 * Copyright (C) 2000-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

#ifndef MATCHREPORT_H
#define MATCHREPORT_H

#include "multidef.h"
#include "virtualdef.h"

/*EE
 Sequence type reported in a \texttt{Matchreport} structure.
*/
typedef enum
{
    MATCH_SEQ_NONE=0,    // nothing
    MATCH_SEQ_MULTISEQ,  // match found on \texttt{Multiseq} structure
    MATCH_SEQ_VTREE,     // match found on \texttt{Virtualtree} structure
    MATCH_SEQ_PLAIN      // match found in plain sequence
} Matchreportseqtype;

/*EE
 Arguments passed by a matcher. The arguments are specific to the matcher
 being used and provide the caller additional information.
*/
typedef enum
{
    MATCH_ARGS_NONE=0,    // match report without arguments
    MATCH_ARGS_PROFILE,   // match report with \texttt{profile} structure initialized
    MATCH_ARGS_HYPANODE,  // match report with \texttt{hnodes} structure initialized
    MATCH_ARGS_RPPATTERN, // match report with \texttt{rppattern} structure initialized
    MATCH_ARGS_OTHER      // non-specific match report with \texttt{voidptr} initialized
} Matchreportargstype;

/*EE
 This structure is passed to the caller as the only argument of his callback
 function for each match found by a matcher (see \texttt{Matchreportfunc}).
 It is designed to enable (but not require) the caller to write a single
 catch-all callback function that can handle any matcher answer and process
 it in a unified way. It provides all information about a match required to
 process it further. Some matchers provide additional information like a
 profile matrix.
*/
typedef struct
{
  Matchreportseqtype seqtype;  // content type, determines how the structure content must be read
  Matchreportargstype argtype; // which arguments are set
  Uint matchlen,               // length of match
       matchend,               // end of match as absolute offset
       suffix;                 // the suffix in the suffix array (only for \texttt{MATCH\SY{95}SEQ\SY{95}VTREE})
  Uint exactprefixlength;      // the length of the requested exact prefix
  BOOL reverse_search;         // \texttt{True} if the search was performed the reverse way
  Alphabet *alpha;             // pointer to an alphabet, or \texttt{NULL}
  void *user_data;             // pointer to user defined data

  Uchar insertions;
  Uchar deletions;
  Uchar replacements;
  
  // Union of sequences, content determined by \texttt{seqtype}.
  union
  {
    Multiseq *multiseq;
    Virtualtree *vtree;
    Uchar *sequence;
  } seq;

  // Union of arguments, content determined by \texttt{argtype}.
  union
  {
    struct
    {
      void *profile;        // pointer to a \texttt{Profilematrix} structure
      union
      {
        Sint   v_sint;
        Uint   v_uint;
        float  v_float;
        double v_double;
      } score;              // the score the match achieved
    } profile;

    struct
    {
      void **variables;     // pointer to an array of \texttt{Node} structures
      Uint numofvars,       // number of nodes in \texttt{variables}
           changedvar;      // the index of the variable that has changed its content
      void *cache;          // a pointer to a \texttt{Nodecache} structure to prevent evaluation
                            // of the variable tree when it is not required
    } hnodes;
    
    struct
    {
      const void *motifset; // pointer to a \texttt{Motifset} structure
      Uint motif;           // index of the \texttt{Motif} the match is found for
      void *parts;          // array of partial matches for raw patterns, absolute start and end positions
    } rppattern;
    
    struct
    {
      Uint id;              // arbitrary application specific ID
      void *voidptr;        // pointer to some data
    } other;
  } args;
} Matchreport;

/*EE
 Type of the callback function the caller is expected to provide when using
 a matcher.
*/
typedef Sint (*Matchreportfunc)(const Matchreport *match);

#define INITMATCHREPORT(MR,STYPE,ATYPE,RS,EPL,ALPHA,U)\
  (MR)->seqtype=(STYPE);\
  (MR)->argtype=(ATYPE);\
  (MR)->reverse_search=(RS);\
  (MR)->exactprefixlength=(EPL);\
  (MR)->alpha=(ALPHA);\
  (MR)->user_data=(U);\
  (MR)->insertions=(MR)->deletions=(MR)->replacements=0

#define INITMATCHREPORT_GENERIC(MR,RS,EPL,U)\
  INITMATCHREPORT(MR,MATCH_SEQ_NONE,MATCH_ARGS_NONE,RS,EPL,NULL,U)

#define INITMATCHREPORT_VTREE(MR,RS,EPL,U,V,A)\
  INITMATCHREPORT(MR,MATCH_SEQ_VTREE,MATCH_ARGS_NONE,RS,EPL,A,U);\
  (MR)->seq.vtree=(V)

#define INITMATCHREPORT_MULTISEQ(MR,RS,EPL,U,M,A)\
  INITMATCHREPORT(MR,MATCH_SEQ_MULTISEQ,MATCH_ARGS_NONE,RS,EPL,A,U);\
  (MR)->seq.multiseq=(M)

#define INITMATCHREPORT_PLAIN(MR,RS,EPL,U,S,A)\
  INITMATCHREPORT(MR,MATCH_SEQ_PLAIN,MATCH_ARGS_NONE,RS,EPL,A,U);\
  (MR)->seq.sequence=(S)

#define INITMATCHREPORT_PROFILE(MR,RS,EPL,U,P)\
  INITMATCHREPORT(MR,MATCH_SEQ_NONE,MATCH_ARGS_PROFILE,RS,EPL,(P)->alpha,U);\
  (MR)->args.profile.profile=(void *)(P)

#define INITMATCHREPORT_HYPANODE(MR,RS,EPL,U,V,A,VS,VN,C)\
  INITMATCHREPORT(MR,MATCH_SEQ_VTREE,MATCH_ARGS_HYPANODE,RS,EPL,A,U);\
  (MR)->seq.vtree=(V);\
  (MR)->args.hnodes.variables=(void **)(VS);\
  (MR)->args.hnodes.numofvars=(VN);\
  (MR)->args.hnodes.changedvar=0;\
  (MR)->args.hnodes.cache=(void *)(C)

#define INITMATCHREPORT_RPPATTERN(MR,RS,EPL,U,M,A,MS,PS)\
  INITMATCHREPORT(MR,MATCH_SEQ_MULTISEQ,MATCH_ARGS_RPPATTERN,RS,EPL,A,U);\
  (MR)->seq.multiseq=(M);\
  (MR)->args.rppattern.motifset=(MS);\
  (MR)->args.rppattern.motif=0;\
  (MR)->args.rppattern.parts=(PS)

#endif /* MATCHREPORT_H */
/*
 * libpssm -- a library for index based searching with PSSMs
 * Copyright (C) 2003-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

/*
*/

#include <float.h>

#include "types.h"
#include "alphadef.h"
#include "intbits.h"

typedef enum
{
  PROFILETYPE_SINT=0,
  PROFILETYPE_FLOAT,
  PROFILETYPE_NUMOFTYPES,
  PROFILETYPE_UNDEF
} Profiletype;

typedef union
{
  Sint v_sint;
  float v_float;
} Profilevalue;

typedef union
{
  Sint *v_sint;
  float *v_float;
} Profilevalueptr;

/*
 The ill definition of \texttt{PRSCORE\SY{95}SINT\SY{95}MAX} and
 \texttt{PRSCORE\SY{95}SINT\SY{95}MIN} prevents integer overflow warnings.
 Note that \texttt{PRSCORE\SY{95}SINT\SY{95}MIN} and
 \texttt{PRSCORE\SY{95}SINT\SY{95}MAX} are the minimum and maximum values
 the data type storing the score can represent.
*/
#define PRSCORE_SINT_MAX  ((Sint)((~(Sint)0)&~FIRSTTWOBITS))
#define PRSCORE_SINT_MIN  ((Sint)(-PRSCORE_SINT_MAX))

/*
 The same for floats, but nicer definitions (\texttt{float.h} must be used
 anyway, but we don't want to rely on \texttt{limits.h} in the above case).
*/
#define PRSCORE_FLOAT_MAX ((float)(FLT_MAX/2))
#define PRSCORE_FLOAT_MIN ((float)-PRSCORE_FLOAT_MAX)

#define PRTYPESWITCH(TYPE,CASE_SINT,CASE_FLOAT,FAILCODE)\
  switch(TYPE)\
  {\
   case PROFILETYPE_SINT:\
    CASE_SINT;\
    break;\
   case PROFILETYPE_FLOAT:\
    CASE_FLOAT;\
    break;\
   default:\
    FAILCODE;\
  }

#define PRGETVALUE(TYPE,VAR,V,FAILCODE)\
  PRTYPESWITCH(TYPE,\
               (VAR)=(V).v_sint,\
               (VAR)=(V).v_float,\
               FAILCODE)

#define PRGETCASTVALUE(TYPE,VAR,CTYPE,V,FAILCODE)\
  PRTYPESWITCH(TYPE,\
               (VAR)=(CTYPE)(V).v_sint,\
               (VAR)=(CTYPE)(V).v_float,\
               FAILCODE)

#define PRSETVALUE(TYPE,VAR,V,FAILCODE)\
  PRTYPESWITCH(TYPE,\
               (VAR).v_sint=(Sint)(V),\
               (VAR).v_float=(float)(V),\
               FAILCODE)

#define PRSETMINVALUE(TYPE,VAR,FAILCODE)\
  PRTYPESWITCH(TYPE,\
               (VAR).v_sint=PRSCORE_SINT_MIN,\
               (VAR).v_float=PRSCORE_FLOAT_MIN,\
               FAILCODE)

#define PRSETMAXVALUE(TYPE,VAR,FAILCODE)\
  PRTYPESWITCH(TYPE,\
               (VAR).v_sint=PRSCORE_SINT_MAX,\
               (VAR).v_float=PRSCORE_FLOAT_MAX,\
               FAILCODE)

#define PRSETTHRESHOLD(P,V,FAILCODE) PRSETVALUE((P)->type,(P)->threshold,V,FAILCODE)

/*EE
 Definition of a profile matrix. Table access can be accelerated by using
 the \texttt{multtab} table which stores the index of the $i$th row in
 array element $i$. To obtain, say, the third element of the fifth row, write
 \texttt{profile->scoretab[profile->multtab[4]+2]} where \texttt{profile} is
 a pointer to a \texttt{Profilematrix} structure.
*/
typedef struct
{
  Profiletype type;
  Uint dimension,                 // dimension of the matrix
       *multtab;                  // contains multiples of mapsize, which are indices into \texttt{scoretab}
  Profilevalueptr itmthreshold,   // intermediate thresholds
                  scoretab,       // table of scores
                  minvec,         // row minima
                  maxvec;         // row maxima
  Profilevalue threshold,         // global threshold
               minscore,          // minimum achivable score
               maxscore;          // maximum achivable score
  Alphabet *alpha;                // an alphabet
} Profilematrix;

#define PRGETSCORE(S,F,I,J)\
  ((S)->scoretab.F[(S)->multtab[(Uint)(I)]+(Uint)(J)])

#define PRSETSCORE(S,I,J,V,FAILCODE)\
  PRTYPESWITCH((S)->type,\
               PRGETSCORE(S,v_sint,I,J)=(Sint)(V),\
               PRGETSCORE(S,v_float,I,J)=(float)(V),\
               FAILCODE)

#define PRSETMINSCORE(S,I,J,FAILCODE)\
  PRTYPESWITCH((S)->type,\
               PRGETSCORE(S,v_sint,I,J)=PRSCORE_SINT_MIN,\
               PRGETSCORE(S,v_float,I,J)=PRSCORE_FLOAT_MIN,\
               FAILCODE)

#define PRSETMAXSCORE(S,I,J,FAILCODE)\
  PRTYPESWITCH((S)->type,\
               PRGETSCORE(S,v_sint,I,J)=PRSCORE_SINT_MAX,\
               PRGETSCORE(S,v_float,I,J)=PRSCORE_FLOAT_MAX,\
               FAILCODE)

/*
 * libpssm -- a library for index based searching with PSSMs
 * Copyright (C) 2003-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */


/*
*/
#include "virtualdef.h"

/*EE
 Profile array search algorithm for profile matrices.
*/

/*
 * libpssm -- a library for index based searching with PSSMs
 * Copyright (C) 2003-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */


/*EE
 Profile array search algorithm for profile matrices.
*/

/*
 * libpssm -- a library for index based searching with PSSMs
 * Copyright (C) 2003-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */


/*EE
 Simple search algorithm for profile matrices.
*/

/*
 * libbasics -- a convenience library for libautomata and libpssm
 * Copyright (C) 2000-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

/*
*/
#ifndef BOYERMOORE_H
#define BOYERMOORE_H

/*EE
 Implementation of Boyer and Moore's exact pattern searching algorithm.
*/
#include "types.h"

typedef struct
{
  Uchar *pattern;
  Uint patternlen;
  Uint delta1[UCHAR_MAX+1];
  Uint *delta2;
} Boyermooredata;

#endif /* BOYERMOORE_H */
/*
 * libpssm -- a library for index based searching with PSSMs
 * Copyright (C) 2003-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */


/*EE
 Profile matrix macros and definitions.
*/
#ifdef KBESTPROFILE
extern BOOL show_kbest_suffix_touches;
extern BOOL show_kbest_threshold_updates;
extern BOOL show_kbest_shortstats;
#endif /* KBESTPROFILE */

/*
 * libpssm -- a library for index based searching with PSSMs
 * Copyright (C) 2003-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */


/*EE
 Definitions for adaptive $k$-best PSSM matchers.
*/

/*EE
 Basic information about a match candidate. An array of these is maintained
 to keep track of the $k$ best matches.
*/
typedef struct
{
  Profilevalue score;
  Uint suffix;
} Goodmatch;

DECLAREARRAYSTRUCT(Goodmatch);

/*EE
 An array of at most $k$ matches as containers, index access done through a
 red/black tree.
*/
typedef struct
{
  ArrayGoodmatch matches;
  void *rbtree;
} Goodmatches;

/*
 * libpssm -- a library for index based searching with PSSMs
 * Copyright (C) 2003-2006  Robert Homann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */


/*
*/
#include "types.h"

extern const char libpssm_build_string[];
extern const char libpssm_feature_string[];
#ifdef __cplusplus
extern "C" {
#endif
Sint profilematrix_simplesearch(Multiseq *multiseq, Profilematrix *profile,
                                const PairUint *seqnumrange, Uint startpos,
                                Uint readingdepth, Matchoptions matchopt,
                                Matchreportfunc matchfunc, void *user_data);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint profilematrix_lookaheadsearch(Multiseq *multiseq, Profilematrix *profile,
                                   const PairUint *seqnumrange, Uint startpos,
                                   Uint readingdepth, Uint exactprefixlength,
                                   Matchoptions matchopt,
                                   Matchreportfunc matchfunc, void *user_data);

Sint profilematrix_lookaheadsearch_fast(Multiseq *multiseq,
                                        Profilematrix *profile,
                                        const PairUint *seqnumrange,
                                        Uint startpos,
                                        Matchreportfunc matchfunc,
                                        void *user_data);

Sint profilematrix_lookaheadsearch_kbest(Multiseq *multiseq,
                                         Profilematrix *profile,
                                         const PairUint *seqnumrange,
                                         Uint startpos, Uint readingdepth,
                                         Uint k, Matchreportfunc matchfunc,
                                         void *user_data);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint profilematrix_arraysearch(Virtualtree *vtree, Profilematrix *profile,
                               Matchoptions matchopt, Uint startsuffix,
                               Uint startdepth, Uint exactprefixlength,
                               Matchreportfunc matchfunc, void *user_data);

Sint profilematrix_arraysearch_fast(Virtualtree *vtree,
                                    Profilematrix *profile,
                                    Matchreportfunc matchfunc,
                                    void *user_data);

Sint profilematrix_arraysearch_cachetest(Virtualtree *vtree,
                                         Profilematrix *profile,
                                         Matchreportfunc matchfunc,
                                         void *user_data);

Sint profilematrix_arraysearch_kbest(Virtualtree *vtree,
                                     Profilematrix *profile, Uint k,
                                     Matchreportfunc matchfunc,
                                     void *user_data);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint profilematrix_init(/*@out@*/ Profilematrix *profile, Profiletype type,
                        Alphabet *alpha, Uint dimension);

Sint profilematrix_init_with_th(/*@out@*/ Profilematrix *profile,
                                Profiletype type, Alphabet *alpha,
                                Uint dimension, Profilevalue threshold);

void profilematrix_calc_thresholds(Profilematrix *profile);

void profilematrix_calc_minmax_vectors(Profilematrix *profile);

void profilematrix_copy(const Profilematrix *src,
                        /*@out@*/ Profilematrix *dest, BOOL with_itm);

void profilematrix_reverse(Profilematrix *profile, BOOL calc_itm);

void profilematrix_reverse_copy(const Profilematrix *src,
                                /*@out@*/ Profilematrix *dest,
                                BOOL calc_itm);

void profilematrix_recode(Profilematrix *profile, const Uchar *transalpha);

void profilematrix_recode_copy(const Profilematrix *src,
                               /*@out@*/ Profilematrix *dest,
                               const Uchar *transalpha);

Sint profilematrix_resize(Profilematrix *profile, Uint dimension, BOOL init);

void profilematrix_free_minmax(Profilematrix *profile);

void profilematrix_free(Profilematrix *profile);

void profilematrix_dump(const Profilematrix *profile, BOOL with_itm);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !LIBPSSM_H */
