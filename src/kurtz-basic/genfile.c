
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdarg.h>
#include <ctype.h>
#include "genfile.h"
#include "spacedef.h"
#include "failures.h"
#include "functrace.h"

#ifndef Z_PRINTF_BUFSIZE
#  define Z_PRINTF_BUFSIZE 4096
#endif

/*@null@*/ GENFILE *genopen(Genfilemode genfilemode, const char *path, 
                            const char *mode)
{
  GENFILE *genfile;

  ALLOCASSIGNSPACE(genfile, NULL, GENFILE, UintConst(1));

  genfile->genfilemode = genfilemode;

  switch(genfilemode)
  {
    case STANDARD:
      genfile->fileptr.file = fopen(path, mode);
      if(genfile->fileptr.file == NULL)
      {
        FREESPACE(genfile);
        return NULL;
      }
      break;
    case GZIP:
      genfile->fileptr.gzfile = gzopen(path, mode);
      if(genfile->fileptr.gzfile == NULL)
      {
        FREESPACE(genfile);
        return NULL;
      }
      break;
    case BZIP2:
#ifdef HAVE_BZLIB_H
      genfile->fileptr.bzfile = BZ2_bzopen(path, mode);
      if(genfile->fileptr.bzfile == NULL)
      {
        FREESPACE(genfile);
        return NULL;
      }
      break;
#else
      fprintf(stderr, "%s() has been compiled without -DHAVE_BZLIB_H\n"
             , FUNCTIONNAME);
      exit(EXIT_FAILURE);
#endif
    default:
      fprintf(stderr, "%s(): illegal generic file mode\n", FUNCTIONNAME);
      exit(EXIT_FAILURE);
  }
  
  return genfile;
}

Fclosereturntype genclose(GENFILE *genfile)
{
  Fclosereturntype rval = 0;

  switch(genfile->genfilemode)
  {
    case STANDARD:
      rval = fclose(genfile->fileptr.file);
      break;
    case GZIP:
      rval = gzclose(genfile->fileptr.gzfile);
      break;
    case BZIP2:
#ifdef HAVE_BZLIB_H
      BZ2_bzclose(genfile->fileptr.bzfile);
      break;
#else
      fprintf(stderr, "%s() has been compiled without -DHAVE_BZLIB_H\n"
             , FUNCTIONNAME);
      exit(EXIT_FAILURE);
#endif
    default:
      fprintf(stderr, "%s(): illegal generic file mode\n", FUNCTIONNAME);
      exit(EXIT_FAILURE);
  }

  FREESPACE(genfile);

  return rval;
}

static Fprintfreturntype vgzprintf(gzFile file, const char *format, va_list va)
{
  char buf[Z_PRINTF_BUFSIZE];
  Sprintfreturntype len;

  len = vsnprintf(buf, sizeof(buf), format, va);
  if(len > Z_PRINTF_BUFSIZE)
  {
    fprintf(stderr, "%s(): buffer buf[Z_PRINTF_BUFSIZE] to small\n"
           , FUNCTIONNAME);
    exit(EXIT_FAILURE);
  }

  return gzwrite(file, buf, (unsigned) len);
}

#ifdef HAVE_BZLIB_H
static Fprintfreturntype vbzprintf(BZFILE *file, const char *format, va_list va)
{
  char buf[Z_PRINTF_BUFSIZE];
  Fprintfreturntype len;

  len = vsnprintf(buf, sizeof(buf), format, va);
  if(len > Z_PRINTF_BUFSIZE)
  {
    fprintf(stderr, "%s(): buffer buf[Z_PRINTF_BUFSIZE] to small\n"
           , FUNCTIONNAME);
    exit(EXIT_FAILURE);
  }

  return BZ2_bzwrite(file, buf, len);
}
#endif

Fprintfreturntype vgenprintf(GENFILE *genfile, const char *format, va_list va) 
{
  Fprintfreturntype rval;

  if(genfile == NULL) // implies stdout
  {
    rval = vfprintf(stdout, format, va);
  }
  else
  {
    switch(genfile->genfilemode)
    {
      case STANDARD:
        rval = vfprintf(genfile->fileptr.file, format, va);
        break;
      case GZIP:
        rval = vgzprintf(genfile->fileptr.gzfile, format, va);
        break;
      case BZIP2:
#ifdef HAVE_BZLIB_H
        rval = vbzprintf(genfile->fileptr.bzfile, format, va);
        break;
#else
        fprintf(stderr, "%s() has been compiled without -DHAVE_BZLIB_H\n"
               , FUNCTIONNAME);
        exit(EXIT_FAILURE);
#endif
      default:
        fprintf(stderr, "%s(): illegal generic file mode\n", FUNCTIONNAME);
        exit(EXIT_FAILURE);
    }
  }

  return rval;
}

void genprintf(GENFILE *genfile, const char *format, ...)
{
  va_list va;

  va_start(va, format);

  if(vgenprintf(genfile, format, va) < 0)
  {
    fprintf(stderr, "%s(): vgenptrintf() returned negative value\n"
           , FUNCTIONNAME);
    exit(EXIT_FAILURE);
  }

  va_end(va);
}

#ifdef HAVE_BZLIB_H
static Fgetcreturntype BZ2_bzgetc(BZFILE *bzfile)
{
  Uchar c;

  return BZ2_bzread(bzfile, &c, 1) == 1 ? (Fgetcreturntype) c : -1;
}
#endif

#ifdef HAVE_BZLIB_H
static Fputcreturntype BZ2_bzputc(BZFILE *bzfile, Fputcfirstargtype c)
{ 
    // comment and code taken from gzio.c from zlib-1.2.1:
    // required for big endian systems
    Uchar cc = (Uchar) c; 

    return BZ2_bzwrite(bzfile, &cc, 1) == 1 ? (Fputcreturntype) cc : -1;
}
#endif

Fgetcreturntype gengetc(GENFILE *genfile)
{
  switch(genfile->genfilemode)
  {
    case STANDARD:
      return fgetc(genfile->fileptr.file);
    case GZIP:
      return gzgetc(genfile->fileptr.gzfile);
    case BZIP2:
#ifdef HAVE_BZLIB_H
      return  BZ2_bzgetc(genfile->fileptr.bzfile);
#else
      fprintf(stderr, "%s() has been compiled without -DHAVE_BZLIB_H\n"
             , FUNCTIONNAME);
      exit(EXIT_FAILURE);
#endif
    default:
      fprintf(stderr, "%s(): illegal generic file mode\n", FUNCTIONNAME);
      exit(EXIT_FAILURE);
  }
}

Fputcreturntype genputc(Fputcfirstargtype c, GENFILE *genfile)
{
  if(genfile == NULL)
  {
    return putc(c, stdout);
  }
  switch(genfile->genfilemode)
  {
    case STANDARD:
      return putc(c, genfile->fileptr.file);
    case GZIP:
      return gzputc(genfile->fileptr.gzfile, c);
    case BZIP2:
#ifdef HAVE_BZLIB_H
      return BZ2_bzputc(genfile->fileptr.bzfile, c);
#else
      fprintf(stderr, "%s() has been compiled without -DHAVE_BZLIB_H\n"
             , FUNCTIONNAME);
      exit(EXIT_FAILURE);
#endif
    default:
      fprintf(stderr, "%s(): illegal generic file mode\n", FUNCTIONNAME);
      exit(EXIT_FAILURE);
  }
}
