
//\Ignore{


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifndef NOSPACEBOOKKEEPING

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#ifndef _WIN32
#include <sys/resource.h>
#endif
#include <string.h>
#include <errno.h>
#include "types.h"
#include "errordef.h"
#include "megabytes.h"
#include "failures.h"
#include "redblackdef.h"
#include "debugdef.h"

#include "redblack.pr"

//}

/*EE
  This file contains functions to store pointers to dynamically allocated
  spaceblocks, and to maintain the number of cells and their size in
  each block. The arguments \texttt{file} and \texttt{line} (if they occur)
  are always the filename and the linenumber the function is called from.
  To supply these arguments, we recommend to call the corresponding 
  functions via some useful macros, as defined in the file 
  \texttt{spacedef.h}. 
  \begin{enumerate}
  \item
  The function \texttt{allocandusespaceviaptr} should be called
  via the macro \texttt{ALLOCASSIGNSPACE}.
  \item
  The function \texttt{freespaceviaptr} should be called
  via the macro \texttt{FREESPACE}.
  \item
  The function \texttt{dynamicstrdup} should be called
  via the macro \texttt{ASSIGNDYNAMICSTRDUP}.
  \end{enumerate}
*/

/*EE
  The following is a general macro to print fatal error messages.
*/

Sint checkenvvaronoff(char *varname);

#define SPACEMONITORON  UintConst(1)
#define SPACEMONITOROFF UintConst(2)

#define ALLOCERRORMSG(FUN,MSG) \
        fprintf(stderr,\
                "file \"%s\", line %lu; %s failed; "\
                "cannot allocate %lu blocks of size %lu; diagnosis: "\
                "%s, current number of allocated bytes is %lu, "\
                "%s\n",\
                file,\
                (Showuint) linenum,\
                FUN,\
                (Showuint) number,\
                (Showuint) size,\
                MSG,\
                (Showuint) currentspace,\
                strerror(errno))

#define ALLOCSUCCESS\
        if(spacemonitor == SPACEMONITORON)\
        {\
          printf("file \"%s\", line %lu; allocspace okay; "\
                 "allocated %lu blocks of size %lu; "\
                 "current number of allocated bytes is %lu\n",\
                 file,\
                 (Showuint) linenum,\
                 (Showuint) number,\
                 (Showuint) size,\
	         (Showuint) currentspace);\
        }

#define FREESUCCESS\
        if(spacemonitor == SPACEMONITORON)\
        {\
          printf("file \"%s\", line %lu; free okay; "\
                 "free %lu blocks of size %lu; "\
                 "current number of allocated bytes is %lu\n",\
                 file,\
                 (Showuint) linenum,\
                 (Showuint) resourceptr->blockdescription.numberofcells,\
                 (Showuint) resourceptr->blockdescription.sizeofcells,\
	         (Showuint) currentspace);\
         }

typedef struct
{
  Uint sizeofcells,    // size of cells of the block
       numberofcells;  // number of cells in the block
  char *fileallocated; // the filenames where the block was allocated
  Uint lineallocated;  // the linenumber where the
} Blockdescription;
                
typedef Sint (*Blockaction)(void *,Blockdescription *);

typedef struct
{
  Blockaction blockaction;
  void *blockinfo;
} Blocktreetraverseinfo;

typedef struct
{
  void *keyvalue;                      // the key value
  Blockdescription blockdescription;
} ResourceEntry;

static Uint currentspace = 0,   // currently allocated num of bytes
            spacemonitor = 0,   // monitor space allocation events
            spacepeak = 0;      // maximally allocated num of bytes

 /*@null@*/ static void *blocktree = NULL;

/*
  The following function sets the soft limit on the data size to the hard
  limit, if it is not already identical to this.
  This is necessary for the DEC alpha platform, where the soft limit
  is too small. \texttt{setmaxspace} is called for the first time, 
  space is allocated.
*/

