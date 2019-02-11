

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "types.h"
#include "debugdef.h"
#include "chardef.h"
#include "drand48.h"

#include "reverse.pr"

#define MAXPATTERNLEN 1024

#ifdef DEBUG
static void showargs(char *argv[],Argctype argc)
{
  Argctype argnum;
  for(argnum=0; argnum<argc; argnum++)
  {
    fprintf(stderr,"%s ",argv[argnum]);
  }
  (void) putc('\n',stderr);
}

static BOOL bmhsearch(/*@unused@*/ void *info,Uchar *text,
                      Uint textlen,Uchar *pattern,
                      Uchar *patternright)
{
  Uint m, i, j, rmostocc[UCHAR_MAX+1] = {0};
  Sint k;

  m = (Uint) (patternright - pattern + 1);
  for (i=0; i<m-1; i++)
  {
    rmostocc[(Uint) pattern[i]] = i+1;
  }
  for(j = 0; j <= textlen-m; j += m-rmostocc[(Uint) text[j+m-1]])
  {
    for(k=(Sint) (m-1); k>=0 && pattern[k] == text[j+k]; k--)
      /* nothing */ ;
    if (k < 0)
    {
      return True;
      // pattern at position j+m
    }
  }
  return False;
}

static void showpatternstat(Uint *patternstat)
{
  Uint i;

  for(i=0; i<= (Uint) MAXPATTERNLEN; i++)
  {
    if(patternstat[i] > 0)
    {
      printf("%lu patterns of length %lu\n",(Showuint) patternstat[i],
                                            (Showuint) i);
    }
  }
}
#endif

void searchpatterngeneric(
       BOOL(*reallyoccurs)(void *,Uchar *,Uint,Uchar *,Uchar *),
       BOOL(*occurs)(void *,Uchar *,Uint,Uchar *,Uchar *),
       /*@unused@*/ char *argv[],
       /*@unused@*/ Argctype argc,
       void *occursinfo,
       Uchar *text,Uint textlen,float trialpercentage,
       Uint minpatternlen,
       Uint maxpatternlen,
       void (*showpattern)(void *,Uchar *,Uint),
       void *showpatterninfo)
{
  Uint pcount, j, trials, start, patternlen, patternstat[MAXPATTERNLEN+1] = {0};
  BOOL special;
  Uchar pattern[MAXPATTERNLEN+1];
#ifdef DEBUG
  BOOL patternoccurs, patternreallyoccurs;
#endif

  if(maxpatternlen > (Uint) MAXPATTERNLEN)
  {
    fprintf(stderr,"maxpatternlen=%lu > %lu\n",
            (Showuint) maxpatternlen,(Showuint) MAXPATTERNLEN);
    exit(EXIT_FAILURE);
  }
  if(maxpatternlen < minpatternlen)
  {
    fprintf(stderr,"maxpatternlen=%lu < %lu\n",(Showuint) maxpatternlen,
                                               (Showuint) minpatternlen);
    exit(EXIT_FAILURE);
  }
  if(textlen <= maxpatternlen)
  {
    fprintf(stderr,"textlen=%lu <= maxpatternlen = %lu\n",
                    (Showuint) textlen,
                    (Showuint) maxpatternlen);
    exit(EXIT_FAILURE);
  }
  if(trialpercentage >= 0.0)
  {
    trials = (Uint) (trialpercentage * textlen);
  } else
  {
    trials = (Uint) -trialpercentage;
  }
  printf("# trials %lu minpat %lu maxpat %lu\n",
             (Showuint) trials,
             (Showuint) minpatternlen,
             (Showuint) maxpatternlen);
  srand48(42349421);
  for(pcount=0; pcount<trials; pcount++)
  {
    if(minpatternlen == maxpatternlen)
    {
      patternlen = minpatternlen;
    } else
    {
      patternlen = (Uint) (minpatternlen + 
                           (drand48() * 
                            (double) (maxpatternlen-minpatternlen+1)));
    }
    patternstat[patternlen]++;
    start = (Uint) (drand48() * (double) (textlen-patternlen));
    if(start > textlen - patternlen)
    {
      fprintf(stderr,"Not enough characters left\n");
      exit(EXIT_FAILURE);
    }
    special = False;
    for(j=0; j<patternlen; j++)
    {
      pattern[j] = text[start+j];
      if(ISSPECIAL(pattern[j]))
      {
        special = True;
        break;
      }
    }
    if(!special)
    {
      if(pcount & 1)
      {
        reverseinplace(pattern,patternlen);
      }
#ifdef DEBUG
      patternoccurs = occurs(occursinfo,text,textlen,pattern,
                             pattern+patternlen-1);
      patternreallyoccurs 
        = reallyoccurs(occursinfo,text,textlen,pattern,pattern+patternlen-1);
      if(patternoccurs != patternreallyoccurs)
      {
        showargs(argv,argc);
        fprintf(stderr,"pattern %lu: \"",(Showuint) pcount);\
        showpattern(showpatterninfo,pattern,patternlen);
        fprintf(stderr,"\" %s found, this is not supposed to happen\n",
                   patternreallyoccurs ? "not" : "");
        exit(EXIT_FAILURE);
      }
#endif
    }
  }
  DEBUGCODE(1,showpatternstat(&patternstat[0]));
  DEBUG1(1,"%lu pattern processed as expected\n",(Showuint) trials); 
}

