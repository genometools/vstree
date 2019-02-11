//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "types.h"
#include "fhandledef.h"
#include "debugdef.h"
#include "errordef.h"
#include "failures.h"
#include "megabytes.h"

#include "filehandle.pr"

//}

/*EE
  This module defines functions for handling debug levels and
  other information related to producing debugging messages.
  The debug mechanism is only available if the \texttt{DEBUG} compiler 
  flag is used.
*/

#ifdef DEBUG

#define CHECKTYPESIZE(T,OP,S)\
        if(sizeof(T) OP (S))\
        {\
          DEBUG4(1,"# sizeof(%s) %s (%ld bytes,%ld bits) as expected\n",\
                  #T,#OP,(Showsint) sizeof(T),\
                         (Showsint) (CHAR_BIT * sizeof(T)));\
        } else\
        {\
          fprintf(stderr,"typesize constraint\n");\
          fprintf(stderr,"  sizeof(%s) = (%ld bytes,%ld bits) %s %lu bytes\n",\
                  #T,\
                  (Showsint) sizeof(T),\
                  (Showsint) (CHAR_BIT * sizeof(T)),\
                  #OP,\
                  (Showuint) (S));\
          fprintf(stderr,"does not hold\n");\
          exit(EXIT_FAILURE);\
        }

/*
  The following function checks some type constraints 
*/

#define CHECKALLTYPESIZES\
        CHECKTYPESIZE(char,==,(size_t) 1)\
        CHECKTYPESIZE(short,==,(size_t) 2)\
        CHECKTYPESIZE(int,==,(size_t) 4)\
        CHECKTYPESIZE(long,>=,(size_t) 4)\
        CHECKTYPESIZE(void *,>=,(size_t) 4)

static Sint debuglevel = 0;        // the value of \texttt{DEBUGLEVEL}
static BOOL debugwhere = False;    // the value of \texttt{DEBUGWHERE}
/*@null@*/ static FILE 
           *debugfileptr = NULL;  // the file pointer to show the debug info

/*EE
  The following function returns the \texttt{DEBUGLEVEL}.
*/

Sint getdebuglevel(void)
{
  return debuglevel;
}

/*EE
  The following function returns the value of \texttt{DEBUGWHERE}.
*/

BOOL getdebugwhere(void)
{
  return debugwhere;
}

/*EE
  The following function sets the debug level by looking up the 
  environment variable \texttt{DEBUGLEVEL}. Moreover, the environment 
  variable \texttt{DEBUGWHERE} is read and \texttt{debugwhere} is set
  accordingly.
*/

void setdebuglevel(void)
{
  char *envstring;
  Scaninteger readint;

  debugfileptr = stdout;
  if((envstring = getenv("DEBUGLEVEL")) != NULL)
  {
    if(!(strlen(envstring) == (size_t) 1 && 
       isdigit((Ctypeargumenttype) *envstring)))
    {
      fprintf(stderr,"environment variable DEBUGLEVEL=%s, ",envstring);
      fprintf(stderr,"it must be a digit between 0 and %lu\n",
              (Showuint) MAXDEBUGLEVEL);
      exit(EXIT_FAILURE);
    }
    if (sscanf(envstring,"%ld",&readint) != 1 || readint < 0 ||
        readint > (Scaninteger) MAXDEBUGLEVEL)
    {
      fprintf(stderr,"environment variable DEBUGLEVEL=%s, ",envstring);
      fprintf(stderr,"it must be a digit between 0 and %lu\n",
              (Showuint) MAXDEBUGLEVEL);
      exit(EXIT_FAILURE);
    }
    debuglevel = (Sint) readint;
  }
  if((envstring = getenv("DEBUGWHERE")) != (char *) NULL)
  {
    if(strcmp(envstring,"on") == 0)
    {
      debugwhere = True;
    } else
    {
      if(strcmp(envstring,"off") == 0)
      {
        debugwhere = False;
      } else
      {
        fprintf(stderr,"environment variable DEBUGWHERE=%s, ",envstring);
        fprintf(stderr,"it must be set to \"on\" or \"off\"\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  CHECKALLTYPESIZES
}

/*EE
  The following function opens the given filename for writing the debug 
  messages  to. It also sets the debug level. 
  This function is called only very rarely. If only \texttt{setdebuglevel}
  is called, then the output goes to standard output.
*/

void setdebuglevelfilename(char *filename)
{
  debugfileptr = CREATEFILEHANDLE(filename,WRITEMODE);
  if(debugfileptr == NULL)
  {
    fprintf(stderr,"%s\n",messagespace());
    NOTSUPPOSED;
  }
  setdebuglevel();
}

/*EE
  The following function looks up the output pointer. 
*/

FILE *getdbgfp(void)
{
  if(debugfileptr == NULL)
  {
    fprintf(stderr,"DEBUGLEVELSET not called\n");
    exit(EXIT_FAILURE);
  }
  return debugfileptr;
}

/*EE
  The following function closes the debug output pointer, if it is not 
  standard out.
*/

void debugclosefile(void)
{
  if(debugfileptr == NULL)
  {
    fprintf(stderr,"cannot close debugfileptr\n");
    exit(EXIT_FAILURE);
  }
  if(DELETEFILEHANDLE(debugfileptr) != 0)
  {
    fprintf(stderr,"%s\n",messagespace());
    NOTSUPPOSED;
  }
}

#endif  /* DEBUG */
