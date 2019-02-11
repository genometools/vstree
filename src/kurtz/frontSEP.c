

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "match.h"
#include "frontdef.h"
#include "minmax.h"
#include "debugdef.h"
#include "spacedef.h"
#include "chardef.h"
#include "gextenddef.h"

/*
  This file implements a threshold sensitive algorithm to compute the unit
  edit distance of two strings. It correctly recognizes 
  \texttt{WILDCARD}-symbols. For a more comprehensive documentation, see
  the file front.c
*/

#define COMPARESYMBOLS(A,B)\
        if((A) == SEPARATOR)\
        {\
          gl->ubound = uptr;\
        }\
        if((B) == SEPARATOR)\
        {\
          gl->vbound = vptr;\
        }\
        if((A) != (B) || ISSPECIAL(A))\
        {\
          break;\
        }

#define ROWVALUE(FVAL)                *(FVAL)
#define SETDIRECTION(FVAL,DIRECTION)  /* Nothing */
#define SHOWDIRECTION(FVAL)           /* Nothing */

/*
  This program can be optimized by dividing the loop of p
  into a first phase with with r <= 0 and a second phase with 
  r > 0. In the first phase, r is not important and several tests can be 
  simplified.
*/

#include "front.gen"

#define INITGREEDYALIGNRESERVOIR    void initgreedyextendreservoir
#define RESIZEGREEDYALIGNFRONTSPACE static void resizegreedyextendfrontspace
#define WRAPTGREEDYALIGNRESERVOIR   void wrapgreedyextendreservoir

#include "galspace.gen"

/*
  The following function evaluates an entry \(front(k,p)\) in a 
  backward direction. The first part is identical to the previous function
*/

static Sint evalentrybackward(FrontResource *gl,
                              BOOL *foundseed,
                              Frontspec *fspec,
                              Sint k,
                              Uint reachlength)
{
  Sint matchlength, value, t;
  Uchar *uptr, *vptr, *ustart, a, b;
  Frontvalue *fptr;

  DEBUG1(3,"evalentrybackward(k=%ld)\n",(Showsint) k);
  fptr = gl->frontspace + fspec->offset - fspec->left;
  t = accessfront(gl,fptr,fspec,k) + 1;           // same diagonal
  DEBUG2(3,"same: access(k=%ld)=%ld\n",(Showsint) k,(Showsint) (t-1));

  value = accessfront(gl,fptr,fspec,k-1);         // diagonal below: INSERTION
  DEBUG2(3,"below: access(k=%ld)=%ld\n",(Showsint) (k-1),(Showsint) value);
  if(t < value)
  {
    t = value;
  }
  value = accessfront(gl,fptr,fspec,k+1) + 1;     // diagonal above: DELETION
  DEBUG2(3,"above: access(k=%ld)=%ld\n",(Showsint) (k+1),
                                        (Showsint) (value-1));
  if(t < value)
  {
    t = value;
  }
  DEBUG1(3,"maximum: t=%ld\n",(Showsint) t);  // the maximum over three values
  if(t < 0 || t + k < 0)
  {
    DEBUG2(3,"t=%ld < 0 || t+k=%ld=>return MINUSINFINITYFRONT\n",
              (Showsint) t,
              (Showsint) (t+k));
    return MINUSINFINITYFRONT(gl);
  }
  if(gl->ulen != 0 && gl->vlen != 0)     // only for nonempty strings
  {
    uptr = gl->useq - t;
    vptr = gl->vseq - (t + k);
    if(uptr == vptr)    // strings are equal
    {
      t = gl->ulen-1;
    } else
    {
      ustart = uptr;
      while(uptr > gl->ubound && vptr > gl->vbound)
      {
        a = *uptr;
        b = *vptr;
        COMPARESYMBOLS(a,b);
        uptr--;
        vptr--;
      }
      matchlength = (Sint) (ustart - uptr);
      if((Uint) matchlength >= reachlength)
      {
        DEBUG1(3,"seed of length %ld detected while scanning:\n",
                  (Showsint) matchlength);
        DEBUG1(3,"prefix(u at pos %ld) and ",(Showsint) (gl->ulen - 1 - t));
        DEBUG1(3,"prefix(v at pos %ld)\n",(Showsint) (gl->vlen - 1 - (t + k)));
        *foundseed = True;
        return MINUSINFINITYFRONT(gl);
      }
      t += matchlength;
    }
  }
  if(gl->useq - t < gl->ubound || gl->vseq - (t + k) < gl->vbound)
  {
    return MINUSINFINITYFRONT(gl);
  }
  return t;
}

/* 
  The following function evaluates a front in backward direction.
  It returns True if any of the front values is at least 0.
*/

