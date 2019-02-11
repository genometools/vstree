#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "arraydef.h"
#include "errordef.h"
#include "genfile.h"
#include "alphadef.h"

#ifdef ESASTREAMACCESS
#include "esastream.h"
typedef Uint Previoussuffixtype;
#else
#ifdef ESAMERGEDACCESS
#include "encseq-def.h"
#include "emimergeesa.h"
typedef Indexedsuffix Previoussuffixtype;
#include "mergeesa.pr"
#else
#include "virtualdef.h"
typedef Uint Previoussuffixtype;
#endif /* ESASTREAMACCESS */
#endif /* ESAMERGEDACCESS */

#include "vmatfind-def.h"

#ifdef ESAMERGEDACCESS 
typedef Sint(*OutputfunctionPrevioussuffixtype)(void *,
                                                Uint,
                                                Previoussuffixtype,
                                                Previoussuffixtype);
#else
typedef Outputfunction OutputfunctionPrevioussuffixtype;
#endif

#ifndef DYNAMICALPHABET
#include "alphasize.h"
#endif /* DYNAMICALPHABET */

#include "distri.pr"

#define ISLEFTDIVERSE   (state->alphabetsize)
#define INITIALCHAR     (state->alphabetsize+1)

#define MAXSTATICPOSLIST UintConst(16)

#define CHECKCHAR(CC)\
        if(father->commonchar != (CC) || (CC) >= ISLEFTDIVERSE)\
        {\
          father->commonchar = ISLEFTDIVERSE;\
        }

#define PROCESSLEAFEDGE(FIRSTSUCC)\
        if(processleafedge(state,FIRSTSUCC,&TOP,leftchar,previoussuffix) != 0)\
        {\
          return (Sint) -1;\
        }

#define PROCESSBRANCHEDGE(FIRSTSUCC,D,L,R)\
        if(processbranch(state,FIRSTSUCC,&TOP) != 0)\
        {\
          return (Sint) -2;\
        }

#ifdef DYNAMICALPHABET

/*
  The following two macros are used in vdfstrav.c
*/

#define ALLOCATEEXTRASTACKELEMENTS(STPTR,SZ)\
        ALLOCASSIGNSPACE(state->nodeposlistreservoir,\
                         NULL,\
                         Listtype,\
                         UintConst(SZ) * state->alphabetsize);\
        initlocalstackelements(state,\
                               STPTR,\
                               state->alphabetsize,\
                               (SZ)-UintConst(1))

#define REALLOCATEEXTRASTACKELEMENTS(STPTR,PSZ,SZ)\
        ALLOCASSIGNSPACE(state->nodeposlistreservoir,\
                         state->nodeposlistreservoir,\
                         Listtype,\
                         (PSZ+SZ) * state->alphabetsize);\
        initlocalstackelements(state,\
                               STPTR,\
                               state->alphabetsize,\
                               PSZ+SZ-UintConst(1))

#ifdef ESASTREAMACCESS
#define VMATMAXOUT strmvmatmaxoutdynamic
#else
#ifdef ESAMERGEDACCESS
#define VMATMAXOUT mergevmatmaxoutdynamic
#else
#define VMATMAXOUT vmatmaxoutdynamic
#endif /* ESASTREAMACCESS */
#endif /* ESAMERGEDACCESS */

#endif /* DYNAMICALPHABET */

#define NODEPOSLISTENTRY(NN,SYM)\
        (NN)->nodeposlist[SYM]

#define NODEPOSLISTLENGTH(NN,SYM)\
        NODEPOSLISTENTRY(NN,SYM).length

#define NODEPOSLISTSTART(NN,SYM)\
        NODEPOSLISTENTRY(NN,SYM).start

typedef struct
{
  BOOL lastisleafedge;
  Uchar commonchar;
  Uint depth,
       uniquecharposstart,
       uniquecharposlength; // uniquecharpos[start..start+len-1]
#ifdef DYNAMICALPHABET
  Listtype *nodeposlist;
#else
  Listtype nodeposlist[ALPHABETSIZE];
#endif
} Nodeinfo;

DECLAREARRAYSTRUCT(Nodeinfo);

typedef struct
{
  Uint nextfreePoslist, 
       allocatedPoslist; 
  Previoussuffixtype *refPoslist,
                     spacePoslist[MAXSTATICPOSLIST];
} Poslist;

DECLAREARRAYSTRUCT(Previoussuffixtype);

