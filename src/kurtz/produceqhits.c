#include <math.h>
#include "types.h"
#include "divmodmul.h"
#include "chardef.h"
#include "debugdef.h"
#include "arraydef.h"
#include "minmax.h"
#include "vnodedef.h"
#include "virtualdef.h"

#include "alphabet.pr"
#include "maxpref.pr"

#include "qgram2code.c"

/*
static BOOL extendqgramtoright(Uchar *seq0,
                               Uchar *endseq0,
                               Uchar *seq1,
                               Uchar *endseq1,
                               Uint currentlength,
                               Uint extendlength)
{
  if(seq0 + extendlength - currentlength - 1 > endseq0)
  {
    return False;
  }
  if(seq1 + extendlength - currentlength - 1 > endseq1)
  {
    return False;
  }
  while(currentlength < extendlength)
  {
    if(*seq0 != *seq1 || ISSPECIAL(*seq0))
    {
      return False;
    }
    seq0++;
    seq1++;
    currentlength++;
  }
  return True;
}
*/

static Uint extendtorightmaximalmatch(Uchar *seq0,
                                      Uchar *endseq0,
                                      Uchar *seq1,
                                      Uchar *endseq1)
{
  Uchar *ptr0, *ptr1; 

  for(ptr0=seq0, ptr1 = seq1; ptr0 <= endseq0 && ptr1 <= endseq1; 
      ptr0++, ptr1++)
  {
    if(*ptr0 != *ptr1 || ISSPECIAL(*ptr0))
    {
      break;
    }
  }
  return (Uint) (ptr0 - seq0);
}

#ifdef DEBUG
static void checkthecode(Uint position,
                         BOOL codeokay,
                         Uint code,
                         Uint numofchars,
                         Uint prefixlength,
                         Uchar *qgram)
{
  Uint realcode = 0;
  BOOL reallycodeokay;

  reallycodeokay = qgram2code(&realcode,numofchars,prefixlength,qgram);
  if(reallycodeokay != codeokay)
  {
    fprintf(stderr,"position = %lu\n",(Showuint) position);
    fprintf(stderr,"reallycodeokay = %s != %s = codeokay\n",
                    SHOWBOOL(reallycodeokay),
                    SHOWBOOL(codeokay));
    exit(EXIT_FAILURE);
  }
  if(codeokay)
  {
    if(realcode != code)
    {
      fprintf(stderr,"position = %lu\n",(Showuint) position);
      fprintf(stderr,"realcode = %lu != %lu = code\n",
                      (Showuint) realcode,
                      (Showuint) code);
      exit(EXIT_FAILURE);
    }
  }
}
#endif

Sint produceqhitsimple(Uchar *dbseq,
                       Uint dblen,
                       Uint *suftab,
                       Uchar *queryseq,
                       Uint querylen,
                       Uint fixedmatchlength,
                       Outputfunction processqhit,
                       void *processinfo)
{
  Uchar *qgram;
  Uint sufindex;
  Vnode vnode;

  for(qgram = queryseq; qgram <= queryseq + querylen - fixedmatchlength; 
      qgram++)
  {
    vnode.offset = 0;
    vnode.left = 0;
    vnode.right = dblen;
    if(mmsearch(dbseq,dblen,suftab,&vnode,qgram,fixedmatchlength))
    {
      for(sufindex = vnode.left; sufindex <= vnode.right; sufindex++)
      {
        if(processqhit(processinfo,
                       fixedmatchlength,
                       suftab[sufindex],
                       (Uint) (qgram-queryseq)) != 0)
        {
          return (Sint) -1;
        }
      }
    }
  }
  return 0;
}

Sint produceqhits(Uchar *dbseq,
                  Uint dblen,
                  Uint *bcktab,
                  Uint *suftab,
                  Uchar *queryseq,
                  Uint querylen,
                  Uint numofchars,
                  Uint prefixlength,
		  BOOL onlyqhits,
		  Uint fixedmatchlength,
                  Outputfunction1 processposition,
                  Outputfunction processqhit,
                  void *processinfo)
{
  BOOL codeokay = False;
  Uchar *qgram, lchar, rchar;
  Uint code = 0, 
       sufindex, 
       jpos, 
       *bckptr,
       qgram2codecalls = 0,
       startpos,
       extendlength,
       mappower[UCHAR_MAX+1];
  Vnode vnode;

  initmappower(&mappower[0],numofchars,prefixlength);
  for(qgram = queryseq; qgram <= queryseq + querylen - fixedmatchlength; 
      qgram++)
  {
    if(codeokay)
    {
      lchar = *(qgram-1);
      if(ISSPECIAL(lchar))
      {
        codeokay = False;
      } else
      {
        rchar = *(qgram+prefixlength-1);
        if(ISSPECIAL(rchar))
        {
          codeokay = False;
        } else
        {
          if(numofchars == UintConst(4))
          {
            code = MULT4(code - mappower[(Uint) lchar]) + (Uint) rchar;
          } else
          {
            code = (code - mappower[(Uint) lchar]) * numofchars 
                   + (Uint) rchar;
          }
        }
      }
    } else
    {
      codeokay = qgram2code(&code,numofchars,prefixlength,qgram);
      qgram2codecalls++;
    }
#ifdef DEBUG
    checkthecode((Uint) (qgram-queryseq),
                 codeokay,
                 code,
                 numofchars,
                 prefixlength,
                 qgram);
#endif
    jpos = (Uint) (qgram - queryseq);
    if(processposition != NULL)
    {
      if(processposition(processinfo,jpos) != 0)
      {
        return (Sint) -1;
      }
    }
    if(codeokay)
    {
      bckptr = bcktab + MULT2(code);
      if(onlyqhits)
      {
        vnode.offset = prefixlength;
        vnode.left = *bckptr;
        if((vnode.right = *(bckptr+1)) > vnode.left)
        {
           vnode.right--;
           if(mmsearch(dbseq,dblen,suftab,
                      &vnode,qgram,
                      fixedmatchlength))
          {
            for(sufindex = vnode.left; sufindex <= vnode.right; sufindex++)
            {
              if(processqhit(processinfo,
                             fixedmatchlength,
                             suftab[sufindex],
                             jpos) != 0)
              {
                return (Sint) -1;
              }
            }
          }
        }
      } else
      {
        for(sufindex = *bckptr; sufindex < *(bckptr+1); sufindex++)
        {
          startpos = suftab[sufindex];
          if(startpos == 0 || 
             jpos == 0 ||
             ISSPECIAL(dbseq[startpos-1]) ||
             ISSPECIAL(queryseq[jpos-1]) ||
             dbseq[startpos-1] != queryseq[jpos-1])
          {
            extendlength
              = extendtorightmaximalmatch(dbseq+startpos+prefixlength,
                                          dbseq + dblen - 1,
                                          queryseq + jpos + prefixlength,
                                          queryseq + querylen - 1);
            if(extendlength + prefixlength >= fixedmatchlength)
            {
              if(processqhit(processinfo,
                             extendlength + prefixlength,
                             startpos,
                             jpos) != 0)
              {
                return (Sint) -1;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
