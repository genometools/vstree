
/*
  This module implements the standard dynamic programming algorithm
  to compute the unit edit distance between two strings in linear space.
  Additionally checkpoints are computed which allow to recover
  an optimal alignment using only linear space. This idea was 
  first described in the following paper:

  \begin{verbatim}
  @ARTICLE{POW:ALL:DIX:1999,
        author =        "Powell, D.R. and Allison, L. and Dix, T.I.",
        title =         {{A Versatile Divide and Conquer Technique for 
                          Optimal String Alignment}},
        journal =       {{Information Processing Letters}},
        volume =        {{70}},
        pages =         {{127-139}},
        year =          1999}
  \end{verbatim}

  The code is partly based on some code written by
  Jens Stoye, Technical Faculty, University of Bielefeld.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "divmodmul.h"
#include "spacedef.h"
#include "debugdef.h"
#include "fhandledef.h"

#include "filehandle.pr"

#define ADDUNIT(A,B) ((A) == (B) ? 0 : 1)
#define GAP          ((Uchar) '-')

#ifdef DEBUG
#define FWRITE(SEQ,LEN)\
        if(WRITETOFILEHANDLE(SEQ,(Uint) sizeof(Uchar),LEN,stdout) != 0)\
        {\
          fprintf(stderr,"%s\n",messagespace());\
          exit(EXIT_FAILURE);\
        }
#endif

static Uint distance;
static BOOL distanceset;

#ifdef DEBUG
static Uint evaluatealignment(Uchar *ali1,Uchar *ali2,Uint lenalg)
{
  Uint pos, differences;

  printf("\"");
  for(pos = 0; pos < lenalg; pos++)
  {
    (void) putchar((Fputcfirstargtype) ali1[pos]);
  }
  printf("\"\n\"");
  for(pos = 0; pos < lenalg; pos++)
  {
    (void) putchar((Fputcfirstargtype) ali2[pos]);
  }
  printf("\"\n\n");
  for(differences=0,pos=0; pos < lenalg; pos++) /* compute alignment distance */
  {
    if(ali1[pos] != ali2[pos])
    {
      DEBUG1(1,"pos %lu differs\n",(Showuint) pos);
      differences++;
    }
  }
  return differences;
}
#endif

static void evaluateedtabrtab(Uchar *u,
                             Uchar *v,
                             Uint ulen,
                             Uint vlen,
                             Uint colmid,
                             Uint *edtab,
                             Uint *rtab)
{
  Uint rowindex, colindex, val, nwedtab, weedtab, nwrtab, wertab;

  for(rowindex=0; rowindex<=ulen; rowindex++)
  {
    edtab[rowindex] = rowindex;
    rtab[rowindex] = rowindex;
  }
  for(colindex=UintConst(1); colindex<=colmid; colindex++)
  {
    weedtab = edtab[0];
    edtab[0] = colindex;
    for(rowindex=UintConst(1); rowindex<=ulen; rowindex++)
    {
      nwedtab = weedtab;
      weedtab = edtab[rowindex];
      edtab[rowindex]++;
      if((val=nwedtab+ADDUNIT(u[rowindex-1],v[colindex-1])) < edtab[rowindex])
      {
        edtab[rowindex] = val;
      }
      if((val=edtab[rowindex-1]+1) < edtab[rowindex])
      {
        edtab[rowindex] = val;
      }
    }
  }
  /*
    Now colindex = colmid + 1
  */
  for(/* Nothing */ ; colindex<=vlen; colindex++)
  {
    weedtab = edtab[0];
    wertab = 0;
    edtab[0] = colindex;
    rtab[0] = 0;
    for(rowindex=UintConst(1); rowindex<=ulen; rowindex++)
    {
      nwedtab = weedtab;
      nwrtab = wertab;
      weedtab = edtab[rowindex];
      wertab = rtab[rowindex];
      edtab[rowindex]++;
      // rtab[rowindex] is unchanged
      if((val=nwedtab+ADDUNIT(u[rowindex-1],v[colindex-1])) < edtab[rowindex])
      {
        edtab[rowindex] = val;
        rtab[rowindex] = nwrtab;
      }
      if((val=edtab[rowindex-1]+1) < edtab[rowindex])
      {
        edtab[rowindex] = val;
        rtab[rowindex] = rtab[rowindex-1];
      }
    }
  }
}

