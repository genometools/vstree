
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include "errordef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "vnodedef.h"
#include "chardef.h"
#include "spacedef.h"

#include "binsplitvn.pr"

typedef Vnode Queueelem;

#ifdef DEBUG
static void showVnode(Vnode vnode)
{
  printf("(%lu,%lu,%lu)\n",(Showuint) vnode.offset,
                           (Showuint) vnode.left,
                           (Showuint) vnode.right);
}
#endif

#include "queue.c"

static Uint countspecial(Multiseq *multiseq)
{
  Uint i, specialnum = 0;

  for(i=0; i<multiseq->totallength; i++)
  {
    if(ISSPECIAL(multiseq->sequence[i]))
    {
      specialnum++;
    }
  }
  return specialnum + 1;
}

Sint breadthfirstvstree(Virtualtree *virtualtree,
                        Sint (*applyfs)(Virtualtree *,void *,Vnode *,Vnode *),
                        void *info)
{
  Queue queue;
  Vbound *vbounds;
  Vnode father, child;
  Uint i, l, r, vnodemaxsize, vboundscount;
  
#ifdef DEBUG
  queue.showelem = showVnode;
#endif
  emptyqueue(&queue,UintConst(1024));
  father.offset = 0;
  father.left = 0;
  father.right = virtualtree->multiseq.totallength;
  vnodemaxsize = countspecial(&virtualtree->multiseq) +
                 virtualtree->alpha.mapsize -1 + 1;
  ALLOCASSIGNSPACE(vbounds,NULL,Vbound,vnodemaxsize);
  enqueue(&queue,father);
  while(!queueisempty(&queue))
  {
    father = dequeue(&queue);
    vboundscount = splitnodewithcharbin(&virtualtree->multiseq,
                                        virtualtree->suftab,
                                        vbounds,
                                        vnodemaxsize, 
                                        father.offset,father.left,
                                        father.right);
    for(i=0; i<vboundscount; i++)
    {
      l = vbounds[i].bound;
      r = vbounds[i+1].bound-1;
      DEBUG3(3,"%lu-(%lu,%lu) ",(Showuint) vbounds[i].inchar,
                                (Showuint) l,
                                (Showuint) r);
      if(l < r)
      {
        child.offset = binlcpvalue(&virtualtree->multiseq,
                                   virtualtree->suftab,
                                   father.offset,l,r);
        child.left = l;
        child.right = r;
        if(applyfs(virtualtree,info,&father,&child) != 0)
        {
          return (Sint) -1;
        }
        enqueue(&queue,child);
      }
    }
  }
  FREESPACE(vbounds);
  wrapqueue(&queue);
  return 0;
}
