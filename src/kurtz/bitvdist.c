#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "spacedef.h"
#include "dpbitvec48.h"
#include "chardef.h"

#define STANDARDEBIT (ONEDPbitvector4 << (DPWORDSIZE4-1))

static Uint determinenumberofblocks(Uint slen)
{
  if(slen < DPWORDSIZE4)
  {
    return UintConst(1);
  }
  return UintConst(1) + (slen-UintConst(1))/DPWORDSIZE4;
}

#ifdef DEBUG
static void showallcolumnblocks(Uint firstvalue,
                                DPbitvectormulti *eqsmultivector,
                                Uint numofblocks,
                                Uint ulen)
{
  Uint i, bnum, remaining, currentblockbits, score = firstvalue;
  DPbitvector4 mask;

  for(bnum = 0, remaining = ulen; 
      bnum < numofblocks; 
      bnum++, remaining -= DPWORDSIZE4)
  {
    if(bnum == numofblocks - 1)
    {
      currentblockbits = remaining;
    } else
    {
      currentblockbits = DPWORDSIZE4;
    }
    for(i=0, mask = ONEDPbitvector4; i < currentblockbits; i++, mask <<= 1)
    {
      if(eqsmultivector[bnum].Pvmulti4 & mask)
      {
        score++;
      } else
      {
        if(eqsmultivector[bnum].Mvmulti4 & mask)
        {
#ifdef DEBUG
          if(score == 0)
          {
            fprintf(stderr,"cannot decrement 0\n");
            exit(EXIT_FAILURE);
          }
#endif
          score--;
        }
      }
    }
  }
  printf("score=%lu\n",(Showuint) score);
}
#endif

void initDPbitvectorreservoir(DPbitvectorreservoir *dpbvres,
                              Uint mapsize,Uint maxlength)
{
  Uint bnum, eqsoffset, maxnumofblocks;

  maxnumofblocks = determinenumberofblocks(maxlength);
  if(maxnumofblocks > (Uint) STATICNUMOFBLOCKS)
  {
    ALLOCASSIGNSPACE(dpbvres->eqsmultivector,NULL,DPbitvectormulti,
                     maxnumofblocks);
  } else
  {
    dpbvres->eqsmultivector = &dpbvres->eqsmultivectorspace[0];
  }
  if(maxnumofblocks * mapsize > 
     (Uint) (STATICNUMOFBLOCKS * STATICMAPSIZE))
  {
    ALLOCASSIGNSPACE(dpbvres->eqsmultiflat4,NULL,DPbitvector4,
                     maxnumofblocks * mapsize);
  } else
  {
    dpbvres->eqsmultiflat4 = &dpbvres->eqsmultiflat4space[0];
  }
  for(bnum = 0, eqsoffset = 0; bnum < maxnumofblocks; 
      bnum++, eqsoffset += mapsize)
  {
    dpbvres->eqsmultivector[bnum].eqsmulti4
      = dpbvres->eqsmultiflat4 + eqsoffset;
  }
}

void freeDPbitvectorreservoir(DPbitvectorreservoir *dpbvres)
{
  if(dpbvres->eqsmultivector != &dpbvres->eqsmultivectorspace[0])
  {
    FREESPACE(dpbvres->eqsmultivector);
  }
  if(dpbvres->eqsmultiflat4 != &dpbvres->eqsmultiflat4space[0])
  {
    FREESPACE(dpbvres->eqsmultiflat4);
  }
}

#define EVALUATEDISTANCEOFSHORTSTRINGS evaluatedistanceofshortstrings4
#define DPBITVECTOR DPbitvector4
#define ONEDPBITVECTOR ONEDPbitvector4
#define GETEQS getEqs4
#define ISLARGEPATTERN ISLARGEPATTERN4

#include "distanceshort.gen"

#undef EVALUATEDISTANCEOFSHORTSTRINGS
#undef DPBITVECTOR
#undef ONEDPBITVECTOR
#undef GETEQS
#undef ISLARGEPATTERN

#define EVALUATEDISTANCEOFSHORTSTRINGS evaluatedistanceofshortstrings8
#define DPBITVECTOR DPbitvector8
#define ONEDPBITVECTOR ONEDPbitvector8
#define GETEQS getEqs8
#define ISLARGEPATTERN ISLARGEPATTERN8

#include "distanceshort.gen"

static Sint advancemyersblock(DPbitvectormulti *eqsmultivector,
                              BOOL firstblock,
                              Uchar currentchar, 
                              Sint previouscarry,
                              DPbitvector4 Ebit)
{
  DPbitvector4 Pv,
               Mv,
               Eq, 
               Xv, 
               Xh,
               Ph, 
               Mh; 
  Sint hout = 0;

  Pv = eqsmultivector->Pvmulti4;
  Mv = eqsmultivector->Mvmulti4;
  Eq = eqsmultivector->eqsmulti4[currentchar];
  Xv = Eq | Mv;
  /* added additonal line to handle previouscarry */
  if(previouscarry < 0)
  {
    Eq |= ONEDPbitvector4;
  }
  Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

  Ph = Mv | ~ (Xh | Pv);
  Mh = Pv & Xh;

  UPDATECOLUMNSCORE(hout);

  if(firstblock)
  {
    Ph = (Ph << 1) | ONEDPbitvector4;
  } else
  {
    Ph <<= 1;
  }
  Mh <<= 1;
  /* added additonal lines to handle previouscarry bit */
  if(previouscarry < 0)
  {
    Mh |= ONEDPbitvector4;
  } else
  {
    if(previouscarry > 0)
    {
      Ph |= ONEDPbitvector4;
    }
  }
  Pv = Mh | ~ (Xv | Ph);
  Mv = Ph & Xv;
  eqsmultivector->Pvmulti4 = Pv;
  eqsmultivector->Mvmulti4 = Mv;
  return hout;
}

