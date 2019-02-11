//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "file.h"

//}

/*EE
  This file implements the function \texttt{getdirname} according to the 
  specification in
  \begin{center}
  \texttt{http://www.unix-systems.org/onlinepubs/7908799/xsh/dirname.html}
  \end{center}
  \texttt{getdirname} is equivalent to the function \texttt{dirname} which
  is available on most unix systems, but in different libraries and 
  slightly different functionality.
  
  \texttt{dirname} takes a pointer to a character string that 
  contains a pathname, and returns a pointer to a string that 
  is a pathname of the parent directory of that file. 
  Trailing \CharQuote{/} characters in the path are not counted as part of
  the path.  If path does not contain a \CharQuote{/}, then \texttt{dirname}
  returns a pointer to the string \StringQuote{.}. If path is a null pointer 
  or points to an empty string, \texttt{getdirname} returns a pointer to 
  the string \StringQuote{.}. 

  The \texttt{getdirname} function returns a pointer to a string that is 
  the parent directory of path. If path is a null pointer or points to 
  an empty string, a pointer to a string \StringQuote{.} is returned. 

  \begin{center}
  \begin{tabular}{|l|l|}\hline
  \multicolumn{2}{|c|}{Example:}\\
  Input String&Output String\\\hline
  \StringQuote{/usr/lib}  &\StringQuote{/usr}\\
  \StringQuote{/usr/}     &\StringQuote{/}\\
  \StringQuote{usr}       &\StringQuote{.}\\
  \StringQuote{/}         &\StringQuote{/}\\
  \StringQuote{.}         &\StringQuote{.}\\
  \StringQuote{..}        &\StringQuote{.}\\
  \hline
  \end{tabular}
  \end{center}

  The \texttt{getdirname} function may modify the string pointed to by path, 
  and may return a pointer to static storage that may then be 
  overwritten by subsequent calls to \texttt{getdirname}. 

  The \texttt{getdirname} and \texttt{basename} functions together 
  yield a complete pathname. The expression \texttt{dirname(path)} 
  obtains the pathname of the directory where \texttt{basename(path)} is found. 
*/

/*@null@*/ char *getdirname(char *path)
{
  char *sbuf;
  Uint i, lastslash = 0, countslash = 0;
  Uchar lastslashdefined = 0;
  size_t pathlen;

  pathlen = strlen(path);
  sbuf = malloc(pathlen+2);
  if(sbuf == NULL)
  {
    fprintf(stderr,"getdirname: cannot allocate memory for copy of \"%s\"\n",
                   path);
    exit(EXIT_FAILURE);
  }
  if(path == NULL || *path == '\0')
  {
    strcpy(sbuf,".");
    return sbuf;
  }
  strcpy(sbuf,path);
  for(i=0; sbuf[i] != '\0'; i++)
  {
    if(sbuf[i] == PATH_SEPARATOR)
    {
      lastslash = i;
      lastslashdefined = (Uchar) 1;
      countslash++;
    } 
  }
  if(lastslashdefined)
  {
    if(lastslash == i-1 && countslash == UintConst(1))
    {
      strcpy(sbuf,"");
    } else
    {
      for(i=lastslash; i > 0 && sbuf[i] == PATH_SEPARATOR; i--)
      {
        sbuf[i] = '\0';
      }
    }
  } else
  {
    strcpy(sbuf,".");
  }
  return sbuf;
}
