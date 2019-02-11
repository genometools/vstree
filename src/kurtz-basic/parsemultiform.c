//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <ctype.h>
#include <string.h>
#include "types.h"
#include "inputsymbol.h"
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "multidef.h"
#include "alphadef.h"

#include "readgzip.pr"

#ifdef DEBUG
void checkfilesep(Uint numoffiles,Uint *filesep,Uchar *sequence)
{
  Uint i;

  for(i=0; i<numoffiles-1; i++)
  {
    if(filesep[i] == UNDEFFILESEP ||
       sequence[filesep[i]] != FASTASEPARATOR)
    {
      fprintf(stderr,"filesep[%lu]=%lu incorrect\n",
              (Showuint) i,
              (Showuint) filesep[i]);
      exit(EXIT_FAILURE);
    }
  }
}
#endif

//}

/*EE
  This file implements some functions for reading in multiple sequences
  in different formats, namely \textsf{Fasta}, \textsf{Embl}, 
  \textsf{Swissprot}, and \textsf{Genbank}.
*/

/*
  The  function \texttt{memstring} scans the first \texttt{textlen}
  bytes of the memory area pointed to by \texttt{text} for the string pattern 
  of length \texttt{plen}. The first byte where pattern occurs stops the 
  operation. The \texttt{memstring} function returns a pointer to the matching 
  byte or \texttt{NULL} if the pattern does not occur in the given memory area.
  We use the Boyer-Moore Horspool algorithm to find the occurrence of 
  \texttt{pattern} in \texttt{text}.
*/

/*@null@*/ static Uchar *memstring(Uchar *text,Uint textlen,
                                   Uchar *pattern,Uint plen)
{
  Uchar *tptr;
  Uint i, ppos, rmostocc[UCHAR_MAX+1];

  if(plen == 0 || plen > textlen)
  {
    return NULL;
  }
  for(i=0; i<=UCHAR_MAX; i++)
  {
    rmostocc[i] = plen;
  }
  for(ppos=0; ppos<plen-1; ppos++)
  {
    rmostocc[(Uint) pattern[ppos]] = plen - ppos - 1;
  }
  for(tptr = text; tptr <= text + textlen - plen; 
      tptr += rmostocc[(Uint) tptr[plen-1]])
  {
    i=plen-1;
    while(True)
    {
      if(pattern[i] != tptr[i])
      {
        break;
      }
      if(i==0)
      {
        return tptr;
      }
      i--;
    }
  } 
  return NULL;
}

/*
  The following function copies the description from \texttt{sptr} to 
  \texttt{origseq} beginning with the symbol \texttt{>}. So the 
  description will be in Fasta format.
*/

static Uint copyDEfield(Uchar *writespace,Uchar *sptr)
{
  Uint i;

  for(i=0; sptr[i] != (Uchar) '\n'; i++)
  {
    writespace[i] = sptr[i];
  }
  writespace[i] = (Uchar) '\n';
  return i+1; // return sequence length
}

/*
  The following functions copies the sequence pointed to by
  \texttt{*s} into the memory area pointed to by \texttt{origstring}.
  The copying stops as soon as the char \texttt{endsymbol} has been 
  detected. Finally \texttt{*s} is updated to point to the area following
  the scanned part.
*/

static Sint copysequence(Uchar *origstring,Uchar **s,Uchar endsymbol)
{
  Uchar *sptr, *optr;

  for(optr = origstring, sptr = *s ; *sptr != endsymbol; sptr++)
  {
    if(*sptr != ' ' && *sptr != '\n' && !isdigit(*sptr))
    {
      *optr++ = *sptr;
    }
  }
  *optr++ = (Uchar) '\n';
  sptr++;
  if(*sptr != endsymbol)
  {
    ERROR1("input sequence does no contain second '%c'",endsymbol);
    return (Sint) -1;
  }
  while(*sptr != (Uchar) '\n')
  {
    sptr++;
  }
  *s = sptr+1;
  return (Sint) (optr - origstring);
}

/*
  The following function reads the first line of the \texttt{database}.
  If the line ends with the string \texttt{bp.}, then \textsf{EMBL} DNA 
  sequence format is assumed.
  If the line ends with the string \texttt{aa.}, then \textsf{SWISSPROT}
  protein sequence format is assumed. If none of these two cases apply, the
  function returns a negative value.
*/

static Sint getseqtype(Uchar *database,Uint dblen)
{
  Uchar *seqptr;
 
  for(seqptr = database; 
      seqptr < database + dblen && *seqptr != (Uchar) '\n'; seqptr++)
    /* Nothing */ ;
  seqptr -= 3;
  if(tolower(seqptr[0]) == 'b' && tolower(seqptr[1]) == 'p' && seqptr[2] == '.')
  {
    return 0; // EMBL dna sequence
  }
  if(tolower(seqptr[0]) == 'a' && tolower(seqptr[1]) == 'a' && seqptr[2] == '.')
  {
    return (Sint) 1; // SWISSPROT protein sequence
  }
  ERROR0("cannot detect sequence type:  "
         "first line must end with \"aa.\" or \"bp.\" in lower or upper case");
  return (Sint) -1;
}

static Uchar *getseqIDptr(Uchar *database,Uint offset,Uint dblen)
{
  Uchar *seqptr;
 
  for(seqptr = database + offset; 
      seqptr < database + dblen && isspace(*seqptr); 
      seqptr++)
  {
    /* Nothing */ ;
  }
  return seqptr;
}

static Uint copyIDfield(Uchar *writespace,Uchar *sptr)
{
  Uint i;

  writespace[0] = (Uchar) FASTASEPARATOR;
  for(i=0; !isspace(sptr[i]); i++)
  {
    writespace[i+1] = sptr[i];
  }
  writespace[i+1] = (Uchar) ' ';
  return i+2;
}

