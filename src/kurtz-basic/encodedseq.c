#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <limits.h>
#include "types.h"
#include "intbits.h"
#include "alphadef.h"
#include "chardef.h"
#include "divmodmul.h"
#include "esastream.h"
#include "debugdef.h"
#include "extractcc.h"

#include "encseq-def.h"

#include "bytehandler.pr"

 DECLAREREADFUNCTION(Uchar);

#define CASENAME(V) case V: return #V

#define CHECKANDUPDATE(VAL)\
        tmp = determinesizeofrepresentation(VAL,\
                                            totallength,\
                                            specialcharacters);\
        if(tmp < cmin)\
        {\
          cmin = tmp;\
          cret = VAL;\
        }

#define NUMOFINTSFORBITSUint64(N)\
        ((DIVWORDSIZE(N) == 0)\
           ? (Uint64) 1 \
           : ((Uint64) 1 + DIVWORDSIZE((N) - (Uint64) 1)))

/*@null@*/ static char *accesstype2name(Positionaccesstype sat)
{
  switch(sat)
  {
    CASENAME(Viadirectaccess);
    CASENAME(Viabitaccess);
    CASENAME(Viauchartables);
    CASENAME(Viaushorttables);
    CASENAME(Viauinttables);
    CASENAME(Viauint64tables);
    default: break;
  }
  return NULL;
}

static Uint64 determinesizeofrepresentation(Positionaccesstype sat,
                                            Uint64 totallength,
                                            Uint specialcharacters)
{
  Uint64 sum, 
         sizeforfixedbits = (MULT2(totallength))/CHAR_BIT;

  switch(sat)
  {
    case Viadirectaccess:
         return totallength * (Uint64) sizeof(Uchar);
    case Viabitaccess:
         sum = sizeforfixedbits;
         if(specialcharacters > 0)
         {
           sum += (Uint64) sizeof(Uint) * 
                  (Uint64) NUMOFINTSFORBITSUint64(totallength) +
                  (Uint64) sizeof(void *);
         }
         return sum;
    case Viauchartables:
         sum = sizeforfixedbits;
         if(specialcharacters > 0)
         {
           sum += (Uint) sizeof(Uchar) * specialcharacters +
                  (Uint) sizeof(void *) * (totallength/UCHAR_MAX+1) +
                  UintConst(3) * (Uint) sizeof(void *);
         }
         return sum;
    case Viaushorttables:
         sum = sizeforfixedbits;
         if(specialcharacters > 0)
         {
           sum += (Uint64) sizeof(Ushort) * (Uint64) specialcharacters +
                  (Uint64) sizeof(void *) * (totallength/USHRT_MAX+1) +
                  UintConst(3) * (Uint) sizeof(void *);
         }
         return sum;
    case Viauinttables:
         sum = sizeforfixedbits;
         if(specialcharacters > 0)
         {
           sum += (Uint64) sizeof(Uint) * (Uint64) specialcharacters + 
                  (Uint64) 3 * (Uint64) sizeof(void *);
         }
         return sum;
    case Viauint64tables:
         sum = sizeforfixedbits;
         if(specialcharacters > 0)
         {
           sum += (Uint64) sizeof(Uint64) * (Uint64) specialcharacters + 
                  (Uint64) sizeof(void *);
         }
         return sum;
    default:
         fprintf(stderr,"determinesizeofrepresentation(%lu) undefined\n",
                 (Showuint) sat);
         exit(EXIT_FAILURE);
  }
}

#define ADDTYPE(V)               uchar##V
#define ACCESSENCSEQ(ES,V)       (ES)->uchar##V
#define SPECIALTYPE              Uchar
#define MAXSPECIALTYPE           UCHAR_MAX
#define DIVMAXSPECIALTYPE(V)     ((V) >> 8)

#include "accessspecial.gen"
#include "accessspecial64.gen"

#undef ADDTYPE
#undef ACCESSENCSEQ
#undef SPECIALTYPE
#undef MAXSPECIALTYPE
#undef DIVMAXSPECIALTYPE

