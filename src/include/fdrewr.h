

//\IgnoreLatex{

#ifndef FDREWR_H
#define FDREWR_H
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//}

/*
  This module defines some macros for opening, creating, reading
  and writing file via file descriptions.
  These macros are outdated and should not
  be used any more. Instead use the macros in fhandledef.h.
*/

#define FDOPENMODE(FD,FILENAME,FLAGS,MODE)\
        if((FD = open(FILENAME,FLAGS,MODE)) == -1)\
        {\
          fprintf(stderr,"Cannot open file \"%s\": %s\n",\
                         FILENAME,\
                         strerror(errno));\
          exit(EXIT_FAILURE);\
        }

#define FDOPEN(FD,FILENAME,FLAGS)\
        if((FD = open(FILENAME,FLAGS)) == -1)\
        {\
          fprintf(stderr,"Cannot open file \"%s\": %s\n",\
                         FILENAME,\
                         strerror(errno));\
          exit(EXIT_FAILURE);\
        }

#define FDCREAT(FD,FILENAME,FLAGS)\
        if((FD = creat(FILENAME,O_WRONLY)) == -1)\
        {\
          fprintf(stderr,"Cannot create file \"%s\": %s\n",\
                         FILENAME.\
                         strerror(errno));\
          exit(EXIT_FAILURE);\
        }

#define BINREAD(FD,BUF,SIZE)\
        if(read(FD,BUF,SIZE) != (SIZE))\
        {\
          fprintf(stderr,"%s line %lu: read failed: %s\n",\
                  __FILE__,\
                  (Showuint) __LINE__,\
                  strerror(errno));\
          exit(EXIT_FAILURE);\
        }

#define BINWRITE(FD,BUF,SIZE)\
        if(write(FD,BUF,SIZE) != (SIZE))\
        {\
          fprintf(stderr,"%s line %lu: fwrite failed: %s\n",\
                  __FILE__,\
                  (Showuint) __LINE__,\
                 strerror(errno));\
          exit(EXIT_FAILURE);\
        }

//\IgnoreLatex{

#endif

//}
