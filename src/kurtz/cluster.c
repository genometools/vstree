//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "types.h"
#include "intbits.h"
#include "debugdef.h"
#include "spacedef.h"
#include "errordef.h"
#include "clusterdef.h"

#include "seterror.pr"

//}


/*
  \texttt{SINGLETON(E)} is true iff the element \texttt{E} is in its own
  cluster. \texttt{CINFO(I)} is just an abbreviation for record storing
  information for the cluster with number \texttt{I}.
*/

#define SINGLETON(E)  !ISIBITSET(clusterset->alreadyincluster,E)
#define CINFO(I)      (clusterset->cinfo.spaceClusterInfo[I])

/*
  An element to put into a cluster must always be a number in the range
  \texttt{[0..clusterset->numofelems-1]}.
*/

//\Ignore{

#ifdef DEBUG
#define DEBUGCHECKELEM(E)\
        if((E) >= clusterset->numofelems)\
        {\
          ERROR2("elem number %lu is too large, must be smaller than %lu",\
                  (Showuint) (E),\
                  (Showuint) clusterset->numofelems);\
          return (Sint) -1;\
        }

/*
  A cluster number must always be in the range 
  \texttt{[0..clusterset->cinfo.nextfreeClusterInfo-1]}.
*/

#define DEBUGCHECKCLUSTER(I)\
        if((I) >= clusterset->cinfo.nextfreeClusterInfo)\
        {\
          ERROR2("cluster %lu is too large, must be smaller than %lu",\
                  (Showuint) (I),\
                  (Showuint) clusterset->cinfo.nextfreeClusterInfo);\
          return (Sint) -1;\
        }\
        if(CINFO(I).csize == 0)\
        {\
          ERROR1("cluster %lu is empty",(Showuint) (I));\
          return (Sint) -1;\
        }


#define DEBUGADDELEM(I)\
        if(ISIBITSET(occurselem,I))\
        {\
          ERROR1("element %lu already occurs",(Showuint) (I));\
          return (Sint) -1;\
        } \
        SETIBIT(occurselem,I);\
        occurselemcount++

#define DEBUGADDEDGE(I)\
        if(ISIBITSET(occursedge,I))\
        {\
          ERROR1("edge %lu already occurs",(Showuint) (I));\
          return (Sint) -1;\
        } \
        SETIBIT(occursedge,I);\
        occursedgecount++

#else
#define DEBUGADDELEM(I)      /* Nothing */
#define DEBUGADDEDGE(I)      /* Nothing */
#define DEBUGCHECKELEM(E)    /* Nothing */
#define DEBUGCHECKCLUSTER(I) /* Nothing */
#endif

//}

/*EE
  This file implements the data type \texttt{ClusterSet} for 
  single linkage clustering.
  The following function shows a set of clusters represented
  by \texttt{clusterset}. However, the corresponding cluster files
  are only output if the size of the cluster is between \texttt{clminsize}
  and \texttt{clmaxsize}. \texttt{showClusterSet}
  is parameterized by functions to output the clusters. 
  \begin{itemize}
  \item
  The function 
  \texttt{showclusterstart} is called for each cluster before
  any element of the cluster is processed. The arguments are
  the pointer \texttt{infocstart}, number of the cluster, and its size.
  \item
  The function \texttt{showelem} is called for each element of the cluster. 
  The arguments are \texttt{info1} and the element \texttt{i} in the cluster.
  \item
  The function 
  \texttt{showclusterend} is called for each cluster after
  all element of the cluster have been processed. The arguments are
  the pointer \texttt{infocend} and the number of the cluster.
  \item
  For each edge between two elements of the same cluster,
  the function \texttt{showedge} is called. The first argument 
  is \texttt{info1}, the second is \texttt{info2}, and the last argument
  is the number of the edge. 
  \end{itemize}
  The function returns a negative return code in case of failure.
  In case of success, 0 is returned.
*/

