

//\IgnoreLatex{

#ifndef IUBDEF_H
#define IUBDEF_H

#include "divmodmul.h"

//}

/* 
  Given a pair of different bases \texttt{B1} and \texttt{B2}, 
  \texttt{IUBSYMBOL(B1,B2)} returns the IUB-symbol denoting the set 
  of bases consisting of \texttt{B1} and \texttt{B2}. The corresponding 
  mapping is defined by the following table.

  \[\begin{array}{*{5}{c}}
     &A&C&G&T\\\hline
    A& &m&r&w\\
    C&m& &s&y\\
    G&r&s& &k\\
    T&w&y&k&\\\hline
    \end{array}\]
*/

#define IUBSTRING        "-MRWM-SYRS-KWYK-"
#define IUBSYMBOL(B1,B2) IUBSTRING[((MULT4(B1)) + (B2))]

//\IgnoreLatex{

#endif

//}
