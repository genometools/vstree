
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/uio.h>
#include <sys/mman.h>
#else
#include <assert.h>
#include <windows.h>
#endif
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "types.h"
#include "errordef.h"
#include "megabytes.h"
#include "debugdef.h"

#define MAXMAPPEDFILES 64
#define CANTREADMSG "%s, line %lu: cannot read file \"%s\": "

//}

/*EE
  This file contains functions to map files to memory, to store pointers to
  the corresponding secondary memory, and to maintain the size of the mapped
  memory. The arguments \texttt{file} and \texttt{line} (if they occur)
  are always the filename and the linenumber the function is called from.
  To supply the arguments, we recommend to call the corresponding functions
  via some useful macros, as defined in the file \texttt{spacedef.h}.
  \begin{enumerate}
  \item
  The function \texttt{creatememorymap} should be called
  via the macro \texttt{CREATEMEMORYMAP}.
  \item
  The function \texttt{deletememorymap} should be called
  via the macro \texttt{DELETEMEMORYMAP}.
  \end{enumerate}
*/

#ifdef _WIN32
/*
   On Windows we need additional file handle and file mapping objects for memory
   maps.
*/

static HANDLE filehandle[MAXMAPPEDFILES] = {NULL};
static HANDLE filemapping[MAXMAPPEDFILES] = {NULL};
#endif

/*
  Pointers to the memory areas, initialized to NULL, i.e. not occupied.
*/

static void *memoryptr[MAXMAPPEDFILES] = {NULL};

static Uint currentspace = 0,              // currently mapped num of bytes
            spacepeak = 0,                 // maximally mapped num of bytes
            mappedbytes[MAXMAPPEDFILES] = {0};  // size of the memory map

/*
  The following two tables store important information to
  generate meaningfull error messages.
*/


/*
  file where the mmap was created
*/

static char *filemapped[MAXMAPPEDFILES] = {NULL};

/*
  line where the mmap was created
*/

static Uint linemapped[MAXMAPPEDFILES] = {0};

/*
  The following two functions \texttt{mmaddspace} and \texttt{mmsubtractspace}
  maintain the variables \texttt{currentspace} and \texttt{spacepeak}.
*/

static void mmaddspace(Uint space)
{
  currentspace += space;
  if(currentspace > spacepeak)
  {
    spacepeak = currentspace;
    DEBUG1(2,"# mmap spacepeak = %.2f reached\n",MEGABYTES(spacepeak));
  }
}

static void mmsubtractspace(Uint space)
{
  if(space > currentspace)
  {
    fprintf(stderr,"want to unmap %lu bytes, "
                   "but only %lu bytes have been allocated\n",
                   (Showuint) space,(Showuint) currentspace);
    exit(EXIT_FAILURE);
  }
  currentspace -= space;
}

/*EE
  The following function opens a file, and stores the size of the
  file in \texttt{numofbytes}. It returns the file descriptor of the
  file, or a negative error code if something went wrong.
*/

Filedesctype simplefileOpen(char *file,
                            Uint line,
                            const char *filename,
                            Uint *numofbytes)
{
  Filedesctype fd;
  struct stat buf;

  if((fd = open(filename,O_RDONLY)) == -1)
  {
     ERROR4(CANTREADMSG
            "open failed: %s",
            file,(Showuint) line,filename,strerror(errno));
     return -1;
  }
  if(fstat(fd,&buf) == -1)
  {
     ERROR4(CANTREADMSG
            "cannot access status of file: %s",
            file,(Showuint) line,filename,strerror(errno));
     return -2;
  }
  *numofbytes = (Uint) buf.st_size;
  return (Filedesctype) fd;
}

/*EE
  The following function creates a memory map for a given file
  descriptor \texttt{fd}. \texttt{writemap} is true iff the map
  should be writable, and \texttt{numofbytes} is the
  size of the file to be mapped.
*/