#ifndef _WIN32
static void setmaxspace(void)
{
  Getrlimitreturntype rc;
  struct rlimit rls;

  /*@ignore@*/
  if((rc = getrlimit(RLIMIT_DATA,&rls)) != 0)
  /*@end@*/
  {
    fprintf(stderr,"cannot find rlimit[RLIMIT_DATA]\n");
    exit(EXIT_FAILURE);
  }

  if(rls.rlim_cur < rls.rlim_max)
  {
    rls.rlim_cur = rls.rlim_max;
    /*@ignore@*/
    if((rc = setrlimit(RLIMIT_DATA, &rls)) != 0)
    /*@end@*/
    {
      fprintf(stderr,"cannot set rlimit[RLIMIT_DATA]\n");
      exit(EXIT_FAILURE);
    }
  }
}
#endif

/*
  The following two functions \texttt{addspace} and \texttt{subtractspace} 
  maintain the variables \texttt{currentspace} and \texttt{spacepeak}.
*/

void addspace(Uint space)
{
  if(currentspace == 0)
  {
#ifndef _WIN32
    setmaxspace();
#endif
    currentspace = space;
  } else
  {
    currentspace += space;
  }
  if(currentspace > spacepeak)
  {
    spacepeak = currentspace;
    DEBUG1(2,"# spacepeak = %.2f reached\n",MEGABYTES(spacepeak));
  }
}

void subtractspace(Uint space)
{
  if(space > currentspace)
  {
    fprintf(stderr,"want to free %lu bytes, "
                   "but only %lu bytes have been allocated\n",
                   (Showuint) space,(Showuint) currentspace);
    exit(EXIT_FAILURE);
  } 
  currentspace -= space;
}

static Sint compareaddress(const Keytype avalue,
                           const Keytype bvalue,
                           /*@unused@*/ void *info)
{
  if(((ResourceEntry *) avalue)->keyvalue <
     ((ResourceEntry *) bvalue)->keyvalue)
  {
    return (Sint) -1;
  }
  if(((ResourceEntry *) avalue)->keyvalue > 
     ((ResourceEntry *) bvalue)->keyvalue)
  {
    return (Sint) 1;
  }
  return 0;
}

/*@null@*/ static ResourceEntry *findthespaceblock(void *ptr)
{
  ResourceEntry findblock;

  findblock.keyvalue = ptr;
  return (ResourceEntry *) redblacktreefind ((const Keytype) &findblock,
                                             &blocktree,
                                             compareaddress,
                                             NULL);
}

static ResourceEntry *addnewspaceblock(char *file,
                                       Uint linenum, 
                                       Uint size,
                                       Uint number)
{
  ResourceEntry *insertresource;
  BOOL nodecreated;

  insertresource = malloc(sizeof(ResourceEntry));
  if(insertresource == NULL)
  {
    ALLOCERRORMSG("addnewspaceblock",
                  "not enough space for the block description tree");
    exit(EXIT_FAILURE);
  }
  insertresource->keyvalue = realloc(NULL,(size_t) (size * number));
  if(insertresource->keyvalue == NULL)
  {
    ALLOCERRORMSG("addnewspaceblock",
                  "not enough space for the space block");
    exit(EXIT_FAILURE);
  }
  insertresource->blockdescription.fileallocated = file;
  insertresource->blockdescription.lineallocated = linenum;
  insertresource->blockdescription.sizeofcells = size;
  insertresource->blockdescription.numberofcells = number;
  (void) redblacktreesearch (insertresource,
                             &nodecreated,
                             &blocktree,
                             compareaddress,
                             NULL);
  return insertresource;
}

static ResourceEntry *reusespaceblock(void *ptr,
                                      char *file,
                                      Uint linenum,
                                      Uint size,
                                      Uint number)
{
  void *tmpptr;
  ResourceEntry *reuseresource;

  tmpptr = realloc(ptr,(size_t) (size*number));
  if(tmpptr == NULL)
  {
    ALLOCERRORMSG("reusespaceblock",
                  "not enough space for the space block");
    exit(EXIT_FAILURE);
  }
  reuseresource = findthespaceblock(ptr);
  if(reuseresource == NULL)
  {
    ALLOCERRORMSG("reusespaceblock",
                  "cannot find space block");
    exit(EXIT_FAILURE);
  }
  if(ptr != tmpptr)
  {
    BOOL nodecreated;

    if (redblacktreedelete(reuseresource,
                           &blocktree,
                           compareaddress,
                           NULL) != 0)
    {
      ALLOCERRORMSG("reusespaceblock",
                    "deletion of resourcevalue failed");
      exit(EXIT_FAILURE);
    }
    reuseresource->keyvalue = tmpptr;
    (void) redblacktreesearch (reuseresource,
                               &nodecreated,
                               &blocktree,
                               compareaddress,
                               NULL);
  }
  subtractspace(reuseresource->blockdescription.numberofcells * 
                reuseresource->blockdescription.sizeofcells);
  reuseresource->blockdescription.sizeofcells = size;
  reuseresource->blockdescription.numberofcells = number;
  reuseresource->blockdescription.fileallocated = file;
  reuseresource->blockdescription.lineallocated = linenum;
  return reuseresource;
}

