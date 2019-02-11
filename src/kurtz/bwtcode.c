
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "debugdef.h"
#include "chardef.h"
#include "spacedef.h"

//}

/*EE
  This file implements decoding functions for the Burrows
  and Wheeler Transformation as described in the following paper:

  Balkenhol, B. and Kurtz, S.: Universal Data Compression Based on the 
  Burrows and Wheeler Transformation: Theory and Practice, IEEE Transactions
  on Computers, Vol 49(1):1043-1046, 2000.
*/

#define USEDBITS 24                              // the number of used bits
#define GETCHARVALUE(C)\
        ((Uchar) ((C) >> USEDBITS))              // 8 most significant
#define GETOFFSETVALUE(C)\
        ((Uint)  ((C) & ((UintConst(1) << USEDBITS) - 1))) // 24 least signifcant 
#define ASSIGNOFFSET(I)\
        acode = (Uint) lastcolumn[I];\
        offset[I] = (acode << USEDBITS) | count[acode]++

/*EE
  The function \texttt{bwtdecode} decodes the Burrows and
  Wheeler Transformation. It has fours arguments:
  \begin{itemize}
  \item
  The length \texttt{textlen} (i.e.\ \(n\)) of the encoded text.
  \item
  A pointer to a block of at least \(n+1\) unsigned integers to hold the
  table \texttt{offset}.
  \item
  An array \texttt{lastcolumn} (i.e.\ \(\varphi\)).
  \item
  an integer \texttt{longest}.
  \end{itemize}
  To obtain maximal speed we use pointer arithmetic. One
  easily observes that in the third phase of the algorithm
  the arrays \(\varphi\) and \(\mathit{offset}\) are accessed in parallel, i.e.\
  \(\varphi[i]\) is accessed \Iff\ \(\mathit{offset}[i]\) is accessed.
  In the case that \(n\) is very large, this leads to a poor cache coherence.
  In order to improve cache coherence, we store the character \(\varphi[i]\) 
  in the most significant 8 bits of the unsigned integer at address
  \(\texttt{offset}+i\). The remaining 24 bits of that unsigned integer 
  are used to hold the value \(\mathit{offset}[i]\). The encoding requires some 
  additional bit operations, but pays off for large files. 
  Using the unsigned integers to store the \(\varphi\)-characters has a 
  second advantage. The space \texttt{lastcolumn} points to is available in the 
  third phase. We use it to store the decoded string. Thus after the 
  third phase is completed, \texttt{lastcolumn} points to the decoded
  string.
  
  The first phase of the algorithm is split into two parts:
  One for-loop is over the indices strictly smaller than \(\mathit{longest}\)
  and the other for-loop is over the indices strictly larger than 
  \(\mathit{longest}\). In this way, it is not necessary to check whether 
  \(i=\mathit{longest}\) holds.
  We only have one array to hold the values for \(\mathit{count}\) and
  \(\mathit{base}\). The latter is computed from right to left.
*/

void bwtdecode(Uint textlen,Uint *offset,Uchar *lastcolumn,Uint longest)
{
  Uint i, acode, *offsetptr, *baseptr, count[UCHAR_MAX+1] = {0};
  Uchar *dcwordptr;

  for(i = 0; i < longest; i++)             // first phase, first part
  {
    ASSIGNOFFSET(i);
  }
  for(i = longest+1; i <= textlen; i++)    // first phase, second part
  {
    ASSIGNOFFSET(i);
  }
  baseptr = count + UCHAR_MAX;
  *baseptr = textlen - *baseptr;
  for(baseptr--; baseptr >= count; baseptr--) //   second phase: compute base
  {
    *baseptr = *(baseptr+1) - *baseptr;
  }
  offsetptr = offset + textlen; 
  for(dcwordptr = lastcolumn+textlen-1; dcwordptr >= lastcolumn; dcwordptr--) // third phase: do the decoding
  {
    *dcwordptr = GETCHARVALUE(*offsetptr);
    offsetptr = offset + count[(Uint) *dcwordptr] + GETOFFSETVALUE(*offsetptr);
  }
}

//\Ignore{

/* 
  The following function is like bwtdecode. It additionally takes an argument
  \texttt{sndstring} which is permuted in the same way as lastcolumn.
*/

void bwtdecode2(Uint textlen,Uint *offset,Uchar *lastcolumn,
                Uint longest,Uchar *sndstring)
{
  Uint i, j, oj, acode, *baseptr, count[UCHAR_MAX+1] = {0};
  Uchar *sndbuffer, *sndptr, *dcwordptr;

  for(i = 0; i < longest; i++)             // first phase, first part
  {
    ASSIGNOFFSET(i);
  }
  for(i = longest+1; i <= textlen; i++)    // first phase, second part
  {
    ASSIGNOFFSET(i);
  }
  baseptr = count + UCHAR_MAX;
  *baseptr = textlen - *baseptr;
  for(baseptr--; baseptr >= count; baseptr--) //   second phase: compute base
  {
    *baseptr = *(baseptr+1) - *baseptr;
  }
  ALLOCASSIGNSPACE(sndbuffer,NULL,Uchar,textlen);
  j = textlen; 
  for(dcwordptr = lastcolumn+textlen-1, sndptr = sndbuffer+textlen-1; 
      dcwordptr >= lastcolumn; 
      dcwordptr--,
      sndptr--) // third phase: do the decoding
  {
    oj = offset[j];
    *dcwordptr = GETCHARVALUE(oj);
    *sndptr = sndstring[j];
    j = count[(Uint) *dcwordptr] + GETOFFSETVALUE(oj);
  }
  memcpy(sndstring,sndbuffer,(size_t) textlen);
  FREESPACE(sndbuffer);
}