typedef struct
{
  BOOL initialized;
  Uint depth,            // value changes with each new match
       searchlength,
       alphabetsize;
  void *outinfo;         // Info-record for output function
  OutputfunctionPrevioussuffixtype output; // Output function
  ArrayPrevioussuffixtype uniquechar;
#ifdef DYNAMICALPHABET
  Listtype *nodeposlistreservoir;
  Poslist poslist[UCHAR_MAX+1];
#else
  Poslist poslist[ALPHABETSIZE];
#endif
} State;

#ifdef DYNAMICALPHABET

static void initlocalstackelements(State *state,
                                   Nodeinfo *nodeptr,
                                   Uint numofunits,
                                   Uint end)
{
  Uint i, offset;

  for(i=0, offset = 0; i <= end; i++, offset += numofunits)
  {
    nodeptr[i].nodeposlist = state->nodeposlistreservoir + offset;
  }
}

#endif

static void addtoposlist(State *state,Nodeinfo *ninfo,Uint base,
                         Previoussuffixtype leafnumber)
{
  Poslist *ptr;

  if(base >= state->alphabetsize)
  {
    ninfo->uniquecharposlength++;
    CHECKARRAYSPACE(&state->uniquechar,Previoussuffixtype,32);
    state->uniquechar.spacePrevioussuffixtype[
           state->uniquechar.nextfreePrevioussuffixtype++] = leafnumber;
#ifdef ESAMERGEDACCESS
    DEBUG3(4,"uniquecharpos[%lu]=(%lu at index %lu)\n",
            (Showuint) (state->uniquechar.nextfreePrevioussuffixtype-1),
            (Showuint) leafnumber.startpos,
            (Showuint) leafnumber.idx);
#else
    DEBUG2(4,"uniquecharpos[%lu]=%lu\n",
            (Showuint) (state->uniquechar.nextfreePrevioussuffixtype-1),
            (Showuint) leafnumber);
#endif
    DEBUG1(4,"uniquecharpositions=%lu\n",
            (Showuint) ninfo->uniquecharposlength);
  } else
  {
    ptr = &state->poslist[base];
    if(ptr->nextfreePoslist >= ptr->allocatedPoslist)
    {
      if(ptr->allocatedPoslist == MAXSTATICPOSLIST)
      {
        ALLOCASSIGNSPACE(ptr->refPoslist,NULL,Previoussuffixtype,
                         ptr->nextfreePoslist + MAXSTATICPOSLIST);
        memcpy(ptr->refPoslist,&ptr->spacePoslist[0],
               (size_t) (sizeof(Uint) * MAXSTATICPOSLIST));
      } else
      {
        ALLOCASSIGNSPACE(ptr->refPoslist,ptr->refPoslist,Previoussuffixtype,
                         ptr->allocatedPoslist + MAXSTATICPOSLIST);
      }
      ptr->allocatedPoslist += MAXSTATICPOSLIST;
    }
    ptr->refPoslist[ptr->nextfreePoslist++] = leafnumber;
    NODEPOSLISTLENGTH(ninfo,base)++;
    // ninfo->nodeposlist[base].length++;
  }
}

static void concatlists(State *state,Nodeinfo *father,Nodeinfo *son)
{
  Uint base;

  for(base = 0; base < state->alphabetsize; base++)
  {
    NODEPOSLISTLENGTH(father,base) += NODEPOSLISTLENGTH(son,base);
    // father->nodeposlist[base].length += son->nodeposlist[base].length;
  }
  father->uniquecharposlength += son->uniquecharposlength;
  DEBUG2(4,"uniquecharpositions[start %lu, length %lu]\n",
            (Showuint) father->uniquecharposstart,
            (Showuint) father->uniquecharposlength);
}

/* 
  compute [output (a,pos) | a<-list]
*/

