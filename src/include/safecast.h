//\IgnoreLatex{

#ifndef SAFECAST_H
#define SAFECAST_H

#ifdef __cplusplus
  extern "C" {
#endif

Sint safecasttoSint(char *filename, Uint line, Uint value);
Uint safecasttoUint(char *filename, Uint line, Sint value);

#ifdef __cplusplus
}
#endif

//}


/*
  This file defines macros to simplify the calls to the functions
  \begin{itemize}
  \item
  \texttt{safecasttoSint}
  \item
  \texttt{safecasttoUint}
  \end{itemize}
*/

#define SAFECASTTOSINT(A)\
        safecasttoSint(__FILE__, (Uint) __LINE__, (A))

#define SAFECASTTOUINT(A)\
        safecasttoUint(__FILE__, (Uint) __LINE__, (A))

#endif