#ifdef DEBUG

void checkbwtdecode(char *filename,Uchar *text, Uint offset,
                    Uint textlen,Uchar *dcword)
{
  if(memcmp(dcword,text,(size_t) textlen) != 0)
  {
    fprintf(stderr,"mccsort for file \"%s\" (%lu,%lu) failed\n",
                    filename,(Showuint) offset,(Showuint) (offset+textlen-1));
    exit(EXIT_FAILURE);
  }
  DEBUG0(2,"checkbwtdecode ok\n");
}

#endif

//}

/*EE
  The following function is a variant of \texttt{bwtdecode} which additionally
  decodes the table \(\mathit{suftab}\) from the Burrows and Wheeler 
  Transformation. This is stored in \texttt{decodesuftab}. The decoded
  text is stored in \texttt{decodetext}. \texttt{offset} is the table
  to store the \texttt{offset}-table. When calling the function,
  the memory block for \texttt{decodesuftab} can also be used for 
  \texttt{offset}. The string \texttt{bwt} stores the Burrows and Wheeler
  Transformation. \texttt{longest} is the integer \(i\) such that
  \(\mathit{suftab}[i]=0\). \texttt{textlen} is the length of 
  the original text.
*/

void decodeburrowswheeler(Uint *decodesuftab,Uchar *decodetext,
                          Uint *offset,Uchar *bwt,Uint longest,Uint textlen)
{
  Sint j;
  Uchar acode;
  Uint offsetval, r, i, *baseptr, count[UCHAR_MAX+1] = {0};

  for(i = 0; i < longest; i++)             // first phase, first part
  {
    offset[i] = count[(Sint) bwt[i]]++;
  }
  for(i = longest+1; i <= textlen; i++)    // first phase, second part
  {
    offset[i] = count[(Sint) bwt[i]]++;
  }
  baseptr = count + UCHAR_MAX;
  *baseptr = textlen - *baseptr;
  for(baseptr--; baseptr >= count; baseptr--) //   second phase: compute base
  {
    *baseptr = *(baseptr+1) - *baseptr;
  }
  r = textlen; 
  offsetval = offset[r];
  for(j=(Sint) (textlen-1); j >= 0; j--) // third phase: do the decoding
  {
    acode = decodetext[j] = bwt[r];
    r = count[(Sint) acode] + offsetval;
    offsetval = offset[r];
    decodesuftab[r] = (Uint) j;
  }
  decodesuftab[textlen] = textlen;
}

/*EE
  The following function is a variant of \texttt{decodeburrowswheeler}
  which handles sequences containing the special symboles
  \texttt{WILDCARD} and \texttt{SEPARATOR} as defined in 
  \texttt{chardef.h}.
*/

void decodeburrowswheelerspecial(Uint *decodesuftab,Uchar *decodetext,
                                 Uint *offset,Uchar *bwt,Uint longest,
                                 Uint textlen)
{
  Sint j;
  Uchar acode;
  Uint markbase, countmarker = 0, offsetval, r, i, count[UCHAR_MAX+1] = {0},
       base[UCHAR_MAX+1];

  for(i = 0; i <= textlen; i++)             // first phase
  {
    if(i != longest)
    {
      acode = bwt[i];
      if(ISSPECIAL(acode))
      {
        countmarker++;
        offset[i] = 0;
      } else
      {
        offset[i] = count[(Sint) acode]++;
      }
    }
  }
  base[0] = 0;
  for(i=UintConst(1); i<=UCHAR_MAX; i++)   //   second phase: compute base
  {
    base[i] = base[i-1] + count[i-1];
  }
  r = textlen; 
  offsetval = offset[r];
  markbase = base[UCHAR_MAX] + countmarker - 1;
  for(j=(Sint) (textlen-1); j >= 0; j--) // third phase: do the decoding
  {
    acode = decodetext[j] = bwt[r];
    if(ISSPECIAL(acode))
    {
      r = markbase--;
    } else
    {
      r = base[(Sint) acode] + offsetval;
    }
    offsetval = offset[r];
    decodesuftab[r] = (Uint) j;
  }
  decodesuftab[textlen] = textlen;
}

//\Ignore{

#ifdef DEBUG
static void showencoding(Uchar *bwt,Uint *suftab,Uint textlen)
{
  Uint i, longest = 0;

  printf("# ");
  for(i=0; i<=textlen; i++)
  {
    if(suftab[i] == 0)
    {
      (void) putchar('~');
      longest = i;
    } else
    {
      if(isprint(bwt[i]))
      {
        (void) putchar((Fputcfirstargtype) bwt[i]);
      } else
      {
        (void) printf("\\%lu",(Showuint) bwt[i]);
      }
    }
  }
  printf("\n# longest=%lu\n",(Showuint) longest);
}
#endif

Sint encodeburrowswheeler(Uchar *bwt,Uint *suftab,
                          Uchar *text,Uint textlen)
{
  Uint i;
  
  for(i=0; i<=textlen; i++)
  {
    if(suftab[i] > 0)
    {
      bwt[i] = text[suftab[i] - 1];
    } else
    {
      /* We use this symbol only for the burrows and wheeler transformation */
      bwt[i] = (Uchar) UNDEFBWTCHAR;
    }
  }
  DEBUGCODE(3,showencoding(bwt,suftab,textlen));
  return 0;
}

//}