/*@null@*/ void *creatememorymapforfiledesc(char *file,
                                            Uint line,
                                            const char *fdfile,
                                            Filedesctype fd,
                                            BOOL writemap,
                                            Uint numofbytes)
{
  DEBUG1(2,"# creatememorymapforfiledesc %ld\n",(Showsint) fd);
  if(numofbytes == 0)
  {
    ERROR3(CANTREADMSG
           "it is empty",file,
           (Showuint) line,fdfile);
    return NULL;
  }
  if(fd < 0)
  {
    ERROR4(CANTREADMSG
           "filedescriptor %ld is negative",
           file,(Showuint) line,fdfile,(Showsint) fd);
    return NULL;
  }
  if(fd >= (Filedesctype) MAXMAPPEDFILES)
  {
    ERROR4(CANTREADMSG
           "filedescriptor %ld is too large",
           file,(Showuint) line,fdfile,(Showsint) fd);
    return NULL;
  }
  if(memoryptr[fd] != NULL)
  {
    ERROR4(CANTREADMSG
           "filedescriptor %ld already in use",
           file,(Showuint) line,fdfile,(Showsint) fd);
    return NULL;
  }
  mmaddspace(numofbytes);
  DEBUG2(2,"# memorymap:fd=%ld: %lu bytes\n",(Showsint) fd,
                                             (Showuint) numofbytes);
  mappedbytes[fd] = numofbytes;
#ifndef _WIN32
  memoryptr[fd]
    = (void *) mmap(0,
                    (size_t) numofbytes,
                    writemap ? (PROT_READ | PROT_WRITE) : PROT_READ,
                    MAP_PRIVATE,
                    fd,
                    (off_t) 0);
  if(memoryptr[fd] == (void *) MAP_FAILED)
  {
    ERROR5(CANTREADMSG
           "memorymapping via filedescriptor %ld failed: %s",
           file,(Showuint) line,fdfile,(Showsint) fd,strerror(errno));
    return NULL;
  }
#else
  assert(filehandle[fd] == NULL);
  filehandle[fd] = CreateFile(fdfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
  if(filehandle[fd] == NULL)
  {
    ERROR5(CANTREADMSG
          "memorymapping via filedescriptor %ld failed: 0x%08x",
           file,(Showuint) line,fdfile,(Showsint) fd,
           (unsigned int) GetLastError());
    return NULL;
  }
  assert(filemapping[fd] == NULL);
  filemapping[fd] = CreateFileMapping(filehandle[fd], NULL, PAGE_READONLY, 0, 0,
                                      0);
  if(filemapping[fd] == NULL)
  {
    CloseHandle(filehandle[fd]);
    filehandle[fd] = NULL;
    ERROR5(CANTREADMSG
          "memorymapping via filedescriptor %ld failed: 0x%08x",
           file,(Showuint) line,fdfile,(Showsint) fd,
           (unsigned int) GetLastError());
    return NULL;
  }
  memoryptr[fd] = MapViewOfFile(filemapping[fd],
                                writemap ? FILE_MAP_COPY : FILE_MAP_READ,
                                0, 0, 0);
  if(memoryptr[fd] == NULL)
  {
    CloseHandle(filemapping[fd]);
    filemapping[fd] = NULL;
    CloseHandle(filehandle[fd]);
    filehandle[fd] = NULL;
    ERROR5(CANTREADMSG
          "memorymapping via filedescriptor %ld failed: 0x%08x",
           file,(Showuint) line,fdfile,(Showsint) fd,
           (unsigned int) GetLastError());
    return NULL;
  }
#endif
#ifndef NOMADVISE
  if (madvise(memoryptr[fd],
              (size_t) numofbytes,
              MADV_WILLNEED) == -1)
  {
    ERROR5(CANTREADMSG
           "memorymapping via filedescriptor %ld failed: "
           "madvise throws error: %s",
           file,(Showuint) line,fdfile,(Showsint) fd,strerror(errno));
    return NULL;
  }
#endif
  filemapped[fd] = file;
  linemapped[fd] = line;
  return memoryptr[fd];
}

/*EE
  The following function returns a memory map for a given filename, or
  \texttt{NULL} if something went wrong.
*/

/*@null@*/ void *creatememorymap(char *file,Uint line,
                                 const char *filename,
                                 BOOL writemap,Uint *numofbytes)
{
  Filedesctype fd;

  DEBUG3(2,"\n# creatememorymap(file=%s,line=%ld) for file %s:\n",
              file,(Showsint) line,
              filename);
  fd = simplefileOpen(file,line,filename,numofbytes);
  if(fd < 0)
  {
    return NULL;
  }
  return creatememorymapforfiledesc(file,line,filename,fd,
                                    writemap,*numofbytes);
}

/*EE
  The following function unmaps the memorymap referenced by
  \texttt{mappedfile}. It returns a negative value if the
  \texttt{mappedfile} is \texttt{NULL}, or the corresponding
  filedescriptor cannot be found, or the \texttt{munmap} operation
  fails.
*/

Sint deletememorymap(char *file,Uint line,const void *mappedfile)
{
  Filedesctype fd;

  if(mappedfile == NULL)
  {
    ERROR2("%s: l. %ld: deletememorymap failed: mapped file is NULL",
             file,(Showsint) line);
    return (Sint) -1;
  }
  for(fd=0; fd<MAXMAPPEDFILES; fd++)
  {
    if(memoryptr[fd] == mappedfile)
    {
      break;
    }
  }
  if(fd == MAXMAPPEDFILES)
  {
    ERROR2("%s: l. %ld: deletememorymap failed: cannot find filedescriptor "
           "for given address",
           file,(Showsint) line);
    return (Sint) -2;
  }
#ifndef _WIN32
  if(munmap(memoryptr[fd],(size_t) mappedbytes[fd]) != 0)
  {
    ERROR5("%s: l. %ld: deletememorymap failed: munmap failed:"
           " mapped in file \"%s\",line %lu: %s",
            file,
            (Showsint) line,
            filemapped[fd],
            (Showuint) linemapped[fd],
            strerror(errno));
    return (Sint) -3;
  }
#else
  if(!UnmapViewOfFile(memoryptr[fd]))
  {
    ERROR5("%s: l. %ld: deletememorymap failed: UnmapViewOfFile failed:"
           " mapped in file \"%s\",line %lu: 0x%08x",
            file,
            (Showsint) line,
            filemapped[fd],
            (Showuint) linemapped[fd],
            (unsigned int) GetLastError());
    return (Sint) -3;
  }
  if(!CloseHandle(filemapping[fd]))
  {
    ERROR5("%s: l. %ld: deletememorymap failed: CloseHandle failed:"
           " mapped in file \"%s\",line %lu: 0x%08x",
            file,
            (Showsint) line,
            filemapped[fd],
            (Showuint) linemapped[fd],
            (unsigned int) GetLastError());
    return (Sint) -3;
  }
  filemapping[fd] = NULL;
  if(!CloseHandle(filehandle[fd]))
  {
    ERROR5("%s: l. %ld: deletememorymap failed: CloseHandle failed:"
           " mapped in file \"%s\",line %lu: 0x%08x",
            file,
            (Showsint) line,
            filemapped[fd],
            (Showuint) linemapped[fd],
            (unsigned int) GetLastError());
    return (Sint) -3;
  }
  filehandle[fd] = NULL;
#endif
  DEBUG4(2,"# file \"%s\", line %ld: deletememorymap:fd=%ld (%lu) bytes:\n",
          file,
          (Showsint) line,
          (Showsint) fd,
          (Showuint) mappedbytes[fd]);
  DEBUG2(2,"# mapped in file \"%s\", line %lu\n",filemapped[fd],
                                                 (Showuint) linemapped[fd]);
  memoryptr[fd] = NULL;
  mmsubtractspace(mappedbytes[fd]);
  mappedbytes[fd] = 0;
  if(close(fd) != 0)
  {
    ERROR4("%s: l. %lu: deletememorymap failed: cannot close file \"%s\": %s",
            file,(Showuint) line,filemapped[fd],strerror(errno));
    return (Sint) -4;
  }
  return 0;
}

/*EE
  The following function checks if all files previously mapped, have
  been unmapped. If there is a file that was not unmapped, then
  an error is reported accordingly. We recommend to call this function
  before the program terminates. This easily allows to discover
  space leaks.
*/

void mmcheckspaceleak(void)
{
  Filedesctype fd;

  for(fd=0; fd<MAXMAPPEDFILES; fd++)
  {
    if(memoryptr[fd] != NULL)
    {
      fprintf(stderr,"space leak: memory for filedescriptor %ld not freed\n",
              (Showsint) fd);
      fprintf(stderr,"mapped in file \"%s\", line %lu\n",
              filemapped[fd],
              (Showuint) linemapped[fd]);
      exit(EXIT_FAILURE);
    }
  }
}

/*EE
  The following function shows the space peak in megabytes on \texttt{stderr}.
*/

void mmshowspace(void)
{
  fprintf(stderr,"# mmap space peak in megabytes: %.2f\n",MEGABYTES(spacepeak));
}

/*EE
  The following function returns the space peak in bytes.
*/

Uint mmgetspacepeak(void)
{
  return spacepeak;
}

#ifdef SIXTYFOURBITS

#define ENVPAGESIZELEVEL "VMATCHPAGESIZELEVEL"
#define MAXPAGESIZELEVEL 3

Sint setlocalpagesize(void)
{
  char *envpagesizelevel;

  envpagesizelevel = getenv(ENVPAGESIZELEVEL);
  if(envpagesizelevel != NULL)
  {
/*
    Scaninteger pagesizelevel;
    struct memcntl_mha mha;
    Sint pagesizetable[] = {   SintConst(8192),
                              SintConst(65536),
                             SintConst(524288),
                            SintConst(4194304)};

    if(sscanf(envpagesizelevel,"%ld",&pagesizelevel) != 1 ||
       pagesizelevel < 0 || pagesizelevel > (Scaninteger) MAXPAGESIZELEVEL)
    {
      ERROR3("illegal value \"%s\" in environment variable \"%s\":"
             " use integer in the range 0..%ld",
             envpagesizelevel,
             ENVPAGESIZELEVEL,
             (Showsint) MAXPAGESIZELEVEL);
      return (Sint) -1;
    }
    mha.mha_cmd = (uint_t) MHA_MAPSIZE_BSSBRK;
    mha.mha_flags = 0;
    mha.mha_pagesize = (size_t) pagesizetable[pagesizelevel];
    fprintf(stderr,"# set pagesize to %ld\n",(Showsint) mha.mha_pagesize);
    if(memcntl(NULL, 0, MC_HAT_ADVISE, (char *) &mha, 0, 0) != 0)
    {
      ERROR0("memcntl failed");
      return (Sint) -1;
    }
*/
  }
  return 0;
}
#endif