Uint evaluatedistanceoflongstrings(DPbitvectorreservoir *dpvres,
                                   Uint mapsize,
                                   Uchar *useq,
                                   Uint ulen,
                                   Uchar *vseq,
                                   Uint vlen)
{
  Uchar *vptr;
  Uint uoffset, remaining;
  Sint score, carry;
  DPbitvector4 Ebit;
  DPbitvectormulti *eqsmvptr;

  dpvres->numofblocks = determinenumberofblocks(ulen);
  for(eqsmvptr = dpvres->eqsmultivector, uoffset = 0;
      /* Nothing */;
      eqsmvptr++, uoffset += DPWORDSIZE4)
  {
    eqsmvptr->Pvmulti4 = (DPbitvector4) ~0;
    eqsmvptr->Mvmulti4 = (DPbitvector4) 0;
    if(eqsmvptr == dpvres->eqsmultivector + dpvres->numofblocks - 1)
    {
      getEqs4(eqsmvptr->eqsmulti4,mapsize,useq+uoffset,ulen-uoffset);
      break;
    }
    getEqs4(eqsmvptr->eqsmulti4,mapsize,useq+uoffset,DPWORDSIZE4);
  }
#ifdef DEBUG
  showallcolumnblocks(0,
                      dpvres->eqsmultivector,
                      dpvres->numofblocks,
                      ulen);
#endif
  score = (Sint) ulen;
  for(vptr = vseq; vptr < vseq + vlen; vptr++)
  {
    if(*vptr == SEPARATOR)
    {
      fprintf(stderr,"programming error: cannot align sequences containing "
                     "separators\n");
      exit(EXIT_FAILURE);
    }
    carry = 0;
    for(eqsmvptr = dpvres->eqsmultivector, remaining = ulen; 
        eqsmvptr < dpvres->eqsmultivector + dpvres->numofblocks;
        eqsmvptr++, remaining -= DPWORDSIZE4)
    {
      if(remaining >= DPWORDSIZE4)
      {
        Ebit = (DPbitvector4) (ONEDPbitvector4 << (DPWORDSIZE4-1));
      } else
      {
        Ebit = (DPbitvector4) (ONEDPbitvector4 << (remaining-1));
      }
      carry = advancemyersblock(eqsmvptr,
                                eqsmvptr == dpvres->eqsmultivector 
                                  ? True : False, 
                                *vptr,
                                carry,
                                Ebit);
    }
    if(carry < 0)
    {
      score--;
    } else
    {
      if(carry > 0)
      {
        score++;
      }
    }
#ifdef DEBUG
    showallcolumnblocks((Uint) (vptr-vseq+1),
                        dpvres->eqsmultivector,
                        dpvres->numofblocks,
                        ulen);
#endif
  }
#ifdef DEBUG
  if(score < 0)
  {
    fprintf(stderr,"programming error: score = %ld < 0",(Showsint) score);
    exit(EXIT_FAILURE);
  }
#endif
  return (Uint) score;
}

#define EVALUATEDIST(MINSEQ,MINLEN,MAXSEQ,MAXLEN)\
        if(ISLARGEPATTERN8(MINLEN))\
        {\
          return evaluatedistanceoflongstrings(dpvres,\
                                               mapsize,\
                                               MINSEQ,\
                                               MINLEN,\
                                               MAXSEQ,\
                                               MAXLEN);\
        }\
        if(ISLARGEPATTERN4(MINLEN))\
        {\
          return evaluatedistanceofshortstrings8(dpvres->eqsspace8,\
                                                 mapsize,\
                                                 MINSEQ,\
                                                 MINLEN,\
                                                 MAXSEQ,\
                                                 MAXLEN);\
        }\
        return evaluatedistanceofshortstrings4(dpvres->eqsspace4,\
                                               mapsize,\
                                               MINSEQ,\
                                               MINLEN,\
                                               MAXSEQ,\
                                               MAXLEN)

Uint distanceofstrings(DPbitvectorreservoir *dpvres,
                       Uint mapsize,
                       Uchar *useq,
                       Uint ulen,
                       Uchar *vseq,
                       Uint vlen)
{
  if(ulen == 0)
  {
    return vlen;
  }
  if(vlen == 0)
  {
    return ulen;
  }
  if(ulen == UintConst(1) && vlen == UintConst(1))
  {
    if(useq[0] != vseq[0] || ISSPECIAL(useq[0]))
    {
      return UintConst(1);
    }
    return 0;
  }
  if(ulen < vlen)
  {
    EVALUATEDIST(useq,ulen,vseq,vlen);
  } else
  {
    EVALUATEDIST(vseq,vlen,useq,ulen);
  }
}
