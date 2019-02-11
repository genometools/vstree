
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "chardef.h"
#include "galigndef.h"
#include "mcldef.h"

#include "frontSEP.pr"
#include "matchclust.pr"
#include "safescpy.pr"

#define STOREUEDISTCLUSTEREDGE(I,J,ML,ED)\
        GETNEXTFREEINARRAY(matchedgeptr,&mclinfo.matchedgetab,Matchedge,128);\
        matchedgeptr->matchnum0 = I;\
        matchedgeptr->matchnum1 = J;\
        matchedgeptr->edgedata.uedata.minlen = ML;\
        matchedgeptr->edgedata.uedata.edist = (Uint) ED

#define GETUEDIST(VAR,IVAL,JVAL,LEFT,RIGHT)\
        VAR = verifysmalldistance(virtualmultiseq->sequence +\
                          matchtab->spaceStoreMatch[IVAL].Storeposition##LEFT,\
                          matchtab->spaceStoreMatch[IVAL].Storelength##LEFT,\
                          virtualmultiseq->sequence +\
                          matchtab->spaceStoreMatch[JVAL].Storeposition##RIGHT,\
                          matchtab->spaceStoreMatch[JVAL].Storelength##RIGHT,\
                          maxdist)


/*
  Given a \texttt{multiseq} and a \texttt{matchtab}, the following
  function checks for all given matches if the edit distance 
  of a pair of matching sequences is at most \texttt{maxdist}. 
  If this is the case, then the corresponding sequence number is
  output as well as the corresponding edit distance.
*/

static Sint unitcheckedistall(Uchar *useq,Uint ulen,
                              Uchar *vseq,Uint vlen,Uint maxdist)
{
  Uint i;
  Sint edist;

  if(maxdist == 0)
  {
    if(ulen != vlen)
    {
      return (Sint) -1;
    }
    for(i=0; i<ulen; i++)
    {
      if(useq[i] != vseq[i] || ISSPECIAL(useq[i]) || ISSPECIAL(vseq[i]))
      {
        return (Sint) -1;
      }
    }
    return 0;
  }
  edist = unitedistfrontSEPgeneric(True,
                                   maxdist,
                                   useq,
                                   (Sint) ulen,
                                   vseq,
                                   (Sint) vlen);
  return edist;
}

static Sint verifysmalldistance(Uchar *seq1,
                                Uint len1,
                                Uchar *seq2,
                                Uint len2,
                                Uint maxdist)
{
  Uint difflen;
  Sint edist;

  //printf("verifysmalldistance for sequence of length %lu and %lu\n",
  //        (Showuint) len1,(Showuint) len2);
  if(len1 > len2)
  {
    difflen = len1 - len2;
  } else
  {
    if(len1 < len2)
    {
      difflen = len2 - len1;
    } else
    {
      if(seq1 == seq2)
      {
        return 0;
      }
      difflen = 0;
    }
  }
  if(difflen > maxdist)
  {
    return (Sint) -1;
  }
  edist = unitcheckedistall(seq1,len1,seq2,len2,maxdist);
  return edist;
}

static Sint cluedistmlclinkinfo(void *info,Matchedge *matchedge)
{
  Mclinfo *mclinfo = (Mclinfo *) info;

  fprintf(mclinfo->matchfileoutptr,
          "edit distance %lu (error rate %.2f%%)\n",
          (Showuint) matchedge->edgedata.uedata.edist,
          100.00 * (double) matchedge->edgedata.uedata.edist/
                            matchedge->edgedata.uedata.minlen);
  return 0;
}

Sint uedistcluster(Multiseq *virtualmultiseq,
                   Alphabet *virtualalpha,
                   Multiseq *querymultiseq,
                   ArrayStoreMatch *matchtab,
                   char *outprefix,
                   char *mfargs,
                   Uint errorrate)
{
  Uint i, j, len1, minlen, maxdist;
  Sint edist;
  Mclinfo mclinfo;
  Matchedge *matchedgeptr;

  INITARRAY(&mclinfo.matchedgetab,Matchedge);
  for(i=0; i < matchtab->nextfreeStoreMatch; i++)
  {
    len1 = matchtab->spaceStoreMatch[i].Storelength1;
    if(len1 > matchtab->spaceStoreMatch[i].Storelength2)
    {
      len1 = matchtab->spaceStoreMatch[i].Storelength2;
    }
    for(j=i+1; j < matchtab->nextfreeStoreMatch; j++)
    {
      minlen = matchtab->spaceStoreMatch[j].Storelength1;
      if(minlen > matchtab->spaceStoreMatch[j].Storelength2)
      {
        minlen = matchtab->spaceStoreMatch[j].Storelength2;
      }
      if(minlen > len1)
      {
        minlen = len1;
      }
      maxdist = (Uint) (minlen * (double) errorrate/100.0);
      GETUEDIST(edist,i,j,1,1);
      if(edist >= 0)
      {
        STOREUEDISTCLUSTEREDGE(i,j,minlen,edist);
      } else
      {
        GETUEDIST(edist,i,j,1,2);
        if(edist >= 0)
        {
          STOREUEDISTCLUSTEREDGE(i,j,minlen,edist);
        } else
        {
          GETUEDIST(edist,i,j,2,1);
          if(edist >= 0)
          {
            STOREUEDISTCLUSTEREDGE(i,j,minlen,edist);
          } else
          {
            GETUEDIST(edist,i,j,2,2);
            if(edist >= 0)
            {
              STOREUEDISTCLUSTEREDGE(i,j,minlen,edist);
            }
          }
        }
      }
    }
  }
  mclinfo.virtualmultiseq = virtualmultiseq;
  mclinfo.querymultiseq = querymultiseq;
  mclinfo.matchtab = matchtab;
  mclinfo.mfargs = mfargs;
  mclinfo.virtualalpha = virtualalpha;
  mclinfo.mcllinkinfo = cluedistmlclinkinfo;
  if(safestringcopy(&mclinfo.clusterfilenameprefix[0],
                    outprefix,PATH_MAX) != 0)
  {
    return (Sint) -1;
  }
  if(domatchclustering(matchtab->nextfreeStoreMatch,&mclinfo) != 0)
  {
    return (Sint) -2;
  }
  FREEARRAY(&mclinfo.matchedgetab,Matchedge);
  return 0;
}
