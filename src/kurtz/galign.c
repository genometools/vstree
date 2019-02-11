#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "frontdef.h"
#include "minmax.h"
#include "debugdef.h"
#include "chardef.h"
#include "errordef.h"
#include "alignment.h"
#include "minmax.h"
#include "divmodmul.h"
#include "galigndef.h"

/*
  This file implements a threshold sensitive algorithm to compute the unit
  edit distance of two strings. It correctly recognizes 
  \texttt{WILDCARD}-symbols and computes alignments.
  For a more comprehensive documentation, see the file front.c
*/

#define COMPARESYMBOLS(A,B)\
        if((A) != (B) || ISSPECIAL(A))\
        {\
          break;\
        }

#ifdef DEBUG
#define CHECKINTERVAL(D)\
        if(fspec->left > (D) || (D) >= fspec->left + fspec->width)\
        {\
          fprintf(stderr,"undefined value of front\n");\
          exit(EXIT_FAILURE);\
        }
#else
#define CHECKINTERVAL(D) /* Nothing */
#endif

#define ROWVALUE(FVAL)                (FVAL)->dptabrow
#define SETDIRECTION(FVAL,DIRECTION)  (FVAL)->dptabdirection = DIRECTION
#define SHOWDIRECTION(FVAL)\
        {\
          Sint c = bit2char((FVAL)->dptabdirection);\
          if(c < 0)\
          {\
            fprintf(stderr,"no dptabdirection\n");\
            exit(EXIT_FAILURE);\
          }\
          printf("%c ",(char) c);\
        }

#ifdef DEBUG
#define ASSIGNEOPPTR(EOPPTR,VALUE)\
        if((EOPPTR) >= alignment->spaceEditoperation +\
                       alignment->allocatedEditoperation)\
        {\
          fprintf(stderr,"program error: not enoug space for alignment\n");\
          fprintf(stderr,"nextfree=%lu\n",\
                   (Showuint) alignment->nextfreeEditoperation);\
          fprintf(stderr,"allocated=%lu\n",\
                   (Showuint) alignment->allocatedEditoperation);\
          exit(EXIT_FAILURE);\
        }\
        DEBUG2(3,"assign edit[%lu]=%hu\n",\
               (Showuint) ((EOPPTR) - alignment->spaceEditoperation),\
               VALUE);\
        *(EOPPTR) = VALUE
#else
#define ASSIGNEOPPTR(EOPPTR,VALUE)\
        *(EOPPTR) = VALUE
#endif

#define ADDIDENTICAL\
        while(True)\
        {\
          ASSIGNEOPPTR(eopptr,(Editoperation) (lenid & MAXIDENTICALLENGTH));\
          DEBUG1(3,"(L %hu)",*eopptr);\
          eopptr++;\
          if(lenid <= MAXIDENTICALLENGTH)\
          {\
            break;\
          }\
          lenid -= MAXIDENTICALLENGTH;\
        } 

#ifdef DEBUG

static Sint bit2char(Uchar dptabdirection)
{
  if(dptabdirection & REPLACEMENTBIT)
  {
    return (Sint) 'R';
  }
  if(dptabdirection & INSERTIONBIT)
  {
    return (Sint) 'I';
  }
  if(dptabdirection & DELETIONBIT)
  {
    return (Sint) 'D';
  }
  return (Sint) -1;
}

#endif

static void resizealignmentspace(ArrayEditoperation *alignment,
                                 Uint distance,
                                 Uint ulen)
{
  Uint maxeopnumber;

  if(distance == 0)
  {
    maxeopnumber = ulen/MAXIDENTICALLENGTH + 1;
  } else
  {
    maxeopnumber = MULT2(distance+1);
  }
  if(maxeopnumber > alignment->allocatedEditoperation)
  {
    alignment->allocatedEditoperation = maxeopnumber;
    ALLOCASSIGNSPACE(alignment->spaceEditoperation,
                     alignment->spaceEditoperation,
                     Editoperation,
                     alignment->allocatedEditoperation);
  }
}

