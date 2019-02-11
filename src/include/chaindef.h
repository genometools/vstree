//\Ignore{

#ifndef CHAINDEF_H
#define CHAINDEF_H

#include "types.h"
#include "arraydef.h"

//}

/*
  The following defines the type for the weight of fragment and
  for the scores of a chain.
*/

typedef Sint Chainscoretype;

/*
  The following type defines the possible kinds of chaining.
  The mode can be one of the two following values.
*/

typedef enum
{
  GLOBALCHAINING,              // global chaining without gap costs
  GLOBALCHAININGWITHGAPCOST, // global chaining with L1 gap costs
  GLOBALCHAININGWITHOVERLAPS,  // chaining with overlaps
  LOCALCHAININGMAX,            // local chaining; one maximum is reported
  LOCALCHAININGTHRESHOLD,      // local chaining; all chains >= minscore report.
  LOCALCHAININGBEST,           // local chaining; k best local chains reported
  LOCALCHAININGPERCENTAWAY     // local chaining; percent away from best score
} Chainkind;


/*
  A chain consists of an array of integers. These refer to the array of
  fragment informations.
*/

typedef struct
{
  ArrayUint chainedfragments;
  Chainscoretype scoreofchain;
} Chain;

/*
  We use functions of the following type to report chains.
*/

typedef Sint (*Chainprocessor)(void *,Chain *);

/*
  The basic information required for each fragment is stored
  in a structure of the following type. The user has to specify
  those components which a tagged `user defined'. The chaining
  algorithms computes the remaining components score and previous
  in chain.
*/

typedef struct
{
  Uint startpos[2],      // start of fragment in the two dimensions, user defined
       endpos[2],        // end of fragment in the two dimensions, user defined
       firstinchain,     // first element in chain, compute
       previousinchain;  // previous index in chain, compute
  Chainscoretype weight, // weight of fragment, user defined
                 score,  // score of highest scoreing chain ending here, compute
                 initialgap, // gap to start of sequences, user defined
                 terminalgap;// gap to last positions of fragment, user defined
} Fragmentinfo;

DECLAREARRAYSTRUCT(Fragmentinfo);

/*
  The following type defines the chain mode consisting of a chainkind.
  If chainkind = LOCALCHAININGTHRESHOLD, then an additional
  component minimumscore is used.
  If chainkind = LOCALCHAININGBEST, then  an additional
  component howmanybest is used.
  If chainkind = LOCALCHAININGPERCENTAWAY, then  an additional
  component percentawayfrombest is defined
*/

typedef struct
{
  Chainkind chainkind;
  Uint maxgapwidth;            // 0 if undefined or otherwise maximal width of 
                               // gap
  Chainscoretype minimumscore; // only defined if 
                               // chainkind = LOCALCHAININGTHRESHOLD
  Uint howmanybest,            // only defined if 
                               // chainkind = LOCALCHAININGBEST
       percentawayfrombest;    // only defined if 
                               // chainkind = LOCALCHAININGPERCENTAWAY
} Chainmode;

//\Ignore{

#endif

//}
