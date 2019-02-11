//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "file.h"

//}

/*EE
  This file implements the function \texttt{getbasename} according to the 
  specification in
  \begin{center}
  \texttt{http://www.unix-systems.org/onlinepubs/7908799/xsh/basename.html}
  \end{center}
  \texttt{getbasename} is equivalent to the function \texttt{basename} which
  is available on most unix systems, but in different libraries and 
  slightly different functionality.

  \texttt{getbasename} takes the pathname pointed to by 
  \texttt{path} and returns a pointer to the final 
  component of the pathname, deleting any trailing 
  \texttt{\CharQuote{/}} characters. 

  If \texttt{path} consists entirely of the \texttt{\CharQuote{/}} 
  character,  then \texttt{getbasename} returns a pointer to the string 
  \StringQuote{/}.

  If \texttt{path} is a null pointer or points to an empty string, 
  \texttt{getbasename} returns a pointer to the string
  \StringQuote{.}.

  \texttt{getbasename} function returns a pointer to static storage that 
  may then be overwritten by a subsequent call to \texttt{getbasename}. 

  \begin{center}
  \begin{tabular}{|l|l|}\hline
  \multicolumn{2}{|c|}{Example:}\\
  Input String&Output String\\\hline
  \StringQuote{/usr/lib}  &\StringQuote{lib}\\
  \StringQuote{/usr/}     &\StringQuote{usr}\\
  \StringQuote{/}         &\StringQuote{/}\\
  \StringQuote{}          &\StringQuote{.}\\
  \hline
  \end{tabular}
  \end{center}
*/

/*@null@*/ char *vm_getbasename(const char *path)
{
  char *sbuf, *c;
  unsigned char foundother = 0;
  size_t pathlen;

  pathlen = strlen(path);
  sbuf = malloc(pathlen+2);
  if(sbuf == NULL)
  {
    fprintf(stderr,"vm_getbasename: cannot allocate memory for copy of \"%s\"\n",
                   path);
    exit(EXIT_FAILURE);
  }
  if(path == NULL || *path == '\0')
  {
    strcpy(sbuf,".");
    return sbuf;
  }
  strcpy(sbuf,path);
  for(c = sbuf + pathlen - 1; c >= sbuf; c--)
  {
    if(*c == PATH_SEPARATOR)
    {
      if(foundother)
      {
        size_t i;
        c++;
        for(i=0; c[i] != '\0'; i++)
        {
          sbuf[i] = c[i];
        }
        sbuf[i] = '\0';
        break;
      } 
      if(c > sbuf)
      {
        *c = '\0';
      }
    } else
    {
      foundother = 1U;
    }
  }
  return sbuf;
}