static BOOL evalfrontbackward(FrontResource *gl,
                              BOOL *foundseed,
                              Frontspec *prevfspec,Frontspec *fspec,
                              Sint r,Uint reachlength)
{
  Frontvalue *fval;
  Sint k;
  BOOL defined = False;

  for(fval = gl->frontspace + fspec->offset, k=fspec->left; 
      k < fspec->left + fspec->width; k++, fval++)
  {
    if(r <= 0 || k <= -r || k >= r)
    {
      STOREFRONT(gl,ROWVALUE(fval),
                 evalentrybackward(gl,foundseed,
                                   prevfspec,k,reachlength));
      if(ROWVALUE(fval) >=0)
      {
        defined = True;
      }
      DEBUG2(3,"store front[k=%ld]=%ld ",(Showsint) k,
                                         (Showsint) ROWVALUE(fval));
      DEBUG1(3,"at index %ld\n",(Showsint) (fval-gl->frontspace));
    } else
    {
      DEBUG1(3,"store front[k=%ld]=MINUSINFINITYFRONT ",(Showsint) k);
      DEBUG1(3,"at index %ld\n",(Showsint) (fval-gl->frontspace));
      STOREFRONT(gl,ROWVALUE(fval),MINUSINFINITYFRONT(gl));
    }
  }
  DEBUG1(3,"frontvalues[r=%ld]=",(Showsint) r);
  DEBUGCODE(3,showfront(gl,fspec,r));
  return defined;
}

static void firstfront(FrontResource *gl,Frontspec *fspec)
{
  STOREFRONT(gl,gl->frontspace[0],0);
  fspec->left = fspec->offset = 0;
  fspec->width = (Sint) 1;
}

/*
  Extend a given seed in forward direction, i.e. to the right. 
  \(h\) is the maximal error allowed.
*/

Sint extendedrightSEP(Greedyalignreservoir *gar,
                      Sint maxdist,
                      Uchar *useq,
                      Sint ulen,
                      Uchar *vseq,
                      Sint vlen)
{
  FrontResource gl;
  Frontspec *fspec;
  Sint i, offset, p, r;

  DEBUG3(2,"extendedrightSEP(maxdist=%ld,ulen=%ld,vlen=%ld)\n",
             (Showsint) maxdist,
             (Showsint) ulen,
             (Showsint) vlen);
  gl.ubound = useq + ulen;
  for(i=0; i<MIN((Sint) (maxdist+(Sint) 1),ulen); i++)
  {
    if(useq[i] == SEPARATOR)
    {
      gl.ubound = useq + i;
      break;
    }
  }
  gl.vbound = vseq + vlen;
  for(i=0; i<MIN((Sint) (maxdist+(Sint) 1),vlen); i++)
  {
    if(vseq[i] == SEPARATOR)
    {
      gl.vbound = vseq + i;
      break;
    }
  }
  gl.useq = useq;
  gl.vseq = vseq;
  gl.ulen = ulen;
  gl.vlen = vlen;
  gl.integermin = -MAX(ulen,vlen);
  resizegreedyextendfrontspace(gar,(Uint) maxdist);
  gl.frontspace = gar->frontvaluereservoir.spaceFrontvalue;
  fspec = gar->frontspecreservoir.spaceFrontspec;
  firstfront(&gl,fspec);
  if(gl.ulen != gl.vlen || gl.vlen != 0)
  {
    for(p=(Sint) 1, r=1-MIN(gl.ulen,gl.vlen); p<=maxdist; p++, r++)
    {
      offset = fspec->offset + fspec->width;
      fspec++;
      fspec->offset = offset;
      frontspecparms(&gl,fspec,p,r);
      if(!evalfrontforward(&gl,fspec-1,fspec,r))
      {
        return p-1;
      }
    }
    return maxdist;
  }
  return 0;
}

/*
  Extend a given seed in backward direction, i.e. to the right. 
  \(h\) is the maximal error allowed.
*/

