
#include "debugdef.h"
#include "match.h"
#include "fhandledef.h"
#include "outinfo.h"
#include "megabytes.h"
#include "chaindef.h"
#include "chaincall.h"
#include "threaddef.h"

#include "matsort.pr"
#include "chain2dim.pr"
#include "filehandle.pr"
#include "redblack.pr"

#include "echomatch.pr"
#include "procargs.pr"
#include "threadchain.pr"

typedef struct
{
  Chaincallinfo *chaincallinfo;
  void *voidoutinfo;
  StoreMatch *storematchtab;
  Uint chaincounter;
  char *argumentline;
} Vmchainoutinfo;

static void vmatchinitfragmentinfo(BOOL addterminal,
                                   Fragmentinfo *fragmentinfo,
                                   double weightfactor,
                                   StoreMatch *storematchtab,
                                   Uint numofmatches)
{
  StoreMatch *mptr;
  Fragmentinfo *fragmentinfoptr;
  Chainscoretype largestdim1 = 0, largestdim2 = 0;

  if(addterminal)
  {
    Chainscoretype tmp;

    for(mptr = storematchtab;
        mptr < storematchtab + numofmatches;
        mptr++)
    {
      tmp = (Chainscoretype) (mptr->Storeposition1 + mptr->Storelength1 - 1);
      if(largestdim1 < tmp)
      {
        largestdim1 = tmp;
      }
      tmp = (Chainscoretype) (mptr->Storeposition2 + mptr->Storelength2 - 1);
      if(largestdim2 < tmp)
      {
        largestdim2 = tmp;
      }
    }
  }
  for(mptr = storematchtab, fragmentinfoptr = fragmentinfo; 
      mptr < storematchtab + numofmatches; 
      mptr++, fragmentinfoptr++)
  {
    fragmentinfoptr->weight 
      = (Chainscoretype) (weightfactor * (double) ABS(DISTANCE2SCORE(mptr)));
    fragmentinfoptr->startpos[0] = mptr->Storeposition1;
    fragmentinfoptr->endpos[0] = mptr->Storeposition1 + mptr->Storelength1 - 1;
    fragmentinfoptr->startpos[1] = mptr->Storeposition2;
    fragmentinfoptr->endpos[1] = mptr->Storeposition2 + mptr->Storelength2 - 1;
    if(addterminal)
    {
      fragmentinfoptr->initialgap 
        = (Chainscoretype) (mptr->Storeposition1 + mptr->Storeposition2);
      fragmentinfoptr->terminalgap 
        = (Chainscoretype) (largestdim1 - fragmentinfoptr->endpos[0] + 
                            largestdim2 - fragmentinfoptr->endpos[1]);
    }
  }
}

