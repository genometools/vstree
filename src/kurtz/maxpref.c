
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include "types.h"
#include "divmodmul.h"
#include "virtualdef.h"
#include "vnodedef.h"
#include "debugdef.h"
#include "chardef.h"
#include "minmax.h"

#include "binsplitvn.pr"

#ifdef COUNT
static Uint charcomp = 0;
static Uint pushop = 0;
#define CHARCOMP  charcomp++
#else
#define CHARCOMP  /* Nothing */
#endif

/* 
  The following correctly check for WILDCARDs, even if they are the same in 
  both strings.
*/

#define CHECKRETURN\
        if(retcode == 0)\
        {\
          if(ISSPECIAL(*sptr) && ISSPECIAL(query[lcplen]))\
          {\
            retcode = (Sint) -1;\
            break;\
          }\
        } else\
        {\
          break;\
        }

/* 
  The following allows that the symbol WILDCARD matches itself, if it occurs 
  in the query sequence and the database sequence.
*/

#define COMPARE(SEQ)\
        for(sptr = (SEQ) + lcplen;\
            /* Nothing */; sptr++, lcplen++)\
        {\
          if(lcplen >= querylen)\
          {\
            retcode = 0;\
            break;\
          }\
          if(sptr >= sentinel)\
          {\
            retcode = (Sint) -1;\
            break;\
          }\
          retcode = (Sint) (query[lcplen] - *sptr);\
          CHARCOMP;\
          CHECKRETURN;\
        }

/*

#define UPDATEOFFSET\
        if(lcplen > offset)\
        {\
          offset = lcplen;\
        }

*/

#define PUSHFINDMAXPREFIXQUADRUPLE(LEFT,RIGHT,LCPLEN,RPREF)  /* Nothing */
#define FINDMAXPREFIXLENFUNCTION "findmaxprefixlen"
#define DECLAREFINDMAXSTACKPTR   /* Nothing */
#define FINDMAXPREFIXLEN\
        void findmaxprefixlen(Virtualtree *virtualtree,\
                              Vnode *vnode,\
                              Uchar *query,\
                              Uint querylen,\
                              PairUint *maxwit)

#include "findmaxpref.gen"

static Uint findmaxprefixlenwitness(Virtualtree *virtualtree,Vnode *vnode,
                                    Uchar *query,Uint querylen)
{
  register Uint left,     // left boundary of interval
                right,    // right boundary of interval
                mid,      // mid point of interval
                lcplen,   // length of common prefix of all suffixes in interval
                lpref,    // length of prefix of left interval
                rpref,    // length of prefix of right interval
                maxprefix;
  register Sint retcode;   // retcode from comparison
  register Uchar *sptr,   // pointer to suffix in index
                 *sentinel = virtualtree->multiseq.sequence + 
                             virtualtree->multiseq.totallength; // points to $
 
  DEBUG3(3,"findmaxprefixlenwitness(l=%lu,r=%lu,querylen=%lu)\n",
            (Showuint) vnode->left,
            (Showuint) vnode->right,
            (Showuint) querylen);
#ifdef DEBUG
  if(querylen == 0)
  {
    fprintf(stderr,"cannot match empty string\n");
    exit(EXIT_FAILURE);
  }
#endif
  lcplen = vnode->offset;
  COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[vnode->left]);
  maxprefix = lcplen;
  if(retcode <= 0)
  {
    return maxprefix;
  }
  lpref = lcplen;
  lcplen = vnode->offset;
  COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[vnode->right]);
  rpref = lcplen;
  if(lpref < rpref)
  {
    maxprefix = rpref;
    lcplen = lpref;
  } else
  {
    maxprefix = lpref;
  }
  if(retcode >= 0 || maxprefix >= querylen)
  {
    vnode->offset = MIN(lpref,rpref);
    return maxprefix;
  }
  left = vnode->left;
  right = vnode->right;
  while(right > left + 1)
  {
#ifdef COUNT
    pushop++;
#endif
    mid = DIV2(left+right);
    COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[mid]);
    if(maxprefix < lcplen)
    {
      maxprefix = lcplen;
    }
    if(retcode < 0)
    {
      rpref = lcplen;
      if(lpref < rpref)
      {
        lcplen = lpref;
      }
      right = mid;
    } else
    {
      if(retcode > 0)
      {
        lpref = lcplen;
        if(rpref < lpref)
        {
          lcplen = rpref;
        }
        left = mid;
      } else
      {
        break;
      }
    }
  }
  return maxprefix;
}

