#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <ctype.h>
#include "errordef.h"
#include "alphadef.h"
#include "scoredef.h"
#include "arraydef.h"
#include "fhandledef.h"
#include "debugdef.h"
#include "genfile.h"

#include "scanpaths.pr"
#include "readnextline.pr"
#include "filehandle.pr"
#include "cmpalpha.pr"

/*
  We use the following macro to access the \texttt{I}-th character of
  a line.
*/

#define LINE(I)          line.spaceUchar[I]


/*
  The following function is given an alphabet. It computes
  a table which maps each ASCII symbol to a unique integer in the range
  [0,mapsize-1]. All ASCII characters not occurring in the alphabet are
  assigned the undefsymbol of the alphabet.
*/

static void initonetoonesymbolmap(Alphabet *alpha)
{
  Uint i;

  for(i=0; i<=UCHAR_MAX; i++)
  {
    alpha->symbolmap[i] = alpha->undefsymbol;
  }
  for(i=0; i<alpha->mapsize; i++)
  {
    alpha->symbolmap[(Uint) alpha->characters[i]] = i;
  }
}

/*
  The following function reads the line defining the alphabet over which 
  the Blast formatted maxtrix is defined. The line is length \texttt{linelen}.
*/

static void readblastmatrixcharacters(Alphabet *alpha,Uchar *line,Uint linelen)
{
  Uint i, j;

  for(j=i=0; i<linelen; i++)
  {
    if(line[i] != (Uchar) ' ')
    {
      alpha->mapdomain[j] = (Uchar) line[i];
      alpha->characters[j++] = (Uchar) line[i];
      alpha->symbolmap[(Uint) line[i]] = i;
    }
  }
  alpha->characters[j] = (Uchar) '\0';
  alpha->characters[WILDCARD] = alpha->characters[j-1];
  alpha->domainsize = alpha->mapsize = j;
  alpha->mappedwildcards = UintConst(1);
}

/*
  The following function reads a score line of a Blast matrix.
*/

static Sint scanscoreline(Scorematrix *scorematrix,
                          Uchar *line,Uint linelen,Uint linecount)
{
  Uint i = UintConst(1);
  Scaninteger readint;
  BOOL scanned = False;

  while(True)
  {
    if(i==linelen)
    {
      break;
    }
    if(line[i] == (Uchar) ' ' || line[i] == (Uchar) '\t')
    {
      i++;
      scanned = False;
    } else
    {
      if(line[i] == (Uchar) '-' || isdigit((Ctypeargumenttype) line[i]))
      {
        if(!scanned)
        {
          if(sscanf((char *) (line+i),"%ld",&readint) != 1)
          {
            ERROR2("line %lu: corrupted line\n\"%s\"",(Showuint) linecount,
                                                      (char *) (line+i));
            return (Sint) -1;
          }
          if(readint < 0)
          {
            scorematrix->negativevalues = True;
          }
          scorematrix->scoretab[scorematrix->nextfreeint++] = (SCORE) readint;
          scanned = True;
        } else
        {
          i++;
        }
      }
    }
  }
  return 0;
}

static SCORE minimumscorevalue(SCORE *scoretab,Uint numofentries)
{
  Uint i;
  SCORE minvalue;

  if(numofentries == 0)
  {
    fprintf(stderr,"no entries in scoretab\n");
    exit(EXIT_FAILURE);
  }
  minvalue = scoretab[0];
  for(i=UintConst(1); i<numofentries; i++)
  {
    if(minvalue > scoretab[i])
    {
      minvalue = scoretab[i];
    }
  }
  return minvalue-1;
}

void initmulttab(Uint *multtab,Uint rows,Uint columns)
{
  Uint i, sum;

  for(i=0, sum=0; i<rows; i++, sum+=columns)
  {
    multtab[i] = sum;
  }
}