void searchpattern(BOOL(*occurs)(void *,Uchar *,Uint,Uchar *,Uchar *),
                   char *argv[],Argctype argc,
                   void *occursinfo,Uchar *text,Uint textlen,
                   float trialpercentage,
                   Uint minpatternlen,
                   Uint maxpatternlen,
                   void (*showpattern)(void *,Uchar *,Uint),
                   void *showpatterninfo)
{
#ifdef DEBUG
  searchpatterngeneric(bmhsearch,
#else
  searchpatterngeneric(NULL,
#endif
                       occurs,argv,argc,occursinfo,text,textlen,
                       trialpercentage,minpatternlen,maxpatternlen,
                       showpattern,showpatterninfo);
}

void searchpatternapprox(void(*apm)(void *,Uint,Uchar *,Uint,Uchar *,Uint),
                         /*@unused@*/ char *argv[],
                         /*@unused@*/ Argctype argc,
                         void *occursinfo,
                         float errorrate,
                         Uchar *text,Uint textlen,float trialpercentage,
                         Uint minpatternlen,
                         Uint maxpatternlen)
{
  Uint i, trials, start, patternlen, threshold;
  Uchar pattern[MAXPATTERNLEN+1];

  if(maxpatternlen > (Uint) MAXPATTERNLEN)
  {
    fprintf(stderr,"maxpatternlen=%lu > %lu\n",
            (Showuint) maxpatternlen,(Showuint) MAXPATTERNLEN);
    exit(EXIT_FAILURE);
  }
  if(textlen <= maxpatternlen)
  {
    fprintf(stderr,"textlen=%lu <= maxpatternlen = %lu\n",
                   (Showuint) textlen,
                   (Showuint) maxpatternlen);
    exit(EXIT_FAILURE);
  }
  if(trialpercentage >= 0.0)
  {
    trials = (Uint) (trialpercentage * textlen);
  } else
  {
    trials = (Uint) -trialpercentage;
  }
  DEBUG2(2,"#trials %lu maxpat %lu\n",(Showuint) trials,
                                      (Showuint) maxpatternlen);
  srand48(42349421);
  for(i=0; i<trials; i++)
  {
    patternlen = minpatternlen + 
                 (Uint) (drand48() * (double) (maxpatternlen-minpatternlen+1));
    threshold = (Uint) (errorrate * patternlen);
    DEBUG2(3,"m=%lu, k=%lu\n",(Showuint) patternlen,(Showuint) threshold);
    if(threshold >= minpatternlen)
    {
      fprintf(stderr,"threshold=%lu <= minpatternlen = %lu\n",
                  (Showuint) threshold,
                  (Showuint) minpatternlen);
      exit(EXIT_FAILURE);
    }
    start = (Uint) (drand48() * (double) (textlen-patternlen));
    memcpy(pattern,text + start,(size_t) patternlen);
    pattern[patternlen] = (Uchar) '\0';
    if(i & 1)
    {
      reverseinplace(pattern,patternlen);
    }
    apm(occursinfo,threshold,pattern,patternlen,text,textlen);
  }
  DEBUG1(1,"%lu pattern processed as expected\n",
                  (Showuint) trials); 
}
