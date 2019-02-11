
//\Ignore{

#ifndef BESTINFO_H
#define BESTINFO_H

//}

/*
  The following describes the possible modes for computing matches according
  to the fixed-error-number extension strategt. It either means to show
  all maximal matches, to show all best matches,
  or to show the best \emph{bestnumber} of matches. In the latter case,
  the number \emph{bestnumber} is stored in a structure of type
  \texttt{Bestinfo}.
*/

typedef enum
{
  Allmaximalmatches,
  Allbestmatches,
  Fixednumberofbest
} Bestflag;                            // \Typedef{Bestflag}

typedef struct
{
  Bestflag bestflag;
  Uint bestnumber;
} Bestinfo;                            // \Typedef{Bestinfo}

//\Ignore{

#endif

//}
