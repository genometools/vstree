#include "divmodmul.h"
#include "debugdef.h"
#include "mcldef.h"

#include "matchclust.pr"
#include "safescpy.pr"

#define STORECLUSTEREDGE(I,J,VALUE)\
        GETNEXTFREEINARRAY(matchedgeptr,&mclinfo.matchedgetab,Matchedge,128);\
        matchedgeptr->matchnum0 = I;\
        matchedgeptr->matchnum1 = J;\
        matchedgeptr->edgedata.VALUE = VALUE

typedef struct
{
  Uint startmatch,
       matchnum;
} Matchreference;

static Qsortcomparereturntype cmpmatchreferences (Matchreference *amref,
                                                  Matchreference *bmref)
{
  if(amref->startmatch < bmref->startmatch)
  {
    return (Qsortcomparereturntype) -1;
  } 
  if(amref->startmatch > bmref->startmatch)
  {
    return (Qsortcomparereturntype) 1;
  }
  return 0;
}

static Matchreference *mirrorandsortmatches(const ArrayStoreMatch *matchtab)
{
  Matchreference *mref;
  Uint i, j;

  ALLOCASSIGNSPACE(mref,NULL, Matchreference, 
                   MULT2(matchtab->nextfreeStoreMatch));
  for(i=0, j=0; i < matchtab->nextfreeStoreMatch; i++, j+=2)
  {
    mref[j].startmatch = matchtab->spaceStoreMatch[i].Storeposition1;
    mref[j].matchnum = i;
    mref[j+1].startmatch = matchtab->spaceStoreMatch[i].Storeposition2;
    mref[j+1].matchnum = i;
  }
  qsort(mref, (size_t) MULT2(matchtab->nextfreeStoreMatch),
        sizeof(Matchreference),  (Qsortcomparefunction) cmpmatchreferences); 
  return mref;
}

static Sint gapmlclinkinfo(void *info,Matchedge *matchedge)
{
  Mclinfo *mclinfo = (Mclinfo *) info;

  fprintf(mclinfo->matchfileoutptr,
          "gapsize %lu\n",(Showuint) matchedge->edgedata.gapsize);
  return 0;
}

static Sint overlapmlclinkinfo(void *info,Matchedge *matchedge)
{
  Mclinfo *mclinfo = (Mclinfo *) info;

  fprintf(mclinfo->matchfileoutptr,
          "overlap percentage %.2f\n",
          matchedge->edgedata.overlap);
  return 0;
}

Sint gapclustermatches(Multiseq *virtualmultiseq,
                       Alphabet *virtualalpha,
                       Multiseq *querymultiseq,
                       ArrayStoreMatch *matchtab,
                       char *outprefix,
                       char *mfargs,
                       Uint maxgapsize)
{
  Uint i, j, gapsize;
  Matchreference *mref;
  Matchedge *matchedgeptr;
  Mclinfo mclinfo;

  INITARRAY(&mclinfo.matchedgetab,Matchedge);
  mref = mirrorandsortmatches(matchtab);
  for(i = 0; i < MULT2(matchtab->nextfreeStoreMatch) - 1; i++)
  {
    for(j = i + 1; j < MULT2(matchtab->nextfreeStoreMatch); j++)
    {
      gapsize = mref[j].startmatch - 
                (mref[i].startmatch + 
                 matchtab->spaceStoreMatch[mref[i].matchnum].Storelength1);
      if(gapsize > maxgapsize)
      {
        break;
      } 
      if(mref[i].matchnum != mref[j].matchnum)
      {
        DEBUG4(1, "link: (%lu) %lu <-> %lu (%lu)\n",
               (Showuint) mref[i].startmatch,
               (Showuint) mref[i].matchnum, 
               (Showuint) mref[j].matchnum,
               (Showuint) mref[j].startmatch);
        STORECLUSTEREDGE(mref[i].matchnum,mref[j].matchnum,gapsize);
      }
    }
  }
  mclinfo.virtualmultiseq = virtualmultiseq;
  mclinfo.querymultiseq = querymultiseq;
  mclinfo.matchtab = matchtab;
  mclinfo.mfargs = mfargs;
  mclinfo.virtualalpha = virtualalpha;
  mclinfo.mcllinkinfo = gapmlclinkinfo;
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
  FREESPACE(mref);
  return 0;
}

Sint overlapclustermatches(Multiseq *virtualmultiseq,
                           Alphabet *virtualalpha,
                           Multiseq *querymultiseq,
                           ArrayStoreMatch *matchtab,
                           char *outprefix,
                           char *mfargs,
                           Uint minpercentoverlap)
{
  Uint i, j, longermatch;
  double overlap;
  Matchreference *mref;
  Matchedge *matchedgeptr;
  Mclinfo mclinfo;

  INITARRAY(&mclinfo.matchedgetab,Matchedge);
  mref = mirrorandsortmatches(matchtab);
  for(i = 0; i < MULT2(matchtab->nextfreeStoreMatch) - 1; i++)
  {
    for(j = i + 1; j < MULT2(matchtab->nextfreeStoreMatch); j++)
    {
      if(mref[i].startmatch
          + matchtab->spaceStoreMatch[mref[i].matchnum].Storelength1
          < mref[j].startmatch)
      {
        break;
      } 
      if(mref[i].matchnum != mref[j].matchnum)
      {
        if(matchtab->spaceStoreMatch[mref[i].matchnum].Storelength1
           >= matchtab->spaceStoreMatch[mref[j].matchnum].Storelength1)
        {
          longermatch = i;
        } else
        {
          longermatch = j;
        }
        overlap = (double) 
                  ((mref[i].startmatch
                   + matchtab->spaceStoreMatch[mref[i].matchnum].Storelength1
                   - mref[j].startmatch)*100.0)
                   / matchtab->spaceStoreMatch[mref[longermatch].matchnum].
                     Storelength1;
        if(overlap >= (double) minpercentoverlap)
        {
          DEBUG4(1, "link: (%lu) %lu <-> %lu (%lu)\n",
                 (Showuint) mref[i].startmatch,
                 (Showuint) mref[i].matchnum, 
                 (Showuint) mref[j].matchnum,
                 (Showuint) mref[j].startmatch);
          STORECLUSTEREDGE(mref[i].matchnum,mref[j].matchnum,overlap); 
        }
      }
    }
  }
  mclinfo.virtualmultiseq = virtualmultiseq;
  mclinfo.querymultiseq = querymultiseq;
  mclinfo.matchtab = matchtab;
  mclinfo.mfargs = mfargs;
  mclinfo.virtualalpha = virtualalpha;
  mclinfo.mcllinkinfo = overlapmlclinkinfo;
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
  FREESPACE(mref);
  return 0;
}
