
#include <stdio.h>
#include "intbits.h"
#include "debugdef.h"
#include "genfile.h"
#include "outinfo.h"
#include "inputsymbol.h"
#include "markinfo.h"

#include "multiseq.pr"
#include "multiseq-adv.pr"

static void showcharacterdistribution(FILE *outfp,
                                      Uchar *characters,
                                      Uchar *seq,
                                      Uint len)
{
  Uint i, count[UCHAR_MAX+1] = {0};

  for(i=0; i<len; i++)
  {
    count[(Uint) seq[i]]++;
  }
  for(i=0; i<=UCHAR_MAX; i++)
  {
    if(count[i] > 0)
    {
      fprintf(outfp," %c=%.2f",characters[i],(double) count[i]/len);
    }
  }
}

static Sint shownomatch(Multiseq *multiseq,
                        Alphabet *alpha,
                        Uint posoffset,
                        Uint seqnumoffset,
                        Uint start,
                        Uint seqnum,
                        Uint seqnumstart,
                        Uint len,
                        Uint showstring,
                        Uint showmode,
                        Showdescinfo *showdesc,
                        FILE *outfp)
{
#ifdef DEBUG
  PairUint pos;

  if(pos2pospair(multiseq,&pos,start) != 0)
  {
    return (Sint) -1;
  }
  if(pos.uint0 - seqnumoffset != seqnum)
  {
    fprintf(stderr,"start = %lu: pos.uint0 = %lu != %lu seqnum\n",
             (Showuint) start,
             (Showuint) (pos.uint0 - seqnumoffset),
             (Showuint) seqnum);
    exit(EXIT_FAILURE);
  }
  if(pos.uint1 != start-posoffset-seqnumstart)
  {
    fprintf(stderr,"start = %lu: pos.uint1 = %lu != %lu = relpos\n",
             (Showuint) start,
             (Showuint) pos.uint0,
             (Showuint) (start-posoffset-seqnumstart));
    exit(EXIT_FAILURE);
  }
#endif
/*
  printf("posoffset=%lu\n",(Showuint) posoffset);
  printf("seqnumoffset=%lu\n",(Showuint) seqnumoffset);
  printf("start=%lu\n",(Showuint) start);
  printf("seqnum=%lu\n",(Showuint) seqnum);
  printf("seqnumstart=%lu\n",(Showuint) seqnumstart);
  printf("len=%lu\n",(Showuint) len);
*/
  (void) putc(FASTASEPARATOR,outfp);
  if(showmode & SHOWFILE)
  {
    Sint fnum = getfilenum(multiseq,start-posoffset);
    if(fnum < 0)
    {
      return (Sint) -1;
    }
    fprintf(outfp,"%s ",multiseq->allfiles[fnum].filenamebuf);
  }
  if(showmode & SHOWABSOLUTE)
  {
    fprintf(outfp,"%lu",(Showuint) (start-posoffset));
  } else
  {
    if(showdesc->defined)
    {
      if(multiseq->descspace.spaceUchar == NULL)
      {
        fprintf(outfp,"sequence%lu",(Showuint) seqnum);
      } else
      {
        echothedescription(outfp,showdesc,multiseq,seqnum);
      }
    } else
    {
      fprintf(outfp,"%lu",(Showuint) seqnum);
    }
    fprintf(outfp," %lu",(Showuint) (start-posoffset-seqnumstart));
  }
  fprintf(outfp," %lu",(Showuint) len);
  if(showstring > 0)
  {
    showcharacterdistribution(outfp,
                              alpha->characters,
                              multiseq->sequence+start,
                              len);
    (void) putc('\n',outfp);
    if(formatseq(outfp,
                 alpha->mapsize,
                 alpha->characters,
                 alpha->undefsymbol,
                 showstring & MAXLINEWIDTH,
                 multiseq->sequence+start,
                 len) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    (void) putc('\n',outfp);
  }
  return 0;
}

static void nomatchexplain(Showverbose showverbose,
                           Uint nomatchlength,
                           Uint showmode,
                           Uint showstring,
                           Showdescinfo *showdesc)
{
  Sprintfreturntype start;
  char msg[512];

  sprintf(&msg[0],"output regions of minimum length %lu "
                  "containing no match",
                  (Showuint) nomatchlength);
  showverbose(msg);
  start = sprintf(&msg[0],"each region is characterized by a line "
                          "of the form %c",FASTASEPARATOR);
  if(showmode & SHOWFILE)
  {
    start += sprintf(&msg[start],"filename ");
  }
  if(showmode & SHOWABSOLUTE)
  {
    start += sprintf(&msg[start],"abs_startpos ");
  } else
  {
    if(showdesc->defined)
    {
      start += sprintf(&msg[start],"seq_desc ");
    } else
    {
      start += sprintf(&msg[start],"seq_num ");
    }
    start += sprintf(&msg[start],"startpos ");
  }
  start += sprintf(&msg[start],"length");
  if(showstring > 0)
  {
    start += sprintf(&msg[start]," character_distribution");
  }
  showverbose(msg);
  if(showstring > 0)
  {
    sprintf(&msg[0],"followed by the sequence itself");
    showverbose(msg);
  }
}
    
Sint nomatchsubstringsout(Showverbose showverbose,
                          Multiseq *multiseq,
                          Alphabet *alpha,
                          Uint *markmatchtab,
                          Uint posoffset,
                          Uint seqnumoffset,
                          Uint len,
                          FILE *outfp,
                          Uint showstring,
                          Uint showmode,
                          Showdescinfo *showdesc,
                          Nomatch *nomatch)
{
  Uint seqnum = 0, seqnumstart = 0, i, nextmarkpos,
       nomatchstart, undefnomatchstart;
  
  if(showverbose != NULL)
  {
    nomatchexplain(showverbose,
                   nomatch->nomatchlength,
                   showmode,
                   showstring,
                   showdesc);
  }
  if(multiseq->numofsequences == UintConst(1))
  {
    nextmarkpos = multiseq->totallength;
  } else
  {
    nextmarkpos = multiseq->markpos.spaceUint[0];
  }
  DEBUG3(3,"nomatchsubstringsout(posoffset=%lu,len=%lu,nomatchlength=%lu)\n",
            (Showuint) posoffset,
            (Showuint) len,
            (Showuint) nomatch->nomatchlength);
  nomatchstart = undefnomatchstart = posoffset + len;
  for(i=posoffset; i < posoffset + len; i++) 
  {
    if(ISIBITSET(markmatchtab,i)) 
    {
      if(nomatchstart != undefnomatchstart)
      {
        if(i - nomatchstart >= nomatch->nomatchlength)
        {
          if(shownomatch(multiseq,
                         alpha,
                         posoffset,
                         seqnumoffset,
                         nomatchstart,
                         seqnum,
                         seqnumstart,
                         i-nomatchstart,
                         showstring,
                         showmode,
                         showdesc,
                         outfp) != 0)
          {
            return (Sint) -1;
          }
        }
        nomatchstart = undefnomatchstart;
      }
    } else
    {
      if(nomatchstart == undefnomatchstart)
      {
        nomatchstart = i;
      } 
    }
    if(i == nextmarkpos)
    {
      seqnum++;
      seqnumstart = i+1;
      if(seqnum == multiseq->numofsequences - 1)
      {
        nextmarkpos = multiseq->totallength;
      } else
      {
        nextmarkpos = multiseq->markpos.spaceUint[seqnum];
      }
    }
  }
  if(nomatchstart != undefnomatchstart && 
     i - nomatchstart >= nomatch->nomatchlength)
  {
    if(shownomatch(multiseq,
                   alpha,
                   posoffset,
                   seqnumoffset,
                   nomatchstart,
                   seqnum,
                   seqnumstart,
                   i-nomatchstart,
                   showstring,
                   showmode,
                   showdesc,
                   outfp) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}