static void recursivelyevaluatecrossrows(Uchar *u,
                                         Uchar *v,
                                         Uint ulen,
                                         Uint vlen,
                                         Uint *edtab,
                                         Uint *rtab,
                                         Uint *crossrows,
                                         Uint offset)
{
  Uint midrow, midcol;

  if(vlen >= UintConst(2))
  {
    midcol = DIV2(vlen);
    evaluateedtabrtab(u,v,ulen,vlen,midcol,edtab,rtab);
    midrow = rtab[ulen];
    if(!distanceset)
    {
      distance = edtab[ulen];
      distanceset = True;
    }
    crossrows[midcol] = offset + midrow;
    recursivelyevaluatecrossrows(u,
                                 v,
                                 midrow,
                                 midcol,
                                 edtab,
                                 rtab,
                                 crossrows,
                                 offset);
    recursivelyevaluatecrossrows(u+midrow,
                                 v+midcol,
                                 ulen-midrow,
                                 vlen-midcol,
                                 edtab,
                                 rtab,
                                 crossrows+midcol,
                                 offset+midrow);
  } 
}

static Uint determinecrossrow0(Uint *crossrow,
                               Uchar v0,
                               Uchar *u)
{
  Uint rowindex;

  for(rowindex=0; rowindex < crossrow[1]; rowindex++)
  {
    if(v0 == u[rowindex])
    {
      crossrow[0] = rowindex;
      return crossrow[1] - 1;
    }
  }
  if(crossrow[1] > 0)
  {
    crossrow[0] = crossrow[1]-1;
  } else
  {
    crossrow[0] = 0;
  }
  return crossrow[1];
}


/*
  Assumption: ulen > 0 and vlen > 0
*/

static Uint linearspacedivideandconquer(Uchar *u,
                                        Uchar *v,
                                        Uint ulen,
                                        Uint vlen,
                                        Uchar *ali1,
                                        Uchar *ali2)
{
  Uint lenalg, rowindex, colindex, *edtab, *rtab, *crossrows;

  ALLOCASSIGNSPACE(edtab,NULL,Uint,ulen+1);  // store distance value
  ALLOCASSIGNSPACE(rtab,NULL,Uint,ulen+1);  
  ALLOCASSIGNSPACE(crossrows,NULL,Uint,vlen+1);
  if(vlen == UintConst(1))
  {
    crossrows[1] = ulen;
    distance = determinecrossrow0(crossrows,v[0],u);
    distanceset = True;
  } else
  {
    recursivelyevaluatecrossrows(u,v,ulen,vlen,edtab,rtab,crossrows,0);
    crossrows[vlen] = ulen;
    (void) determinecrossrow0(crossrows,v[0],u);
  }
  /* reconstruct upper alignment sequence */
  for(lenalg=0,colindex=0,rowindex=0; colindex<vlen; colindex++) 
  {
    // remain in same column, no advance in u
    if(crossrows[colindex] == crossrows[colindex+1]) 
    {
      ali1[lenalg++] = (Uchar) GAP;
    }
    while(rowindex < crossrows[colindex+1])
    {
      ali1[lenalg++] = u[rowindex++];
    }
  }
  /* reconstruct lower alignment sequence */
  for(lenalg=0,rowindex=0,colindex=0; colindex<=vlen; colindex++) 
  {
    while(rowindex < crossrows[colindex])
    {
      ali2[lenalg++] = (Uchar) GAP;
      rowindex++;
    }
    if(colindex < vlen)
    {
      ali2[lenalg++] = v[colindex];
      rowindex = crossrows[colindex]+1;
    }
  }
  FREESPACE(edtab);
  FREESPACE(rtab);
  FREESPACE(crossrows);
  return lenalg;
}

Uint alignlinearspace(Uchar *u,
                      Uchar *v,
                      Uint ulen,
                      Uint vlen,
                      Uchar *ali1,
                      Uchar *ali2)
{
  Uint lenalg;

#ifdef DEBUG
  Uint differences;
  printf("align \"");
  FWRITE(u,ulen);
  printf("\" \"");
  FWRITE(v,vlen);
  printf("\"\n");
#endif
  distanceset = False;
  if(vlen == 0)
  {
    for(lenalg=0; lenalg<ulen; lenalg++) 
    {
      ali1[lenalg] = u[lenalg];
      ali2[lenalg] = GAP;
    }
    distanceset = True;
    distance = ulen;
  } else
  {
    if(ulen == 0)
    {
      for(lenalg=0; lenalg<vlen; lenalg++) 
      {
        ali1[lenalg] = GAP;
        ali2[lenalg] = v[lenalg];
      }
      distanceset = True;
      distance = vlen;
    } else
    {
      lenalg = linearspacedivideandconquer(u,v,ulen,vlen,ali1,ali2);
    }
  }
  if(!distanceset)
  {
    fprintf(stderr,"distance is not set yet\n");
    exit(EXIT_FAILURE);
  }
#ifdef DEBUG
  differences = evaluatealignment(ali1,ali2,lenalg);
  if(differences != distance)
  {
    fprintf(stderr,"differences = %lu != %lu = distance\n",
           (Showuint) differences,(Showuint) distance);
    exit(EXIT_FAILURE);
  }
#endif
  return distance;
}
