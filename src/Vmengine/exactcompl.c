
#include "virtualdef.h"
#include "arraydef.h"
#include "debugdef.h"
#include "vnodedef.h"
#include "qsortdef.h"
#include "scoredef.h"
#include "fhandledef.h"

#include "filehandle.pr"
#include "alphabet.pr"
#include "bmhfun.pr"

#include "qgram2code.c"

#include "pminfo.h"

#include "initcompl.pr"

typedef struct
{
  Matchstate *matchstate;
  Uint seqnum2, 
       plen;
} Offlinematchstruct;

#ifdef DEBUG
static void checkmatches(Virtualtree *virtualtree,Uchar *pattern,Uint plen,
                         PairUint *leftright)
{
  Uint i, j, width;
  ArrayUint matches;

  INITARRAY(&matches,Uint);
  width = (Uint) (leftright->uint1 - leftright->uint0 + 1);
  CHECKARRAYSPACEMULTI(&matches,Uint,width);
  for(i=leftright->uint0; i<=leftright->uint1; i++)
  {
    matches.spaceUint[matches.nextfreeUint++] = virtualtree->suftab[i];
  }
  qsortUint(matches.spaceUint,matches.spaceUint + matches.nextfreeUint - 1);
  if(virtualtree->alpha.mapsize > 0)
  {
    vm_showsymbolstring(&virtualtree->alpha,pattern,plen);
  } else
  {
    if(WRITETOFILEHANDLE(pattern,(Uint) sizeof(Uchar),plen,stdout) != 0)
    {
      fprintf(stderr,"%s\n",messagespace());
      exit(EXIT_FAILURE);
    }
  }
  for(j=0; j<matches.nextfreeUint; j++)
  {
    printf(" %lu",(Showuint) matches.spaceUint[j]);
  }
  (void) putchar('\n');
  bmhcheck(pattern,plen,virtualtree->multiseq.sequence,
           virtualtree->multiseq.totallength,&matches);
  FREEARRAY(&matches,Uint);
}
#endif

