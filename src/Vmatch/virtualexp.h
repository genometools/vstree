/*
  Copyright (c) 2000-2006 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/* This file is generated. Do not edit */
/* It contains the prototypes for some important exported functions. */
#ifndef VIRTUALEXP_H
#define VIRTUALEXP_H
#include "matchinfo.h"
#include "matchtask.h"
#include "chaindef.h"
#include "xdropdef.h"
#ifdef __cplusplus
extern "C" {
#endif
void assignvirtualdigits(Digits *digits,Multiseq *multiseq);

void assignquerydigits(Digits *digits,Multiseq *querymultiseq);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint echomatch2file(FILE *outfp,
                    BOOL fancy,
                    Uint showmode,
                    Showdescinfo *showdesc,
                    Uint showstring,
                    Digits *digits,
                    Multiseq *virtualmultiseq,
                    Multiseq *querymultiseq,
                    Alphabet *virtualalpha,
                    StoreMatch *storematch);

Sint immediatelyechothematch(void *voidoutinfo,
                             /*@unused@*/ Multiseq *virtualmultiseq,
                             /*@unused@*/ Multiseq *querymultiseq,
                             StoreMatch *storematch);

Sint simpleechomatch2file(FILE *outfp,
                          Multiseq *virtualmultiseq,
                          Multiseq *querymultiseq,
                          StoreMatch *storematch);

/*@null@*/ Uchar *echomatch2ram(BOOL fancy,
                                Uint showmode,
                                Showdescinfo *showdesc,
                                Uint showstring,
                                Digits *digits,
                                Multiseq *virtualmultiseq,
                                Alphabet *virtualalpha,
                                Multiseq *querymultiseq,
                                StoreMatch *storematch,
                                Uint numofmatches,
                                Uint *numofbytes,
                                Tmpfiledesc *tmpfiledesc);

Sint clearMatchram(Uchar **eptr,Tmpfiledesc *tmpfiledesc);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint determineMatchinfo(FILE *outfp,
                        Matchinfo *matchinfo,
                        Argctype argc,
                        const char **argv,
                        const char *matchfile,
                        Matchparam *selectmatchparam,
                        /*@unused@*/ void *dbparminfo,
                        BOOL withinputsequences,
                        Showverbose showverbose);

Sint freeMatchinfo(Matchinfo *matchinfo);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint savethearguments(char **args,BOOL basic,Argctype argc,
                      const char * const*argv,const char *indexormatchfile);

Sint showargumentline(SelectmatchHeader paramselectmatchHeader,
                      FILE *outfp,
                      char *argstring,
                      Argctype callargc,
                      const char **callargv);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint freeMatchcallinfo(Matchcallinfo *matchcallinfo);

Sint parsevmatchargs(BOOL withindexfile,
                     Argctype argc,
                     const char * const*argv,
                     Showverbose showverbose,
                     FILE *outfp,
                     SelectBundle *precompiledselectbundle,
                     Matchcallinfo *matchcallinfo);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint callvmatch(Argctype argc,
                const char **argv,
                void *processinfo,
                void initinfo(void *,void *),
                const char *functionname,
                Showmatchfuntype showmatchfun,
                Showverbose showverbose,
                FILE *outfp,
                SelectBundle *precompiledselectbundle,
                Virtualtree *virtualtree,
                Virtualtree *queryvirtualtree,
                Virtualtree *sixframeofqueryvirtualtree,
                Virtualtree *dnavirtualtree);

Sint wrapvmatch(Virtualtree *virtualtree,
                Virtualtree *queryvirtualtree,
                Virtualtree *sixframeofqueryvirtualtree,
                Virtualtree *dnavirtualtree);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint parselocalchainingparameter(Chainmode *chainmode,
                                 const char *option,
                                 const char *lparam);

Sint parsechain2dim(BOOL fromvmatch,
                    Chaincallinfo *chaincallinfo,
                    const char * const*argv,
                    Argctype argc);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
Sint openformatchaining(Chaincallinfo *chaincallinfo);

#ifdef __cplusplus
}
#endif
#endif
