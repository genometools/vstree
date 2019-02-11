//\IgnoreLatex{

#ifndef CHARDEF_H
#define CHARDEF_H
#include "types.h"
#include <limits.h>

//}

/*
  This file defines some character values used when storing
  multiple sequences.
*/

/*
  separator symbol in multiple seq
*/

#define SEPARATOR       UCHAR_MAX         

/*
  wildcard symbol in multiple seq
*/

#define WILDCARD        (SEPARATOR-1)

/*
  undefined character, only to be used in conjunction with symbol maps
*/

#define UNDEFCHAR       (SEPARATOR-2)     

/*
  either WILDCARD or SEPARATOR
*/

#define ISSPECIAL(C)    ((C) >= (Uchar) WILDCARD)

/*
  neither WILDCARD nor SEPARATOR
*/

#define ISNOTSPECIAL(C) ((C) < (Uchar) WILDCARD)

/*
  undefined character, only to be used in conjunction with the Burrows-Wheeler
  transform
*/

#define UNDEFBWTCHAR    UNDEFCHAR

/*
  Either special character or UNDEFBWTCHAR
*/

#define ISBWTSPECIAL(C)    ((C) >= (Uchar) UNDEFBWTCHAR)

/*
  the size of the DNA alphabet
*/

#define DNAALPHASIZE UintConst(4)

//\IgnoreLatex{

#endif

//}
