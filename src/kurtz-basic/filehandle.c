//\Ignore{
  
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "types.h"
#include "fhandledef.h"
#include "spacedef.h"
#include "errordef.h"
#include "debugdef.h"
#include "failures.h"

//}

/*EE
  This file contains functions to store file handles for
  user opened files. A file handle consists of 
  \begin{itemize}
  \item
  the filepointer
  \item
  a string describing the open mode, corresponding to the second
  argument of fopen
  \item
  the line number and the program file call the the open
  function was done.
  \end{itemize}
  The arguments \texttt{file} and \texttt{line} 
  (if they occur) are always the filename and the linenumber, the 
  function is called from. To supply the arguments, we recommend to 
  call the corresponding functions via some useful macros, as defined in the 
  file \texttt{fhandledef.h}. 
  \begin{enumerate}
  \item
  The function \texttt{createfilehandle} should be called
  via the macro \texttt{CREATEFILEHANDLE}.
  \item
  The function \texttt{deletefilehandle} should be called
  via the macro \texttt{DELETEFILEHANDLE}.
  \end{enumerate}
*/

/*
  The maximal length of the string specifying the open mode of the
  call to function createfilehandle.
*/

#define MAXOPENMODE 2

typedef struct
{
  char path[PATH_MAX+1],
       createmode[MAXOPENMODE+1],
       *createfile;
  Uint createline;
} Filehandle;

/*
  The following variable is a references to the table 
  of Filehandles.
*/

/*@null@*/ static Filehandle *filehandle = NULL;

/*
  current number of open files and of allocated file handles.
*/

static Uint allocatedFilehandle = 0,
            currentopen = 0;

/*
  The following three tables store important information to
  generate meaningfull error messages.
*/

static Uint fromfileptr2filedesc(char *file,
                                 Uint line,
                                 BOOL existing,
                                 FILE *fp)
{
  Filedesctype fd;

  fd = fileno(fp);
  if(fd == -1)
  {
    fprintf(stderr,"cannot find filedescriptor: %s\n",strerror(errno));
    NOTSUPPOSED;
  }
  if(existing)
  {
    if(allocatedFilehandle <= (Uint) fd)
    {
      fprintf(stderr,"file %s, line %lu: cannot open file: fd=%lu, "
             "not enough file handles available\n",
             file,
             (Showuint) line,
             (Showuint) fd);
      NOTSUPPOSED;
    } else
    {
      NOTSUPPOSEDTOBENULL(filehandle);
      if(filehandle[fd].createfile == NULL)
      {
        fprintf(stderr,"file %s, line %lu: cannot open file: fd=%lu, "
                "file handle not occurpied\n",
                file,
                (Showuint) line,
                (Showuint) fd);
        NOTSUPPOSED;
      }
    }
  }
  return (Uint) fd;
}

/*@null@*/ static char *fileptr2filename(char *file,
                                         Uint line,
                                         FILE *fp)
{
  if(fp == stdout)
  {
    return "stdout";
  }
  if(fp == stderr)
  {
    return "stderr";
  }
  NOTSUPPOSEDTOBENULL(filehandle);
  return filehandle[fromfileptr2filedesc(file,line,True,fp)].createfile;
}

#define INCFILEHANDLES 16

static void assignfilehandleinformation(char *file,
                                        Uint line,
                                        const Uint fd,
                                        const char *path,
                                        const char *mode)
{
  while(fd >= allocatedFilehandle)
  {
    Uint i;

    ALLOCASSIGNSPACE(filehandle,
                     filehandle,
                     Filehandle,
                     allocatedFilehandle+INCFILEHANDLES);
    for(i=allocatedFilehandle; i<allocatedFilehandle+INCFILEHANDLES; i++)
    {
      filehandle[i].createfile = NULL;
    }
    allocatedFilehandle += INCFILEHANDLES;
  }
  NOTSUPPOSEDTOBENULL(filehandle);
  if(filehandle[fd].createfile != NULL)
  {
    fprintf(stderr,"file %s, line %lu: open file \"%s\" with mode \"%s\": "
                   "handle %lu already occupied\n",
           file,(Showuint) line,path,mode,(Showuint) fd);
    NOTSUPPOSED;
  }
  if(strlen(mode) > (size_t) MAXOPENMODE)
  {
    fprintf(stderr,"file %s, line %lu: cannot open file \"%s\": "
                   "illegal open mode \"%s\"\n",file,(Showuint) line,path,mode);
    NOTSUPPOSED;
  }
  strcpy(filehandle[fd].createmode,mode);
  if(strlen(path) > PATH_MAX)
  {
    fprintf(stderr,"file %s, line %lu: cannot open file \"%s\": "
                   "path is too long\n",file,(Showuint) line,path);
    NOTSUPPOSED;
  }
  strcpy(filehandle[fd].path,path);
  filehandle[fd].createfile = file;
  filehandle[fd].createline = line;
  DEBUG5(2,"# file %s, line %lu: assign file \"%s\" for"
           " mode=%s to file handle %lu\n",
           file,(Showuint) line,path,mode,(Showuint) fd);
  currentopen++;
}