/*
  The following function is a generic function to scan a database 
  in the different format. \texttt{database} points to a memory area of
  length \texttt{dblen}. If the argument \texttt{checktype} is true,
  then the function tries to find out the sequence type. 
  \texttt{writespace} points to a memory area where the resulting
  sequence is to be stored.  The number of characters parsed is
  stored in the variable \texttt{parsed}. The strings 
  \texttt{firststring}, \texttt{secondstring} and \texttt{thirdstring}
  store the key words to look for in the database string
  to find the particular regions.
*/

static Sint parsegenericdatabase(BOOL checktype,
                                 Uchar *database,
                                 Uint dblen,
                                 Uchar *writespace,
                                 Uint *parsed,
                                 BOOL *dnatype,
                                 char *firststring,
                                 char *secondstring,
                                 char *thirdstring)
{
  Uchar *sptr, *idptr;
  Uint written;
  Sint retcode,
      firstlen = (Sint) strlen(firststring),
      secondlen = (Sint) strlen(secondstring),
      thirdlen = (Sint) strlen(thirdstring);

  *dnatype = True;
  // check if database is of appropriate length and contains ID or LOCUS
  if(dblen >= (Uint) firstlen &&
     memcmp(database,(Uchar *) firststring,(size_t) firstlen) == 0)
  {
    if(checktype)
    {
      retcode = getseqtype(database,dblen);
      if(retcode < 0)
      {
        return (Sint) -1;
      }
      *dnatype = (retcode == 0) ? True : False;
    }
    idptr = getseqIDptr(database,(Uint) firstlen,dblen);
    // find occurrence of DE or DEFINITION
    sptr = memstring(database,dblen,(Uchar *) secondstring,(Uint) secondlen);
    if(sptr == NULL)
    {
      ERROR1("missing \"%s\" in database file",secondstring);
      return (Sint) -2;
    }
    sptr += secondlen;    // skip key
    while(sptr < database + dblen && *sptr == ' ')   // skip blanks
    {
      sptr++;
    }
    written = copyIDfield(writespace,idptr);
    written += copyDEfield(writespace+written,sptr);
    // find occurrence of SQ or ORIGIN
    sptr = memstring(sptr,dblen - (Uint) (sptr - database),
                       (Uchar *) thirdstring,(Uint) thirdlen);
    if(sptr == NULL)
    {
      ERROR1("missing \"%s\" in GenBank file",thirdstring);
      return (Sint) -3;
    }
    sptr += thirdlen;  // skip SQ or ORIGIN 
    while(sptr < database + dblen && *sptr != '\n')  // and the rest of the line
    {
      sptr++;
    }
    retcode = copysequence(writespace+written,&sptr,(Uchar) '/');
    if(retcode < 0)
    {
      return (Sint) -4;
    }
    while(sptr < database + dblen && isspace((Ctypeargumenttype) *sptr))
    {
      sptr++;
    }
    written += (Uint) retcode;
    *parsed = (Uint) (sptr - database);
    return (Sint) written;
  }
  return 0;
}

/*
  The following function iterates the previous function
  over the database, until everything is parsed.
*/

static Sint iterparsegenericdatabase(BOOL checktype,
                                     Uchar *database,
                                     Uint dblen,
                                     BOOL *dnatype,
                                     char *firststring,
                                     char *secondstring,
                                     char *thirdstring)
{
  Uint parsed, dbstart = 0, totallength = 0;
  Sint retcode;

  while(True)
  {
    if(dblen <= dbstart)
    {
      return (Sint) totallength;
    }
    retcode = parsegenericdatabase(checktype,
                                   database + dbstart,dblen - dbstart,
                                   database + totallength,
                                   &parsed,dnatype,firststring,secondstring,
                                   thirdstring);
    if(retcode <= 0)
    {
      return retcode;
    }
    dbstart += parsed;
    totallength += retcode;
  }
  /*@notreached@*/ return retcode;
}

/*
  The following function parses \textsf{GENBANK} format.
*/

static Sint parseGENBANK(BOOL *dnatype,Uchar *database,Uint dblen)
{
  return iterparsegenericdatabase(False,database,dblen,dnatype,
                                  "LOCUS","\nDEFINITION","\nORIGIN");
}

/*
  The following function parses \textsf{EMBL} and \textsf{SWISSPROT} format.
*/

static Sint parseEMBLorSWISSPROT(BOOL *dnatype,Uchar *database,Uint dblen)
{
  return iterparsegenericdatabase(True,database,dblen,dnatype,
                                  "ID   ","\nDE   ","\nSQ   ");
}

/*EE
  The following function parses a \texttt{database} of length \texttt{dblen}
  in any of the three formats mentioned above.
*/

Sint parseMultiformat(BOOL *dnatype,Uchar *database,Uint dblen)
{
  Sint retcode;
  Uint skipprefix;

  for(skipprefix=0; isspace(database[skipprefix]); skipprefix++)
    /* Nothing */ ;
  if(database[skipprefix] == FASTASEPARATOR)
  {
    if(skipprefix > 0)
    {
      memmove((void *) database,
              (void *) (database+skipprefix),(size_t) (dblen-skipprefix));
    }
    retcode = (Sint) (dblen - skipprefix);  // fasta file
  } else
  {
    retcode = parseGENBANK(dnatype,database+skipprefix,dblen);
    if(retcode < 0) // error occurred
    {
      return (Sint) -1;
    }
    if(retcode == 0)  // format not recognized
    {
      retcode = parseEMBLorSWISSPROT(dnatype,database,dblen);
      if(retcode < 0) // error occurred
      {
        return (Sint) -2;
      }
    } 
  }
  return retcode;
}

