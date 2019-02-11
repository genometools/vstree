#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "errordef.h"
#include "arraydef.h"
#include "alphasize.h"

#include "genfile.h"
#include "virtualdef.h"

#include "distri.pr"

#define ADDAMOUNT 128

#define ISLEFTDIVERSE   (state->alphabetsize)
#define INITIALCHAR     (state->alphabetsize+1)

#define CHECKCHAR(CC)\
        if(father->commonchar != (CC) || (CC) >= ISLEFTDIVERSE)\
        {\
          father->commonchar = ISLEFTDIVERSE;\
        }

#define PROCESSLEAFEDGE(FIRSTSUCC)\
        if(processleafedge(state,FIRSTSUCC,&TOP,leftchar) != 0)\
        {\
          return (Sint) -1;\
        }

#define PROCESSBRANCHEDGE(FIRSTSUCC,D,L,R)\
        if(processbranch(state,FIRSTSUCC,&TOP) != 0)\
        {\
          return (Sint) -2;\
        }

typedef struct
{
  BOOL lastisleafedge;
  Uchar commonchar;
  Uint depth,
       uniquecharposcount,
       poslisttotalcount,
       poslistcount[ALPHABETSIZE];
} Nodeinfo;

DECLAREARRAYSTRUCT(Nodeinfo);

typedef struct
{
  ArrayUint *countallmatches;
  Uint depth,              // value changes with each new match
       searchlength,
       alphabetsize;
} State;

static void addtoposlist(State *state,Nodeinfo *ninfo,Uint base)
{
  if(base >= state->alphabetsize)
  {
    ninfo->uniquecharposcount++;
    DEBUG1(3,"uniquecharposcount=%lu\n",(Showuint) ninfo->uniquecharposcount);
  } else
  {
    ninfo->poslisttotalcount++;
    ninfo->poslistcount[base]++;
    DEBUG1(3,"poslisttotalcount=%lu\n",(Showuint) ninfo->poslisttotalcount);
    DEBUG2(3,"poslistcount[%lu]=%lu\n",(Showuint) base,
                                       (Showuint) ninfo->poslistcount[base]);
  }
}

static void concatlists(State *state,Nodeinfo *father,Nodeinfo *son)
{
  Uint base;

  for(base = 0; base < state->alphabetsize; base++)
  {
    father->poslistcount[base] += son->poslistcount[base];
    DEBUG2(3,"poslistcount[%lu]=%lu\n",(Showuint) base,
                                       (Showuint) father->poslistcount[base]);
  }
  father->poslisttotalcount += son->poslisttotalcount;
  father->uniquecharposcount += son->uniquecharposcount;
  DEBUG1(3,"poslisttotalcount = %lu\n",(Showuint) father->poslisttotalcount);
  DEBUG1(3,"uniquecharposcount = %lu\n",(Showuint) father->uniquecharposcount);
}

static void cartproduct(State *state,Nodeinfo *father,Nodeinfo *son)
{
  Uint cnt, chfather;

  for(chfather = 0; chfather < state->alphabetsize; chfather++)
  {
    cnt = father->poslistcount[chfather];
    if(cnt > 0)
    {
      DEBUG2(3,"(1) increment %lu by %lu\n",(Showuint) state->depth,
                  (Showuint) (cnt * (son->poslisttotalcount - 
                                     son->poslistcount[chfather] + 
                                     son->uniquecharposcount)));
      addmultidistribution(state->countallmatches,
                           state->depth,
                           cnt * (son->poslisttotalcount - 
                           son->poslistcount[chfather] + 
                           son->uniquecharposcount));
    }
  }
}

static Sint processleafedge(State *state,BOOL firstsucc,Nodeinfo *father,
                            Uchar leftchar)
{
  Uint base;
  
  if(father->depth < state->searchlength)
  {
    return 0;
  }
  state->depth = father->depth;
  DEBUG1(5,"processleafedge: leftchar %lu\n",(Showuint) leftchar);
  if(firstsucc)
  {
    father->commonchar = leftchar;
    father->uniquecharposcount = 0;
    father->poslisttotalcount = 0;
    for(base = 0; base < state->alphabetsize; base++)
    {
      father->poslistcount[base] = 0;
    }
    addtoposlist(state,father,leftchar);
    return 0;
  }
  if(father->commonchar != ISLEFTDIVERSE)
  {
    CHECKCHAR(leftchar);
  }
  if(father->commonchar == ISLEFTDIVERSE)
  {
    if(leftchar >= state->alphabetsize)
    {
      DEBUG2(3,"(2) increment %lu by %lu\n",
             (Showuint) state->depth,
             (Showuint) (father->poslisttotalcount + 
                         father->uniquecharposcount));
      addmultidistribution(state->countallmatches,state->depth,
                           father->poslisttotalcount + father->uniquecharposcount);
    } else
    {
      DEBUG2(3,"(3) increment %lu by %lu\n",
                  (Showuint) state->depth,
                  (Showuint) (father->poslisttotalcount -
                              father->poslistcount[leftchar] +
                              father->uniquecharposcount));
      addmultidistribution(state->countallmatches,
                           state->depth,
                           father->poslisttotalcount 
                            - father->poslistcount[leftchar]
                            + father->uniquecharposcount);
    }
  }
  addtoposlist(state,father,leftchar);
  return 0;
}

static Sint processbranch(State *state,BOOL firstsucc,Nodeinfo *father)
{
  Nodeinfo *son;

  DEBUG2(3,"processbranch: firstsucc=%s, fatherdepth=%lu\n",
          SHOWBOOL(firstsucc),(Showuint) father->depth);
  if(father->depth < state->searchlength)
  {
    return 0;
  }
  state->depth = father->depth;
  if(firstsucc)
  {
    return 0;
  }
  son = father + 1;
  if(father->commonchar != ISLEFTDIVERSE)
  {
    if(son->commonchar != ISLEFTDIVERSE)
    {
      CHECKCHAR(son->commonchar);
    } else
    {
      father->commonchar = ISLEFTDIVERSE;
    }
  }
  if(father->commonchar == ISLEFTDIVERSE)
  {
    cartproduct(state,father,son);
    DEBUG2(3,"(4) increment(%lu) by %lu\n",(Showuint) state->depth,
                (Showuint) (father->uniquecharposcount * 
                            (son->poslisttotalcount+son->uniquecharposcount)));
    addmultidistribution(state->countallmatches,state->depth,
                         father->uniquecharposcount * 
                         (son->poslisttotalcount + son->uniquecharposcount));
  }
  concatlists(state,father,son);
  return 0;
}

#define WITHLEFTCHAR
typedef Uint Previoussuffixtype;
#include "vdfstrav.c"

Sint VMATMAXCOUNT(Virtualtree *virtualtree,
                  /*@unused@*/ Uint numberofprocessors,
                  Uint searchlength,
                  ArrayUint *countallmatches)
{
  State state;

  if(virtualtree->alpha.mapsize == 0)
  {
    ERROR0("alphabet transformation required");
    return (Sint) -1;
  }
  if(virtualtree->alpha.mapsize-1 > ALPHABETSIZE)
  {
    ERROR2("alphabet size %lu too large: maximum is %lu",
            (Showuint) (virtualtree->alpha.mapsize-1),
            (Showuint) ALPHABETSIZE);
    return (Sint) -2;
  }
  state.alphabetsize = virtualtree->alpha.mapsize-1;
  state.searchlength = searchlength;
  state.countallmatches = countallmatches;
  if(depthfirstvstree(&state,
                      numberofprocessors,
                      virtualtree) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}
