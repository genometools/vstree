#ifndef INPUTDEF_H
#define INPUTDEF_H

#include <stdio.h>
#include "types.h"
#include "virtualdef.h"

#define ALLDEMANDTABS (LCPTAB | SUFTAB | SKPTAB | BWTTAB | BCKTAB |\
                       TISTAB | OISTAB | STITAB | STI1TAB | ISOTAB | CLDTAB)

#define SOMETHINGOUT(I) ((I)->demand & ALLDEMANDTABS)

typedef struct
{
  BOOL storeonfile,            // True iff index is to be stored on file
       rev,                    // compute virtual tree for reverse sequence
       cpl,                    // compute virtual tree for complement sequence
       rcm;                    // compute virtual tree for reverse complement
  Uint transnum,               // if > 0, compute virtual tree for 6 frame trans
       demand,                 // what tables are to be constructed
       numofchars;             // alpha.mapsize-1 if dna, protein,
                               // or symbolmapfile, otherwise alpha.mapsize
  char indexname[PATH_MAX+4+1]; // name of index
  Inputalpha inputalpha;       // name of symbolmapfile or dna/protein-flag
  Showverbose showverbose;     // function to show whats going on
  DefinedUint maxdepth;
} Input;

/*
  indexname and inputalpha are only used where storeonfile is True.
*/

#endif
