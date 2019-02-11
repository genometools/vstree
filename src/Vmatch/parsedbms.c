
/*
  This file contains all code needed to handle arguments given to a
  program like vmatch or vmatchselect needed to make a connection to a
  dbms.
*/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "errordef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "arraydef.h"
#include "fhandledef.h"
#include "optdesc.h"
#include "vmdbparms.h"

#include "dstrdup.pr"
#include "splitargs.pr"
#include "filehandle.pr"

#define MAXNUMOFDBMSARGSINFILE 12

/*EE
  The following function \texttt{initdbmsparms} initializes the given database
  parameter record, that means, that all pointers are set to \texttt{NULL} and
  that \texttt{usedatabase} is set to \texttt{False}
*/

void initdbmsparms (Databaseparms *dbparms)
{
  dbparms->usedatabase = False;
  dbparms->sharedlibname = NULL;
  dbparms->username = NULL;
  dbparms->password = NULL;
  dbparms->hostname = NULL;
  dbparms->hostaddress = NULL;
  dbparms->portnumber = NULL;
  dbparms->extra1 = NULL;
  dbparms->extra2 = NULL;
  dbparms->databasename = NULL;
  dbparms->sql = NULL;
}

/*EE
  The following function \texttt{freedbmsparms} frees the char space
  pointed to by the members of the database parameter record
*/

void freedbmsparms (Databaseparms *dbparms)
{
  FREESPACE(dbparms->sharedlibname);
  FREESPACE(dbparms->username);
  FREESPACE(dbparms->password);
  FREESPACE(dbparms->hostname);
  FREESPACE(dbparms->hostaddress);
  FREESPACE(dbparms->portnumber);
  FREESPACE(dbparms->extra1);
  FREESPACE(dbparms->extra2);
  FREESPACE(dbparms->databasename);
  FREESPACE(dbparms->sql);
}

/*
  The following macro \texttt{GETDBOPTIONIFGIVEN} parses a database
  parameter argstring in the form of argname=arg. If the left side of the
  equatation symbol fits parameter \texttt{S} then the right side will
  be stored in the database parameter record by dynamic string duplication
*/

// Remark: Possibly function getPass needs a replacement.

#define GETDBOPTIONIFGIVEN(M,S)\
        if ((!optiontaken) && strncmp(arg,S,(size_t) ((eqpos-1)-arg))==0)\
          {\
            if (M != NULL)\
            {\
              ERROR2("argument \"%s\" to option %s was given twice",\
                     S,DBOPTIONNAME);\
              return (Sint) -2;\
            }\
            if (strncmp(arg,"password",(size_t) ((eqpos-1)-arg))==0 &&\
                strlen(eqpos+1)==0)\
            {\
              ASSIGNDYNAMICSTRDUP(M,\
                                  getpass("Enter password for database access: "));\
            } else\
            {\
              ASSIGNDYNAMICSTRDUP(M,eqpos+1);\
            }\
            optiontaken = True;\
          }

/*
  The following function \texttt{parsedbmsoptarg} parses a database
  parameter argstring in the form of argname=arg. It is not expected to
  be an argument beginning with "optfile=" 
  If \texttt{arg} does not fit any of the option names tested here, than
  the function will return an error code.
*/

#ifdef VMATCHDB
static Sint parsebasicdbmsoptarg(Databaseparms *dbparms, char *arg)
{
  char *eqpos;
  BOOL optiontaken = False;
  eqpos = strchr(arg,'=');
  if (eqpos != NULL)
  {
    GETDBOPTIONIFGIVEN(dbparms->username,"user");
    GETDBOPTIONIFGIVEN(dbparms->password,"password");
    GETDBOPTIONIFGIVEN(dbparms->hostname,"host");
    GETDBOPTIONIFGIVEN(dbparms->hostaddress,"hostaddr");
    GETDBOPTIONIFGIVEN(dbparms->portnumber,"port");
    GETDBOPTIONIFGIVEN(dbparms->extra1,"extra1");
    GETDBOPTIONIFGIVEN(dbparms->extra2,"extra2");
    GETDBOPTIONIFGIVEN(dbparms->databasename,"dbname");
  }
  if (!optiontaken)
  {
    ERROR2("argument \"%s\" to option %s is invalid",arg,DBOPTIONNAME);
    return (Sint) -1;
  }
  return 0;
}
#endif


/*
  The following function \texttt{parsedbmsoptarg} parses a database
  parameter argstring in the form of argname=arg. If an argument is
  found like "optfile=arg", arg is expected to be a name
  of a file which contains valid arguments for the database option.
*/

