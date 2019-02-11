#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <zlib.h>
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "fhandledef.h"

#include "filehandle.pr"
#include "readgzip.pr"
#include "checkgzip.pr"

#undef NOGZIP

#ifdef NOGZIP
/*@null@*/ Uchar *readgzippedfile(const char *filename,Uint *filesize)
{
  ERROR1("reading of gzipped file \"%s\" does not work "
         " for this program version",filename);
  return NULL;
}
#else

#define BUFLEN        16384

typedef  enum
{
  GzipFilesize,
  GzipEcho,
  GzipStore
} GzipAction;

typedef struct
{
  Uchar *filecontentptr;
  Uint filecontentlen;
} Gzipresult;

static void initgzipresult(Gzipresult *gzipresult)
{
  gzipresult->filecontentlen = 0;
  gzipresult->filecontentptr = NULL;
}

static Sint gzipuncompressfilesize (const char *filename,
                                    Gzipresult *gzipresult,
                                    gzFile gzhandle)
{
  const char *errmsg;
  char buf[BUFLEN];
  Sint totaluncompressed = 0, uncompressedbytes;
  Gzerrorflagtype err;

  for (;;)
  {
    uncompressedbytes = (Sint) gzread (gzhandle, buf, 
                                       (Gzreadthirdarg) sizeof (buf));
    if (uncompressedbytes < 0)
    {
      errmsg = gzerror (gzhandle, &err);
      if (err == Z_ERRNO)
      {
        ERROR1 ("file system error when decompressing file \"%s\"",filename);
        return (Sint) -1;
      }
      ERROR2 ("error when decompressing file \"%s\": %s",filename, errmsg);
      return (Sint) -2;
    }
    if (uncompressedbytes == 0)
    {
      break;
    }
    totaluncompressed += uncompressedbytes;
  }
  gzipresult->filecontentlen = (Uint) totaluncompressed;
  return 0;
}

static Sint gzipuncompressecho (const char *filename,gzFile gzhandle)
{
  const char *errmsg;
  char buf[BUFLEN];
  Sint uncompressedbytes;
  Gzerrorflagtype err;

  for (;;)
  {
    uncompressedbytes = (Sint) gzread (gzhandle, buf, 
                                       (Gzreadthirdarg) sizeof (buf));
    if (uncompressedbytes < 0)
    {
      errmsg = gzerror (gzhandle, &err);
      if (err == Z_ERRNO)
      {
        ERROR1 ("file system error when decompressing file \"%s\"",filename);
        return (Sint) -1;
      }
      ERROR2 ("error when decompressing file \"%s\": %s", filename,errmsg);
      return (Sint) -2;
    }
    if (uncompressedbytes == 0)
    {
      break;
    }
    if (WRITETOFILEHANDLE(buf, (Uint) sizeof(char),
                          (Uint) uncompressedbytes, 
                          stdout) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

static Sint gzipuncompressstore (const char *filename,
                                 Gzipresult *gzipresult,
                                 gzFile gzhandle)
{
  const char *errmsg;
  Sint uncompressedbytes;
  Gzerrorflagtype err;

  ALLOCASSIGNSPACE(gzipresult->filecontentptr,
                   gzipresult->filecontentptr,Uchar,
                   gzipresult->filecontentlen);
  uncompressedbytes = (Sint) gzread (gzhandle,
                                     gzipresult->filecontentptr, 
                                     (Gzreadthirdarg) 
                                         gzipresult->filecontentlen);
  if (uncompressedbytes < 0)
  {
    errmsg = gzerror (gzhandle, &err);
    if (err == Z_ERRNO)
    {
      ERROR1 ("file system error when decompressing file \"%s\"",filename);
      return (Sint) -1;
    }
    ERROR2 ("error when decompressing file \"%s\": %s",filename,errmsg);
    return (Sint) -2;
  }
  return 0;
}

static Sint gzipfileuncompress(Gzipresult *gzipresult,
                               GzipAction gzipaction,
                               const char *filename)
{
  gzFile gzhandle;
  Sint retcode;

  gzhandle = gzopen (filename, "rb");
  if (gzhandle == NULL)
  {
    if(errno == 0)
    {
      ERROR1 ("error when uncompressing file \"%s\": "
              "not enough memory to allocate the decompression state",
              filename);
    } else
    {
      ERROR2 ("error when uncompressing file \"%s\": "
              "cannot open file \"%s\"", filename,filename);
    }
    return (Sint) -2;
  }
  switch(gzipaction)
  {
    case GzipFilesize:
      retcode = gzipuncompressfilesize (filename,gzipresult,gzhandle);
      break;
    case GzipEcho:
      retcode = gzipuncompressecho (filename,gzhandle);
      break;
    case GzipStore:
      retcode = gzipuncompressstore (filename,gzipresult,gzhandle);
      break;
    default:
      fprintf(stderr,"unknown case %lu\n",(Showuint) gzipaction);
      exit(EXIT_FAILURE);
  }
  if(retcode < 0)
  {
    return (Sint) -3;
  }
  if (gzclose (gzhandle) != Z_OK)
  {
    ERROR1 ("cannot close gzipped file \"%s\"",filename);
    return (Sint) -4;
  }
  return 0;
}

/*@null@*/ Uchar *readgzippedfile(const char *filename,Uint *filesize)
{
  Sint retcode;
  Gzipresult gzipresult;

  initgzipresult(&gzipresult);
  retcode = gzipfileuncompress (&gzipresult,GzipFilesize,filename);
  if(retcode < 0)
  {
    return NULL;
  } 
  retcode = gzipfileuncompress (&gzipresult, GzipStore, filename);
  if(retcode < 0)
  {
    return NULL;
  }
  *filesize = gzipresult.filecontentlen;
  return gzipresult.filecontentptr;
}
#endif