Sint showClusterSet(ClusterSet *clusterset,
                    Uint clminsize,Uint clmaxsize,
                    void *infocstart,
                    Sint (*showclusterstart)(void *,Uint,Uint),
                    void *info1,     
                    Sint (*showelem)(void *,Uint),
                    void *infocend,  
                    Sint (*showclusterend)(void *,Uint),
                    void *info2,     
                    Sint (*showedge)(void *,void *,Uint))
{
  Uint cnum, start, end, i, j, nonemptycnum = 0, csize;

  for(cnum=0; cnum < clusterset->cinfo.nextfreeClusterInfo; cnum++)
  {
    csize = CINFO(cnum).csize;
    if(csize > 0)
    {
      if(csize >= clminsize && csize <= clmaxsize)
      {
        if(showclusterstart != NULL)
        {
          if(showclusterstart(infocstart,nonemptycnum,CINFO(cnum).csize) != 0)
          {
            return (Sint) -1;
          }
        }
        if(showelem != NULL)
        {
          j=CINFO(cnum).firstelem;
          while(True)
          {
            if(showelem(info1,j) != 0)
            {
              return (Sint) -3;
            }
            if((j = clusterset->celems[j].nextelem) == CLUSTERNIL)
            {
              /*@innerbreak@*/ break;
            }
          }
        }
        if(showedge != NULL)
        {
          start = CINFO(cnum).startedges;
          if(cnum < clusterset->cinfo.nextfreeClusterInfo - 1)
          {
            end = CINFO(cnum+1).startedges-1;
          } else
          {
            end = clusterset->numofedges-1;
          }
          for(i=start; i<=end; i++)
          {
           if(showedge(info1,info2,clusterset->cedges[i]) != 0)
           {
             return (Sint) -2;
           }
          }
        }
        if(showclusterend != NULL)
        {
          if(showclusterend(infocend,nonemptycnum) != 0)
          {
            return (Sint) -4;
          }
        }
      }
      nonemptycnum++;
    }
  }
  return 0;
}

Sint showClusterSetwithmaxelem(ClusterSet *clusterset,
                               Uint clminsize,Uint clmaxsize,
                               void *infocstart,
                               Sint (*showclusterstart)(void *,Uint,Uint),
                               void *cmpinfo,
                               Sint (*cmpelems)(void *,Uint,Uint),
                               void *info1,     
                               Sint (*showelem)(void *,Uint),
                               void *infocend,  
                               Sint (*showclusterend)(void *,Uint),
                               void *info2,     
                               Sint (*showedge)(void *,void *,Uint))
{
  Uint cnum, start, end, i, j, nonemptycnum = 0, csize, maxelem;
  Sint ret;

  for(cnum=0; cnum < clusterset->cinfo.nextfreeClusterInfo; cnum++)
  {
    csize = CINFO(cnum).csize;
    if(csize > 0)
    {
      if(csize >= clminsize && csize <= clmaxsize)
      {
        if(showclusterstart != NULL)
        {
          if(showclusterstart(infocstart,nonemptycnum,CINFO(cnum).csize) != 0)
          {
            return (Sint) -1;
          }
        }
        if(showelem != NULL)
        {
          j=CINFO(cnum).firstelem;
          maxelem = j;
          while(True)
          {
            j = clusterset->celems[j].nextelem;
            if(j == CLUSTERNIL)
            {
              /*@innerbreak@*/ break;
            }
            ret = cmpelems(cmpinfo,maxelem,j);
            /*printf("ret=%ld\n",(long) ret);*/
            if (ret == (Sint) -2) /* an error occurred */
            {
              return (Sint) -1;
            }
            if (ret == (Sint) -1) /* elem j is smaller than maxelem */
            {
              maxelem = j;
            }
          }
          /*printf("show elem %lu\n",(unsigned long) maxelem);*/
          if(showelem(info1,maxelem) != 0)
          {
            return (Sint) -3;
          }
        }
        if(showedge != NULL)
        {
          start = CINFO(cnum).startedges;
          if(cnum < clusterset->cinfo.nextfreeClusterInfo - 1)
          {
            end = CINFO(cnum+1).startedges-1;
          } else
          {
            end = clusterset->numofedges-1;
          }
          for(i=start; i<=end; i++)
          {
           if(showedge(info1,info2,clusterset->cedges[i]) != 0)
           {
             return (Sint) -2;
           }
          }
        }
        if(showclusterend != NULL)
        {
          if(showclusterend(infocend,nonemptycnum) != 0)
          {
            return (Sint) -4;
          }
        }
      }
      nonemptycnum++;
    }
  }
  return 0;
}

Sint showSingletonSet(ClusterSet *clusterset,
                      void *info1,Sint (*showelem)(void *,Uint))
{
  Uint i;

  for(i=0; i < clusterset->numofelems; i++)
  {
    if(SINGLETON(i))
    {
      if(clusterset->subclusterelems == NULL ||
         ISIBITSET(clusterset->subclusterelems,i))
      {
        if(showelem(info1,i) != 0)
        {
          return (Sint) -1;
        }
      }
    }
  }
  return 0;
}

//\Ignore{

