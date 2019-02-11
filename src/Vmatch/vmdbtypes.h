
//\Ignore{

#ifndef VMDBTYPES_H
#define VMDBTYPES_H
#include <limits.h>
#include "types.h"
#include "arraydef.h"
#include "vmdbparms.h"
#include "evaluedef.h"

//}

/*
  taken from mparms.h, include/xdropdef.h
*/
#define UNDEFEVALUE ((Evaluetype) (3.40282347e+38F))
#define UNDEFSCORE 0
#define UNDEFDISTANCE 0


#define DEBUGPRINT(S) printf("%s",S);
#define DEBUGPRINTCR(S) printf("%s\n",S);

/*
  Maximum lengths for Strings used in records below.
*/
#define MAXSTRLENSYMBOLMAP 3*UCHAR_MAX //worst case: UCHAR maps to UCHAR
                                       //each seperated by linefeed
#define STRLENVERSION 10               //length of ISO-Date YYYY-MM-DD
#define STRLENMD5 32                   //a md5sum has 128 bit. Represented by
                                       //hex chars 32 chars are needed 

/*
  This is how boolean values are represented in database
*/
#define DBCHARYES 'Y'
#define DBCHARNO  'N'


/* 
  The following structures are used for comunication purposes with a
  particular database. No assumptions on the used database management
  system nor the database type system can be made here, therefore no
  type casting is made in this place. (But we assume a DBMS that uses
  some kind of records, e.g. a row in a table, that's why we use a
  structure).
*/


/* 
  This structure holds information about a run of vmatch. One run
  may yield 0 to many matches (see below).
*/

typedef struct
{
  Uint   runid;                         //unique identifier
  char   versiondate[STRLENVERSION+1],  //date of vmatch compiletime
         *indexname,                    //path to index used for run
	 *params,                       //params used for run
	 selfcomparison;                //is Run based on a self
	                                //comparison of the index?
  char completed;                       //could storing the run succeed?
} Runrecord; // \Typedef{Runrecord}


/* 
  This structure holds information about a file containing sequences.
*/

typedef struct
{
  Uint  seqfileid;                       //unique database identifier
  char  md5[STRLENMD5+1],                //checksum for identification
        *filename,                       //name of the sequence file
        symbolmap[MAXSTRLENSYMBOLMAP+1], //symbolmapping
        haswildcard;                     // Wildcard defined fpr symbolmapping?
  Uint  length;                          //length of file for identification
} Seqfilerecord;  // \Typedef{Seqfilerecord}



/* 
  This structure holds information about a sequence.
*/

typedef struct
{
  Uint  seqid,                  //unique database identifier
        seqfileid;              //unique database identifier for a seqfile
  char  md5[STRLENMD5+1],       //checksum for sequence identification
        *description;           //the description of the sequence
  Uint  length;                 //length of original sequence
  char  *sequence;              //pointer to original sequence (no term-char!)
} Seqrecord;  // \Typedef{Seqrecord}


/* 
  This structure holds information about the role a sequence file may
  take part in a run.
*/

typedef struct
{
  Uint   seqfileid,     //unique database identifier for a seqfile
         runid,         //unique database identifier for a run
	 filenum;       //file number
  char   isinquery;     //is sequence used as apart of a query?
} Seqfilerolerecord; // \Typedef{Seqfilerolerecord}

/* 
  This structure holds information about a match found by a run of vmatch.
  Data that all matches of a certain run have in common is stored in a
  structure of type Runrecord (see above).
*/

typedef struct
{
  Uint runid;        //unique identifier of a run, to which match belongs
  Sint matchid;      //the unique identifier of a match
  Uint length1;      //length of left match instance
  Sint position1,    //position in left match multi-seq space
       seqnum1;      //sequence number of left match instance
  Uint seqid1,       //id of sequence of left match instance
       seqfileid1;   //id of sequencefile of left match instance
  Sint relpos1,      //relative sequence position of left match instance
       filepos1;     //relative file position of left match instance
  Uint length2;      //length of right match instance
  Sint position2,    //position in right match multi-seq space
       seqnum2;      //sequence number of right match instance
  Uint seqid2,       //id of sequence of right match instance 
       seqfileid2;   //id of sequencefile of right match instance
  Sint relpos2,      //relative sequence position of right match instance
       filepos2;     //relative file position of right match instance
  double evalue;     //the E-value of the match
  Sint   distance,   //the distance, as defined for match
         score;      //derived attribute, see manual on vmatch
  double identity;   //derived attribute, see manual on vmatch
  Uint   xdropvalue; //maximal xdrop value
  char   ispalindromic,      //flag for palindromic match
         isselfpalindromic,  //flag for self palindromic match
         isscorematch;       //flag for match w.r.t. scoring
  char completed;            //could storing the run succeed?
} Matchrecord;   // \Typedef{Matchrecord}


