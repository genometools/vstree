#ifndef BOUNDEDGAPS_H
#define BOUNDEDGAPS_H

#include "types.h"

/*
  type for user functions to specify gap interval bounds
*/

typedef Sint(*Boundfunction)(Uint, void*); // \Typedef{Boundfunction}

typedef struct
{
  BOOL lowergapdefined,         // is lowergaplength defined?
       uppergapdefined;         // is uppergaplength defined?
  Boundfunction lowerboundfun, // user specified lower bound function or NULL
                upperboundfun; // user specified upper bound function or NULL
  void *lowerinfo,    // info record for lower bound function
       *upperinfo;    // info record for upper bound function
  Sint lowergaplength,          // lower length of a gap between rep instances
       uppergaplength;          // upper length of a gap between rep instances
  double skiplistprobability;
  Uint skiplistmaxsize;
} Repeatgapspec;                // \Typedef{Repeatgapspec} 

#endif