void alignequalstrings(ArrayEditoperation *alignment,
                       Uint lengthequalstring)
{
  Uint lenid = lengthequalstring;
  Editoperation *eopptr;

  resizealignmentspace(alignment,0,lengthequalstring);
  eopptr = alignment->spaceEditoperation + alignment->nextfreeEditoperation;
  while(True)
  {
    ASSIGNEOPPTR(eopptr,(Editoperation) (lenid & MAXIDENTICALLENGTH));
    DEBUG1(3,"(L %hu)",*eopptr);
    eopptr++;
    if(lenid <= MAXIDENTICALLENGTH)
    {
      break;
    }
    lenid -= MAXIDENTICALLENGTH;
  }
  alignment->nextfreeEditoperation
    = (Uint) (eopptr - alignment->spaceEditoperation);
}

Uint hammingalignment(ArrayEditoperation *alignment,
                      Uchar *useq,
                      Uint ulen,
                      Uchar *vseq)
{
  Sint i;
  Uint j, lenid = 0, countmismatches = 0;
  BOOL inequalrange = False;
  Editoperation *eopptr;
  Uchar a, b;

  for(j=0; j<ulen; j++)
  {
    a = useq[j];
    b = vseq[j];
    if(a != b || ISSPECIAL(a))
    {
      countmismatches++;
    }
  }
  resizealignmentspace(alignment,countmismatches,ulen);
  eopptr = alignment->spaceEditoperation + alignment->nextfreeEditoperation;
  for(i=(Sint) (ulen-1); i>=0; i--)
  {
    a = useq[i];
    b = vseq[i];
    if(a != b || ISSPECIAL(a))
    {
      if(inequalrange)
      {
        ADDIDENTICAL;
        inequalrange = False;
      }
      ASSIGNEOPPTR(eopptr,MISMATCHEOP);
      DEBUG2(3,"(R %lu %lu)",(Showuint) i,(Showuint) i);
      eopptr++;
    } else
    {
      if(inequalrange)
      {
        lenid++;
      } else
      {
        lenid = UintConst(1);
        inequalrange = True;
      }
    }
  }
  if(inequalrange)
  {
    ADDIDENTICAL;
  }
  alignment->nextfreeEditoperation 
    = (Uint) (eopptr - alignment->spaceEditoperation);
  return countmismatches;
}

static void backtracefront(ArrayEditoperation *alignment,
                           Sint distance,
                           Frontspec *fspecspace,
                           Frontvalue *frontspace,
                           Uchar *useq,
                           Sint ulen,
                           Uchar *vseq,
                           Sint vlen)
{
  Sint d, k, i, j, starti;
  Frontspec *fspec;
  Frontvalue *fval;
  Editoperation *eopptr;
  Uchar a, b;
  Uint lenid;

  DEBUG3(2,"backtracefront(distance=%ld,ulen=%ld,vlen=%ld)\n",
             (Showsint) distance,
             (Showsint) ulen,
             (Showsint) vlen);

  eopptr = alignment->spaceEditoperation + alignment->nextfreeEditoperation;
  if(ulen != vlen || vlen != 0)
  {
    d = vlen - ulen;
    i = ulen-1;
    j = vlen-1;
    for(k=distance; k>0; k--)
    {
      fspec = fspecspace + k;
      DEBUG5(2,"k=%ld,d=%ld,offset=%ld,left=%ld,width=%ld\n",
                (Showsint) k,
                (Showsint) d,
                (Showsint) fspecspace[k].offset,
                (Showsint) fspecspace[k].left,
                (Showsint) fspecspace[k].width);
      DEBUG1(3,"access %ld\n",(Showsint) (fspec->offset-fspec->left+d));
      fval = frontspace + fspec->offset - fspec->left + d;
      CHECKINTERVAL(d);
      starti = i;
      for(/* Nothing */; i>=0 && j>=0; i--, j--)
      {
        a = useq[i];
        b = vseq[j];
        COMPARESYMBOLS(a,b);
      }
      if(i < starti)
      {
        lenid = (Uint) (starti - i);
        ADDIDENTICAL;
      }
      if(fval->dptabdirection & REPLACEMENTBIT)
      {
        DEBUG2(3,"(R %ld %ld)",(Showsint) i,(Showsint) j);
        ASSIGNEOPPTR(eopptr,MISMATCHEOP);
        eopptr++;
        i--;
        j--;
      } else
      {
        if(fval->dptabdirection & DELETIONBIT)
        {
          DEBUG1(3,"(D %ld)",(Showsint) i);
          ASSIGNEOPPTR(eopptr,DELETIONEOP);
          eopptr++;
          i--;
          d++;
        } else
        {
          if(fval->dptabdirection & INSERTIONBIT)
          {
            DEBUG1(3,"(I %ld)",(Showsint) j);
            ASSIGNEOPPTR(eopptr,INSERTIONEOP);
            eopptr++;
            j--;
            d--;
          } 
#ifdef DEBUG
          else
          {
            fprintf(stderr,"no bit set\n");
            exit(EXIT_FAILURE);
          }
#endif
        }
      }
    }
    if(i >= 0)
    {
      lenid = (Uint) (i+1);
      ADDIDENTICAL;
    }
    DEBUG0(3,"\n");
  }  
  alignment->nextfreeEditoperation 
    = (Uint) (eopptr - alignment->spaceEditoperation);
}