/* 
  The following structures are used to hold an arbitrary amount of
  database records. The space for them will be usally allocated in the
  shared object, communicating with the dbms.
*/

DECLAREARRAYSTRUCT(Runrecord);
DECLAREARRAYSTRUCT(Seqfilerecord);
DECLAREARRAYSTRUCT(Seqrecord);
DECLAREARRAYSTRUCT(Seqfilerolerecord);
DECLAREARRAYSTRUCT(Matchrecord);


/* 
  The following declarations are needed to access the functions
  of the shared object, that communicates directly with a particular dbms,
  e.g. MySQL or PostgreSQL. These functions are mainly writing and
  reading the data structures from and to a database.
*/

typedef Sint (*DbmsConnect) (Databaseparms *);
typedef void (*DbmsDisconnect) (void);
typedef void (*DbmsCheckspaceleak) (void);
typedef Sint (*DbmsExecquery) (char *);
typedef char* (*DbmsLasterror)(void);
typedef Sint (*DbmsAllocCharspace) (char **, Uint);
typedef Sint (*DbmsSaveRun) (Runrecord *);
typedef Sint (*DbmsSaveSeqfile) (Seqfilerecord *);
typedef Sint (*DbmsSaveSeq) (Seqrecord *);
typedef Sint (*DbmsSaveSeqfilerole) (Seqfilerolerecord *);
typedef Sint (*DbmsSaveMatch) (Matchrecord *);
typedef Sint (*DbmsMarkRunCompleted) (Uint);
typedef Sint (*DbmsDeleteRun) (Uint);
typedef Sint (*DbmsFindSeqfileid) (Seqfilerecord *);
typedef Sint (*DbmsFindSeqid) (Seqrecord *);
typedef Sint (*DbmsCountRuns) (Runrecord *, Uint *);
typedef Sint (*DbmsCountMatches) (Matchrecord , Uint *);
typedef Sint (*DbmsCountSeqfiles) (Seqfilerecord *, Uint *);
typedef Sint (*DbmsCountSeqs) (Seqrecord *, Uint *);
typedef Sint (*DbmsCountSeqfileroles) (Seqfilerolerecord *, Uint *);
typedef Sint (*DbmsLoadRuns) (char *, Runrecord *, ArrayRunrecord *);
typedef Sint (*DbmsLoadMatches) (char *, Matchrecord *, ArrayMatchrecord *);
typedef Sint (*DbmsLoadSeqfiles)
              (char *, Seqfilerecord *, ArraySeqfilerecord *);
typedef Sint (*DbmsLoadSeqs) (char *, Seqrecord *, ArraySeqrecord *);
typedef Sint (*DbmsLoadSeqfileroles)
              (char *, Seqfilerolerecord *,ArraySeqfilerolerecord *);