#define ADDTYPE(V)               ushort##V
#define ACCESSENCSEQ(ES,V)       (ES)->ushort##V
#define SPECIALTYPE              Ushort
#define MAXSPECIALTYPE           USHRT_MAX
#define DIVMAXSPECIALTYPE(V)     ((V) >> 16)

#include "accessspecial.gen"
#include "accessspecial64.gen"

#undef ADDTYPE
#undef ACCESSENCSEQ
#undef SPECIALTYPE
#undef MAXSPECIALTYPE
#undef DIVMAXSPECIALTYPE

#define ADDTYPE(V)               uint##V
#define ACCESSENCSEQ(ES,V)       (ES)->uint##V
#define SPECIALTYPE              Uint
#define MAXSPECIALTYPE           UINT_MAX
#define DIRECTBINSEARCH
#define IGNORECHECKSPECIALPOSITIONS

#include "accessspecial.gen"

#define DIVMAXSPECIALTYPE(V)     ((V) >> 32)

#include "accessspecial64.gen"

#undef ADDTYPE
#undef ACCESSENCSEQ
#undef SPECIALTYPE
#undef MAXSPECIALTYPE
#undef DIVMAXSPECIALTYPE
#undef DIRECTBINSEARCH
#undef IGNORECHECKSPECIALPOSITIONS

#define ADDTYPE(V)               uint64##V
#define ACCESSENCSEQ(ES,V)       (ES)->ADDTYPE(V)
#define SPECIALTYPE              Uint64

#include "uintaccessspecial.gen"

