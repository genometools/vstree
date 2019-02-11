#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "failures.h"
#include "minmax.h"
#include "chardef.h"
#include "errordef.h"
#include "debugdef.h"
#include "spacedef.h"

#include "extractcc.h"
#include "errordef.h"

#define HANDLESPECIAL(CC) (ISBWTSPECIAL(CC) ? 0 : (CC))

void bytecode2string(Uchar *seqbuf, const Uchar *bytecode,const Uint len)
{
  Uint i, j, shiftright = UintConst(6);

  for(i=0, j=0; i<len; i++)
  {
    seqbuf[i] = (bytecode[j] >> shiftright) & UintConst(3);
    if(shiftright == 0)
    {
      shiftright = UintConst(6);
      j++;
    } else
    {
      shiftright -= 2;
    }
  }
}

#ifdef DEBUG

void showbytecode(FILE *outfp,const Uchar *bytecode,const Uint len)
{
  Uint i;
  Uchar *stringbuffer;
  char *characters = "acgt";

  ALLOCASSIGNSPACE(stringbuffer,NULL,Uchar,len);
  bytecode2string(stringbuffer,bytecode,len);
  for(i=0; i<len; i++)
  {
    (void) putc((Fputcfirstargtype) characters[stringbuffer[i]],outfp);
  }
  FREESPACE(stringbuffer);
}

static void checkdecodebytestring(Uchar *bytebuffer,
                                  const Uchar *seq,
                                  const Uint len)
{
  Uint i;
  Uchar *stringbuffer;

  ALLOCASSIGNSPACE(stringbuffer,NULL,Uchar,len);
  bytecode2string(stringbuffer,bytebuffer,len);
  for(i=0; i<len; i++)
  {
    if(stringbuffer[i] != seq[i])
    {
      fprintf(stderr,"%lu: stringbuffer = %lu != %lu = seq\n",
                      (Showuint) i,
                      (Showuint) stringbuffer[i],
                      (Showuint) seq[i]);
      exit(EXIT_FAILURE);
    }
  }
  FREESPACE(stringbuffer);
}
#endif

void string2bytecode(Uchar *bytecode,Uchar *seq,const Uint len)
{
  Uint j, remaining;
  Uchar *seqptr;

  for(seqptr=(Uchar *) seq, j=0; 
      seqptr < seq + len - 3; seqptr+=4, j++)
  {
    bytecode[j] = (seqptr[0] << UintConst(6)) | 
                  (seqptr[1] << UintConst(4)) | 
                  (seqptr[2] << UintConst(2)) |
                   seqptr[3];
  }
  remaining = MOD4(len);
  if(remaining != 0)
  {
    if(remaining == UintConst(1))
    {
      bytecode[j] = seqptr[0] << UintConst(6);
    } else
    {
      if(remaining == UintConst(2))
      {
        bytecode[j] = (seqptr[0] << UintConst(6)) |
                      (seqptr[1] << UintConst(4));
      } else
      {
        if(remaining == UintConst(3))
        {
          bytecode[j] = (seqptr[0] << UintConst(6)) | 
                        (seqptr[1] << UintConst(4)) | 
                        (seqptr[2] << UintConst(2));
        } else
        {
          NOTSUPPOSED;
        }
      }
    }
  }
#ifdef DEBUG
  checkdecodebytestring(bytecode,seq,len);
#endif
}

void shiftbytecode(Uchar *dest,const Uchar *source,
                   const Uint startindex,const Uint len)
{
  Uint i, j, remaining;

  if(len >= UintConst(3))
  {
    /*
      for(i=startindex, j=0; i < startindex + DIV4(len) - 1; i++, j++)
      {
        dest[j] = source[i];
      }
    */
    for(i=startindex, j=0; i < startindex + len - 3; i+=4, j++)
    {
      dest[j] = (EXTRACTENCODEDCHAR(source,i) << UintConst(6)) | 
                (EXTRACTENCODEDCHAR(source,i+1) << UintConst(4)) | 
                (EXTRACTENCODEDCHAR(source,i+2) << UintConst(2)) |
                EXTRACTENCODEDCHAR(source,i+3);
    }
  } else
  {
    i = startindex;
    j = 0;
  }
  remaining = MOD4(len);
  if(remaining != 0)
  {
    if(remaining == UintConst(1))
    {
      dest[j] = EXTRACTENCODEDCHAR(source,i) << UintConst(6);
    } else
    {
      if(remaining == UintConst(2))
      {
        dest[j] = (EXTRACTENCODEDCHAR(source,i) << UintConst(6)) |
                  (EXTRACTENCODEDCHAR(source,i+1) << UintConst(4));
      } else
      {
        if(remaining == UintConst(3))
        {
          dest[j] = (EXTRACTENCODEDCHAR(source,i) << UintConst(6)) | 
                    (EXTRACTENCODEDCHAR(source,i+1) << UintConst(4)) | 
                    (EXTRACTENCODEDCHAR(source,i+2) << UintConst(2));
        } else
        {
          NOTSUPPOSED;
        }
      }
    }
  }
}

