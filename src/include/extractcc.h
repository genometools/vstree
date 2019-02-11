#ifndef EXTRACTCC_H
#define EXTRACTCC_H

#include "divmodmul.h"

#define EXTRACTENCODEDCHAR(ESEQ,IDX)\
        ((ESEQ[DIV4(IDX)] >> (UintConst(6) - MULT2(MOD4(IDX)))) & UintConst(3))

#define EXTRACTENCODEDCHARUint64(ESEQ,IDX)\
        ((ESEQ[(Uint) DIV4(IDX)] >> (UintConst(6) - (Uint) MULT2(MOD4(IDX)))) & UintConst(3))

#endif
