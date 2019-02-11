//\IgnoreLatex{

#ifndef CLUSTERDEF_H
#define CLUSTERDEF_H
#include "types.h"
#include "arraydef.h"

//}

/*
  This file defines some types and macros related to computing 
  single linkage clusters.
*/

/*
  A cluster is of maximal size if it contains all elements to be
  clustered.
*/

#define MAXCLUSTERSIZE(C) ((C)->numofelems)

/*
  Define the end of the clusterlist
*/

#define CLUSTERNIL (clusterset->numofelems)

/*
  An element of a cluster is of type \texttt{ClusterElem}.
*/

typedef struct
{
  Uint clusternumber, // cluster number the element belongs to
       nextelem;      // reference to next element of the same cluster
} ClusterElem;        // \Typedef{ClusterElem}

/*
  For each cluster we need a reference to the first and the last
  element, and the size of the cluster. This is stored in
  a \texttt{ClusterInfo}.
*/

typedef struct
{
  Uint firstelem,    // reference to first element of cluster
       lastelem,     // reference to last element of cluster
       csize,        // size of the cluster, i.e. number of elements
       startedges;   // start index of edges
} ClusterInfo;       // \Typedef{ClusterInfo}

DECLAREARRAYSTRUCT(ClusterInfo);

/*
  A \texttt{ClusterSet} consists of a set of elements belonging 
  to one cluster of the cluster set, and an array of references 
  to the first element of each cluster.
*/

typedef struct
{
  ClusterElem *celems;      // address to elements of cluster
  ArrayClusterInfo cinfo;   // info for each cluster
  Uint *alreadyincluster,   // bittable to mark singleton clusters
       *cedges,             // Array of cluster edges
       numofelems,          // number of elements in clusters
       numofedges,          // number of edges
       subclusterelemcount, // counter for elements in subcluster
       *subclusterelems;    // NULL or table of bits for elements
                            // relevant for output of the cluster
} ClusterSet;               // \Typedef{ClusterSet}

//\IgnoreLatex{

#endif

//}
