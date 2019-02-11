//\IgnoreLatex{

#ifndef VNODEDEF_H
#define VNODEDEF_H

#include "types.h"
#include "virtualdef.h"
#include "arraydef.h"

#define MAXCHILDNUM     100        // maximal number of children

//}

/*
  The following type implements a virtual node. It refers to certain intervals 
  of the table $\mathsf{suftab}$. Each of the suffixes in the boundaries
  \texttt{left} and \texttt{right} have a common prefix of length
  \texttt{offset}. The additional component rpref is only used when
  matching virtual trees against virtual trees.
*/

typedef struct
{
  Uint left, 
       right, 
       offset;
} Vnode;             // \Typedef{Vnode}

/*
  We sometimes need an array of virtual nodes
*/

DECLAREARRAYSTRUCT(Vnode);

typedef struct
{
  Uint left, 
       right, 
       offset,
       rpref;
} Vnoderpref;        // \Typedef{Vnoderpref}

/*
  We sometimes need an array of virtual nodes
*/

DECLAREARRAYSTRUCT(Vnoderpref);

/*
  The following type implements a virtual leaf. It refers to the suffix
  with number \texttt{suffixnum} and a minimal prefix length 
  \texttt{uniquelength} which makes the prefix unique.
*/

typedef struct
{
  Uint suffixnum,
       uniquelength;
} Vleaf;             // \Typedef{Vleaf}

/*
  The following type implements the left bound of an interval with
  an additional component for a character.
*/

typedef struct
{
  Uint bound;
  Uchar inchar;
} Vbound;            // \Typedef{Vbound}

DECLAREARRAYSTRUCT(Vbound);

/*
  The following type implement lists of children.
*/

typedef struct
{
  Uint numofsingletons,           // number of child-intervals [l,l]
       numofchildren;             // number of lcp child-intervals
  Vnode intervaltab[MAXCHILDNUM];  // Array of lcp child-intervals
} Childlist;

void findmaxprefixlen(Virtualtree *virtualtree,
                              Vnode *vnode,
                              Uchar *query,
                              Uint querylen,
                              PairUint *maxwit);

void findmaxprefixlenstack(Virtualtree *virtualtree,
                           Vnoderpref *vnode,
                           Uchar *query,
                           Uint querylen,
                           PairUint *maxwit,
                           ArrayVnoderpref *stack);

//\IgnoreLatex{

#endif

//}
