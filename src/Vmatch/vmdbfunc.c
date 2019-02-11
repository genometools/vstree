

/*
  This file contains a function needed to initialise the database-API
*/


#include <stdio.h>
#include <stdlib.h>
#include "dlopen.h"
#include "debugdef.h"
#include "spacedef.h"
#include "vmdbtypes.h"


/*
  The macro \texttt{GETDLFUNC} retrieves a pointer to function in a
  DL-Libraray and stores it in a function bundle record.
  If the function can't be found a negative error code will be returned.
*/

#define GETDLFUNC(R,T,N)\
        (R) = (T) dlsym(handle,#N);\
        if ((R) == NULL)\
        {\
          ERROR2("dbms function \"%s\" not found in shared library \"%s\"",\
                       #N,filename);\
          return (Sint) -3;\
        }

/*EE
  Function \texttt{openDbmsBundle} finds pointers to functions
  in a shared object, which communicates directly with a particular
  dbms like MySQL or PostgreSQL.
*/

#ifdef VMATCHDB
Sint openDbmsBundle(char *filename, DbmsBundle *bundle)
{
  void *handle;

  if (filename == NULL)
  {
    ERROR0("error opening shared object for database access. "
           "No filename given.");
    return (Sint) -1;
  }

  /* 
    acquiring handle to shared object
  */
  handle = dlopen(filename,RTLD_LAZY);
  if (handle == NULL)
  {
    ERROR1("error opening shared object for database access \"%s\"",filename);
    return (Sint) -2;
  }

  /* 
    finding addresses of needed dbms-related functions
  */
  GETDLFUNC(bundle->connect,DbmsConnect,dbmsConnect);
  GETDLFUNC(bundle->disconnect,DbmsDisconnect,dbmsDisconnect);
  GETDLFUNC(bundle->checkspaceleak,DbmsCheckspaceleak,dbmsCheckspaceleak);
  GETDLFUNC(bundle->lasterror,DbmsLasterror,dbmsLasterror);
  GETDLFUNC(bundle->execquery,DbmsExecquery,dbmsExecquery);
  GETDLFUNC(bundle->alloccharspace,DbmsAllocCharspace,dbmsAllocCharspace);

  GETDLFUNC(bundle->runbundle.initrec,DbmsInitRun,dbmsInitRun);
  GETDLFUNC(bundle->runbundle.initrecs,DbmsInitRuns,dbmsInitRuns);
  GETDLFUNC(bundle->runbundle.freerec,DbmsFreeRun,dbmsFreeRun);
  GETDLFUNC(bundle->runbundle.freerecs,DbmsFreeRuns,dbmsFreeRuns);
  GETDLFUNC(bundle->runbundle.saverec,DbmsSaveRun,dbmsSaveRun);
  GETDLFUNC(bundle->runbundle.countrecs,DbmsCountRuns,dbmsCountRuns);
  GETDLFUNC(bundle->runbundle.loadrecs,DbmsLoadRuns,dbmsLoadRuns);
  GETDLFUNC(bundle->runbundle.loadrec,DbmsLoadRun,dbmsLoadRun);
  GETDLFUNC(bundle->runbundle.fprintrec,DbmsFprintRun,dbmsFprintRun);
  GETDLFUNC(bundle->runbundle.markcompleted,
           DbmsMarkRunCompleted,dbmsMarkRunCompleted);
  GETDLFUNC(bundle->runbundle.deleterec,DbmsDeleteRun,dbmsDeleteRun);

  GETDLFUNC(bundle->seqbundle.initrec,DbmsInitSeq,dbmsInitSeq);
  GETDLFUNC(bundle->seqbundle.initrecs,DbmsInitSeqs,dbmsInitSeqs);
  GETDLFUNC(bundle->seqbundle.freerec,DbmsFreeSeq,dbmsFreeSeq);
  GETDLFUNC(bundle->seqbundle.freerecs,DbmsFreeSeqs,dbmsFreeSeqs);
  GETDLFUNC(bundle->seqbundle.saverec,DbmsSaveSeq,dbmsSaveSeq);
  GETDLFUNC(bundle->seqbundle.countrecs,DbmsCountSeqs,dbmsCountSeqs);
  GETDLFUNC(bundle->seqbundle.loadrecs,DbmsLoadSeqs,dbmsLoadSeqs);
  GETDLFUNC(bundle->seqbundle.loadrec,DbmsLoadSeq,dbmsLoadSeq);
  GETDLFUNC(bundle->seqbundle.fprintrec,DbmsFprintSeq,dbmsFprintSeq);
  GETDLFUNC(bundle->seqbundle.getrecid,DbmsFindSeqid,dbmsFindSeqid);

  GETDLFUNC(bundle->seqfilebundle.initrec,DbmsInitSeqfile,dbmsInitSeqfile);
  GETDLFUNC(bundle->seqfilebundle.initrecs,DbmsInitSeqfiles,dbmsInitSeqfiles);
  GETDLFUNC(bundle->seqfilebundle.freerec,DbmsFreeSeqfile,dbmsFreeSeqfile);
  GETDLFUNC(bundle->seqfilebundle.freerecs,DbmsFreeSeqfiles,dbmsFreeSeqfiles);
  GETDLFUNC(bundle->seqfilebundle.saverec,DbmsSaveSeqfile,dbmsSaveSeqfile);
  GETDLFUNC(bundle->seqfilebundle.countrecs,
            DbmsCountSeqfiles,dbmsCountSeqfiles);
  GETDLFUNC(bundle->seqfilebundle.loadrecs,
            DbmsLoadSeqfiles,dbmsLoadSeqfiles);
  GETDLFUNC(bundle->seqfilebundle.loadrec,
            DbmsLoadSeqfile,dbmsLoadSeqfile);
  GETDLFUNC(bundle->seqfilebundle.fprintrec,
            DbmsFprintSeqfile,dbmsFprintSeqfile);
  GETDLFUNC(bundle->seqfilebundle.getrecid,
            DbmsFindSeqfileid,dbmsFindSeqfileid);

  GETDLFUNC(bundle->seqfilerolebundle.initrecs,
            DbmsInitSeqfileroles,dbmsInitSeqfileroles);
  GETDLFUNC(bundle->seqfilerolebundle.initrec,
            DbmsInitSeqfilerole,dbmsInitSeqfilerole);
  GETDLFUNC(bundle->seqfilerolebundle.freerec,
            DbmsFreeSeqfilerole,dbmsFreeSeqfilerole);
  GETDLFUNC(bundle->seqfilerolebundle.freerecs,
            DbmsFreeSeqfileroles,dbmsFreeSeqfileroles);
  GETDLFUNC(bundle->seqfilerolebundle.saverec,
            DbmsSaveSeqfilerole,dbmsSaveSeqfilerole);
  GETDLFUNC(bundle->seqfilerolebundle.countrecs,
            DbmsCountSeqfileroles,dbmsCountSeqfileroles);
  GETDLFUNC(bundle->seqfilerolebundle.loadrecs,
            DbmsLoadSeqfileroles,dbmsLoadSeqfileroles);
  GETDLFUNC(bundle->seqfilerolebundle.fprintrec,
            DbmsFprintSeqfilerole,dbmsFprintSeqfilerole);

  GETDLFUNC(bundle->matchbundle.initrec,DbmsInitMatch,dbmsInitMatch);
  GETDLFUNC(bundle->matchbundle.initrecs,DbmsInitMatches,dbmsInitMatches);
  GETDLFUNC(bundle->matchbundle.freerec,DbmsFreeMatch,dbmsFreeMatch);
  GETDLFUNC(bundle->matchbundle.freerecs,DbmsFreeMatches,dbmsFreeMatches);
  GETDLFUNC(bundle->matchbundle.saverec,DbmsSaveMatch,dbmsSaveMatch);
  GETDLFUNC(bundle->matchbundle.countrecs,DbmsCountMatches,dbmsCountMatches);
  GETDLFUNC(bundle->matchbundle.loadrecs,DbmsLoadMatches,dbmsLoadMatches);
  GETDLFUNC(bundle->matchbundle.loadrec,DbmsLoadMatch,dbmsLoadMatch);
  GETDLFUNC(bundle->matchbundle.fprintrec,DbmsFprintMatch,dbmsFprintMatch);

  return 0;
}
#endif