Sint extendedleftSEP(Greedyalignreservoir *gar,
                     Sint maxdist,
                     Uchar *useq,
                     Sint ulen,
                     Uchar *vseq,
                     Sint vlen,
                     Uint reachlength)
{
  FrontResource gl;
  Frontspec *fspec;
  Sint imin, i, offset, p = 0, r;
  BOOL foundseed = False, ret;

  DEBUG4(2,"extenddedleftSEP(maxdist=%ld,ulen=%ld,vlen=%ld,rlen=%lu)\n",
               (Showsint) maxdist,
               (Showsint) ulen,
               (Showsint) vlen,
               (Showuint) reachlength);

  gl.ubound = useq - 1;
  if(ulen > (Sint) maxdist)
  {
    imin = ulen - maxdist;
  } else
  {
    imin = 0;
  }
  for(i=ulen-1; i>=imin; i--)
  {
    if(useq[i] == SEPARATOR)
    {
      gl.ubound = useq + i;
      break;
    }
  }
  gl.vbound = vseq - 1;
  if(vlen > (Sint) maxdist)
  {
    imin = vlen - maxdist;
  } else
  {
    imin = 0;
  }
  for(i=vlen-1; i>=imin; i--)
  {
    if(vseq[i] == SEPARATOR)
    {
      gl.vbound = vseq + i;
      break;
    }
  }
  gl.useq = useq + ulen - 1;
  gl.vseq = vseq + vlen - 1;
  gl.ulen = ulen;
  gl.vlen = vlen;
  gl.integermin = -MAX(ulen,vlen);
  resizegreedyextendfrontspace(gar,(Uint) maxdist);
  gl.frontspace = gar->frontvaluereservoir.spaceFrontvalue;
  fspec = gar->frontspecreservoir.spaceFrontspec;
  firstfront(&gl,fspec);
  if(gl.ulen != gl.vlen || gl.vlen != 0)
  {
    for(p=(Sint) 1, r=1-MIN(gl.ulen,gl.vlen); p<=maxdist; p++, r++)
    {
      offset = fspec->offset + fspec->width;
      fspec++;
      fspec->offset = offset;
      frontspecparms(&gl,fspec,p,r);
      ret = evalfrontbackward(&gl,&foundseed,fspec-1,
                              fspec,r,reachlength);
      if(ret && foundseed)
      {
        return p;
      }
      if(!ret)
      {
        return p-1;
      }
    }
    return maxdist;
  } 
  return 0;
}

Sint unitedistfrontSEPgeneric(BOOL withmaxdist,
                              Uint maxdist,
                              Uchar *useq,
                              Sint ulen,
                              Uchar *vseq,
                              Sint vlen)
{
  Uint currentallocated;
  FrontResource gl;
  Frontspec frontspecspace[2],
            *fspec, 
            *prevfspec;
  Frontvalue *fptr;
  Sint k, r, 
       realdistance;

  DEBUG2(2,"unitedistcheckSEPgeneric(ulen=%ld,vlen=%ld)\n",
             (Showsint) ulen,
             (Showsint) vlen);

  if(withmaxdist)
  {
    currentallocated = (maxdist+1) * (maxdist+1) + 1;
  } else
  {
    currentallocated = (Uint) 1;
  }
  ALLOCASSIGNSPACE(gl.frontspace,NULL,Frontvalue,currentallocated);
  gl.useq = useq;
  gl.vseq = vseq;
  gl.ubound = useq + ulen;
  gl.vbound = vseq + vlen;
  gl.ulen = ulen;
  gl.vlen = vlen;
  gl.integermin = -MAX(ulen,vlen);
  prevfspec = &frontspecspace[0];
  firstfrontforward(&gl,prevfspec);
  if(gl.ulen == gl.vlen && ROWVALUE(&gl.frontspace[0]) == gl.vlen)
  {
    realdistance = 0;
  } else
  {
    for(k=(Sint) 1, r=1-MIN(gl.ulen,gl.vlen); /* Nothing */ ; k++, r++)
    {
      if(withmaxdist && k > (Sint) maxdist)
      {
        realdistance = (Sint) (maxdist + 1);
        break;
      }
      if(prevfspec == &frontspecspace[0])
      {
        fspec = &frontspecspace[1];
      } else
      {
        fspec = &frontspecspace[0];
      }
      fspec->offset = prevfspec->offset + prevfspec->width;
      frontspecparms(&gl,fspec,k,r);
      while((Uint) (fspec->offset + fspec->width) >= currentallocated)
      {
        if(withmaxdist)
        {
          fprintf(stderr,"Not enough frontspace: "
                         "maxdist=%lu,p=%ld,offset=%ld,width=%ld\n",
                          (Showuint) maxdist,
                          (Showsint) k,
                          (Showsint) fspec->offset,
                          (Showsint) fspec->width);
          exit(EXIT_FAILURE);
        }
        currentallocated += (k+1);
        ALLOCASSIGNSPACE(gl.frontspace,gl.frontspace,
                         Frontvalue,currentallocated);
      }
      (void) evalfrontforward(&gl,prevfspec,fspec,r);
      fptr = gl.frontspace + fspec->offset - fspec->left;
      if(accessfront(&gl,fptr,fspec,(Sint) (vlen - ulen)) == ulen)
      {
        realdistance = k;
        break;
      }
      if(prevfspec == &frontspecspace[0])
      {
        prevfspec = &frontspecspace[1];
      } else
      {
        prevfspec = &frontspecspace[0];
      }
    }
    if(withmaxdist)
    {
      if(realdistance <= (Sint) maxdist)
      {
        DEBUG1(3,"unitedistfrontSEP returns %ld\n",(Showsint) realdistance);
        FREESPACE(gl.frontspace);
        return realdistance;
      }
      DEBUG0(3,"unitedistfrontSEP returns -1\n");
      FREESPACE(gl.frontspace);
      return (Sint) -1;
    }
  }
  DEBUG1(3,"unitedistfrontSEP returns %ld\n",(Showsint) realdistance);
  FREESPACE(gl.frontspace);
  return realdistance;
}