/*EE
  The following functions opens a file named \texttt{path}. 
  The mode of the file is given by \texttt{mode}. The file
  pointer for the file is returned, or \texttt{NULL}, if the
  file could not be opened. Moreover, with each file created,
  a file handle is generated storing all relevant information.
*/

/*@null@*/ FILE *createfilehandle(char *file,
                                  const Uint line,
                                  const char *path,
                                  const char *mode)
{
  FILE *fp;

  fp = fopen(path,mode);
  if(fp == NULL)
  {
    ERROR2("cannot open file \"%s\": %s",path,strerror(errno));
    return NULL;
  }
  assignfilehandleinformation(file,
                              line,
                              fromfileptr2filedesc(file,line,False,fp),
                              path,
                              mode);
  return fp;
}

void inittmpfiledesc(Tmpfiledesc *tmpfiledesc)
{
  tmpfiledesc->tmpfilenamebuffer = NULL;
  tmpfiledesc->tmpfilenamelength = 0;
}

/*EE
  The following function creates a temporary file. The file pointer
  to this file as well as the name of the temporary file is stored
  in \texttt{tmpfiledesc}. The return code is 0, if everything is
  okay. Otherwise, a negative error code is returned.
*/

Sint maketmpfile(char *file,
                 Uint line,
                 Tmpfiledesc *tmpfiledesc,
                 char *fileprefix)
{
  FILE *fp;
  Filedesctype fd;
  Uint tmpfilenamelength;
  char *usedfileprefix;
#ifdef _WIN32
  errno_t err;
#endif

  usedfileprefix = (fileprefix == NULL) ? TMPFILEPREFIX : fileprefix;
  tmpfilenamelength = (Uint) (strlen(usedfileprefix) + NUMBEROFX + 1);
  if(tmpfiledesc->tmpfilenamelength < tmpfilenamelength)
  {
    ALLOCASSIGNSPACE(tmpfiledesc->tmpfilenamebuffer,
                     tmpfiledesc->tmpfilenamebuffer,
                     char,tmpfilenamelength);
    tmpfiledesc->tmpfilenamelength = tmpfilenamelength;
  }
  sprintf(tmpfiledesc->tmpfilenamebuffer,"%s%s",usedfileprefix,TMPFILESUFFIX);
#ifndef _WIN32
  fd = mkstemp(tmpfiledesc->tmpfilenamebuffer);
#else
  err = _mktemp_s(tmpfiledesc->tmpfilenamebuffer,
                  strlen(tmpfiledesc->tmpfilenamebuffer) + 1);
  if(err == EINVAL)
  {
    ERROR1("_mktemp_s failed for \"%s\"", tmpfiledesc->tmpfilenamebuffer);
    return (Sint) -1;
  }
  fd = open(tmpfiledesc->tmpfilenamebuffer, O_RDWR, O_EXCL);
#endif
  if(fd == -1)
  {
    ERROR2("cannot open file \"%s\": %s",
            tmpfiledesc->tmpfilenamebuffer,
            strerror(errno));
    return (Sint) -1;
  }
  fp = fdopen(fd,WRITEMODE);
  if(fp == NULL)
  {
    ERROR2("cannot open file \"%s\": %s",tmpfiledesc->tmpfilenamebuffer,
            strerror(errno));
    return (Sint) -2;
  }
  tmpfiledesc->tmpfileptr = fp;
  assignfilehandleinformation(file,
                              line,
                              (Uint) fd,
                              tmpfiledesc->tmpfilenamebuffer,
                              WRITEMODE);
  return 0;
}