static Uint findleft(Virtualtree *virtualtree,Uint l,Uint r,Uint offset,
                     Uchar *query,Uint querylen)
{
  Uint left, mid, right, lcplen, lpref, rpref;
  Sint retcode = 0;
  Uchar *sptr, 
        *sentinel = virtualtree->multiseq.sequence + 
                    virtualtree->multiseq.totallength;

  left = l;
  right = r;
  lcplen = offset;
  COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[left]);
  if(retcode > 0)
  {
    lpref = lcplen;
    rpref = querylen;
    while(right > left + 1)
    {
      mid = DIV2(left+right);
      lcplen = MIN(lpref,rpref);
      COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[mid]);
      if(retcode <= 0)
      {
        right = mid;
        rpref = lcplen;
      } else
      {
        left = mid;
        lpref = lcplen;
      }
    }
    left = right;
  }
  return left;
}

static Uint findright(Virtualtree *virtualtree,Uint l,Uint r,Uint offset,
                     Uchar *query,Uint querylen)
{
  Uint left, mid, right, lcplen, lpref, rpref;
  Sint retcode = 0;
  Uchar *sptr, 
        *sentinel = virtualtree->multiseq.sequence + 
                    virtualtree->multiseq.totallength;
  left = l;
  right = r;
  lcplen = offset;
  COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[right]);
  if(retcode < 0)
  {
    lpref = querylen;
    rpref = lcplen;
    while(right > left + 1)
    {
      mid = DIV2(left+right);
      lcplen = MIN(lpref,rpref);
      COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[mid]);
      if(retcode >= 0)
      {
        left = mid;
        lpref = lcplen;
      } else
      {
        right = mid;
        rpref = lcplen;
      }
    }
    right = left;
  }
  return right;
}

void maxprefixmatch2(Virtualtree *virtualtree,Vnode *vnode,
                     Uchar *query,Uint querylen,ThreeUint *maxwit)
{
  register Uint left,     // left boundary of interval
                right,    // right boundary of interval
                mid,      // mid point of interval
                lcplen,   // length of common prefix of all suffixes in interval
                lpref,    // length of prefix of left interval
                rpref;    // length of prefix of right interval
  register Sint retcode;   // retcode from comparison
  register Uchar *sptr,   // pointer to suffix in index
                 *sentinel = virtualtree->multiseq.sequence + 
                             virtualtree->multiseq.totallength; // points to $
 
  DEBUG3(3,"maxprefixmatch2(l=%lu,r=%lu,querylen=%lu)\n",
          (Showuint) vnode->left,
          (Showuint) vnode->right,
          (Showuint) querylen);
#ifdef DEBUG
  if(querylen == 0)
  {
    fprintf(stderr,"cannot match empty string\n");
    exit(EXIT_FAILURE);
  }
#endif
  lcplen = vnode->offset;
  COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[vnode->left]);
  maxwit->uint0 = lcplen;
  maxwit->uint1 = vnode->left; // Due to comparison of suffix s and 
                               // query at offset 0
  if(retcode <= 0)
  {
    maxwit->uint2 = findright(virtualtree,vnode->left,vnode->right,
                              vnode->offset,query,lcplen);
    return;
  }
  lpref = lcplen;
  lcplen = vnode->offset;
  COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[vnode->right]);
  rpref = lcplen;
  if(lpref < rpref)
  {
    maxwit->uint0 = rpref;
    maxwit->uint1 = findleft(virtualtree,vnode->left,vnode->right,vnode->offset,
                             query,lcplen);
    maxwit->uint2 = vnode->right; // Due to comparison of suffix r and 
                                  // query, offset 0
    lcplen = lpref;
  } else
  {
    maxwit->uint0 = lpref;
    maxwit->uint1 = vnode->left; // Due to comparison of suffix l and 
                                 // query, offset 0
    maxwit->uint2 = findright(virtualtree,vnode->left,vnode->right,
                              vnode->offset,query,lcplen);
    lcplen = rpref;
  }
  if(retcode >= 0 || maxwit->uint0 >= querylen)
  {
    printf("return (%lu,%lu,%lu)\n",(Showuint) maxwit->uint0,
                                    (Showuint) maxwit->uint1,
                                    (Showuint) maxwit->uint2);
    return;
  }
  left = vnode->left;
  right = vnode->right;
  while(right > left + 1)
  {
#ifdef COUNT
    pushop++;
#endif
    mid = DIV2(left+right);
    COMPARE(virtualtree->multiseq.sequence + virtualtree->suftab[mid]);
    if(maxwit->uint0 < lcplen)
    {
      maxwit->uint0 = lcplen;
      maxwit->uint1 = mid;
    }
    if(retcode < 0)
    {
      rpref = lcplen;
      if(lpref < rpref)
      {
        lcplen = lpref;
      }
      right = mid;
    } else
    {
      if(retcode > 0)
      {
        lpref = lcplen;
        if(rpref < lpref)
        {
          lcplen = rpref;
        }
        left = mid;
      } else
      {
        // query completly matched
        break;
      }
    }
  }
  maxwit->uint1 = findleft(virtualtree,vnode->left,vnode->right,vnode->offset,
                           query,maxwit->uint0);
  maxwit->uint2 = findright(virtualtree,vnode->left,vnode->right,vnode->offset,
                            query,maxwit->uint0);
  printf("return (%lu,%lu,%lu)\n",(Showuint) maxwit->uint0,
                                  (Showuint) maxwit->uint1,
                                  (Showuint) maxwit->uint2);
}

