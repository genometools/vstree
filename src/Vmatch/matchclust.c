#include "clusterdef.h"
#include "mcldef.h"
#include "fhandledef.h"
#include "outinfo.h"

#include "cluster.pr"
#include "echomatch.pr"
#include "filehandle.pr"

static Sint showclnum(void *info,Uint clnum,Uint clsize)
{
  Mclinfo *mclinfo = (Mclinfo *) info;

  printf("# create cluster %lu of size %lu\n",
          (Showuint) clnum,
          (Showuint) clsize);
  sprintf(&mclinfo->clusterfilename[0],"%s.%lu.%lu.match",
          &mclinfo->clusterfilenameprefix[0],
          (Showuint) clsize,
          (Showuint) clnum);
  mclinfo->matchfileoutptr 
    = CREATEFILEHANDLE(&mclinfo->clusterfilename[0],WRITEMODE);
  if(mclinfo->matchfileoutptr == NULL)
  {
    return (Sint) -1;
  }
  fprintf(mclinfo->matchfileoutptr,"%s%s\n",ARGUMENTLINEPREFIX,mclinfo->mfargs);
  return 0;
}

static Sint showclelem(void *info,Uint matchnum)
{
  Uint showmode = SHOWDIRECT;
  Uint showstring = 0;
  Showdescinfo defaultshowdesc;
  Digits defaultdigits;
  Mclinfo *mclinfo = (Mclinfo *) info;

  defaultshowdesc.defined = False;
  ASSIGNDEFAULTDIGITS(&defaultdigits);
  fprintf(mclinfo->matchfileoutptr,"# id %lu\n",
          (Showuint) mclinfo->matchtab->spaceStoreMatch[matchnum].idnumber);
  if(echomatch2file(mclinfo->matchfileoutptr,
                    False,
                    showmode,
                    &defaultshowdesc,
                    showstring,
                    &defaultdigits,
                    mclinfo->virtualmultiseq,
                    mclinfo->querymultiseq,
                    mclinfo->virtualalpha,
                    mclinfo->matchtab->spaceStoreMatch+matchnum) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint showclend(void *info,/*@unused@*/ Uint clnum)
{
  Mclinfo *mclinfo = (Mclinfo *) info;

  if(DELETEFILEHANDLE(mclinfo->matchfileoutptr) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint showedge(void *info1,void *info2,Uint i)
{
  Mclinfo *mclinfo = (Mclinfo *) info1;
  Matchedge *matchedge = (Matchedge *) info2;
  StoreMatch *sptr;

  sptr = mclinfo->matchtab->spaceStoreMatch+matchedge[i].matchnum0;
  fprintf(mclinfo->matchfileoutptr,"# linked %lu ",(Showuint) sptr->idnumber);
  sptr = mclinfo->matchtab->spaceStoreMatch+matchedge[i].matchnum1;
  fprintf(mclinfo->matchfileoutptr,"and %lu with ",(Showuint) sptr->idnumber);
  if(mclinfo->mcllinkinfo(info1,matchedge+i) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

Sint domatchclustering(Uint numofmatches,
                       Mclinfo *mclinfo)
{
  ClusterSet clusterset;
  Uint edgenum;

  initClusterSet(&clusterset,numofmatches);
  printf("# cluster %lu matches\n",(Showuint) numofmatches);
  for(edgenum=0; edgenum < mclinfo->matchedgetab.nextfreeMatchedge; edgenum++)
  {
    if(linkcluster(&clusterset,
                   mclinfo->matchedgetab.spaceMatchedge[edgenum].matchnum0,
                   mclinfo->matchedgetab.spaceMatchedge[edgenum].matchnum1) 
                   != 0)
    {
      return (Sint) -1;
    }
  }
  for(edgenum=0; edgenum<mclinfo->matchedgetab.nextfreeMatchedge; edgenum++)
  {
    if(addClusterEdge(&clusterset,
                      mclinfo->matchedgetab.spaceMatchedge[edgenum].matchnum0,
                      mclinfo->matchedgetab.spaceMatchedge[edgenum].matchnum1,
                      edgenum) != 0)
    {
      return (Sint) -2;
    }
  }
  if(showClusterSet(&clusterset,
                    0,
                    MAXCLUSTERSIZE(&clusterset),
                    (void *) mclinfo,showclnum,
                    (void *) mclinfo,showclelem,
                    (void *) mclinfo,showclend,
                    (void *) mclinfo->matchedgetab.spaceMatchedge,showedge) 
                    != 0)
  {
    return (Sint) -3;
  }
  freeClusterSet(&clusterset);
  return 0;
}