typedef Sint (*DbmsLoadMatch) (Matchrecord *);
typedef Sint (*DbmsLoadRun) (Runrecord *);
typedef Sint (*DbmsLoadSeqfile) (Seqfilerecord *);
typedef Sint (*DbmsLoadSeq) (Seqrecord *);
typedef void (*DbmsInitRun) (Runrecord *);
typedef void (*DbmsInitSeq) (Seqrecord *);
typedef void (*DbmsInitSeqfile) (Seqfilerecord *);
typedef void (*DbmsInitMatch) (Matchrecord *);
typedef void (*DbmsInitSeqfilerole) (Seqfilerolerecord *);
typedef void (*DbmsInitRuns) (ArrayRunrecord *);
typedef void (*DbmsInitSeqs) (ArraySeqrecord *);
typedef void (*DbmsInitSeqfiles) (ArraySeqfilerecord *);
typedef void (*DbmsInitSeqfileroles) (ArraySeqfilerolerecord *);
typedef void (*DbmsInitMatches) (ArrayMatchrecord *);
typedef void (*DbmsFreeRun) (Runrecord *);
typedef void (*DbmsFreeSeq) (Seqrecord *);
typedef void (*DbmsFreeSeqfile) (Seqfilerecord *);
typedef void (*DbmsFreeMatch) ( Matchrecord *);
typedef void (*DbmsFreeSeqfilerole) ( Seqfilerolerecord *);
typedef void (*DbmsFreeRuns) (ArrayRunrecord *);
typedef void (*DbmsFreeSeqs) (ArraySeqrecord *);
typedef void (*DbmsFreeSeqfiles) (ArraySeqfilerecord *);
typedef void (*DbmsFreeMatches) (ArrayMatchrecord *);
typedef void (*DbmsFreeSeqfileroles) (ArraySeqfilerolerecord *);
typedef void (*DbmsFprintRun) (FILE *, Runrecord *);
typedef void (*DbmsFprintMatch) (FILE *, Matchrecord *);
typedef void (*DbmsFprintSeqfile) (FILE *, Seqfilerecord *);
typedef void (*DbmsFprintSeqfilerole) (FILE *, Seqfilerolerecord *);
typedef void (*DbmsFprintSeq) (FILE *, Seqrecord *);


typedef struct
{
  DbmsInitRun initrec;
  DbmsInitRuns initrecs;
  DbmsFreeRun freerec;
  DbmsFreeRuns freerecs;
  DbmsSaveRun saverec;
  DbmsCountRuns countrecs;
  DbmsLoadRuns loadrecs;
  DbmsLoadRun loadrec;
  DbmsFprintRun fprintrec;
  DbmsMarkRunCompleted markcompleted;
  DbmsDeleteRun deleterec;
} DbmsRunBundle;


typedef struct
{
  DbmsInitSeq initrec;
  DbmsInitSeqs initrecs;
  DbmsFreeSeq freerec;
  DbmsFreeSeqs freerecs;
  DbmsSaveSeq saverec;
  DbmsCountSeqs countrecs;
  DbmsLoadSeqs loadrecs;
  DbmsLoadSeq loadrec;
  DbmsFprintSeq fprintrec;
  DbmsFindSeqid getrecid;
} DbmsSeqBundle;


typedef struct
{
  DbmsInitSeqfile initrec;
  DbmsInitSeqfiles initrecs;
  DbmsFreeSeqfile freerec;
  DbmsFreeSeqfiles freerecs;
  DbmsSaveSeqfile saverec;
  DbmsCountSeqfiles countrecs;
  DbmsLoadSeqfiles loadrecs;
  DbmsLoadSeqfile loadrec;
  DbmsFprintSeqfile fprintrec;
  DbmsFindSeqfileid getrecid;
} DbmsSeqfileBundle;


typedef struct
{
  DbmsInitSeqfilerole initrec;
  DbmsInitSeqfileroles initrecs;
  DbmsFreeSeqfilerole freerec;
  DbmsFreeSeqfileroles freerecs;
  DbmsSaveSeqfilerole saverec;
  DbmsCountSeqfileroles countrecs;
  DbmsLoadSeqfileroles loadrecs;
  DbmsFprintSeqfilerole fprintrec;
} DbmsSeqfileroleBundle;


typedef struct
{
  DbmsInitMatch initrec;
  DbmsInitMatches initrecs;
  DbmsFreeMatch freerec;
  DbmsFreeMatches freerecs;
  DbmsSaveMatch saverec;
  DbmsCountMatches countrecs;
  DbmsLoadMatches loadrecs;
  DbmsLoadMatch loadrec;
  DbmsFprintMatch fprintrec;
} DbmsMatchBundle;


typedef struct
{
  DbmsConnect connect;
  DbmsDisconnect disconnect;
  DbmsCheckspaceleak checkspaceleak;
  DbmsLasterror lasterror;
  DbmsExecquery execquery;
  DbmsAllocCharspace alloccharspace;
  DbmsRunBundle runbundle;
  DbmsSeqBundle seqbundle;
  DbmsSeqfileBundle seqfilebundle;
  DbmsSeqfileroleBundle seqfilerolebundle;
  DbmsMatchBundle matchbundle;
} DbmsBundle;

//\Ignore{

#endif

//}


