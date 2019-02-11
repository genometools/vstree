
//\IgnoreLatex{

#ifndef XDROPDEF_H
#define XDROPDEF_H
#include <limits.h>
#include "types.h"
#include "alignment.h"

//}

/*
  This file defines some macros and types used for the Xdrop-extension
  algorithm.
*/

#define MINUSINFINITYSCORE ((Xdropscore) integermin)  // minus infinity
#define MISMATCHSCORE      ((Xdropscore) (-1))     // score of mismatch
#define MATCHSCORE         ((Xdropscore) 2)        // score of match
#define HALFMATCHSCORE     ((Xdropscore) 1)        // half the score of match
#define INDELSCORE         (MISMATCHSCORE - HALFMATCHSCORE) // score of ins/del
#define REPSCORE(A,B)      (((A) == (B)) ? MATCHSCORE : MISMATCHSCORE)

/*
  The following functions extend seeds to the right and to the left,
  respectively. \texttt{bestmatch} stores information about the best match
  found. \texttt{useq} is the first sequence and \texttt{vseq} is the
  second sequence. \texttt{ulen} and \texttt{vlen} are the
  sequence length. If an alignment has score smaller than
  \(M-\texttt{xdropbelowscore}\), then this alignment is not extended
  any more. \(M\) is the maximal score achieved so far.
*/

#define EVALXDROPEDITRIGHT\
        void evaleditxdropright(Xdropbest *bestmatch,\
                                Uchar *useq,Sint ulen,\
                                Uchar *vseq,Sint vlen,\
                                Xdropscore xdropbelowscore)
#define EVALXDROPEDITLEFT\
        void evaleditxdropleft(Xdropbest *bestmatch,\
                               Uchar *useq,Sint ulen,\
                               Uchar *vseq,Sint vlen,\
                               Xdropscore xdropbelowscore)

/*
  For the xdrop algorithms we use the following convention.
  If \texttt{xdropbelowscore} is 0, then no extension is necessary.
  If \texttt{xdropbelowscore} is smaller than 0, then the extension 
  only allows replacements and we use \texttt{-xdropbelowscore} as a 
  threshold.
  If \texttt{xdropbelowscore} is larger than 0, then the extension 
  only allows replacements and we use \texttt{xdropbelowscore} as a 
  threshold.
*/

#define UNDEFXDROPBELOWSCORE 0
#define UNDEFXDROPLEASTSCORE 0

/*
  For the best extension we report the score and the 
  maximal pair of values in vertical (\texttt{ivalue}) and 
  in horizontal direction (\texttt{jvalue}).
*/

typedef struct
{
  Uint ivalue, jvalue;
  Xdropscore score;
} Xdropbest;                     // \Typedef{Xdropbest}

typedef struct
{
  Sint smallestk,     // smallest value of k
       largestk;      // largest value of k
  Uint scoreoffset;   // reference to scoretab to store the scores
  Xdropscore ttabvalue;
} AlignmentGeneration;

typedef struct
{
  Sint dbest,
       kbest,
       ivalue,
       jvalue;
  Uint indelcount;
  Xdropscore score;
} Galignbest;

DECLAREARRAYSTRUCT(AlignmentGeneration);
DECLAREARRAYSTRUCT(Xdropscore);

EVALXDROPEDITLEFT;
EVALXDROPEDITRIGHT;

#ifdef __cplusplus
  extern "C" {
#endif

void evalxdroptableright(Galignbest *galignbest,
                         ArrayAlignmentGeneration *generations,
                         ArrayXdropscore *scoretab,
                         Uchar *useq,Sint ulen,
                         Uchar *vseq,Sint vlen,
                         Xdropscore xdropbelowscore);

void evalxdroptableleft(Galignbest *galignbest,
                        ArrayAlignmentGeneration *generations,
                        ArrayXdropscore *scoretab,
                        Uchar *useq,Sint ulen,
                        Uchar *vseq,Sint vlen,
                        Xdropscore xdropbelowscore);

Uint onexdropalignment1(BOOL forward,ArrayEditoperation *alignment,
                        Uchar *useq,Uint ulen,Uchar *vseq, Uint vlen);

Uint onexdropalignment2(BOOL forward,ArrayEditoperation *alignment,
                        Uchar *useq,Sint ulen,Uchar *vseq,Sint vlen,
                        Xdropscore xdropbelowscore);

#ifdef __cplusplus
}
#endif

//\IgnoreLatex{

#endif

//}
