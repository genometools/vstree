#ifndef ENCSEQDEF_H
#define ENCSEQDEF_H

#include "types.h"
#include "extractcc.h"

#define ACCESSENCODEDCHAR_SPECIAL(ENCSEQ,POS)\
        (((ENCSEQ)->specialcharinfo.specialcharacters == 0)\
            ? EXTRACTENCODEDCHAR((ENCSEQ)->fourcharsinonebyte,POS)\
            : (ENCSEQ)->deliverchar(ENCSEQ,POS))

#define ACCESSENCODEDCHAR(ENCSEQ,POS)\
        (((ENCSEQ)->sat == Viadirectaccess) \
            ? (ENCSEQ)->plainseq[POS]\
            : ACCESSENCODEDCHAR_SPECIAL(ENCSEQ,POS))

#define ACCESSENCODEDCHAR_SPECIAL64(ENCSEQ,POS)\
        (((ENCSEQ)->specialcharinfo.specialcharacters == 0)\
            ? EXTRACTENCODEDCHARUint64((ENCSEQ)->fourcharsinonebyte,POS)\
            : (ENCSEQ)->deliverchar64(ENCSEQ,POS))

#define ACCESSENCODEDCHAR64(ENCSEQ,POS)\
        (((ENCSEQ)->sat == Viadirectaccess) \
            ? (ENCSEQ)->plainseq[POS]\
            : ACCESSENCODEDCHAR_SPECIAL64(ENCSEQ,POS))

#define ACCESSSEQUENCELENGTH(ENCSEQ)\
        (ENCSEQ)->totallength

typedef enum
{
  Viadirectaccess,
  Viabitaccess,
  Viauchartables,
  Viaushorttables,
  Viauinttables,
  Viauint64tables,
  NumberofPositionaccesstypes
} Positionaccesstype;

typedef struct _Encodedsequence
{
  /* Common part */
  Uchar *characters;
  Positionaccesstype sat;
  Uint mapsize;
  Specialcharinfo specialcharinfo;
  Uint64 totallength;
  double spaceinbitsperchar;
  char *name;
  Uchar(*deliverchar64)(const struct _Encodedsequence *,Uint64);
  Uchar(*deliverchar)(const struct _Encodedsequence *,Uint);

  /* only for Viabitaccess, 
              Viauchartables,
              Viaushorttables, 
              Viauinttables */
  Uchar *fourcharsinonebyte;

  /* only for Viadirectaccess */
  Uchar *plainseq;

  /* only for Viabitaccess */
  Uint *specialbits;

  /* only for Viauchartables */
  Uchar *ucharspecialpositions, 
        **ucharendspecialsubs,
        **ucharlastendspecialsubs;

  /* only for Viaushorttables */
  Ushort *ushortspecialpositions, 
         **ushortendspecialsubs,
         **ushortlastendspecialsubs;

  /* only for Viauinttables */
  Uint *uintspecialpositions,
       **uintendspecialsubs,
       **uintlastendspecialsubs;

  /* only for Viauint64tables */
  Uint64 *uint64specialpositions;
} Encodedsequence;

#endif
