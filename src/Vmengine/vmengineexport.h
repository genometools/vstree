#ifdef __cplusplus
extern "C" {
#endif
Sint findexactcompletematchescompressedindex(void *info,
                                             Uint seqnum2,
                                             /*@unused@*/ Uchar *pattern,
                                             Uint plen);

Sint findcompletematches(Virtualtree *virtualtree,
                         char *indexormatchfile,
                         Queryinfo *queryinfo,
                         BOOL rcmode,
                         BOOL online,
                         Matchparam *matchparam,
                         Bestflag bestflag,
                         Uint shownoevalue,
                         Uint showselfpalindromic,
                         SelectBundle *selectbundle,
                         void *procmultiseq,
                         Currentdirection currentdirection,
                         Processfinalfunction processfinal,
                         Vpluginbundle *cpridxpatsearchbundle,
                         Cpridxpatsearchdata *cpridxpatsearchdata,
                         Evalues *evalues,
                         BOOL domatchbuffering);

Sint findmaximaluniquematches(Virtualtree *virtualtree,
                              /*@unused@*/ Uint numberofprocessors,
                              Uint searchlength,
                              /*@unused@*/ void *repeatgapspec,
                              void *outinfo,
                              Outputfunction output);

Sint findquerymatches(Virtualtree *virtualtree,
                      Uint onlinequerynumoffset,
                      Queryinfo *queryinfo,
                      BOOL domaximaluniquematch,
                      BOOL domaximaluniquematchcandidates,
                      BOOL rcmode,
                      Matchparam *matchparam,
                      Bestflag bestflag,
                      Uint shownoevalue,
                      Uint showselfpalindromic,
                      SelectBundle *selectbundle,
                      void *procmultiseq,
                      Currentdirection currentdirection,
                      BOOL revmposorder,
                      Processfinalfunction processfinal,
                      Evalues *evalues,
                      BOOL domatchbuffering);

Sint findselfmatches(Sint mode,
                     Virtualtree *virtualtree,
                     Matchparam *matchparam,
                     Bestflag bestflag,
                     Uint shownoevalue,
                     Uint showselfpalindromic,
                     SelectBundle *selectbundle,
                     void *procmultiseq,
                     Currentdirection currentdirection,
                     Processfinalfunction processfinal,
                     Evalues *evalues,
                     BOOL domatchbuffering);

Sint findsupermax(Virtualtree *virtualtree,
                  /*@unused@*/ Uint numberofprocessors,
                  Uint searchlength,
                  /*@unused@*/ void *repeatgapspecinfo,
                  void *outinfo,
                  Outputfunction output);

Sint findtandems(Virtualtree *virtualtree,
                 Matchparam *matchparam,
                 Bestflag bestflag,
                 Uint shownoevalue,
                 Uint showselfpalindromic,
                 SelectBundle *selectbundle,
                 void *procmultiseq,
                 Processfinalfunction processfinal,
                 Evalues *evalues,
                 BOOL domatchbuffering);

#ifdef __cplusplus
}
#endif
