

//\IgnoreLatex{

#ifndef OPTDESC_H
#define OPTDESC_H
#include <ctype.h>
#include "types.h"
#include "errordef.h"

//}

/*
  This file defines some macros to manipulate and a type to store
  information about options.
*/

/*
  The macro \texttt{OPTION} fills the component of a structure of 
  type \texttt{OptionDesc}. This macro should not be used. It
  is mainly for compatibility of the code.
*/

#define OPTION(S,N,O,D)\
        S[N].optname = O;\
        S[N].description = D;\
        S[N].optval = N;\
        S[N].isalreadyset = False;\
        S[N].declared = True

/*
  The macro \texttt{ADDOPTION} has a similar effect, but adding the 
  option is done via the function \texttt{addoption}. This is the
  recommended way to add an option.
*/

#define ADDOPTION(N,O,D)\
        if(addoption(&options[0],(Uint) NUMOFOPTIONS,(Uint) N,O,D) != 0)\
        {\
          return (Sint) -1;\
        }

#define ADDOPTVMATCHVERSION\
        ADDOPTION(OPTVMATCHVERSION,"-version",\
                                   "show the version of the Vmatch package")

/*
  The following macro checks if an option is already set.
*/

#define ISSET(N)  (options[N].isalreadyset)

/*
  The following is the stopsymbol for lists of option numbers
*/

#define STOPSUBTABLE ((Optionnumbertype) -1)

/*
  The following macro checks if an option is used. If not, then
  an error is reported.
*/

#define OPTIONMANDATORY(A)\
        if(!ISSET(A))\
        {\
          ERROR1("option %s is mandatory",options[A].optname);\
          return (Sint) -1;\
        }

/*
  The following four macros check if all options \texttt{B} 
  (\texttt{C}, \texttt{D}, \texttt{E}) are used, whenever option 
  \texttt{A} is used.
*/

#define OPTIONIMPLY(A,B)\
        if(ISSET(A) && !ISSET(B))\
        {\
          ERROR2("option %s requires option %s",\
                  options[A].optname,options[B].optname);\
          return (Sint) -1;\
        }

#define OPTIONIMPLYEITHER2(A,B,C)\
        if(ISSET(A))\
        {\
          if(!ISSET(B) && !ISSET(C))\
          {\
            ERROR3("option %s requires either option %s or %s",\
                    options[A].optname,options[B].optname,\
                    options[C].optname);\
            return (Sint) -1;\
          }\
        }

#define OPTIONIMPLYEITHER3(A,B,C,D)\
        if(ISSET(A))\
        {\
          if(!ISSET(B) && !ISSET(C) && !ISSET(D))\
          {\
            ERROR4("option %s requires one of the options %s, %s, %s",\
                    options[A].optname,\
                    options[B].optname,\
                    options[C].optname,\
                    options[D].optname);\
            return (Sint) -1;\
          }\
        }

#define OPTIONIMPLYEITHER4(A,B,C,D,E)\
        if(ISSET(A))\
        {\
          if(!ISSET(B) && !ISSET(C) && !ISSET(D) && !ISSET(E))\
          {\
            ERROR5("option %s requires one of the options %s, %s, %s, %s",\
                    options[A].optname,\
                    options[B].optname,\
                    options[C].optname,\
                    options[D].optname,\
                    options[E].optname);\
            return (Sint) -1;\
          }\
        }

/*
  The following macro checks if two options are used simultaneously.
  If so, then an error is reported.
*/

#define OPTIONEXCLUDE(A,B)\
        if(ISSET(A) && ISSET(B))\
        {\
          ERROR2("option %s and option %s exclude each other",\
                  options[A].optname,options[B].optname);\
          return (Sint) -1;\
        }

/*
  The following specifies the length of the space buffer to store
  the help text for option transnum used in mkdna6idx and vmatch.
*/

#define MAXTRANSNAMESPACE 1023

/*
  The following macro checks if the string starts with the symbol
  \texttt{\symbol{45}}, and hence is an option.
*/

#define ISOPTION(OPT) (OPT[0] == '-' && isalpha((Ctypeargumenttype) OPT[1]))

/*
  The following macro checks if there is a missing argument
  in the argument list. FIRSTIGNORE is the first element which 
  is ignored.
*/

#define CHECKMISSINGARGUMENTGENERIC(OPTNAME,FIRSTIGNORE)\
        if(argnum >= (Uint) (FIRSTIGNORE) || ISOPTION(argv[argnum]))\
        {\
          ERROR1("missing argument to option %s",OPTNAME);\
          return (Sint) -1;\
        }

