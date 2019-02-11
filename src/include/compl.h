//\IgnoreLatex{

#ifndef COMPL_H
#define COMPL_H
#include <stdio.h>
#include <stdlib.h>

//}

/* 
  The following macro defines a switch statement assigning to the 
  variable \texttt{VL} the complement of \texttt{VR}.
*/

#define ASSIGNCOMPLEMENT(VL,VR)\
        switch(VR)\
        {\
          case 'A' : VL = (Uchar) 'T'; break;\
          case 'C' : VL = (Uchar) 'G'; break;\
          case 'G' : VL = (Uchar) 'C'; break;\
          case 'T' : VL = (Uchar) 'A'; break;\
          case 'a' : VL = (Uchar) 't'; break;\
          case 'c' : VL = (Uchar) 'g'; break;\
          case 'g' : VL = (Uchar) 'c'; break;\
          case 't' : VL = (Uchar) 'a'; break;\
          default  : fprintf(stderr,"Complement of '%c'(%lu) not defined\n",\
                            VR,(Showuint) (VR));\
                     exit(EXIT_FAILURE);\
        }

//\IgnoreLatex{

#endif

//}