#define INITGREEDYALIGNRESERVOIR    void initgreedyalignreservoir
#define RESIZEGREEDYALIGNFRONTSPACE static void resizegreedyalignfrontspace
#define WRAPTGREEDYALIGNRESERVOIR   void wraptgreedyalignreservoir
#define WITHALIGNMENT

#include "galspace.gen"

#include "front.gen"

Sint greedyedistalign(Greedyalignreservoir *gar,
                      BOOL withmaxdist,
                      Sint maxdist,
                      Uchar *useq,
                      Sint ulen,
                      Uchar *vseq,
                      Sint vlen)
{
  FrontResource gl;
  Frontspec *fspec;
  Uint fspecindex;
  Frontvalue *fptr;
  Sint offset, k, r, realdistance;

#ifdef DEBUG
  if(withmaxdist)
  {
    DEBUG3(2,"greedyalign(maxdist=%ld,ulen=%ld,vlen=%ld)\n",
               (Showsint) maxdist,
               (Showsint) ulen,
               (Showsint) vlen);
  } else
  {
    DEBUG2(2,"greedyalign(ulen=%ld,vlen=%ld)\n",
               (Showsint) ulen,
               (Showsint) vlen);
  }
#endif
  gl.useq = useq;
  gl.vseq = vseq;
  gl.ulen = ulen;
  gl.vlen = vlen;
  gl.ubound = useq + ulen;
  gl.vbound = vseq + vlen;
  gl.integermin = -MAX(ulen,vlen);
  if(withmaxdist)
  {
    resizegreedyalignfrontspace(gar,(Uint) maxdist);
  }
  gl.frontspace = gar->frontvaluereservoir.spaceFrontvalue;
  firstfrontforward(&gl,gar->frontspecreservoir.spaceFrontspec);
  fspecindex = 0;
  if(gl.ulen == gl.vlen && 
     ROWVALUE(gar->frontvaluereservoir.spaceFrontvalue) == gl.vlen)
  {
    realdistance = 0;
  } else
  {
    for(k=(Sint) 1, r=1-MIN(gl.ulen,gl.vlen); /* Nothing */ ; k++, r++)
    {
      if(withmaxdist)
      {
        if(k > (Sint) maxdist)
        {
          realdistance = (Sint) (maxdist + 1);
          break;
        }
      } else
      {
        resizegreedyalignfrontspace(gar,(Uint) k);
        gl.frontspace = gar->frontvaluereservoir.spaceFrontvalue;
      }
      fspec = gar->frontspecreservoir.spaceFrontspec + fspecindex;
      offset = fspec->offset + fspec->width;
      fspecindex++;
      fspec++;
      fspec->offset = offset;
      frontspecparms(&gl,fspec,k,r);
      (void) evalfrontforward(&gl,fspec-1,fspec,r);
      fptr = gar->frontvaluereservoir.spaceFrontvalue + 
             fspec->offset - fspec->left;
      if(accessfront(&gl,fptr,fspec,(Sint) (vlen - ulen)) == ulen)
      {
        realdistance = k;
        break;
      }
    }
    if(withmaxdist)
    {
      if(realdistance > maxdist)
      {
        ERROR2("cannot compute edit distance alignment for distance %ld > %ld",
                (Showsint) k,(Showsint) maxdist);
        return (Sint) -2;
      }
    }
  }
  resizealignmentspace(&gar->alignment,(Uint) realdistance,
                       (Uint) MIN(ulen,vlen));
  backtracefront(&gar->alignment,
                 realdistance,
                 gar->frontspecreservoir.spaceFrontspec,
                 gar->frontvaluereservoir.spaceFrontvalue,
                 useq,
                 ulen,
                 vseq,
                 vlen);
  return realdistance;
}