static Sint echovmatchchain(FILE *showchainptr,
                            StoreMatch *storematchtab,
                            Chain *chain,
                            Outinfo *outinfo)
{
  Uint i;

  for(i=0; i<chain->chainedfragments.nextfreeUint; i++)
  {
    if(echomatch2file(showchainptr,
                      False,
                      outinfo->showmode,
                      &outinfo->showdesc,
                      outinfo->showstring,
                      &outinfo->digits,
                      outinfo->outvms,
                      outinfo->outqms,
                      outinfo->outvmsalpha,
                      storematchtab+chain->chainedfragments.spaceUint[i]) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint outvmatchchain(void *cpinfo,Chain *chain)
{
  Vmchainoutinfo *vmchainoutinfo = (Vmchainoutinfo *) cpinfo;
  Outinfo *outinfo;
  FILE *showchainptr;
  char chainoutfile[PATH_MAX+1];

  outinfo = (Outinfo *) vmchainoutinfo->voidoutinfo;
  if(vmchainoutinfo->chaincallinfo->outprefix == NULL)
  {
    showchainptr = outinfo->outfp;
  } else
  {
    Sprintfreturntype writtenbytes;

    writtenbytes = sprintf(chainoutfile,"%s-%lu.chain",
                           vmchainoutinfo->chaincallinfo->outprefix,        
                           (Showuint) vmchainoutinfo->chaincounter);
    if(writtenbytes >= PATH_MAX)
    {
      ERROR0("overflow when writing name of chainfile");
      return (Sint) -1;
    }
    showchainptr = CREATEFILEHANDLE(chainoutfile,WRITEMODE);
    if(showchainptr == NULL)
    {
      return (Sint) -2;
    }
    fprintf(showchainptr,"%s%s\n",ARGUMENTLINEPREFIX,
            vmchainoutinfo->argumentline);
  }
  fprintf(showchainptr,
         "# chain %lu: length %lu score %ld\n",
         (Showuint) vmchainoutinfo->chaincounter,
         (Showuint) chain->chainedfragments.nextfreeUint,
         (Showsint) chain->scoreofchain);
  if(!vmchainoutinfo->chaincallinfo->silent)
  {
    if(echovmatchchain(showchainptr,
                       vmchainoutinfo->storematchtab,
                       chain,
                       outinfo) != 0)
    {
      return (Sint) -4;
    }
  }
  if(vmchainoutinfo->chaincallinfo->outprefix != NULL)
  {
    if(DELETEFILEHANDLE(showchainptr) != 0)
    {
      return (Sint) -5;
    }
  }
  vmchainoutinfo->chaincounter++;
  return 0;
}

#ifndef NOSPACEBOOKKEEPING
static void showchainingspacepeak(Showverbose showverbose,
                                  Uint sizeofinstruct,
                                  Uint numofmatches)
{
  if(showverbose != NULL && numofmatches > 0)
  {
    Uint spacepeak = getspacepeak() + 
                     getredblacktreespacepeak() - 
                     (numofmatches * sizeofinstruct);
    char vbuf[80+1];
    sprintf(vbuf,"overall space peak: main=%.2f MB",MEGABYTES(spacepeak));
    showverbose(vbuf);
  }
}
#endif

static void possiblysortvmatchmatches(Showverbose showverbose,
                                      StoreMatch *tab,
                                      Uint tablen,
                                      Uint presortdim)
{
  StoreMatch *mptr;
  BOOL fragmentsaresorted = True;

  if(tablen > UintConst(1))
  {
    if(presortdim == 0)
    {
      for(mptr = tab; mptr < tab + tablen - 1; mptr++)
      {
        if(mptr->Storeposition1 > (mptr+1)->Storeposition1)
        {
          fragmentsaresorted = False;
          break;
        }
      }
    } else
    {
      for(mptr = tab; mptr < tab + tablen - 1; mptr++)
      {
        if(mptr->Storeposition2 > (mptr+1)->Storeposition2)
        {
          fragmentsaresorted = False;
          break;
        }
      }
    }
    if(!fragmentsaresorted)
    {
      if(showverbose != NULL)
      {
        showverbose("input fragments are not yet sorted => sort them");
      }
      // sort in ascending order of startposition of first or second position
      sortallmatches(tab, tablen, 
                     (presortdim == 0) ? UintConst(2) : UintConst(4));
    }
  }
}

static BOOL allmatchesarewithinborder(StoreMatch *allstorematchtab,
                                      Uint allnumofmatches)
{
  Uint seqnum1, seqnum2;
  StoreMatch *mptr;

  if(allnumofmatches == 0)
  {
    return True;
  }
  seqnum1 = allstorematchtab[0].Storeseqnum1;
  seqnum2 = allstorematchtab[0].Storeseqnum2;
  for(mptr = allstorematchtab+1; mptr < allstorematchtab + allnumofmatches;
      mptr++)
  {
    if(mptr->Storeseqnum1 != seqnum1 || mptr->Storeseqnum2 != seqnum2)
    {
      return False;
    }
  }
  return True;
}

static Sint computevmatchchains(char *argumentline,
                                Outinfo *outinfo,
                                StoreMatch *storematchtab,
                                Uint numofmatches,
                                Chaincallinfo *chaincallinfo,
                                Processmatch *finalprocessthread)
{
  Fragmentinfo *fragmentinfo;
  Chain chain;
  Sint retcode;
  Uint presortdim = UintConst(1);

  possiblysortvmatchmatches(chaincallinfo->showverbose,
                            storematchtab,
                            numofmatches,
                            presortdim);
  ALLOCASSIGNSPACE(fragmentinfo,NULL,Fragmentinfo,numofmatches);
  vmatchinitfragmentinfo((chaincallinfo->chainmode.chainkind == 
                          GLOBALCHAINING) ? False : True,
                         fragmentinfo,
                         chaincallinfo->weightfactor,
                         storematchtab,
                         numofmatches);
  INITARRAY(&chain.chainedfragments,Uint);
  if(chaincallinfo->dothreading)
  {
    Vmchainthreadinfo vmchainthreadinfo;

    vmchainthreadinfo.chaincallinfo = chaincallinfo;
    vmchainthreadinfo.voidoutinfo = (void *) outinfo;
    vmchainthreadinfo.storematchtab = storematchtab;
    vmchainthreadinfo.outvms = outinfo->outvms;
    vmchainthreadinfo.outqms = outinfo->outqms;
    vmchainthreadinfo.mapsize = outinfo->outvmsalpha->mapsize;
    vmchainthreadinfo.supermatchcounter = 0;
    vmchainthreadinfo.finalprocessthread = finalprocessthread;
    retcode = fastchaining(&chaincallinfo->chainmode,
                           &chain,
                           fragmentinfo,
                           numofmatches,
                           True, // gapcostL1
                           presortdim,
                           True,
                           threadchainedfragments,
                           (void *) &vmchainthreadinfo,
                           chaincallinfo->showverbose);
  } else
  {
    Vmchainoutinfo chainoutinfo;

    chainoutinfo.chaincallinfo = chaincallinfo;
    chainoutinfo.voidoutinfo = (void *) outinfo;
    chainoutinfo.storematchtab = storematchtab;
    chainoutinfo.chaincounter = 0;
    chainoutinfo.argumentline = argumentline;
    retcode = fastchaining(&chaincallinfo->chainmode,
                           &chain,
                           fragmentinfo,
                           numofmatches,
                           True, // gapcostL1
                           presortdim,
                           True,
                           outvmatchchain,
                           (void *) &chainoutinfo,
                           chaincallinfo->showverbose);
  }
  if(retcode < 0)
  {
    FREEARRAY(&chain.chainedfragments,Uint);
    FREESPACE(fragmentinfo);
    return (Sint) -2;
  }
  if(retcode == SintConst(1))
  {
    ERROR0("no chains of length > 1 with positive scores available\n");
    return (Sint) -3;
  }
#ifndef NOSPACEBOOKKEEPING
  showchainingspacepeak(chaincallinfo->showverbose,
                        (Uint) sizeof(StoreMatch),
                        numofmatches);
#endif
  FREEARRAY(&chain.chainedfragments,Uint);
  FREESPACE(fragmentinfo);
  return 0;
}


#define EVALDIAGONAL(SPTR)\
        ((Sint) (SPTR)->Storeposition2 - (Sint) (SPTR)->Storeposition1)

static void bucketintobins(StoreMatch *storematchtab,Uint numofmatches)
{
  StoreMatch *sptr;
  Sint numofdiags = 0, previousdiag = EVALDIAGONAL(storematchtab), currentdiag;

  for(sptr = storematchtab+1; sptr < storematchtab+numofmatches; sptr++)
  {
    currentdiag = EVALDIAGONAL(sptr);
    if(previousdiag < currentdiag)
    {
      fprintf(stderr,"previousdiag = %ld < %ld = currentdiag\n",
                      (Showsint) previousdiag,
                      (Showsint) currentdiag);
      exit(EXIT_FAILURE);
    }
    if(previousdiag > currentdiag)
    {
      numofdiags++;
    }
    previousdiag = currentdiag;
  }
  numofdiags++;
  printf("numofmatches=%lu\n",(Showuint) numofmatches);
  printf("numofdiags=%lu\n",(Showuint) numofdiags);
}

static Sint filterinterestingbins(char *argumentline,
                                  Outinfo *outinfo,
                                  StoreMatch *storematchtab,
                                  Uint numofmatches,
                                  Chaincallinfo *chaincallinfo,
                                  Processmatch *finalprocessthread)
{
  if(chaincallinfo->dothreading)
  {
    StoreMatch *sptr;

    qsortStorematchbydiagonal(storematchtab,storematchtab+numofmatches-1); 
    for(sptr = storematchtab; sptr < storematchtab+numofmatches; sptr++)
    {
      printf("diag %ld\n",
             (Showsint) ((Sint) sptr->Storeposition2 - 
                         (Sint) sptr->Storeposition1));
      if(echomatch2file(stdout,
                        False,
                        outinfo->showmode,
                        &outinfo->showdesc,
                        outinfo->showstring,
                        &outinfo->digits,
                        outinfo->outvms,
                        outinfo->outqms,
                        outinfo->outvmsalpha,
                        sptr) != 0)
      {
        return (Sint) -1;
      }
    }
    bucketintobins(storematchtab,numofmatches);
    return 0;
  }
  return computevmatchchains(argumentline,
                             outinfo,
                             storematchtab,
                             numofmatches,
                             chaincallinfo,
                             finalprocessthread);
}

static Sint groupandcomputevmatchchains(char *argumentline,
                                        Outinfo *outinfo,
                                        StoreMatch *allstorematchtab,
                                        Uint allnumofmatches,
                                        Uint numofsequences1,
                                        Chaincallinfo *chaincallinfo,
                                        Processmatch *finalprocessthread)
{
  ArrayStoreMatch matchbuffer;
  Uint *sortedpermutation, *pptr;

  ALLOCASSIGNSPACE(sortedpermutation,NULL,Uint,allnumofmatches);
  groupmatchesbyseqnum(sortedpermutation,
                       allstorematchtab,
                       allnumofmatches,
                       numofsequences1);
  INITARRAY(&matchbuffer,StoreMatch);
  for(pptr = sortedpermutation;
      pptr < sortedpermutation + allnumofmatches - 1;
      pptr++)
  {
    if(allstorematchtab[*pptr].Storeseqnum1 !=
       allstorematchtab[*(pptr+1)].Storeseqnum1 ||
       allstorematchtab[*pptr].Storeseqnum2 !=
       allstorematchtab[*(pptr+1)].Storeseqnum2)
    {
      STOREINARRAY(&matchbuffer,StoreMatch,128,allstorematchtab[*pptr]);
      if(filterinterestingbins(argumentline,
                               outinfo,
                               matchbuffer.spaceStoreMatch,
                               matchbuffer.nextfreeStoreMatch,
                               chaincallinfo,
                               finalprocessthread) != 0)
      {
        return (Sint) -1;
      }
      matchbuffer.nextfreeStoreMatch = 0;
    } else
    {
      STOREINARRAY(&matchbuffer,StoreMatch,128,allstorematchtab[*pptr]);
    }
  }
  STOREINARRAY(&matchbuffer,StoreMatch,128,allstorematchtab[*pptr]);
  if(filterinterestingbins(argumentline,
                           outinfo,
                           matchbuffer.spaceStoreMatch,
                           matchbuffer.nextfreeStoreMatch,
                           chaincallinfo,
                           finalprocessthread) != 0)
  {
    return (Sint) -2;
  }
  FREESPACE(sortedpermutation);
  FREEARRAY(&matchbuffer,StoreMatch);
  return 0;
}

Sint vmatchchaining(char *argumentline,
                    Outinfo *outinfo,
                    StoreMatch *allstorematchtab,
                    Uint allnumofmatches,
                    Uint numofsequences1,
                    Chaincallinfo *chaincallinfo,
                    Processmatch *finalprocessthread)
{
  if(allnumofmatches > 0)
  {
    if(chaincallinfo->withinborders && 
       !allmatchesarewithinborder(allstorematchtab,allnumofmatches))
    {
      if(groupandcomputevmatchchains(argumentline,
                                     outinfo,
                                     allstorematchtab,
                                     allnumofmatches,
                                     numofsequences1,
                                     chaincallinfo,
                                     finalprocessthread) != 0)
      {
        return (Sint) -2;
      }
    } else
    {
      if(filterinterestingbins(argumentline,
                               outinfo,
                               allstorematchtab,
                               allnumofmatches,
                               chaincallinfo,
                               finalprocessthread) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  return 0;
}
