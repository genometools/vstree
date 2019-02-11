
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "multidef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "errordef.h"
#include "genfile.h"
#include "scoredef.h"
#include "matchtask.h"
#include "procmultiseq.h"

#include "multiseq-adv.pr"
#include "readvirt.pr"
#include "alphabet.pr"

#define INCORRDESC   "incorrect position pair of the form \"@start end\" for sequence\nwith description"
#define CANNOTPARSE  "cannot parse position pair of the form \"@start end\" for sequence\nwith description"

#ifdef DEBUG
static void showsubstringpos(ArrayPairUint *substringpos)
{
  Uint seqnum;

  for(seqnum=0; seqnum<substringpos->nextfreePairUint; seqnum++)
  {
    printf("Substringinfo: %lu %lu\n",
            (Showuint) substringpos->spacePairUint[seqnum].uint0,
            (Showuint) substringpos->spacePairUint[seqnum].uint1);
  }
}
#endif

Sint parsesubstringpos(Multiseq *multiseq,ArrayPairUint *substringpos,
                       Multiseq *virtualmultiseq)
{
  Uchar *desc;
  Uint len, seqnum;
  Scaninteger frompos, topos;

  desc = multiseq->descspace.spaceUchar;
  if(desc == NULL)
  {
    return 0;
  }
  for(seqnum=0; seqnum < multiseq->numofsequences; seqnum++)
  {
    desc = DESCRIPTIONPTR(multiseq,seqnum);
    len = DESCRIPTIONLENGTH(multiseq,seqnum);
    if(desc[len-1] != '\n')
    {
      ERROR1("description of sequence %lu does not end with newline",
              (Showuint) seqnum);
      return (Sint) -1;
    }
    desc[len-1] = (Uchar) '\0';  // make sequence end
    if(sscanf((char *) desc,"@%ld %ld",&frompos,&topos) != 2)
    {
      if(seqnum == 0)
      {
        desc[len-1] = (Uchar) '\n';
        return 0;
      }
      ERROR2("%s \"%s\"",CANNOTPARSE,(char *) desc);
      return (Sint) -1;
    }
    if(frompos < 0)
    {
      ERROR2("%s \"%s\": topos must not be negative",
             INCORRDESC,(char *) desc);
      return (Sint) -3;
    }
    if(topos < 0)
    {
      ERROR2("%s \"%s\": topos must not be negative",
             INCORRDESC,(char *) desc);
      return (Sint) -4;
    }
    if(topos <= frompos && topos != 0)
    {
      ERROR2("%s \"%s\": start must be < end",INCORRDESC,(char *) desc);
      return (Sint) -5;
    }
    desc[len-1] = (Uchar) '\n';
    if(seqnum == 0)
    {
      if(virtualmultiseq->numofsequences > UintConst(1))
      {
        ERROR0("can only process substring information for database "
               "with more than one sequence");
        return (Sint) -6;
      }
      ALLOCASSIGNSPACE(substringpos->spacePairUint,NULL,PairUint,
                       multiseq->numofsequences);
      substringpos->allocatedPairUint = multiseq->numofsequences;
    }
    if(topos == 0)
    {
      topos = (Scaninteger) (virtualmultiseq->totallength-UintConst(1));
    }
    substringpos->spacePairUint[seqnum].uint0 = (Uint) frompos;
    substringpos->spacePairUint[seqnum].uint1 = (Uint) topos;
    if(substringpos->spacePairUint[seqnum].uint1 
       >= virtualmultiseq->totallength)
    {
      ERROR2("endposition %lu in substring specification incorrect: "
             "must be smaller than %lu",
             (Showuint) substringpos->spacePairUint[seqnum].uint1,
             (Showuint) virtualmultiseq->totallength);
      return (Sint) -7;
    }
  }
  substringpos->nextfreePairUint = multiseq->numofsequences;
  DEBUGCODE(2,showsubstringpos(substringpos));
  return 0;
}

Sint runforeachquerysubstpos(Virtualtree *queryvirtualtree,
                             Matchcallinfo *matchcallinfo,
                             ArrayPairUint *substringpos,
                             Queryinfo *queryinfo,
                             Procmultiseq *procmultiseq,
                             Evalues *evalues,
                             Sint (*applytoeachquery)(Virtualtree *,
                                                      Uint,
                                                      Matchcallinfo *,
                                                      ArrayPairUint *,
                                                      Queryinfo *,
                                                      Procmultiseq *,
                                                      Evalues *))
{
  Multiseq *subquerymultiseq;
  Virtualtree subqueryvirtualtree;
  Uint seqnum;

  makeemptyvirtualtree(&subqueryvirtualtree);
  subquerymultiseq = &subqueryvirtualtree.multiseq;
  copyMultiseq(subquerymultiseq,&queryvirtualtree->multiseq);
  copyAlphabet(&subqueryvirtualtree.alpha,&queryvirtualtree->alpha);
  subqueryvirtualtree.specialsymbols = queryvirtualtree->specialsymbols;
  subquerymultiseq->numofsequences = UintConst(1); // always one sequence
  for(seqnum = 0; seqnum < queryvirtualtree->multiseq.numofsequences; seqnum++)
  {
    if(seqnum == 0)
    {
      subquerymultiseq->sequence = queryvirtualtree->multiseq.sequence;
      subquerymultiseq->originalsequence 
        = queryvirtualtree->multiseq.originalsequence;
      /*
      subquerymultiseq->descspace.spaceUchar 
        = queryvirtualtree->multiseq.descspace.spaceUchar;
      */
    } else
    {
      Uint offset = queryvirtualtree->multiseq.markpos.spaceUint[seqnum-1] + 1;
      subquerymultiseq->sequence = queryvirtualtree->multiseq.sequence + offset;
      subquerymultiseq->originalsequence 
        = queryvirtualtree->multiseq.originalsequence + offset;
      /*
      subquerymultiseq->descspace.spaceUchar 
        = queryvirtualtree->multiseq.descspace.spaceUchar +
            DESCRIPTIONSTARTDESC(&queryvirtualtree->multiseq,seqnum);
      */
    }
    if(seqnum == queryvirtualtree->multiseq.numofsequences - 1)
    {
      subquerymultiseq->totallength 
        = (Uint) ((queryvirtualtree->multiseq.sequence + 
                   queryvirtualtree->multiseq.totallength) -
                   subquerymultiseq->sequence);
    } else
    {
      subquerymultiseq->totallength
        = (Uint) ((queryvirtualtree->multiseq.sequence + 
                   queryvirtualtree->multiseq.markpos.spaceUint[seqnum]) - 
                   subquerymultiseq->sequence);
    }
    if(applytoeachquery(&subqueryvirtualtree,
                        seqnum,
                        matchcallinfo,
                        substringpos,
                        queryinfo,
                        procmultiseq,
                        evalues) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}
