
#include "types.h"
#include "debugdef.h"
#include "multidef.h"
#include "match.h"
#include "intbits.h"
#include "markinfo.h"

/*
#define DONOTMARK(I,J) printf("do not mark (%lu,%lu)\n",\
                               (Showuint) (I),(Showuint) (J))
*/

#define DONOTMARK(I,J) /* Nothing */

void initmarktable(Uint **marktable,Multiseq *multiseq)
{
  Uint i;

  DEBUG1(2,"initmarktable(sequencelength=%lu)\n",
         (Showuint) multiseq->totallength);
  INITBITTABGENERIC(*marktable,*marktable,multiseq->totallength);
  for(i=0; i < multiseq->numofsequences-1; i++) 
  {
    DEBUG1(2,"mark %lu (boundary)\n",
            (Showuint) multiseq->markpos.spaceUint[i]);
    SETIBIT(*marktable,multiseq->markpos.spaceUint[i]);
  }
}

static void marksinglematch(Uint *marktable,Uint start,Uint len)
{
  Uint i;

  //printf("mark (start=%lu,len=%lu)\n",(Showuint) start,(Showuint) len);
  for (i=start; i<start + len; i++)
  {
    SETIBIT(marktable,i);
  } 
}

Sint markmatches(void *showmatchinfo,
                 Multiseq *virtualmultiseq,
                 /*@unused@*/ Multiseq *querymultiseq,
                 StoreMatch *storematch)
{
  Markinfo *markinfo = (Markinfo *) showmatchinfo;

  DEBUG1(3,"markmatches(markdb=%s,",SHOWBOOL(markinfo->markfields.markdb));
  DEBUG1(3,"virtualmultiseq->numofqueryfiles = %lu)\n",
            (Showuint) virtualmultiseq->numofqueryfiles);
  /*
    the left instance must be marked if markleft is True and
    the database is to be marked.
  */
  if(markinfo->markfields.markleft && 
     markinfo->markfields.markdb)
  {
    if(markinfo->markfields.markleftifdifferentsequence || 
       storematch->Storeseqnum1 != storematch->Storeseqnum2)
    {
      DEBUG2(2,"mark %lu..%lu\n",
              (Showuint) storematch->Storeposition1,
              (Showuint) (storematch->Storeposition1+
                          storematch->Storelength1-1));
      marksinglematch(markinfo->markmatchtable,
                      storematch->Storeposition1,storematch->Storelength1);
    } else
    {
      DONOTMARK(storematch->Storeposition1,
                storematch->Storeposition1+storematch->Storelength1-1);
    }
  } else
  {
    DONOTMARK(storematch->Storeposition2,
              storematch->Storeposition2+storematch->Storelength2-1);
  }
  /*
    Suppose that the database is not to be marked. Then the right
    instance is to be marked. If the database is to be marked,
    and there are no query files, then the right instance of 
    the match is to be marked.
  */
  if(!markinfo->markfields.markdb || markinfo->hasnoqueryfiles)
  {
    if((!markinfo->markfields.markdb || markinfo->markfields.markright) &&
       (markinfo->markfields.markrightifdifferentsequence ||
        storematch->Storeseqnum1 != storematch->Storeseqnum2))
    {
      Uint offset;

      if(markinfo->markfields.markdb ||
         !HASINDEXEDQUERIES(virtualmultiseq))
      {
        offset = 0;
      } else
      {
        offset = DATABASELENGTH(virtualmultiseq)+1;
      }
      DEBUG2(2,"mark %lu..%lu\n",
               (Showuint) (offset+storematch->Storeposition2),
               (Showuint) (offset+storematch->Storeposition2+
                                  storematch->Storelength2-1));
      marksinglematch(markinfo->markmatchtable,
                      offset+storematch->Storeposition2,
                      storematch->Storelength2);
    } else
    {
      DONOTMARK(storematch->Storeposition2,
                storematch->Storeposition2+storematch->Storelength2-1);
    }
  } else
  {
    DONOTMARK(storematch->Storeposition2,
              storematch->Storeposition2+storematch->Storelength2-1);
  }
  return 0;
}