/*
  The following macro checks if there is a missing argument
  in the argument list. The last element in argv at index
  argc-1 is ignored.
*/

#define CHECKMISSINGARGUMENTWITHOUTLAST\
        CHECKMISSINGARGUMENTGENERIC(options[(Uint) optval].optname,argc-1)

/*
  The following macro checks if there is a missing argument
  in the argument list. The last element in argv at index
  argc-1 is taken into account.
*/

#define CHECKMISSINGARGUMENT\
        CHECKMISSINGARGUMENTGENERIC(options[(Uint) optval].optname,argc)

/*
  The following macro tries to read an integer from the
  next argument and checks if the given operation is successful.
*/

#define READINTGENERIC(OPTNAME,VAR,FIRSTIGNORE,VARCMP,S)\
        argnum++;\
        CHECKMISSINGARGUMENTGENERIC(OPTNAME,FIRSTIGNORE);\
        if(sscanf(argv[argnum],"%ld",&VAR) != 1 || (VAR) VARCMP 0)\
        {\
          ERROR3("argument \"%s\" of option %s must be %s number",\
                 argv[argnum],OPTNAME,S);\
          return (Sint) -1;\
        }

/*
  The following macro tries to read a percentage value from the
  next argument.
*/

#define READPERCENTAGE(OPTNAME,VAR,S)\
        READINTGENERIC(OPTNAME,VAR,argc-1,<,"non-negative");\
        if(VAR > (Scaninteger) 100)\
        {\
          ERROR3("%s argument \"%s\" of option %s must be percentage "\
                 "in range [0,100]",\
                 S,argv[argnum],OPTNAME);\
          return (Sint) -1;\
        }

/*
  The following macro tries to read a double value from the
  next argument and checks if the given operation is successful.
*/

#define READDOUBLEGENERIC(OPTNAME,VAR,FIRSTIGNORE,VARCMP,S)\
        argnum++;\
        CHECKMISSINGARGUMENTGENERIC(OPTNAME,FIRSTIGNORE);\
        if(sscanf(argv[argnum],"%lf",&VAR) != 1 || VAR VARCMP 0.0)\
        {\
          ERROR3("argument \"%s\" of option %s must be %s number",\
                 argv[argnum],OPTNAME,S);\
          return (Sint) -1;\
        }

/*
  The following macro checks if the current option has more arguments.
  FIRSTIGNORE is the first element which is ignored in the argument
  list.
*/

#define MOREARGOPTSGENERIC(ARGNUM,FIRSTIGNORE)\
        ((Argctype) ((ARGNUM) + 1) < (FIRSTIGNORE)\
         && !ISOPTION(argv[(ARGNUM)+1]))

/*
  The following macro checks if there are more arguments in the 
  argument list. The last element in argv at index argc-1 is ignored.
*/

#define MOREARGOPTSWITHOUTLAST(ARGNUM)\
        MOREARGOPTSGENERIC(ARGNUM,argc-1)

/*
  The following macro checks if there are more arguments in the 
  argument list. The last element in argv at index argc-1 is not ignored.
*/

#define MOREARGOPTSWITHLAST(ARGNUM)\
        MOREARGOPTSGENERIC(ARGNUM,argc)

/*
  The following macro calls showprogramversion if the option -version is used.
*/

#define CALLSHOWPROGRAMVERSION(PROG)\
        if(argc == 2 && strcmp(argv[1],"-version") == 0)\
        {\
          showprogramversion(stdout,PROG);\
          return EXIT_SUCCESS;\
        }

#define CALLSHOWPROGRAMVERSIONWITHLICENSE(PROG, LICENSE)\
        if(argc == 2 && strcmp(argv[1],"-version") == 0)\
        {\
          showprogramversion(stdout,PROG);\
          putchar('\n');\
          lm_license_show_info(LICENSE);\
          return EXIT_SUCCESS;\
        }

/*
  An option is described by the following type.
*/

typedef struct 
{
  char *optname,             // the option string, begins with -
       *description;         // help text describing purpose of option
  Uint optval;               // the unique number of an option
  BOOL isalreadyset,         // has the option already been set?
       declared;             // is the option declared by
                             // a line of the form \texttt{ADDOPTION}
} OptionDescription;         // \Typedef{OptionDescription}

/*
  A group of options is specified by the number of the first option
  in the group and a description of the group
*/

typedef struct
{
  Uint firstofgroup;
  char *groupdescription;
} OptionGroup;

/*
  The following type allows to specify key words with associated
  bits to be set by the function 
  
*/

typedef struct
{
  char *name;
  Uint bitmask;
} Optionargmodedesc;


//\IgnoreLatex{

#endif 

//}
