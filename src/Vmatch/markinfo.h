
//\Ignore{

#ifndef MARKINFO_H
#define MARKINFO_H

#include "types.h"

//}

#define KEEPFLAGSHELPSIZE 1024   // size of space reservoir for help message 

typedef struct
{
  BOOL markdb,             // if true, then mark the database
                           // if false, then mark the querysequence
       markleft,           // mark the left instance of a match
       markright,          // mark the right instance of a match
       markleftifdifferentsequence, // mark left instance of match if
                                    // instances are in different sequences
       markrightifdifferentsequence; // mark right instance of match if
                                    // instances are in different sequences
} Markfields;

/*
  The following structure is used for storing information 
  when computing non-matching parts in the database or querysequence.
*/

typedef struct
{
  Uint nomatchlength;   // the minimal length of the non-matching part
  BOOL donomatch;       // output non-matching parts
  Markfields markfields;
} Nomatch;              // \Typedef{Nomatch}

/*
  The following structure is used for storing information
  when masking matches in the database or in the querysequence.
*/

typedef struct
{
  Uchar maskchar;       // the character to be used for marking,
                        // or MASKTOUPPER or MASKTOLOWER
  BOOL domaskmatch;     // do the masking
  Markfields markfields;
} Maskmatch;            // \Typedef{Maskmatch}

/*
  The following structure stores information used when marking matches.
*/

typedef struct
{
  BOOL hasnoqueryfiles;        // number of queryfiles is 0
  Uint *markmatchtable;        // table storing the bits to mark
  Markfields markfields;
} Markinfo;                    // \Typedef{Markinfo}

//\Ignore{

#endif

//}
