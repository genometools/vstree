
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "fhandledef.h"
#include "genfile.h"
#include "outinfo.h"
#include "matchinfo.h"
#include "mparms.h"

#include "cmpalpha.pr"
#include "filehandle.pr"
#include "echomatch.pr"
#include "detmatch.pr"
#include "multiseq-adv.pr"

#define COMPARESTOREMATCHINT(INT)\
        if(m1->INT != m2->INT)\
        {\
          fprintf(stderr,"%s.%s: %lu != %lu\n","compareStoreMatch",#INT,\
                  (Showuint) m1->INT,(Showuint) m2->INT);\
          exit(EXIT_FAILURE);\
        }

#define COMPARESTOREMATCHDOUBLE(DOUBLE)\
        if(m1->DOUBLE != m2->DOUBLE)\
        {\
          fprintf(stderr,"%s.%s: %.2e != %.2e\n","compareStoreMatch",#DOUBLE,\
                  m1->DOUBLE,m2->DOUBLE);\
          fprintf(stderr,"difference = %.2e\n",m1->DOUBLE - m2->DOUBLE);\
        }

#define SHOWFLAGS(F)\
        if((F) & FLAGQUERY)\
        {\
          fprintf(stderr,"FLAGQUERY ");\
        }\
        if((F) & FLAGPALINDROMIC)\
        {\
          fprintf(stderr,"FLAGPALINDROMIC ");\
        }\
        if((F) & FLAGSELFPALINDROMIC)\
        {\
          fprintf(stderr,"FLAGSELFPALINDROMIC ");\
        }\
        if((F) & FLAGSCOREMATCH)\
        {\
          fprintf(stderr,"FLAGSCOREMATCH ");\
        }\
        fprintf(stderr,"\n")

static Sint echoimmediately(Multiseq *virtualmultiseq,
                            Alphabet *virtualalpha,
                            Multiseq *querymultiseq,
                            StoreMatch *storematch,
                            Outinfo *outinfo,
                            Uint len)
{
  if(len > 0)
  {
    Uchar *eptr;
    Uint sizeofram;
    Tmpfiledesc tmpfiledesc;

    inittmpfiledesc(&tmpfiledesc);
    eptr = echomatch2ram(False,
                         outinfo->showmode,
                         &outinfo->showdesc,
                         outinfo->showstring,
                         &outinfo->digits,
                         virtualmultiseq,
                         virtualalpha,
                         querymultiseq,
                         storematch,
                         len,
                         &sizeofram,
                         &tmpfiledesc);
    if(eptr == NULL)
    {
      return (Sint) -1;
    }
    if(WRITETOFILEHANDLE(eptr,(Uint) sizeof(Uchar),sizeofram,stdout) != 0)
    {
      return (Sint) -2;
    }
    if(clearMatchram(&eptr,&tmpfiledesc) != 0)
    {
      return (Sint) -3;
    }
    wraptmpfiledesc(&tmpfiledesc);
  }
  return 0;
}

static void compareStoreMatch(StoreMatch *m1,StoreMatch *m2)
{
  COMPARESTOREMATCHINT(Storeflag);
  COMPARESTOREMATCHINT(Storedistance);
  COMPARESTOREMATCHINT(Storeposition1);
  COMPARESTOREMATCHINT(Storelength1);
  COMPARESTOREMATCHINT(Storeseqnum1);
  COMPARESTOREMATCHINT(Storerelpos1);
  COMPARESTOREMATCHINT(Storelength2);
  COMPARESTOREMATCHINT(Storeposition2);
  COMPARESTOREMATCHINT(Storeseqnum2);
  COMPARESTOREMATCHINT(Storerelpos2);
  //COMPARESTOREMATCHDOUBLE(StoreEvalue);
}

static void comparematchtab(ArrayStoreMatch *matchtab1,
                            ArrayStoreMatch *matchtab2)
{
  Uint i;

  if(matchtab1->nextfreeStoreMatch != matchtab2->nextfreeStoreMatch)
  {
    fprintf(stderr,"%s: %lu != %lu\n","comparematchtab.nextfree",
                  (Showuint) matchtab1->nextfreeStoreMatch,
                  (Showuint) matchtab2->nextfreeStoreMatch);
    exit(EXIT_FAILURE);
  }
  for(i=0; i<matchtab1->nextfreeStoreMatch; i++)
  {
    compareStoreMatch(&matchtab1->spaceStoreMatch[i],
                      &matchtab2->spaceStoreMatch[i]);
  }
}

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

MAINFUNCTION
{
  Matchinfo matchinfo1, matchinfo2;

  DEBUGLEVELSET;
  if(argc <= 1)
  {
    fprintf(stderr,"%s requires at least one argument\n",argv[0]);
    return EXIT_FAILURE;
  }
  if(determineMatchinfo(NULL,
                        &matchinfo1,
                        argc-1,
                        argv+1,
                        NULL,
                        NULL,
                        NULL,
                        True,
                        showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  if(determineMatchinfo(NULL,
                        &matchinfo2,
                        argc,
                        argv,
                        argv[1],  // from matchfile
                        NULL,
                        NULL,
                        True,
                        showonstdout) != 0)
  {
    STANDARDMESSAGE;
  } 
  comparematchtab(&matchinfo1.matchtab,&matchinfo2.matchtab);
#ifdef DEBUG
  compareMultiseq(matchinfo1.outinfo.outvms,
                  matchinfo2.outinfo.outvms);
  if(compareAlphabet(matchinfo1.outinfo.outvmsalpha,
                     matchinfo2.outinfo.outvmsalpha) != 0)
  {
    STANDARDMESSAGE;
  }
#endif
  if(echoimmediately(matchinfo1.outinfo.outvms,
                     matchinfo1.outinfo.outvmsalpha,
                     matchinfo1.outinfo.outqms,
                     matchinfo1.matchtab.spaceStoreMatch,
                     &matchinfo1.outinfo,
                     matchinfo1.matchtab.nextfreeStoreMatch) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freeMatchinfo(&matchinfo1) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freeMatchinfo(&matchinfo2) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
