#ifndef VMATFIND_DEF_H
#define VMATFIND_DEF_H

#ifdef ESASTREAMACCESS
Sint strmvmatmaxoutdynamic(Esastream *esastream,
                           Uint numberofprocessors,
                           Uint searchlength,
                           /*@unused@*/ void *repeatgapspecinfo,
                           void *outinfo,
                           void *outputfunction);

#endif

#ifdef ESAMERGEDACCESS
Sint mergevmatmaxoutdynamic(Emissionmergedesa *emmesa,
                            Uint numberofprocessors,
                            Uint searchlength,
                            /*@unused@*/ void *repeatgapspecinfo,
                            void *outinfo,
                            void *outputfunction);

#endif


#if !defined ESASTREAMACCESS && !defined ESAMERGEDACCESS

Sint vmatmaxoutdynamic(Virtualtree *virtualtree,
                       Uint numberofprocessors,
                       Uint searchlength,
                       /*@unused@*/ void *repeatgapspecinfo,
                       void *outinfo,
                       void *outputfunction);

#endif

#endif