#ifdef DEBUG
Sint checkClusterSet(ClusterSet *clusterset,void *info,
                     Sint (*checkrel)(void *,Uint,Uint))
{
  Uint cnum, i, j, e, occurselemcount = 0, *occurselem,
       start, end, occursedgecount = 0, *occursedge;

  INITBITTAB(occurselem,clusterset->numofelems);
  INITBITTAB(occursedge,clusterset->numofedges);
  for(cnum=0; cnum < clusterset->cinfo.nextfreeClusterInfo; cnum++)
  {
    i = CINFO(cnum).firstelem;
    if(i != CLUSTERNIL)
    {
      while(True)
      {
        DEBUGADDELEM(i);
        j = CINFO(cnum).firstelem;
        if(j != CLUSTERNIL)
        {
          while(True)
          {
            if(checkrel(info,i,j) != 0)
            {
              ERROR2("relation of %lu and %lu does not hold",(Showuint) i,
                                                             (Showuint) j);
              return (Sint) -1;
            }
            if((j = clusterset->celems[j].nextelem) == CLUSTERNIL)
            {
              /*@innerbreak@*/ break;
            }
          }
        }
        if((i = clusterset->celems[i].nextelem) == CLUSTERNIL)
        {
          /*@innerbreak@*/ break;
        }
      }
    }
    start = CINFO(cnum).startedges;
    if(cnum < clusterset->cinfo.nextfreeClusterInfo - 1)
    {
      end = CINFO(cnum+1).startedges-1;
    } else
    {
      end = clusterset->numofedges-1;
    }
    for(e=start; e<=end; e++)
    {
      DEBUGADDEDGE(clusterset->cedges[e]);
    }
  }
  for(i=0; i < clusterset->numofelems; i++)
  {
    if(SINGLETON(i))
    {
      DEBUGADDELEM(i);
    }
  }
  if(occurselemcount != clusterset->numofelems)
  {
    ERROR2("only %lu of %lu elements occur",(Showuint) occurselemcount,
                                            (Showuint) clusterset->numofelems);
    return (Sint) -1;
  }
  FREESPACE(occurselem);
  if(occursedgecount != clusterset->numofedges)
  {
    ERROR2("only %lu of %lu edges occur",(Showuint) occursedgecount,
                                         (Showuint) clusterset->numofedges);
    return (Sint) -1;
  }
  FREESPACE(occursedge);
  return 0;
}
#endif

//}

/*EE
  The following function initialized a cluster set with 
  \texttt{numofelems} many elements. In particular, each 
  of the elements \texttt{[0...numofelems-1]} is stored in its own
  cluster. The cluster elements \texttt{[0...numofelems-1]} can thus be
  interpreted as addresses to refer to some more complicated elements.
*/

void initClusterSet(ClusterSet *clusterset,Uint numofelems)
{
  ALLOCASSIGNSPACE(clusterset->celems,NULL,ClusterElem,numofelems);
  clusterset->numofelems = numofelems;
  clusterset->numofedges = 0;
  clusterset->cedges = NULL;
  clusterset->subclusterelemcount = 0;
  clusterset->subclusterelems = NULL;
  INITBITTAB(clusterset->alreadyincluster,numofelems);
  INITARRAY(&clusterset->cinfo,ClusterInfo);
}

/*EE
  The following function returns \texttt{True} iff cluster element with
  number $i$ is singleton, i.e.\ it is not in any cluster.
*/

BOOL isinsingleton(ClusterSet *clusterset,Uint i)
{
  if(i >= clusterset->numofelems)
  {
    fprintf(stderr,
            "reference to element %lu when clustering elements [0,%lu]\n",
            (Showuint) i,(Showuint) (clusterset->numofelems-1));
    exit(EXIT_FAILURE);
  }
  return SINGLETON(i) ? True : False;
}

/*
  The following function changes the cluster number of all elements in 
  a cluster to the number \texttt{target}. \texttt{firstelem} is the
  address of the first element in the cluster.
*/

static void changeclusternumber(ClusterSet *clusterset,Uint firstelem,
                                ClusterElem *celems,Uint target)
{
  Uint i;

  i=firstelem;
  while(True)
  {
    celems[i].clusternumber = target; // add to target cluster
    if((i = celems[i].nextelem) == CLUSTERNIL)
    {
      break;
    }
  }
}

/*
  The following function takes two elements, which are alone in their
  cluster and forms a new cluster consisting of exactly these
  elements.
*/