BOOL mmsearch(Uchar *sequence,Uint totallength,Uint *suftab,Vnode *vnode,
              Uchar *query,Uint querylen)
{
  Uint left, leftsave, mid, right, lcplen, lpref, rpref;
  Sint retcode = 0;
  Uchar *sptr, 
        *sentinel = sequence + totallength;

  leftsave = left = vnode->left;
  right = vnode->right;
  lcplen = vnode->offset;
  COMPARE(sequence + suftab[left]);
  if(retcode > 0)
  {
    lpref = lcplen;
    lcplen = vnode->offset;
    COMPARE(sequence + suftab[right]);
    if(retcode > 0)
    {
      return False;
    } else
    {
      rpref = lcplen;
      while(right > left + 1)
      {
        mid = DIV2(left+right);
        lcplen = MIN(lpref,rpref);
        COMPARE(sequence + suftab[mid]);
        if(retcode <= 0)
        {
          right = mid;
          rpref = lcplen;
        } else
        {
          left = mid;
          lpref = lcplen;
        }
      }
      vnode->left = right;
    }
  }

  left = leftsave;
  right = vnode->right;
  lcplen = vnode->offset;
  COMPARE(sequence + suftab[left]);
  if(retcode < 0)
  {
    return False;
  } else
  {
    lpref = lcplen;
    lcplen = vnode->offset;
    COMPARE(sequence + suftab[right]);
    if(retcode >= 0)
    {
      vnode->right = right;
    } else
    {
      rpref = lcplen;
      while(right > left + 1)
      {
        mid = DIV2(left+right);
        lcplen = MIN(lpref,rpref);
        COMPARE(sequence + suftab[mid]);
        if(retcode >= 0)
        {
          left = mid;
          lpref = lcplen;
        } else
        {
          right = mid;
          rpref = lcplen;
        }
      }
      vnode->right = left;
    }
  }
  return True;
}

void maxprefixmatchbinstep(Virtualtree *virtualtree,Vnode *vnode,
                           Uchar *query,Uint querylen)
{
  Uint i;

  for(i = vnode->offset; i<querylen; i++)
  {
    if(vnode->left < vnode->right)
    {
      if(!findcharintervalbin(&virtualtree->multiseq,virtualtree->suftab,vnode,
                              query[i],vnode->offset,vnode->left,vnode->right))
      {
        break;
      }
      vnode->offset++;
    } else
    {
      Uint j, end;
      Uchar *sentinel = virtualtree->multiseq.sequence + 
                        virtualtree->multiseq.totallength,
            *sptr = virtualtree->multiseq.sequence + 
                    virtualtree->suftab[vnode->left];
  
      end = (Uint) (sentinel - sptr);
      if(querylen < end)
      {
        end = querylen;
      }
      for(j=vnode->offset; j < end; j++)
      {
        if(sptr[j] != query[j] || ISSPECIAL(sptr[j]) || ISSPECIAL(query[j]))
        {
          break;
        }
      }
      vnode->offset=j;
      break;
    }
  }
}

#ifdef DEBUG
#define RETURNISBRANCHING(B)\
        printf("isbranching(%lu,%lu,%lu)=%s\n",\
                 (Showuint) vnode->offset,\
                 (Showuint) vnode->left,\
                 (Showuint) vnode->right,SHOWBOOL(B));\
        return B
#else
#define RETURNISBRANCHING(B) return B
#endif

BOOL isbranching(Virtualtree *virtualtree,Vnode *vnode)
{
  Uchar lc, rc;
  Uint start;

  if(vnode->left < vnode->right)
  {
    start = virtualtree->suftab[vnode->left] + vnode->offset;
    if(start == virtualtree->multiseq.totallength)
    {
      RETURNISBRANCHING(True);
    } 
    lc = virtualtree->multiseq.sequence[start];
    if(ISSPECIAL(lc))
    {
      RETURNISBRANCHING(True);
    }
    start = virtualtree->suftab[vnode->right] + vnode->offset;
    if(start == virtualtree->multiseq.totallength)
    {
      RETURNISBRANCHING(True);
    } 
    rc = virtualtree->multiseq.sequence[start];
    if(lc != rc || ISSPECIAL(rc))
    {
      RETURNISBRANCHING(True);
    }
  } 
  RETURNISBRANCHING(False);
}

