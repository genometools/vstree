
#include "types.h"
#include "dpbitvec48.h"
#include "failures.h"
#include "chardef.h"

#define UPDATELENGTHANDSCORE(SC)\
        if(longest->uint1 >= (SC))\
        {\
          longest->uint0 = (Uint) (vptr - v + 1);\
          longest->uint1 = SC;\
        }

/*
  These functions can be optimized by giving an extra threshold value.
*/

void longpatternlongestmatch (PairUint *longest,
                              Uint *ecol,
                              Uchar *u, 
                              Uint ulen, 
                              Uchar *v, 
                              Uint vlen)
{
  Uint val, we, nw, *ecolptr;
  Uchar *uptr, *vptr;

  for(*ecol = 0, ecolptr = ecol+1, uptr = u; uptr < u + ulen; ecolptr++, uptr++)
  {
#ifdef DEBUG
    if(*uptr == SEPARATOR)
    {
      NOTSUPPOSED;
    }
#endif
    *ecolptr = *(ecolptr-1) + 1;
  }
  longest->uint0 = 0;
  longest->uint1 = ulen;
  for (vptr = v; vptr < v + vlen; vptr++)
  {
    if(*vptr == SEPARATOR)
    {
      return;
    }
    nw = *ecol;
    (*ecol)++;
    for(ecolptr = ecol+1, uptr = u; uptr < u + ulen; ecolptr++, uptr++)
    {
      we = *ecolptr;
      *ecolptr = *(ecolptr-1) + 1;
      if(*uptr != *vptr || *uptr == (Uchar) WILDCARD)
      {
        val = nw + 1;
      } else
      {
        val = nw;
      }
      if(val < *ecolptr)
      {
        *ecolptr = val;
      } 
      if((val = we + 1) < *ecolptr)
      {
        *ecolptr = val;
      }
      nw = we;
    }
    UPDATELENGTHANDSCORE(*(ecolptr-1));
  }
}

/*
  The next function computes the length of the longest prefix of \(v\) matching
  the pattern with bit vector vector \texttt{Eqs}.
  rewrite this such that no alignment beginning with an insertion
  is allowed.
*/

/*
  Implement this and the previous function via templates, once with
  4 and once with 8. XXX
*/

void shortpatternlongestmatch(PairUint *longest,
                              DPbitvector4 *Eqs,
                              Uint ulen,
                              Uchar *v,
                              Uint vlen)
{
  DPbitvector4 Pv = (DPbitvector4) ~0, Mv = (DPbitvector4) 0, Eq, Xv, Xh, 
               Ph, Mh, Ebit = (DPbitvector4) (ONEDPbitvector4 << (ulen-1));
  Uint score = ulen;
  Uchar *vptr;

  longest->uint0 = 0;
  longest->uint1 = ulen;
  for(vptr = v; vptr < v + vlen; vptr++)
  {
    if(*vptr == SEPARATOR)
    {
      return;
    }
    Eq = Eqs[(Uint) *vptr];
    Xv = Eq | Mv;
    Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

    Ph = Mv | ~ (Xh | Pv);
    Mh = Pv & Xh;

    UPDATECOLUMNSCORE(score);

    Ph = (Ph << 1) | ONEDPbitvector4;
    Pv = (Mh << 1) | ~ (Xv | Ph);
    Mv = Ph & Xv;
    UPDATELENGTHANDSCORE(score);
  }
}

void mediumpatternlongestmatch(PairUint *longest,
                               DPbitvector8 *Eqs,
                               Uint ulen,
                               Uchar *v,
                               Uint vlen)
{
  DPbitvector8 Pv = (DPbitvector8) ~0, Mv = (DPbitvector8) 0, Eq, Xv, Xh, 
               Ph, Mh, Ebit = (DPbitvector8) (ONEDPbitvector8 << (ulen-1));
  Uint score = ulen;
  Uchar *vptr;

  longest->uint0 = 0;
  longest->uint1 = ulen;
  for(vptr = v; vptr < v + vlen; vptr++)
  {
    if(*vptr == SEPARATOR)
    {
      return;
    }
    Eq = Eqs[(Uint) *vptr];
    Xv = Eq | Mv;
    Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

    Ph = Mv | ~ (Xh | Pv);
    Mh = Pv & Xh;

    UPDATECOLUMNSCORE(score);

    Ph = (Ph << 1) | ONEDPbitvector8;
    Pv = (Mh << 1) | ~ (Xv | Ph);
    Mv = Ph & Xv;
    UPDATELENGTHANDSCORE(score);
  }
}