static void makenewcluster(ClusterSet *clusterset,Uint e1,Uint e2)
{
  Uint cnum;

  DEBUG2(2,"makenewcluster(e1=%lu,e2=%lu)\n",(Showuint) e1,(Showuint) e2);
  CHECKARRAYSPACE(&clusterset->cinfo,ClusterInfo,128);
  cnum = clusterset->cinfo.nextfreeClusterInfo++;
  clusterset->celems[e1].clusternumber = cnum;
  clusterset->celems[e2].clusternumber = cnum;
  clusterset->celems[e1].nextelem = e2;
  clusterset->celems[e2].nextelem = CLUSTERNIL;
  CINFO(cnum).csize = UintConst(2);
  CINFO(cnum).startedges = UintConst(1);
  CINFO(cnum).firstelem = e1;
  CINFO(cnum).lastelem = e2;
}

/*
  The following function puts \texttt{elem} into the cluster with number
  \texttt{cnum}.
*/

static void appendcluster(ClusterSet *clusterset,Uint cnum,Uint elem)
{
  DEBUG2(2,"appendcluster(cnum=%lu,elem=%lu)\n",(Showuint) cnum,
                                                (Showuint) elem);
  clusterset->celems[elem].clusternumber = cnum;
  clusterset->celems[elem].nextelem = CLUSTERNIL; 
  clusterset->celems[CINFO(cnum).lastelem].nextelem = elem;
  CINFO(cnum).lastelem = elem;
  CINFO(cnum).startedges++;
  CINFO(cnum).csize++;
}

/*
  The following function puts \texttt{elem} into the cluster with number
  \texttt{cnum}.
*/

static void linkmulticluster(ClusterSet *clusterset,Uint target,Uint source)
{
  DEBUG2(2,"linkmulticluster(target=%lu,source=%lu)\n",(Showuint) target,
                                                       (Showuint) source);
  changeclusternumber(clusterset,CINFO(source).firstelem,clusterset->celems,
                      target);
  clusterset->celems[CINFO(target).lastelem].nextelem = CINFO(source).firstelem;
  CINFO(source).firstelem = CLUSTERNIL;
  CINFO(target).lastelem = CINFO(source).lastelem;
  CINFO(target).csize += CINFO(source).csize;
  CINFO(target).startedges += (CINFO(source).startedges + 1);
  CINFO(source).csize = 0;
  CINFO(source).startedges = 0;
}

/*EE
  The following function implements single linkage clustering: 
  it combines the two clusters containing the elements \texttt{e1} and 
  \texttt{e2}.
*/

Sint linkcluster(ClusterSet *clusterset,Uint e1,Uint e2)
{
  Uint c1, c2, target, source;

  DEBUG2(2,"linkcluster %lu %lu\n",(Showuint) e1,(Showuint) e2);
  DEBUGCHECKELEM(e1);
  DEBUGCHECKELEM(e2);
  clusterset->numofedges++;
  if(SINGLETON(e1))
  {
    DEBUG1(3,"e1=%lu is singleton\n",(Showuint) e1);
    if(SINGLETON(e2))
    {
      DEBUG1(3,"e2=%lu is singleton\n",(Showuint) e2);
      makenewcluster(clusterset,e1,e2);
      SETIBIT(clusterset->alreadyincluster,e2);
    } else
    {
      c2 = clusterset->celems[e2].clusternumber;
      DEBUGCHECKCLUSTER(c2);
      DEBUG2(3,"e2=%lu in cluster %lu\n",(Showuint) e2,(Showuint) c2);
      appendcluster(clusterset,c2,e1);
    }
    SETIBIT(clusterset->alreadyincluster,e1);
  } else
  {
    c1 = clusterset->celems[e1].clusternumber;
    DEBUGCHECKCLUSTER(c1);
    DEBUG2(3,"e1=%lu in cluster %lu\n",(Showuint) e1,(Showuint) c1);
    if(SINGLETON(e2))
    {
      DEBUG1(3,"e2=%lu is singleton\n",(Showuint) e2);
      appendcluster(clusterset,c1,e2);
      SETIBIT(clusterset->alreadyincluster,e2);
    } else
    {
      DEBUG2(3,"e2=%lu in cluster %lu\n",
             (Showuint) e2,
             (Showuint) clusterset->celems[e2].clusternumber);
      c2 = clusterset->celems[e2].clusternumber;
      DEBUGCHECKCLUSTER(c2);
      if(c1 == c2)
      {
        CINFO(c1).startedges++;
      } else
      {
        if(CINFO(c1).csize > CINFO(c2).csize) 
        {
          target = c1;  // choose larger cluster to be the target of the join
          source = c2;
        } else
        {
          source = c1;
          target = c2;
        }
        linkmulticluster(clusterset,target,source);
      } 
    }
  }
  return 0;
}