static Uchar delivercharViabitaccess(const Encodedsequence *encseq,
                                     Uint pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(ISIBITSET(encseq->specialbits,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHAR(encseq->fourcharsinonebyte,pos);
}

static Uchar delivercharViabitaccess64(const Encodedsequence *encseq,
                                       Uint64 pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(ISIBITSET(encseq->specialbits,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHARUint64(encseq->fourcharsinonebyte,pos);
}

/* Viauchartables */

static Uchar delivercharViauchartables64(const Encodedsequence *encseq,
                                         Uint64 pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(ucharcheckspecial64(encseq,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHARUint64(encseq->fourcharsinonebyte,pos);
}

static Uchar delivercharViauchartables(const Encodedsequence *encseq,
                                       Uint pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(ucharcheckspecial(encseq,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHAR(encseq->fourcharsinonebyte,pos);
}

/* Viaushorttables */

static Uchar delivercharViaushorttables64(const Encodedsequence *encseq,
                                          Uint64 pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(ushortcheckspecial64(encseq,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHARUint64(encseq->fourcharsinonebyte,pos);
}

static Uchar delivercharViaushorttables(const Encodedsequence *encseq,
                                        Uint pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(ushortcheckspecial(encseq,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHAR(encseq->fourcharsinonebyte,pos);
}

/* Viauinttables */

static Uchar delivercharViauinttables64(const Encodedsequence *encseq,
                                        Uint64 pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(uintcheckspecial64(encseq,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHARUint64(encseq->fourcharsinonebyte,pos);
}

static Uchar delivercharViauinttables(const Encodedsequence *encseq,Uint pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(uintcheckspecial(encseq,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHAR(encseq->fourcharsinonebyte,pos);
}

/* Viauint64tables */

static Uchar delivercharViauint64tables64(const Encodedsequence *encseq,
                                          Uint64 pos)
{
  if(encseq->specialcharinfo.specialcharacters > 0)
  {
    if(uint64checkspecial64(encseq,pos))
    {
      return (Uchar) WILDCARD;
    }
  }
  return EXTRACTENCODEDCHARUint64(encseq->fourcharsinonebyte,pos);
}

static Uchar delivercharViauint64tables(/*@unused@*/ 
                                        const Encodedsequence *encseq,
                                        Uint pos)
{
  fprintf(stderr,"delivercharViauint64tables(%lu) undefined\n",
          (Showuint) pos);
  exit(EXIT_FAILURE);
}

static Positionaccesstype determinesmallestrep(Uint64 totallength,
                                               Uint specialcharacters)
{
  Positionaccesstype cret;
  Uint64 tmp, cmin;

  cmin = determinesizeofrepresentation(Viabitaccess,
                                       totallength,
                                       specialcharacters);
  cret = Viabitaccess;
  CHECKANDUPDATE(Viauchartables);
  CHECKANDUPDATE(Viaushorttables);
  CHECKANDUPDATE(Viauinttables);
  /* CHECKANDUPDATE(Viauint64tables);*/
  return cret;
}

/*
  The following function is exported only for test purposes in encodedseq-mn.c.
  In applications use initencodedseq.
*/

Sint readencodedseq(Encodedsequence *encseq,
                    Positionaccesstype sat,
                    UcharBufferedfile *inputstream,
                    Uint64 totallength,
                    const Specialcharinfo *specialcharinfo,
                    const Alphabet *alphabet,
                    const char *indexname)
{
  Uint i;
  Uchar cc = 0;
  Sint retval;
  Uint64 sizeofrep 
           = determinesizeofrepresentation(sat,
                                           totallength,
                                           specialcharinfo->specialcharacters);
  encseq->deliverchar = NULL;
  encseq->sat = sat;
  encseq->name = accesstype2name(sat);
  encseq->totallength = totallength;
  encseq->mapsize = alphabet->mapsize;
  encseq->specialcharinfo = *specialcharinfo;
  encseq->spaceinbitsperchar 
    = (double) ((Uint64) CHAR_BIT * sizeofrep)/(double) totallength;
  /*@ignore@*/
  printf("# init character encoding for index \"%s\" (%s," FormatUint64 
         " bytes,%.2f bits/symbol)\n",
          indexname,encseq->name,sizeofrep,encseq->spaceinbitsperchar);
  /*@end@*/
  ALLOCASSIGNSPACE(encseq->characters,NULL,Uchar,alphabet->domainsize);
  (void) memcpy(encseq->characters,alphabet->characters,
                (size_t) alphabet->domainsize);
  if(specialcharinfo->specialcharacters > 0)
  {
    switch(encseq->sat)
    {
      case Viabitaccess:
        INITBITTAB(encseq->specialbits,(Uint) encseq->totallength);
        for(i=0; /* Nothing */; i++)
        {
          retval = readnextUchar(&cc,inputstream);
          if(retval < 0)
          {
            return (Sint) -1;
          }
          if(retval == 0)
          {
            /*@loopbreak@*/ break;
          }
          if(ISSPECIAL(cc))
          {
            SETIBIT(encseq->specialbits,i);
          }
        }
        encseq->deliverchar = delivercharViabitaccess;
        encseq->deliverchar64 = delivercharViabitaccess64;
        rewind(inputstream->fp);
        break;

      case Viauchartables:
        if(ucharfillspecialtables(encseq,inputstream) != 0)
        {
          return (Sint) -2;
        }
        encseq->deliverchar = delivercharViauchartables;
        encseq->deliverchar64 = delivercharViauchartables64;
#ifdef DEBUG
        ucharcheckspecialpositions(encseq);
#endif
        rewind(inputstream->fp);
        break;

      case Viaushorttables:
        if(ushortfillspecialtables(encseq,inputstream) != 0)
        {
          return (Sint) -2;
        }
        encseq->deliverchar = delivercharViaushorttables;
        encseq->deliverchar64 = delivercharViaushorttables64;
#ifdef DEBUG
        ushortcheckspecialpositions(encseq);
#endif
        rewind(inputstream->fp);
        break;

      case Viauinttables:
        if(uintfillspecialtables(encseq,inputstream) != 0)
        {
          return (Sint) -2;
        }
        encseq->deliverchar = delivercharViauinttables;
        encseq->deliverchar64 = delivercharViauinttables64;
        rewind(inputstream->fp);
        break;

      case Viauint64tables:
        if(uint64fillspecialtables(encseq,inputstream) != 0)
        {
          return (Sint) -2;
        }
        encseq->deliverchar = delivercharViauint64tables;
        encseq->deliverchar64 = delivercharViauint64tables64;
        rewind(inputstream->fp);
        break;

      default:
        break;
    }
  } else
  {
    encseq->specialbits = NULL;
    encseq->ushortspecialpositions = NULL;
    encseq->ushortendspecialsubs = NULL;
    encseq->ucharspecialpositions = NULL;
    encseq->ucharendspecialsubs = NULL;
    encseq->uintspecialpositions = NULL;
    encseq->deliverchar = NULL; 
    encseq->deliverchar64 = NULL; 
  }
  if(encseq->mapsize == DNAALPHASIZE + 1 && encseq->sat != Viadirectaccess)
  {
    Uint numofbytes;

    if(DIV4(encseq->totallength) == 0)
    {
      numofbytes = UintConst(1);
    } else
    {
      numofbytes = UintConst(1) + (Uint) DIV4(encseq->totallength - 1);
    }
    ALLOCASSIGNSPACE(encseq->fourcharsinonebyte,NULL,Uchar,numofbytes);
    if(string2bytecodewithspecial(encseq->fourcharsinonebyte,
                                  inputstream->fp) != 0)
    {
      return (Sint) -1;
    }
    rewind(inputstream->fp);
  } else
  {
    encseq->fourcharsinonebyte = NULL;
  }
  if(encseq->sat == Viadirectaccess)
  {
    ALLOCASSIGNSPACE(encseq->plainseq,NULL,Uchar,(Uint) encseq->totallength);
    for(i=0; /* Nothing */; i++)
    {
      retval = readnextUchar(&cc,inputstream);
      if(retval < 0)
      {
        return (Sint) -1;
      }
      if(retval == 0)
      {
        break;
      }
      encseq->plainseq[i] = cc;
    }
    encseq->deliverchar = NULL;
    rewind(inputstream->fp);
  }
  return 0;
}

Sint initencodedseq(Encodedsequence *encseq,
                    UcharBufferedfile *inputstream,
                    Uint64 totallength,
                    const Specialcharinfo *specialcharinfo,
                    const Alphabet *alphabet,
                    const char *indexname)
{
  Positionaccesstype sat;
  
  if(alphabet->mapsize == DNAALPHASIZE + 1)
  {
    sat = determinesmallestrep(totallength,specialcharinfo->specialcharacters);
  } else
  {
    sat = Viadirectaccess;
  }
  if(readencodedseq(encseq,
                    sat,
                    inputstream,
                    totallength,
                    specialcharinfo,
                    alphabet,
                    indexname) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

void freeEncodedsequence(Encodedsequence *encseq)
{
  FREESPACE(encseq->characters);
  switch(encseq->sat)
  {
    case Viadirectaccess:
      FREESPACE(encseq->plainseq);
      break;
    case Viabitaccess:
      FREESPACE(encseq->fourcharsinonebyte);
      FREESPACE(encseq->specialbits);
      break;
    case Viauchartables:
      FREESPACE(encseq->fourcharsinonebyte);
      FREESPACE(encseq->ucharspecialpositions);
      FREESPACE(encseq->ucharendspecialsubs);
      break;
    case Viaushorttables:
      FREESPACE(encseq->fourcharsinonebyte);
      FREESPACE(encseq->ushortspecialpositions);
      FREESPACE(encseq->ushortendspecialsubs);
      break;
    case Viauinttables:
      FREESPACE(encseq->fourcharsinonebyte);
      FREESPACE(encseq->uintspecialpositions);
      FREESPACE(encseq->uintendspecialsubs);
      break;
    case Viauint64tables:
      FREESPACE(encseq->fourcharsinonebyte);
      FREESPACE(encseq->uint64specialpositions);
      break;
    default: break;
  }
}