static Sint applyredblackwalk(const Keytype bmkey,
                              VISIT which,
                              /*@unused@*/ Uint depth,
                              void *applyinfo)
{
  if(which == postorder || which == leaf)
  {
    Blocktreetraverseinfo *blocktreetraverseinfo 
      = (Blocktreetraverseinfo *) applyinfo;
    if(blocktreetraverseinfo->blockaction(blocktreetraverseinfo->blockinfo,
                                          &((ResourceEntry *) bmkey)->
                                            blockdescription) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint overallblocks(Blockaction apply,void *info)
{
  Blocktreetraverseinfo blocktreetraverseinfo;

  blocktreetraverseinfo.blockaction = apply;
  blocktreetraverseinfo.blockinfo = info;
  if(redblacktreewalk(blocktree,
                      applyredblackwalk,
                      (void *) &blocktreetraverseinfo) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}


static void checkspacemonitor(void)
{
  if(spacemonitor == 0)
  {
    Sint retcode;

    retcode = checkenvvaronoff("VMATCHSPACEMONITOR");
    if(retcode == (Sint) -1)
    {
      fprintf(stderr,"%s\n",messagespace());
      exit(EXIT_FAILURE);
    }
    if(retcode)
    {
      spacemonitor = SPACEMONITORON;
    } else
    {
      spacemonitor = SPACEMONITOROFF;
    }
  }
}

/*EE
  The following function allocates \texttt{number} cells of \texttt{size}
  for a given pointer \texttt{ptr}. If this is \texttt{NULL}, then the next 
  free block is used. Otherwise, we look for the block number corresponding 
  to \texttt{ptr}. If there is none, then the program exits with exit code 1. 
*/

/*@notnull@*/ void *allocandusespaceviaptr(char *file,
                                           Uint linenum, 
                                           /*@null@*/ void *ptr,
                                           Uint size,
                                           Uint number)
{
  ResourceEntry *resourceptr;

  checkspacemonitor();

  DEBUG2(2,"# allocandusespaceviaptr: %lu cells of size %lu\n",
            (Showuint) number,
            (Showuint) size);
  if(ptr == NULL)
  {
    resourceptr = addnewspaceblock(file,linenum,size,number);
  } else
  {
    resourceptr = reusespaceblock(ptr,file,linenum,size,number);
  }
  addspace(size*number);
  ALLOCSUCCESS;
  DEBUG0(2,"# allocandusespaceviaptr Okay\n");
  NOTSUPPOSEDTOBENULL(resourceptr->keyvalue);
  return resourceptr->keyvalue;
}

/*EE
  The following function frees the space for the given pointer 
  \texttt{ptr}. This cannot be \texttt{NULL}. 
*/

void freespaceviaptr(char *file,Uint linenum,void *ptr)
{
  ResourceEntry *resourceptr;

  DEBUG2(2,"# freespaceviaptr(file=%s,line=%lu):\n",file,(Showuint) linenum);
  if(ptr == NULL)
  {
    fprintf(stderr,"freespaceviaptr(file=%s,line=%lu): Cannot free NULL-ptr\n",
                    file,(Showuint) linenum);
    exit(EXIT_SUCCESS);
  }
  resourceptr = findthespaceblock(ptr);
  if(resourceptr == NULL)
  {
    fprintf(stderr,"freespaceviaptr(file=%s,line=%lu): "
                   " cannot find space block\n",
            file,(Showuint) linenum);
    exit(EXIT_FAILURE);
  }
  FREESUCCESS;
  subtractspace(resourceptr->blockdescription.numberofcells * 
                resourceptr->blockdescription.sizeofcells);
  DEBUG2(2,"# freespaceviaptr:%lu cells of size %lu\n",
            (Showuint) resourceptr->blockdescription.numberofcells,
            (Showuint) resourceptr->blockdescription.sizeofcells);
  DEBUG2(2,"# this block was allocated in file \"%s\", line %lu\n",
            resourceptr->blockdescription.fileallocated,
            (Showuint) resourceptr->blockdescription.lineallocated);
  if (redblacktreedelete(resourceptr,
                         &blocktree,
                         compareaddress,
                         NULL) != 0)
  {
    fprintf(stderr,"freespaceviaptr(file=%s,line=%lu): "
                   " cannot delete block description\n",
                   file,(Showuint) linenum);
    exit(EXIT_FAILURE);
  }
  resourceptr->blockdescription.numberofcells = 0;
  resourceptr->blockdescription.sizeofcells = 0;
  resourceptr->blockdescription.fileallocated = NULL;
  resourceptr->blockdescription.lineallocated = 0;
  free(resourceptr->keyvalue);
  resourceptr->keyvalue = NULL;
  free(resourceptr);
}

//\IgnoreLatex{

/*EE
  The following function prints information about a space block,
  referenced by the given keyvalue. If there is no information,
  then the function exits.
*/

void spaceblockinfo(char *file,Uint linenum,void *ptr)
{
  ResourceEntry *resourceptr;

  printf("# file \"%s\", line %lu: ",file,(Showuint) linenum);
  resourceptr = findthespaceblock(ptr);
  if(resourceptr == NULL)
  {
    fprintf(stderr,"Cannot find space block\n");
    exit(EXIT_FAILURE);
  }
  printf("# spaceblockinfo finds block: ");
  printf(" allocated in file \"%s\", line %lu\n",
          resourceptr->blockdescription.fileallocated,
          (Showuint) resourceptr->blockdescription.lineallocated);
  printf("# it is a block with %lu cells of size %lu\n",
          (Showuint) resourceptr->blockdescription.numberofcells,
          (Showuint) resourceptr->blockdescription.sizeofcells);
}

//}

static Sint activateblockssingleblock(/*@unused@*/ void *info,
                                      Blockdescription *blockptr)
{
  printf("# activated block: allocated in file \"%s\", line %lu\n",
          blockptr->fileallocated,(Showuint) blockptr->lineallocated);
  return 0;
}

/*EE
  The following function prints a list of block numbers 
  which have not been freed. For each block number the filename
  and line number in which the call appears allocating which 
  allocated this block.
*/

void activeblocks(void)
{
  (void) overallblocks(activateblockssingleblock,NULL);
}

static Sint showsinglespaceleak(/*@unused@*/ void *info,
                                Blockdescription *blockptr)
{
  fprintf(stderr,"space leak: main memory for block not freed\n");
  fprintf(stderr,"%lu cells of size %lu\n",
          (Showuint) blockptr->numberofcells,
          (Showuint) blockptr->sizeofcells);
  fprintf(stderr,"allocated: ");
  if(blockptr->fileallocated == NULL)
  {
    fprintf(stderr,"cannot identify\n");
  } else
  {
    fprintf(stderr,"file \"%s\", line %lu\n",
           blockptr->fileallocated,
           (Showuint) blockptr->lineallocated);
  }
  return (Sint) -1;
}

/*EE
  The following function checks if all blocks previously allocated, have 
  explicitely been freed. If there is a block that was not freed, then 
  an error is reported accordingly. We recommend to call this function
  before the program terminates. This easily allows to discover 
  space leaks.
*/

void checkspaceleak(void)
{
  if(overallblocks(showsinglespaceleak,NULL) != 0)
  {
    exit(EXIT_FAILURE);
  }
  currentspace = 0;
  spacepeak = 0;
}

/*EE
  The following function shows the space peak in megabytes on \texttt{stderr}.
*/

void showspace(void)
{
  fprintf(stderr,"# space peak in megabytes: %.2f\n",MEGABYTES(spacepeak));
}

/*EE
  The following function returns the space peak in bytes.
*/

Uint getspacepeak(void)
{
  return spacepeak;
}
#endif