/*EE
  The following function reads a score matrix in Blast format. \texttt{bmfile}
  is the input file bmfile. The alphabet structure is maintained accordingly.
  The function returns a negative value if an error occurred. Otherwise
  the return code is 0.
  The environment variable \texttt{envstring} is read to get the list of 
  directories which are searched for the score matrix.
*/

Sint readblastmatrixgeneric(const char *bmfile,
                            Uint undefsymbol,
                            BOOL readalpha,
                            Alphabet *alpha,
                            Scorematrix *scorematrix,
                            char *envstring,
                            SCORE wildcardmismatch)
{
  Uint scoreline = 0, linecount = 0;
  ArrayUchar line;
  FILE *fpin;

  fpin = scanpathsforfile(envstring,bmfile);
  if(fpin == NULL)
  {
    ERROR2("file \"%s\" not found in directory list specified by "
           "environment variable %s",bmfile,envstring);
    return (Sint) -1;
  }
  alpha->undefsymbol = undefsymbol;
  scorematrix->negativevalues = False;
  INITARRAY(&line,Uchar);
  while(True)
  {
    line.nextfreeUchar = 0;
    if(readnextline(fpin,&line) == (Sint) EOF)
    {
      break;
    }
    NOTSUPPOSEDTOBENULL(line.spaceUchar);
    linecount++;
    if(line.nextfreeUchar > 0)
    {
      if(LINE(0) != (Uchar) '#')
      {
        if(LINE(0) == (Uchar) ' ')
        {
          if(readalpha)
          {
            readblastmatrixcharacters(alpha,line.spaceUchar,
                                      line.nextfreeUchar); 
            initonetoonesymbolmap(alpha);
          } else
          {
            Alphabet newalpha;

            newalpha.undefsymbol = undefsymbol;
            readblastmatrixcharacters(&newalpha,line.spaceUchar,
                                      line.nextfreeUchar); 
            initonetoonesymbolmap(&newalpha);
            initonetoonesymbolmap(alpha);
            if(compareAlphabet(alpha,&newalpha) != 0)
            {
              return (Sint) -2;
            }
          }
          ALLOCASSIGNSPACE(scorematrix->scoretab,NULL,SCORE,
                           alpha->mapsize*alpha->mapsize);
          scorematrix->nextfreeint = 0;
          initmulttab(scorematrix->multtab,alpha->mapsize,alpha->mapsize);
        } else
        {
          if(alpha->characters[scoreline] != LINE(0))
          {
            ERROR2("line %lu: first character must be %c",(Showuint) linecount,
                                                          LINE(0));
            return (Sint) -3;
          }
          if(scanscoreline(scorematrix,line.spaceUchar,line.nextfreeUchar,
                           linecount) != 0)
          {
            return  (Sint) -4;
          }
          scoreline++;
        }
      }
    }
  }
  if(wildcardmismatch == 0)
  {
    scorematrix->wildcardmismatch 
      = minimumscorevalue(scorematrix->scoretab,scorematrix->nextfreeint);
  } else
  {
    scorematrix->wildcardmismatch = wildcardmismatch;
  }
  if(DELETEFILEHANDLE(fpin) != 0)
  {
    return (Sint) -5;
  }
  FREEARRAY(&line,Uchar);
  return 0;
}

/*EE
  The following function reads a score matrix in Blast format. \texttt{bmfile}
  is the input file bmfile. The alphabet structure is maintained accordingly.
  The function returns a negative value if an error occurred. Otherwise
  the return code is 0.
*/

Sint readblastmatrix(const char *bmfile,Uint undefsymbol,BOOL readalpha,
                     Alphabet *alpha,Scorematrix *scorematrix,
                     SCORE wildcardmismatch)
{
  return readblastmatrixgeneric(bmfile,undefsymbol,readalpha,alpha,scorematrix,
                                "SCOREMATRIXDIR",wildcardmismatch);
}

/*EE
  The following function frees the dynamically allocated space for a 
  Blast matrix.
*/