/*EE
  The following function adds the edge number \texttt{edgenum} to the
  cluster containing the elements \texttt{e1} and \texttt{e2}.
  The function can only be called after all clusters have been computed.
*/

Sint addClusterEdge(ClusterSet *clusterset,Uint e1,Uint e2,Uint edgenum)
{
  Uint cnum, c1, c2;

  DEBUG3(2,"addClusterEdge(%lu,%lu,%lu)\n",(Showuint) e1,(Showuint) e2,
                                           (Showuint) edgenum);
  if(clusterset->cedges == NULL)
  {
    ALLOCASSIGNSPACE(clusterset->cedges,NULL,Uint,clusterset->numofedges);
    for(cnum=UintConst(1); 
        cnum < clusterset->cinfo.nextfreeClusterInfo; cnum++)
    {
      CINFO(cnum).startedges += CINFO(cnum-1).startedges;
    }
  }
  DEBUGCHECKELEM(e1);
  DEBUGCHECKELEM(e2);
  c1 = clusterset->celems[e1].clusternumber;
  c2 = clusterset->celems[e2].clusternumber;
  if(c1 != c2)
  {
    ERROR2("elem %lu and %lu do not belong to the same cluster",
             (Showuint) e1,
             (Showuint) e2);
    return (Sint) -1;
  }
  clusterset->cedges[--(CINFO(c1).startedges)] = edgenum;
  return 0;
}

#define ADDAMOUNT 32

static void addclusterdistribution(ArrayUint *dist,Uint ind)
{
  Uint i;

  if(ind >= dist->allocatedUint)
  {
    ALLOCASSIGNSPACE(dist->spaceUint,dist->spaceUint,Uint,ind+ADDAMOUNT);
    for(i=dist->allocatedUint; i<ind+ADDAMOUNT; i++)
    {
      dist->spaceUint[i] = 0;
    }
    dist->allocatedUint = ind+ADDAMOUNT;
  }
  if(ind + 1 > dist->nextfreeUint)
  {
    dist->nextfreeUint = ind+1;
  }
  dist->spaceUint[ind]++;
}

void clusterSizedistribution(ClusterSet *clusterset)
{
  Uint realall, i, cnum, nonemptycnum = 0, csum = 0, tmpcsize, singlets;
  ArrayUint distribution;

  INITARRAY(&distribution,Uint);
  for(cnum=0; cnum < clusterset->cinfo.nextfreeClusterInfo; cnum++)
  {
    tmpcsize = CINFO(cnum).csize;
    csum += tmpcsize;
    if(tmpcsize >= UintConst(2))
    {
      nonemptycnum++;
      addclusterdistribution(&distribution,tmpcsize);
    }
  }
  if(clusterset->subclusterelems == NULL)
  {
    realall = clusterset->numofelems;
  } else
  {
    realall = clusterset->subclusterelemcount;
  }
  printf("# %lu cluster%s\n",(Showuint) nonemptycnum,
                             nonemptycnum == UintConst(1) ? "" : "s");
  printf("# %lu elements out of %lu (%.2f%%) are in clusters\n",
          (Showuint) csum,(Showuint) realall,100.0 * (double) csum/realall);
  singlets = realall - csum;
  printf("# %lu elements out of %lu (%.2f%%) are singlets\n",
          (Showuint) singlets,(Showuint) realall,
          100.0 * (double) singlets/realall);
  NOTSUPPOSEDTOBENULL(distribution.spaceUint);
  for(i=UintConst(2); i< distribution.nextfreeUint; i++)
  {
    if(distribution.spaceUint[i] > 0)
    {
      printf("# %lu cluster",(Showuint) distribution.spaceUint[i]);
      if(distribution.spaceUint[i] > UintConst(1))
      {
        (void) putchar('s');
      }
      printf(" of size %lu\n",(Showuint) i);
    }
  }
  FREEARRAY(&distribution,Uint);
}

/*EE
  The following function frees a cluster set.
*/

void freeClusterSet(ClusterSet *clusterset)
{
  FREESPACE(clusterset->celems);
  FREEARRAY(&clusterset->cinfo,ClusterInfo);
  FREESPACE(clusterset->alreadyincluster);
  FREESPACE(clusterset->cedges);
  clusterset->numofelems = 0;
  clusterset->numofedges = 0;
}