void maxprefixmatchbinstepwithprevious(Virtualtree *virtualtree,Vnode *vnode,
                                       Vnode *branchfather,
                                       Uchar *query,Uint querylen)
{
  Uint i;

  for(i = vnode->offset; i<querylen; i++)
  {
    if(vnode->left < vnode->right)
    {
      if(!findcharintervalbin(&virtualtree->multiseq,virtualtree->suftab,vnode,
                              query[i],vnode->offset,vnode->left,vnode->right))
      {
        break;
      }
      vnode->offset++;
      if(isbranching(virtualtree,vnode))
      {
        *branchfather = *vnode;
      }
    } else
    {
      Uint j, end;
      Uchar *sentinel = virtualtree->multiseq.sequence + 
                        virtualtree->multiseq.totallength,
            *sptr = virtualtree->multiseq.sequence + 
                    virtualtree->suftab[vnode->left];
  
      end = (Uint) (sentinel - sptr);
      if(querylen < end)
      {
        end = querylen;
      }
      for(j=vnode->offset; j < end; j++)
      {
        if(sptr[j] != query[j] || ISSPECIAL(sptr[j]) || ISSPECIAL(query[j]))
        {
          break;
        }
      }
      vnode->offset=j;
      break;
    }
  }
}

void maxprefixmatchmm(Virtualtree *virtualtree,Vnode *vnode,
                      Uchar *query,Uint querylen)
{
  Uint maxprefix, currentlength;
  currentlength = vnode->offset;
  maxprefix = findmaxprefixlenwitness(virtualtree,vnode,query,querylen);
  if(currentlength < maxprefix)
  {
    if(!mmsearch(virtualtree->multiseq.sequence,
                 virtualtree->multiseq.totallength,
                 virtualtree->suftab,
                 vnode,
                 query,
                 maxprefix))
    {
      fprintf(stderr,"mmsearch returns False\n");
      exit(EXIT_FAILURE);
    }
    vnode->offset = maxprefix;
  }
#ifdef DEBUG
  {
    Vnode vnodetmp = *vnode;
    maxprefixmatchbinstep(virtualtree,&vnodetmp,query,querylen);
    if(vnodetmp.offset != vnode->offset)
    {
      fprintf(stderr,"offset: vnodetmp = %lu != %lu = vnode\n",
                              (Showuint) vnodetmp.offset,
                              (Showuint) vnode->offset);
      exit(EXIT_FAILURE);
    }
    if(vnodetmp.left != vnode->left)
    {
      fprintf(stderr,"left: vnodetmp = %lu != %lu = vnode\n",
                              (Showuint) vnodetmp.left,
                              (Showuint) vnode->left);
      exit(EXIT_FAILURE);
    }
    if(vnodetmp.right != vnode->right)
    {
      fprintf(stderr,"right: vnodetmp = %lu != %lu = vnode\n",
                              (Showuint) vnodetmp.right,
                              (Showuint) vnode->right);
      exit(EXIT_FAILURE);
    }
  }
#endif
}

#define PUSHMAXPREF(LEFT,RIGHT,LCPLEN,RPREF)\
        if(stack->nextfreeVnoderpref >= (Uint) MAXVSTACK)\
        {\
          fprintf(stderr,"Vnoderpref stack space is too small\n");\
          exit(EXIT_FAILURE);\
        }\
        stackptr = stack->spaceVnoderpref + stack->nextfreeVnoderpref++;\
        stackptr->left = LEFT;\
        stackptr->right = RIGHT;\
        stackptr->offset = LCPLEN;\
        stackptr->rpref = RPREF;\
        DEBUG5(2,"push(%lu,l=%lu,r=%lu,o=%lu,rp=%lu)\n",\
                  (Showuint) (stack->nextfreeVnoderpref-1),\
                  (Showuint) (LEFT),\
                  (Showuint) (RIGHT),\
                  (Showuint) (LCPLEN),\
                  (Showuint) (RPREF))

#undef PUSHFINDMAXPREFIXQUADRUPLE
#undef FINDMAXPREFIXLENFUNCTION
#undef FINDMAXPREFIXLEN
#undef DECLAREFINDMAXSTACKPTR

#define PUSHFINDMAXPREFIXQUADRUPLE(LEFT,RIGHT,LCPLEN,RPREF)\
        PUSHMAXPREF(LEFT,RIGHT,LCPLEN,RPREF)
#define FINDMAXPREFIXLENFUNCTION "findmaxprefixlenstack"
#define DECLAREFINDMAXSTACKPTR Vnoderpref *stackptr;
#define FINDMAXPREFIXLEN\
        void findmaxprefixlenstack(Virtualtree *virtualtree,\
                                   Vnoderpref *vnode,\
                                   Uchar *query,\
                                   Uint querylen,\
                                   PairUint *maxwit,\
                                   ArrayVnoderpref *stack)

#include "findmaxpref.gen"