static Sint cartproduct1(State *state,Nodeinfo *ninfo,Uint base,
                         Previoussuffixtype leafnumber)
{
  Listtype *pl;
  Previoussuffixtype *spptr, *start;

  pl = &NODEPOSLISTENTRY(ninfo,base);
  // pl = ninfo->nodeposlist + base;
  start = state->poslist[base].refPoslist + pl->start;
  for(spptr = start; spptr < start + pl->length; spptr++)
  {
    if(state->output(state->outinfo,state->depth,leafnumber,*spptr) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

/* 
  compute [output (a,b) | a<-left, b<-right]
*/

static Sint cartproduct2(State *state,
                         Nodeinfo *ninfo1, Uint base1,
                         Nodeinfo *ninfo2, Uint base2)
{
  Listtype *pl1, *pl2;
  Previoussuffixtype *start1, *start2, *spptr1, *spptr2;

  pl1 = &NODEPOSLISTENTRY(ninfo1,base1);
  // pl1 = ninfo1->nodeposlist + base1;
  start1 = state->poslist[base1].refPoslist + pl1->start;
  pl2 = &NODEPOSLISTENTRY(ninfo2,base2);
  // pl2 = ninfo2->nodeposlist + base2;
  start2 = state->poslist[base2].refPoslist + pl2->start;
  for(spptr1 = start1; spptr1 < start1 + pl1->length; spptr1++)
  {
    for(spptr2 = start2; spptr2 < start2 + pl2->length; spptr2++)
    {
      if(state->output(state->outinfo,state->depth,*spptr1,*spptr2) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  return 0;
}

static void setpostabto0(State *state)
{
  Uint base;

  if(!state->initialized)
  {
    for(base = 0; base < state->alphabetsize; base++)
    {
      state->poslist[base].nextfreePoslist = 0;
    }
    state->uniquechar.nextfreePrevioussuffixtype = 0;
    state->initialized = True;
  }
}

/* 
   check if father of leaf with leafnumber is left diverse. 
   If it is not already left diverse, then there is a common character.
   If this common character is different from the character \(currentchar\)
   to the left of suffix \(S_{leafnumber}\), then father becomes left diverse.
   If father is left diverse, then output the cartesian product of all 
   positions at the father (for left characters different from currentchar)
   with leafnumber.
*/

static Sint processleafedge(State *state,BOOL firstsucc,Nodeinfo *father,
                            Uchar leftchar,Previoussuffixtype leafnumber)
{
  Uint base;
  Previoussuffixtype *start, *spptr;

#ifdef ESAMERGEDACCESS
  DEBUG4(4,"processleafedge (%lu at idx %lu): firstsucc=%s, fatherdepth=%lu\n",
            (Showuint) leafnumber.startpos,
            (Showuint) leafnumber.idx,
            SHOWBOOL(firstsucc),
            (Showuint) father->depth);
#else
  DEBUG3(4,"processleafedge %lu: firstsucc=%s, fatherdepth=%lu\n",
            (Showuint) leafnumber,
            SHOWBOOL(firstsucc),
            (Showuint) father->depth);
#endif
  if(father->depth < state->searchlength)
  {
    setpostabto0(state);
    return 0;
  }
  state->initialized = False;
  state->depth = father->depth;
  DEBUG1(4,"processleafedge: leftchar %lu\n",(Showuint) leftchar);
  if(firstsucc)
  {
    father->commonchar = leftchar;
    father->uniquecharposlength = 0;
    father->uniquecharposstart = state->uniquechar.nextfreePrevioussuffixtype;
    for(base = 0; base < state->alphabetsize; base++)
    {
      NODEPOSLISTSTART(father,base) = state->poslist[base].nextfreePoslist;
      // father->nodeposlist[base].start = state->poslist[base].nextfreePoslist;
      NODEPOSLISTLENGTH(father,base) = 0;
      // father->nodeposlist[base].length = 0;
    }
    addtoposlist(state,father,leftchar,leafnumber);
    return 0;
  }
  if(father->commonchar != ISLEFTDIVERSE)
  {
    CHECKCHAR(leftchar);
  }
  if(father->commonchar == ISLEFTDIVERSE)
  {
    for(base = 0; base < state->alphabetsize; base++)
    {
      if(base != leftchar)
      {
        if(cartproduct1(state,father,base,leafnumber) != 0)
        {
          return (Sint) -1;
        }
      }
    }
    start = state->uniquechar.spacePrevioussuffixtype + 
            father->uniquecharposstart;
    for(spptr = start; spptr < start + father->uniquecharposlength; spptr++)
    {
      if(state->output(state->outinfo,state->depth,leafnumber,*spptr) != 0)
      {
        return (Sint) -2;
      }
    }
  }
  addtoposlist(state,father,leftchar,leafnumber);
  return 0;
}

/* 
  process father and son
  if father is not left diverse then there is a common character cf.
  if son is left diverse then father becomes left diverse
  if son is not left diverse, then there is a common character cs.
  if(cs != cf) then father becomes left diverse.
  if father is left diverse, then compute the cartesion product of 
  all positions with different left characters.
  finally append the lists of the son to the list of the father
*/

static Sint processbranch(State *state,BOOL firstsucc,Nodeinfo *father)
{
  Uint chfather, chson;
  Previoussuffixtype *start, *spptr, *fptr, *fstart;
  Nodeinfo *son;

  DEBUG2(4,"processbranch: firstsucc=%s, fatherdepth=%lu\n",
            SHOWBOOL(firstsucc),
            (Showuint) father->depth);
  if(father->depth < state->searchlength)
  {
    setpostabto0(state);
    return 0;
  }
  state->initialized = False;
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
    start = state->uniquechar.spacePrevioussuffixtype + son->uniquecharposstart;
    for(chfather = 0; chfather < state->alphabetsize; chfather++)
    {
      for(chson = 0; chson < state->alphabetsize; chson++)
      {
        if(chson != chfather)
        {
          if(cartproduct2(state,father,chfather,son,chson) != 0)
          {
            return (Sint) -1;
          }
        }
      }
      for(spptr = start; spptr < start + son->uniquecharposlength; spptr++)
      {
        if(cartproduct1(state,father,chfather,*spptr) != 0)
        {
          return (Sint) -2;
        }
      }
    }
    fstart = state->uniquechar.spacePrevioussuffixtype + 
             father->uniquecharposstart;
    for(fptr = fstart; fptr < fstart + father->uniquecharposlength; fptr++)
    {
      for(chson = 0; chson < state->alphabetsize; chson++)
      {
        if(cartproduct1(state,son,chson,*fptr) != 0)
        {
          return (Sint) -3;
        }
      }
      for(spptr = start; spptr < start + son->uniquecharposlength; spptr++)
      {
        if(state->output(state->outinfo,state->depth,*fptr,*spptr) != 0)
        {
          return (Sint) -4;
        }
      }
    }
  }
  concatlists(state,father,son);
  return 0;
}

#define WITHSUFFIX
#define WITHLEFTCHAR
#include "vdfstrav.c"

/* 
   depth first traversal of the tree.
   push branch node on stack when visited first.
   suppose there is an edge \(father -> son\) such that son is the root of
   a subtree which has been processed. 
   If son is leaf then call processleafedge(father,son). 
   If son is not a leaf, then call processbranch(father,son)
*/

Sint VMATMAXOUT(ACCESSTYPE *ACCESSPARAMETER,
                Uint numberofprocessors,
                Uint searchlength,
                /*@unused@*/ void *repeatgapspecinfo,
                void *outinfo,
                void *outputfunction)
{
  Uint base;
  Poslist *ptr;
  State state;

  DEBUG1(2,"searchlength=%lu\n",(Showuint) searchlength);

#ifndef DYNAMICALPHABET
  if(ACCESSPARAMETER->alpha.mapsize-1 > ALPHABETSIZE)
  {
    ERROR2("alphabet size %lu too large: maximum is %lu",
            (Showuint) (ACCESSPARAMETER->alpha.mapsize-1),
            (Showuint) ALPHABETSIZE);
    return (Sint) -2;
  }
#endif
  state.alphabetsize = ACCESSPARAMETER->alpha.mapsize-1;
  state.searchlength = searchlength;
  state.output = (OutputfunctionPrevioussuffixtype) outputfunction;
  state.outinfo = outinfo;
  state.initialized = False;

  INITARRAY(&state.uniquechar,Previoussuffixtype);
  for(base = 0; base < state.alphabetsize; base++)
  {
    ptr = state.poslist + base;
    ptr->refPoslist = &ptr->spacePoslist[0];
    ptr->nextfreePoslist = 0;
    ptr->allocatedPoslist = MAXSTATICPOSLIST;
  }
  if(depthfirstvstree(&state,numberofprocessors,ACCESSPARAMETER) != 0)
  {
    return (Sint) -3;
  }
  FREEARRAY(&state.uniquechar,Previoussuffixtype);
  for(base = 0; base < state.alphabetsize; base++)
  {
    ptr = state.poslist + base;
    if(ptr->allocatedPoslist > MAXSTATICPOSLIST)
    {  
      FREESPACE(ptr->refPoslist);
    }
  }
#ifdef DYNAMICALPHABET
  FREESPACE(state.nodeposlistreservoir);
#endif
  return 0;
}