static Sint readnextstring(Uchar *readbytes,Uint maxnumofchars,FILE *fp)
{
  size_t numofchars;

  numofchars = fread(readbytes,sizeof(Uchar),(size_t) maxnumofchars,fp);
  if(ferror(fp))
  {
    ERROR1("error when trying to read next %lu Uchar values",
            (Showuint) numofchars);
    return (Sint) -1;
  }
  return (Sint) numofchars;
}

/*
static Sint readnextstring(Uchar *readbytes,Uint numofchars,
                           UcharBufferedfile *buf)
{
  if(buf->nextread + numofchars >= buf->nextfree)
  {
    buf->nextfree = (Uint) fread(buf->bufspace,
                                 sizeof(Uchar),
                                 (size_t) FILEBUFFERSIZE,
                                 buf->fp);
    if(ferror(buf->fp))
    {
      ERROR1("error when trying to read next %lu Uchar",(Showuint) numofchars);
      return (Sint) -2;
    }
    buf->nextread = 0;
    if(numofchars >= buf->nextfree)
    {
      ERROR0("unexpected end of file when trying to read Uchar");
      return (Sint) -1;
    }
  }
  memcpy(readbytes,buf->bufspace + buf->nextread,(size_t) numofchars);
  buf->nextread += numofchars;
  return 0;
}
*/

Sint string2bytecodewithspecial(Uchar *bytecode,
                                FILE *fptistabstream)
{
  Uint j, remaining;
  Sint retcode = 0;
  Uchar seqbuffer[4];

  for(j=0; /* Nothing */; j++)
  {
    retcode = readnextstring(&seqbuffer[0],UintConst(4),fptistabstream);
    if(retcode < 0)
    {
      return (Sint) -1;
    }
    if(retcode == (Sint) 4)
    {
      bytecode[j] = (HANDLESPECIAL(seqbuffer[0]) << UintConst(6)) | 
                    (HANDLESPECIAL(seqbuffer[1]) << UintConst(4)) | 
                    (HANDLESPECIAL(seqbuffer[2]) << UintConst(2)) |
                     HANDLESPECIAL(seqbuffer[3]);
    } else
    {
      break;
    }
  }
  remaining = (Uint) retcode;
  if(remaining != 0)
  {
    if(remaining == UintConst(1))
    {
      bytecode[j] = HANDLESPECIAL(seqbuffer[0]) << UintConst(6);
    } else
    {
      if(remaining == UintConst(2))
      {
        bytecode[j] = (HANDLESPECIAL(seqbuffer[0]) << UintConst(6)) |
                      (HANDLESPECIAL(seqbuffer[1]) << UintConst(4));
      } else
      {
        if(remaining == UintConst(3))
        {
          bytecode[j] = (HANDLESPECIAL(seqbuffer[0]) << UintConst(6)) | 
                        (HANDLESPECIAL(seqbuffer[1]) << UintConst(4)) | 
                        (HANDLESPECIAL(seqbuffer[2]) << UintConst(2));
        } else
        {
          NOTSUPPOSED;
        }
      }
    }
  }
  return 0;
}

Uint extractprefixbytecode(const Uint merbytes,
                           const Uint prefixlength,
                           const Uchar *bytecode)
{
  Uint i, code = 0;

  for(i=0; i < MIN((Uint) sizeof(Uint),merbytes); i++)
  {
    code = (code << 8) | bytecode[i];
    // printf("bytcode[%u]=%u,code=%u\n",i,bytecode[i],code);
    if(MULT4(i+1) == prefixlength)
    {
      break;
    }
    if(MULT4(i+1) > prefixlength)
    {
      code >>= MULT2(MULT4(i+1) - prefixlength);
      // printf("code=%u\n",code);
      break;
    }
  }
  return code;
}
