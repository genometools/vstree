/*
  Copyright (C) 2005 Gordon Gremme <gremme@zbh.uni-hamburg.de>
*/

//\IgnoreLatex{

#ifndef GENFILE_H 
#define GENFILE_H 

#include <stdarg.h>
#include <stdio.h>
#include <zlib.h>

#include "types.h"

#include "bzlib.h"

//}

/*
  This file contains the necessary definitions for generic files.
  A generic file is is a file which can be in an uncompressed format or in
  various compressed formats.
  All functions like fprintf, fopen, fread, fwrite have to be replaced by 
  the appropriate generic functions like genprintf, genopen, genwrite,...

  A NULL-pointer as generic file implies stdout!
*/

typedef enum
{
  STANDARD = 0,
  GZIP,
  BZIP2,
  NUMOFGENFILEMODES
} Genfilemode; // \Typedef{Genfilemode}

typedef struct
{
  Genfilemode genfilemode;
  union
  {
    FILE *file;
    gzFile gzfile;
    BZFILE *bzfile;
  } fileptr;
} GENFILE; // \Typedef{GENFILE}

//\IgnoreLatex{

#ifdef __cplusplus
extern "C" {
#endif

/*@null@*/ GENFILE *genopen(Genfilemode genfilemode, const char *path,
                            const char *mode);

Fclosereturntype genclose(GENFILE *genfile);

Fprintfreturntype vgenprintf(GENFILE *genfile, const char *format, va_list va);

void genprintf(GENFILE *genfile, const char *format, ...);

Fgetcreturntype gengetc(GENFILE *genfile);

Fputcreturntype genputc(Fputcfirstargtype c, GENFILE *genfile);

#ifdef __cplusplus
}
#endif

#endif

//}
