//\IgnoreLatex{

#ifndef FOPEN_H
#define FOPEN_H
#include <stdio.h>
#include <stdlib.h>

//}

/*
  This file defines macros for opening and writing files via
  file pointers. These macros are outdated and should not
  be used any more. Instead use the macros in fhandledef.h.
*/

#define FILEOPEN(FP,FILENAME,MODE)\
        if ((FP = fopen(FILENAME,MODE)) == NULL)\
        {\
          fprintf(stderr,"(%s,%lu): Cannot open file \"%s\"\n",\
                  __FILE__,(Showuint) __LINE__,FILENAME);\
          exit(EXIT_FAILURE);\
        }

#define FPBINWRITE(FP,BUF,SIZE)\
        if(fwrite(BUF,SIZE,1,FP) != 1)\
        {\
          fprintf(stderr,"(%s,%lu): fwrite failed\n",__FILE__,\
                  (Showuint) __LINE__);\
          exit(EXIT_FAILURE);\
        }

#define FILEWRITEGEN(V,T,N,FP,RET)\
        if(fwrite(V,sizeof(T),N,FP) != N)\
        {\
          ERROR4("%s %lu: Cannot write %lu items of type %s",\
                 __FILE__,(Showuint) __LINE__,(Showuint) (N),#T);\
          return RET;\
        }

#define FILEWRITE(V,T,N,FP) FILEWRITEGEN(V,T,N,FP,-1)

//\IgnoreLatex{

#endif

//}