#ifdef VMATCHDB
static Sint parsedbmsoptarg(Databaseparms *dbparms, char *arg)
{
  char *eqpos,*dboptfile;
  // Remark: program crashes if using only MAXNUMOFDBMSARGSINFILE+1
  char *vargv[MAXNUMOFDBMSARGSINFILE+2];
  Argctype vargc;
  FILE *dboptfp;
  Fgetcreturntype inputchar;
  Arraychar argbuffer;
  Sint i;

  eqpos = strchr(arg,'=');
  /*
    giving an error if the arg has no equatation sign
  */
  if (eqpos == NULL)
  {
    ERROR2("argument \"%s\" to option %s is invalid",arg,DBOPTIONNAME);
    return (Sint) -1;
  }
  /*
    parsing it normally, as long as no file with further
    args is specified
  */
  if (strncmp(arg,"optfile",(size_t) ((eqpos-1)-arg)) != 0)
  {
    return parsebasicdbmsoptarg(dbparms, arg);
  }

  /*
    a file with further args is specified. So a complete file must be parsed
  */
  dboptfile = eqpos+1;
  dboptfp = CREATEFILEHANDLE(dboptfile,"r");
  if(dboptfp == NULL)
  {
    return (Sint) -2;
  }
  INITARRAY(&argbuffer,char);
  /*
    reading complete contents of the file into a buffer
  */
  while((inputchar = fgetc(dboptfp)) != EOF)
  {
    STOREINARRAY(&argbuffer,char,128,(char) inputchar);
  }
  if(DELETEFILEHANDLE(dboptfp) != 0)
  {
    FREEARRAY(&argbuffer,char);
    return (Sint) -2;
  }
  vargv[0] = "";
  if(splitargstring(argbuffer.spacechar,argbuffer.nextfreechar,
                    (Uint) MAXNUMOFDBMSARGSINFILE, &vargc,
                    &vargv[1]) != 0)
  {
    ERROR1("cannot analyze dbms option file \"%s\"",dboptfile);
    FREEARRAY(&argbuffer,char);
    return (Sint) -2;
  }
  /*
    now iterating all args of this file
  */
  for (i=(Sint) 1;i<(Sint) vargc;i++)
  {
    /*
      chaining option files is not supported
    */
    if (strncmp(vargv[i],"optfile=",(size_t) 8)==0)
    {
      ERROR2("argument \"%s\" to option %s cannot be used in an option file"
             ,"optfile",DBOPTIONNAME);
      FREEARRAY(&argbuffer,char);
      return (Sint) -1;
    }
    if (parsebasicdbmsoptarg(dbparms,vargv[i]) != 0)
    {
      FREEARRAY(&argbuffer,char);
      return (Sint) -1;
    }
  }
  FREEARRAY(&argbuffer,char);
  return 0;
}
#endif


/*EE
  The following function \texttt{parsesqloptarg} expects a pointer to a
  name of a file which contains a SQL statement.
  It will store the contents of the file in \texttt{dbparms->sql}.
  If an error occurs a negative error code is given back.
*/

Sint parsesqloptarg(Databaseparms *dbparms, char *sqlfilename)
{
  FILE *sqlfilep;
  Fgetcreturntype inputchar;
  Arraychar argbuffer;

  sqlfilep = CREATEFILEHANDLE(sqlfilename,"r");
  if(sqlfilep == NULL)
  {
    return (Sint) -2;
  }
  INITARRAY(&argbuffer,char);
  /*
    reading complete contents of the file into a buffer
  */
  while((inputchar = fgetc(sqlfilep)) != EOF)
  {
    STOREINARRAY(&argbuffer,char,128,(char) inputchar);
  }
  if(DELETEFILEHANDLE(sqlfilep) != 0)
  {
    FREEARRAY(&argbuffer,char);
    return (Sint) -2;
  }
  if (argbuffer.nextfreechar == 0)
  {
    ERROR1("sql file \"%s\" is empty",sqlfilename);
    FREEARRAY(&argbuffer,char);
    return (Sint) -3;
  }
  STOREINARRAY(&argbuffer,char,128,(char) '\0');
  ASSIGNDYNAMICSTRDUP(dbparms->sql,argbuffer.spacechar);
  FREEARRAY(&argbuffer,char);
  return 0;
}


/*EE
  The following function texttt{parsedbmsopt} parses all arguments
  of option -dbms. Parameter texttt{numprgargs} determines the number
  of arguments an application expects to be given.
*/
#ifdef VMATCHDB
Sint parsedbmsopt(Databaseparms *dbparms, Argctype argc,char **argv,
                           Uint *argnum,Uint numprgargs)
{
  (*argnum)++;
  dbparms->usedatabase = True;

  if((*argnum) + numprgargs > (Uint) (argc-1) || ISOPTION(argv[(*argnum)]))
  {
    ERROR1("missing argument to option %s",DBOPTIONNAME);
  }
  ASSIGNDYNAMICSTRDUP(dbparms->sharedlibname,argv[(*argnum)]);

  while((((*argnum) + numprgargs) < (Uint) (argc - 1)
               && !ISOPTION(argv[(*argnum)+1])))
  {
    (*argnum)++;
    if(parsedbmsoptarg(dbparms,argv[(*argnum)]) !=  0)
    {
      freedbmsparms (dbparms);
      return (Sint) -1;
    }
  }

  return 0;
}
#endif

