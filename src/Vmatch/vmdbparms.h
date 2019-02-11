
//\Ignore{

#ifndef VMDBPARMS_H
#define VMDBPARMS_H

#include <stdio.h>
#include "types.h"

//}


#define DBOPTIONNAME "-dbms"
#define SQLOPTIONNAME "-sql"


/*
  The following type aggregates information parsed from the
  argument vector, needed to connect to a database
*/

typedef struct
{
  BOOL usedatabase;       // database shall be used
  char *sharedlibname,    // contains functions to manipulate database
       *username,         // name of user to access database
       *password,         // password of user to access database
       *hostname,         // name of host where dbms resides
       *hostaddress,      // tcpip address of host
       *portnumber,       // portnumber to use for host
       *extra1,           // no fixed meaning
       *extra2,           // no fixed meaning
       *databasename;     // name of database
  char *sql;              // contains a SQL statement
} Databaseparms;          // \Typedef{Databaseparms}


//\Ignore{

#endif

//}
