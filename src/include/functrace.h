/*
  Copyright (C) 2003-2004 Gordon Gremme <gremme@zbh.uni-hamburg.de>
*/

#ifndef FUNCTRACE_H
#define FUNCTRACE_H

#include "debugdef.h"

#define FUNCTIONNAME        __FUNCTION__

#define CALLED(LEVEL)   DEBUG3(LEVEL,"%s(): called (line %lu in file %s)\n"\
                               , FUNCTIONNAME , (Showuint) __LINE__, __FILE__)

#define FINISHEDVOID(LEVEL) DEBUG3(LEVEL,"%s(): finished (void function"\
                                         ", line %lu in file %s)\n"\
                                         , FUNCTIONNAME\
                                         , (Showuint) __LINE__, __FILE__)

#define FINISHEDINT(LEVEL,INT) DEBUG4(LEVEL,"%s(): finished (returning %ld"\
                                            ", line %lu in file %s)\n"\
                                            , FUNCTIONNAME , (Showsint) INT\
                                            , (Showuint) __LINE__, __FILE__);\
                               return INT

#define FINISHEDFLOAT(LEVEL,FLOAT) DEBUG4(LEVEL,"%s(): finished (returning "\
                                          "%f, line %lu in file %s)\n"\
                                          , FUNCTIONNAME, (double) FLOAT\
                                          , (Showuint) __LINE__, __FILE__);\
                                   return FLOAT

#define FINISHEDBOOL(LEVEL,BOOL) DEBUG4(LEVEL,"%s(): finished (returning "\
                                              "%s, line %lu in file %s)\n"\
                                             , FUNCTIONNAME, SHOWBOOL(BOOL)\
                                             , (Showuint) __LINE__, __FILE__);\
                                 return BOOL

#define FINISHEDCHAR(LEVEL,CHAR) DEBUG4(LEVEL,"%s(): finished (returning "\
                                              "%c, line %lu in file %s)\n"\
                                             , FUNCTIONNAME, CHAR\
                                             , (Showuint) __LINE__, __FILE__);\
                                 return CHAR

#endif
