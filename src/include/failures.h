

#ifndef FAILURES_H
#define FAILURES_H

/*
  The following is a general error message which leads to a termination
  of the program.
*/

#define NOTSUPPOSED\
        fprintf(stderr,"%s: line %lu: This case is not supposed to occur\n",\
                       __FILE__,(Showuint) __LINE__);\
        exit(EXIT_FAILURE)

#define DEFAULTFAILURE default:\
                       fprintf(stderr, "line %lu in file %s: this point "\
                                       "should never be reached; aborting "\
                                       "program\n" , (Showuint) __LINE__,\
                                       __FILE__);\
                       exit(EXIT_FAILURE)


#define NOTIMPLEMENTED\
        fprintf(stderr, "file %s, line %lu: this case is not implemented\n",\
                __FILE__,(Showuint) __LINE__);\
        exit(EXIT_FAILURE)

/*
  The following macro checks a ptr. If it is \texttt{NULL}, then the 
  program terminates with an error.
*/

#ifdef DEBUG
#define NOTSUPPOSEDTOBENULL(PTR)\
        if((PTR) == NULL)\
        {\
          NOTSUPPOSED;\
        }
#else
#define NOTSUPPOSEDTOBENULL(PTR) /* Nothing */
#endif

/*
  The following macro return with -1 if the given pointer is the NULL
  pointer.
*/

#define IFNULLRETURN(PTR)\
        if((PTR) == NULL)\
        {\
          return (Sint) -1;\
        }

#endif
