//\IgnoreLatex{

#ifndef FHANDLEDEF_H
#define FHANDLEDEF_H
#include <stdio.h>
#include "types.h"
#include "errordef.h"

#define TMPFILESUFFIX        "XXXXXX"
#define NUMBEROFX            strlen(TMPFILESUFFIX)
#define TMPFILEPREFIX        "/tmp/Vmatch"

#define WRITEMODE  "wb"   // writing in binary mode, important for MS-Windows
#define READMODE   "rb"   // reading in binary mode, important for MS-Windows
#define APPENDMODE "ab"   // appending in binary mode, important for MS-Windows

typedef struct
{
  char *tmpfilenamebuffer;
  Uint tmpfilenamelength;
  FILE *tmpfileptr;
} Tmpfiledesc;

//}

/*
  This file defines macros to simplify the calls to the
  functions 
  \begin{itemize}
  \item
  \texttt{createfilehandle}, 
  \item
  \texttt{maketmpfile},
  \item
  \texttt{deletefilehandle},
  \item
  \texttt{writetofilehandle},
  \item 
  and \texttt{readfromfilehandle}.
  \end{itemize}
*/

#define CREATEFILEHANDLE(PATH,MODE)\
        createfilehandle(__FILE__,(Uint) __LINE__,PATH,MODE)

#define MAKETMPFILE(TMPFILEDESC,FILEPREFIX)\
        maketmpfile(__FILE__,(Uint) __LINE__,TMPFILEDESC,FILEPREFIX)

#define DELETEFILEHANDLE(FP)\
        deletefilehandle(__FILE__,(Uint) __LINE__,FP)

#define WRITETOFILEHANDLE(PTR,SZ,NMEMB,FP)\
        writetofilehandle(__FILE__,(Uint) __LINE__,PTR,SZ,NMEMB,FP)

#define READFROMFILEHANDLE(PTR,SZ,NMEMB,FP)\
        readfromfilehandle(__FILE__,(Uint) __LINE__,PTR,SZ,NMEMB,FP)

void checkfilehandles(void);

//\IgnoreLatex{

#endif

//}