void freeblastmatrix(Scorematrix *scoremtrix)
{
  FREESPACE(scoremtrix->scoretab);
}

void fillscorematrix(Scorefunction *scorefunction,
                     Alphabet *alpha,
                     SCORE matchscore,SCORE mismatchscore,SCORE indelscore)
{
  Uint i, j, idx;

  scorefunction->insertionscore = scorefunction->deletionscore = indelscore;
  scorefunction->scorematrix.negativevalues = True;
  scorefunction->scorematrix.nextfreeint 
    = (alpha->mapsize-1) * (alpha->mapsize-1);
  initmulttab(scorefunction->scorematrix.multtab,
              alpha->mapsize-1,
              alpha->mapsize-1);
  scorefunction->scorematrix.wildcardmismatch = mismatchscore;
  ALLOCASSIGNSPACE(scorefunction->scorematrix.scoretab, NULL, SCORE, 
                   scorefunction->scorematrix.nextfreeint);
  for(i=0; i<alpha->mapsize-1; i++)
  {
    for(j=0; j<alpha->mapsize-1; j++)
    {
      idx = SCOREPAIR2INDEX(&scorefunction->scorematrix,i,j);
      scorefunction->scorematrix.scoretab[idx]
        = (i == j) ? matchscore : mismatchscore;
    }
  }
}

/*
  Show an alphabet and a score matrix in the format use by Blast. 
*/

void showblastmatrix(Alphabet *alpha,Scorematrix *scorematrix)
{
  Uint i, j;

  printf("   ");
  for(j = 0; j < alpha->mapsize; j++)
  {
    printf("%c",alpha->characters[j]);
    printf("%s",(j < alpha->mapsize - 1) ? "  " : "\n");
  }
  for(i = 0; i < alpha->mapsize - 1; i++)
  {
    printf("%c ",alpha->characters[i]);
    for(j = 0; j < alpha->mapsize; j++)
    {
      if(j < alpha->mapsize - 1)
      {
        printf("%2ld ",(Showsint) GETSCORE(scorematrix,i,j));
      } else
      {
        printf("%2ld\n",(Showsint) scorematrix->wildcardmismatch);
      }
    }
  }
  printf("%c ",alpha->characters[alpha->mapsize-1]);
  for(j = 0; j < alpha->mapsize; j++)
  {
    if(j < alpha->mapsize - 1)
    {
      printf("%2ld ",(Showsint) scorematrix->wildcardmismatch);
    } else
    {
      printf("%2d\n",1);
    }
  }
}

void showscoredistribution(Alphabet *alpha,Scorematrix *scorematrix)
{
  Uint i, j, *dist;
  SCORE score, maxscore = 0, minscore = 0;
  Uint distwidth;

  for(i = 0; i < alpha->mapsize; i++)
  {
    for(j = 0; j < alpha->mapsize; j++)
    {
      score = GETSCORE(scorematrix,i,j);
      if(i == 0 && j == 0)
      {
        minscore = maxscore = score;
      } else
      {
        if(score < minscore)
        {
          minscore = score;
        } else
        {
          if(score > maxscore)
          {
            maxscore = score;
          }
        }
      }
    } 
  } 
  distwidth = (Uint) (maxscore - minscore + 1);
  ALLOCASSIGNSPACE(dist,NULL,Uint,distwidth);
  for(score = 0; score < (SCORE) distwidth; score++)
  {
    dist[score] = 0;
  }
  for(i = 0; i < alpha->mapsize; i++)
  {
    for(j = 0; j < alpha->mapsize; j++)
    {
      score = GETSCORE(scorematrix,i,j);
      dist[score - minscore]++;
    }
  }
  for(score = 0; score < (SCORE) distwidth; score++)
  {
    if(dist[score] > 0)
    {
      printf("%ld %ld\n",(Showsint) (score + minscore),(Showsint) dist[score]);
    }
  }
}