void wraptmpfiledesc(Tmpfiledesc *tmpfiledesc)
{
  FREESPACE(tmpfiledesc->tmpfilenamebuffer);
  tmpfiledesc->tmpfilenamelength = 0;
}

/*EE
  The following function closes a file and deletes the corresponding
  a file handle. The file is identified by a file pointer.
  The return code is 0, if everything is
  okay. Otherwise, a negative error code is returned.
*/

Sint deletefilehandle(char *file,
                      Uint line,
                      FILE *fp)
{
  Uint fd;

  NOTSUPPOSEDTOBENULL(filehandle);
  fd = fromfileptr2filedesc(file,line,True,fp);
  if(fclose(fp) != 0)
  {
    ERROR2("cannot close file \"%s\": %s",
           filehandle[fd].path,strerror(errno));
    return (Sint) -1;
  }
  DEBUG5(2,"# file %s, line %lu: close file \"%s\" with mode \"%s\" "
           "at handle %lu\n",
         file,
         (Showuint) line,
         filehandle[fd].path,
         filehandle[fd].createmode,
         (Showuint) fd);
  strcpy(filehandle[fd].createmode,"");
  filehandle[fd].createfile = NULL;
  filehandle[fd].createline = 0;
  currentopen--;
  if(currentopen == 0)
  {
    FREESPACE(filehandle);
    allocatedFilehandle = 0;
  }
  return 0;
}

/*EE
  The following function writes \texttt{nmemb} elements each
  of \texttt{size} bytes and referenced by \texttt{ptr} to 
  an output \texttt{stream}.
  The return code is 0, if everything is
  okay. Otherwise, a negative error code is returned.
*/

Sint writetofilehandle(char *file,
                       Uint line,
                       void *ptr,
                       Uint size,
                       Uint nmemb,
                       FILE *stream)
{
  if(fwrite(ptr, (size_t) size, (size_t) nmemb,stream) != (size_t) nmemb)
  {
    ERROR5("%s %lu: cannot write %lu items of size %lu: errormsg=\"%s\"",
           file,(Showuint) line,(Showuint) nmemb,(Showuint) size,
           // fileptr2filename(file,line,stream),
           strerror(errno));
    return (Sint) -1;
  }
  return 0;
}

/*EE
  The following function reads \texttt{nmemb} elements each
  of \texttt{size} bytes and referenced by \texttt{ptr} from 
  an input \texttt{stream}.
  The return code is 0, if everything is
  okay. Otherwise, a negative error code is returned.
*/

Sint readfromfilehandle(char *file,
                       Uint line,
                       void *ptr,
                       Uint size,
                       Uint nmemb,
                       FILE *stream)
{   
  if(fread(ptr, (size_t) size, (size_t) nmemb,stream) != (size_t) nmemb)
  {
    ERROR6("%s %lu: cannot read %lu items of size %lu to %s: %s",
           file,(Showuint) line,(Showuint) nmemb,(Showuint) size,
           fileptr2filename(file,line,stream),
           strerror(errno));
    return (Sint) -1;
  }
  return 0;
}

/*EE
  The following function checks if all file created have
  been closed. If not, then an error message is thrown and 
  the function exits.
*/

void checkfilehandles(void)
{
  Uint fd;

  for(fd=0; fd < allocatedFilehandle; fd++)
  {
    NOTSUPPOSEDTOBENULL(filehandle);
    if(filehandle[fd].createfile != NULL)
    {
      fprintf(stderr,"unclosed file: openend in file %s, line %lu\n",
              filehandle[fd].path,
              (Showuint) filehandle[fd].createline);
      exit(EXIT_FAILURE);
    }
  }
  if(currentopen > 0)
  {
    fprintf(stderr,"unclosed file: currentopen = %lu\n",
            (Showuint) currentopen);
    exit(EXIT_FAILURE);
  }
}

BOOL filealreadyexists(char *filename)
{
  struct stat statbuf;

  if(stat(filename,&statbuf) == 0)
  {
    return True;
  }
  return False;
}
