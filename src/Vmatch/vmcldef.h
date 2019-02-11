
//\Ignore{

#ifndef VMCLDEF_H
#define VMCLDEF_H

#include <stdio.h>
#include "arraydef.h"
#include "clusterdef.h"
#include "virtualdef.h"
#include "outinfo.h"

//}

#define DBCLMAXSIZE 0

typedef struct 
{
  BOOL dbclusterdefined;
  Uint dbclminsize,    // minimum size a cluster must have to be output
       dbclmaxsize;    // maximum size a cluster must have to be output
  Uchar dbclpercsmall, // percentage of small string to be included in match
        dbclperclarge; // percentage of large string to be included in match
  char *dbclusterfilenameprefix, // prefix of outputfiles or NULL
       *nonredundantfile;        // prefix of file to output nonredundant match
} Clusterparms;

typedef struct
{
  BOOL firstelemincluster;
  ClusterSet clusterset;  // the cluster representation
  char *args,
       dbmatchfileoutname[VIRTUALFILENAMEMAX+1],
       dbfnafileoutname[VIRTUALFILENAMEMAX+1];
  FILE *dbfnafileoutptr,   // file pointer for fna file
       *dbmatchfileoutptr; // file pointer for match file
  ArrayStoreMatch matchtab;
  Clusterparms clusterparms;
  Showdescinfo defaultshowdesc;
  Multiseq *virtualmultiseq;
  Outinfo outinfo;
} Sequenceclusterinfo;

//\Ignore{

#endif

//}