static Sint findsufboundaries(Virtualtree *virtualtree,PairUint *maxwit,
                              Uint leastprefixlen,Vnode *vnode,
                              PairUint *leftright)
{
  Uint i, l;
  PairUint *startexception, *lastexception;

  if(maxwit->uint0 < UCHAR_MAX)
  {
    for(i=maxwit->uint1; /* Nothing */; i--)
    {
      if(i == vnode->left || virtualtree->lcptab[i] < (Uchar) leastprefixlen)
      {
        leftright->uint0 = i;
        break;
      }
    }
    for(i=maxwit->uint1+1; /* Nothing */ ; i++)
    {
      if(i > vnode->right || virtualtree->lcptab[i] < (Uchar) leastprefixlen)
      {
        leftright->uint1 = i-1;
        break;
      }
    }
  } else // maxwit->uint0 >= UCHAR_MAX
  {
    startexception = lastexception = NULL;
    for(i=maxwit->uint1; /* Nothing */; i--)
    {
      if(i == vnode->left)
      {
        leftright->uint0 = i;
        break;
      }
      if((l = virtualtree->lcptab[i]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        {
          lastexception = startexception = getexception(virtualtree,i);
        } 
        l = (lastexception--)->uint1;
      }
      if(l < leastprefixlen)
      {
        leftright->uint0 = i;
        break;
      }
    }
    startexception = lastexception = NULL;
    for(i=maxwit->uint1+1; /* Nothing */ ; i++)
    {
      if(i > vnode->right)
      {
        leftright->uint1 = i-1;
        break;
      }
      if((l = virtualtree->lcptab[i]) >= UCHAR_MAX)
      {
        if(startexception == NULL)
        {
          lastexception = startexception = getexception(virtualtree,i);
        } 
        l = (lastexception++)->uint1;
      }
      if(l < leastprefixlen)
      {
        leftright->uint1 = i-1;
        break;
      }
    }
  }
  DEBUG2(3,"second ph.: left=%lu right=%lu\n",
           (Showuint) leftright->uint0,
           (Showuint) leftright->uint1);
  return 0;
}

static Sint processfinalexactmatchinterval(void *info,
                                           PairUint *leftright)
{
  Uint i;
  Match match;
  Offlinematchstruct *offlinematchstruct = (Offlinematchstruct *) info;

  initcompletematchstruct(&match,
                          offlinematchstruct->seqnum2,
                          offlinematchstruct->plen,
                          CHECKSHOWPALINDROMIC(offlinematchstruct->matchstate) 
                            ? True : False);
  match.length1 = offlinematchstruct->plen;
  match.distance = 0;
  for(i=leftright->uint0; i<=leftright->uint1; i++)
  {
    match.position1 = offlinematchstruct->matchstate->virtualtree->suftab[i];
    if(offlinematchstruct->matchstate->processfinal(
                    offlinematchstruct->matchstate,&match) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

Sint computeofflineexactmatches(Virtualtree *virtualtree,
                                Uchar *pattern,
                                Uint plen,
                                Processmatchinterval processmatchinterval,
                                void *info)
{
  Uint code = 0, *bckptr;
  Vnode vnode;
  PairUint leftright, maxwit;
  Sint retcode;

  if(plen < virtualtree->prefixlength)
  {
    ERROR2("patternlength=%lu must be >= %lu=prefixlen",
            (Showuint) plen,
            (Showuint) virtualtree->prefixlength);
    return (Sint) -1;
  }
  if(qgram2code(&code,virtualtree->alpha.mapsize-1,
                virtualtree->prefixlength,pattern))
  {
    bckptr = virtualtree->bcktab + MULT2(code);
    vnode.left = *bckptr;
    if((vnode.right = *(bckptr+1)) > vnode.left)
    {
      vnode.right--;
      vnode.offset = virtualtree->prefixlength;
      findmaxprefixlen(virtualtree,&vnode,pattern,plen,&maxwit);
      if(maxwit.uint0 >= plen)
      {
        if(findsufboundaries(virtualtree,&maxwit,plen,&vnode,
                             &leftright) != 0)
        {
          return (Sint) -2;
        }
        DEBUGCODE(1,checkmatches(virtualtree,pattern,plen,&leftright));
        if(leftright.uint1 >= leftright.uint0)
        {
          retcode = processmatchinterval(info,&leftright);
          if(retcode < 0 || retcode == SintConst(1))
          {
            return retcode;
          }
        }
      }
    }
  }
  return 0;
}

Sint findexactcompletematchesindex(void *info,
                                   Uint seqnum2,
                                   Uchar *pattern,
                                   Uint plen)
{
  Matchstate *matchstate = (Matchstate *) info;
  Offlinematchstruct offlinematchstruct;

  offlinematchstruct.seqnum2 = seqnum2;
  offlinematchstruct.plen = plen;
  offlinematchstruct.matchstate = matchstate;
  if(computeofflineexactmatches(matchstate->virtualtree,
                                pattern,
                                plen,
                                processfinalexactmatchinterval,
                                &offlinematchstruct) != 0)
  {
    return (Sint) -2;
  }

  return 0;
}

Sint checkexactcompletematchesindex(Virtualtree *virtualtree,
                                    Uchar *pattern,
                                    Uint plen)
{
  Uint code = 0, *bckptr;
  Vnode vnode;
  PairUint maxwit;

  if(plen < virtualtree->prefixlength)
  {
    ERROR2("patternlength=%lu must be >= %lu=prefixlen",
            (Showuint) plen,
            (Showuint) virtualtree->prefixlength);
    return (Sint) -1;
  }
  if(qgram2code(&code,
                virtualtree->alpha.mapsize-1,
                virtualtree->prefixlength,
                pattern))
  {
    bckptr = virtualtree->bcktab + MULT2(code);
    vnode.left = *bckptr;
    if((vnode.right = *(bckptr+1)) > vnode.left)
    {
      vnode.right--;
      vnode.offset = virtualtree->prefixlength;
      findmaxprefixlen(virtualtree,&vnode,pattern,plen,&maxwit);
      if(maxwit.uint0 >= plen)
      {
        return (Sint) 1;
      }
    }
  }
  return 0;
}

Sint findexactcompletematchesonline(void *info,
                                    Uint seqnum2,
                                    Uchar *pattern,
                                    Uint plen)
{
  Uint i, ppos, rmostocc[UCHAR_MAX+1], n;
  Uchar *seq;
  Match match;
  Matchstate *matchstate = (Matchstate *) info;

  initcompletematchstruct(&match,seqnum2,plen,
                          CHECKSHOWPALINDROMIC(matchstate) ? True : False);
  match.distance = 0;
  match.length1 = plen;
  n = matchstate->virtualtree->multiseq.totallength;
  if(plen > 0 && plen <= n)
  {
    for(i=0; i<=UCHAR_MAX; i++)
    {
      rmostocc[i] = plen;
    }
    for(ppos=0; ppos<plen-1; ppos++)
    {
      if(!ISSPECIAL(pattern[ppos]))
      {
        rmostocc[(Uint) pattern[ppos]] = plen - ppos - 1;
      }
    }
    for(seq = matchstate->virtualtree->multiseq.sequence;
        seq <= matchstate->virtualtree->multiseq.sequence + n - plen;
        seq += rmostocc[(Uint) seq[plen-1]])
    {
      for(i=plen-1; !ISSPECIAL(seq[i]) && pattern[i] == seq[i]; i--)
      {
        if(i==0)
        {
          match.position1 
            = (Uint) (seq-matchstate->virtualtree->multiseq.sequence);
          if(matchstate->processfinal(matchstate,&match) != 0)
          {
            return (Sint) -1;
          }
          break;
        }
      }
    }
  } 
  return 0;
}

BOOL checkexactcompletematchesonline(Multiseq *multiseq,
                                     Uchar *pattern,
                                     Uint plen)
{
  Uint i, ppos, rmostocc[UCHAR_MAX+1], n;
  Uchar *seq;

  n = multiseq->totallength;
  if(plen > 0 && plen <= n)
  {
    for(i=0; i<=UCHAR_MAX; i++)
    {
      rmostocc[i] = plen;
    }
    for(ppos=0; ppos<plen-1; ppos++)
    {
      if(!ISSPECIAL(pattern[ppos]))
      {
        rmostocc[(Uint) pattern[ppos]] = plen - ppos - 1;
      }
    }
    for(seq = multiseq->sequence;
        seq <= multiseq->sequence + n - plen;
        seq += rmostocc[(Uint) seq[plen-1]])
    {
      for(i=plen-1; !ISSPECIAL(seq[i]) && pattern[i] == seq[i]; i--)
      {
        if(i==0)
        {
          return True;
        }
      }
    }
  } 
  return False;
}
