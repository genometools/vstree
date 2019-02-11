//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "arraydef.h"
#include "fhandledef.h"
#include "debugdef.h"
#include "visible.h"
#include "spacedef.h"
#include "chardef.h"
#include "alphadef.h"
#include "errordef.h"
#include "failures.h"

#include "scanpaths.pr"
#include "dstrdup.pr"
#include "compfilenm.pr"
#include "readnextline.pr"
#include "filehandle.pr"
#include "verbosealpha.pr"

//}

/*EE
  This file implements the datatype \texttt{alphabet}.
*/

/*
  Some constants for the standard alphabet used. The name says it all.
*/

#define DNABASES                     "aAcCgGtTuU"
#define DNAWILDCARDS                 "nsywrkvbdhmNSYWRKVBDHM"
#define MAPSIZEDNA                   UintConst(5)
#define DNAALPHABETDOMAIN            DNABASES DNAWILDCARDS
#define PROTEINUPPERAMINOACIDS       "LVIFKREDAGSTNQYWPHMC"
#define MAPSIZEPROTEIN               UintConst(21)
#define PROTEINWILDCARDS             "XUBZJO*-"
#define PROTEINALPHABETDOMAIN        PROTEINUPPERAMINOACIDS PROTEINWILDCARDS  

/*
  We use the following macro to access the \texttt{I}-th character of
  a line.
*/

#define LINE(I)          line.spaceUchar[I]


/*EE
  The following function compares two alphabets. It returns 0, if the
  alphabets are identical. Otherwise, a negative error code is
  returned.
*/




/*EE
  The following function transforms a string in place according to a given 
  symbol map.
*/

Sint transformstring(Alphabet *alpha,Uchar *seq,Uint seqlen)
{ 
  Uint a;
  Uchar *tptr;

  DEBUG1(3,"new sequence of length %lu\n",(Showuint) seqlen);
  if(seqlen == 0)
  {
    ERROR0("empty file not allowed");
    return (Sint) -1;
  }
  for(tptr = seq; tptr < seq + seqlen; tptr++)
  {
    a = alpha->symbolmap[(Sint) *tptr];
    if(a == alpha->undefsymbol)
    {
      ERROR2("illegal character %c: only ascii characters from %s allowed",
             (char) *tptr,(char *) alpha->characters);
      return (Sint) -1;
    }
    DEBUG2(3,"seq[%lu]=%lu\n",(Showuint) (tptr-seq),(Showuint) a);
    *tptr = (Uchar) a;
  }
  return 0;
}

/*EE
  The following function transforms a string according to a given 
  symbol map and stores the result in outseq.
*/

Sint transformstringincopy(Alphabet *alpha,Uchar *outseq,
                           Uchar *inseq,Uint seqlen)
{ 
  Uint a, i;

  DEBUG1(3,"new sequence of length %lu\n",(Showuint) seqlen);
  if(seqlen == 0)
  {
    ERROR0("empty file not allowed");
    return (Sint) -1;
  }
  for(i = 0; i < seqlen; i++)
  {
    a = alpha->symbolmap[(Sint) inseq[i]];
    if(a == alpha->undefsymbol)
    {
      ERROR2("illegal character %c: only ascii characters from %s allowed",
             (char) inseq[i],(char *) alpha->characters);
      return (Sint) -1;
    }
    DEBUG2(3,"seq[%lu]=%lu\n",(Showuint) i,(Showuint) a);
    outseq[i] = (Uchar) a;
  }
  return 0;
}

/*EE
  We have developed a simple format to specify an alphabet 
  and a corresponding alphabet transformation. This format specifies the 
  characters of the alphabet (including wild card characters) 
  and the pairs of symbols which are to be considered identical. 
  The format is best explained by some examples. Consider a file 
  containing the following lines:
  \begin{alltt}
  aA
  cC
  gG
  tTuU
  nsywrkvbdhmNSYWRKVBDHM
  \end{alltt}
  These line specify that the input sequence
  are allowed to contain the symbols \(a,c,g,t,u,n,s,y\)
  \(w,r,k,v,b,d,h,m\) 
  in either lower or upper case. Moreover, the first four lines specify that
  \(a=A\), \(c=C\), \(g=G\), and \(t=T=u=U\). The last line specifies the 
  wildcard symbols, which are replaced by unique symbols. Note that any 
  wildcard symbol does not even match itself, if it occurs at different 
  positions. Thus no false matches will be delivered. Note that however, 
  a degenerate match may contain a wildcard, since this always leads 
  to a mismatch.

  Consider a file containing the following lines:
  \begin{alltt}
  LVIF i
  KR +
  ED -
  AG s
  ST o
  NQ n
  YW a
  P p
  H h
  M m
  C c
  XBZ* x
  \end{alltt}
  This specifies the Protein alphabet 
  \(L,V,I,F,K,R,E,D,A,G,S,T,N,Q,Y,W,P,H,M,C\) with some extra symbols
  \(X,U,B,Z,\ast\). All symbols occurring on the same line to the left of
  the first white space are considered to be pairwise equivalent. The symbol
  after the first white can be considered to be a comment. The symbols on
  the last line are considered to be wildcards. Again they are replaced 
  by a unique character. This is the given parameter wildcard, if 
  wildcard $>$ 0.
*/

/*
  The following function reads a file in the format as explained above.
  \texttt{mapfile} is the input filename. \texttt{fpin} is the corresponding
  file name. \texttt{undefsymbol} is the undefined symbol. If the argument
  \texttt{wildcard} is larger than 0, then the characters  in the last
  line of the symbol mapping file are mapped to \texttt{wildcard}. Otherwise,
  they are mapped to the value one smaller than the line number they appear
  in (counting from 1). The result of the parsing is stored in 
  \texttt{alpha}.
*/

static Sint readsymbolmapviafp(Alphabet *alpha,Uint undefsymbol,
                               Uint wildcard,const char *mapfile,FILE *fpin)
{
  Uchar cc;
  Uint i, linecount = 0; 
  ArrayUchar line;
  BOOL blankfound, ignore, preamble = True;

  alpha->domainsize = alpha->mapsize = alpha->mappedwildcards = 0;
  alpha->undefsymbol = undefsymbol;
  for(i=0; i<=UCHAR_MAX; i++)
  {
    alpha->symbolmap[i] = alpha->undefsymbol;
  }
  INITARRAY(&line,Uchar);
  while(True)
  {
    line.nextfreeUchar = 0;
    if(readnextline(fpin,&line) == (Sint) EOF)
    {
      break;
    }
    linecount++;
    ignore = False;
    if(line.nextfreeUchar > 0)
    {
      NOTSUPPOSEDTOBENULL(line.spaceUchar);
      if(preamble)
      {
        if(LINE(0) == (Uchar) '#')
        {
          ignore = True;
        } else
        {
          preamble = False;
        }
      }
      if(ignore)
      {
        DEBUG1(2,"# ignore comment: \"%s\"\n",(char *) line.spaceUchar);
      } else
      {
        blankfound = False;
        for(i=0; i<line.nextfreeUchar; i++)  // for all chars in line
        {
          cc = LINE(i);
          if(ispunct((Ctypeargumenttype) cc) || isalnum((Ctypeargumenttype) cc))
          {
            if(alpha->symbolmap[(Uint) cc] != undefsymbol)
            {
              ERROR3("cannot map symbol '%c' to %lu: "
                     "it is already mapped to %lu",
                      cc,
                      (Showuint) alpha->mapsize,
                      (Showuint) alpha->symbolmap[(Uint) cc]);
              return (Sint) -1;
            }
            alpha->symbolmap[(Uint) cc] = alpha->mapsize;  // get same value
            alpha->mapdomain[alpha->domainsize++] = cc;
          } else
          {
            if(cc == (Uchar) ' ')    // first blank in line found
            {
              blankfound = True;
              /*@innerbreak@*/ break;
            }
            ERROR3("illegal character '%c' in line %lu of mapfile %s",
                   cc,(Showuint) linecount,mapfile);
            return (Sint) -2;
          }
        }
        if(blankfound)      
        {
          if(isspace((Ctypeargumenttype) LINE(i+1)))
          {
            ERROR3("illegal character '%c' at the end of line %lu in mapfile %s",
                    LINE(i+1),(Showuint) linecount,mapfile);
            return (Sint) -3;
          }
          // use next character to display character
          alpha->characters[alpha->mapsize++] = LINE(i+1);
        } else
        {
          // use first character of line to display character
          alpha->characters[alpha->mapsize++] = LINE(0);
        }
      }
    }
  }
  for(i=0;i<=UCHAR_MAX; i++)
  {
    if(alpha->symbolmap[i] == alpha->mapsize - 1)
    {
      if(wildcard > 0)
      {
        alpha->symbolmap[i] = wildcard; // modify mapping for wildcard
      }
      alpha->mappedwildcards++;
    }
  }
  if(wildcard > 0)
  {
    alpha->characters[wildcard] = alpha->characters[alpha->mapsize-1];
  }
  FREEARRAY(&line,Uchar);
  return 0;
}

/*EE
  The following function reads in a symbol map. 
  \texttt{mapfile} is the input filename. 
  \texttt{undefsymbol} is the undefined symbol. If the argument
  \texttt{wildcard} is larger than 0, then the characters in the last
  line of the symbol mapping file are mapped to \texttt{wildcard}. Otherwise,
  they are mapped to \(i-1\) if they appear on line number \(i\)
  (counting from 1). The result of the parsing is stored in 
  \texttt{alpha}.
*/

Sint readsymbolmap(Alphabet *alpha,Uint undefsymbol,Uint wildcard,
                   const char *mapfile)
{
  FILE *fpin;

  fpin = scanpathsforfile("MKVTREESMAPDIR",mapfile);
  if(fpin == NULL)
  {
    return (Sint) -1;
  }
  if(readsymbolmapviafp(alpha,undefsymbol,wildcard,mapfile,fpin) != 0)
  {
    return (Sint) -2;
  }
  if(DELETEFILEHANDLE(fpin) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

static void assignDNAsymbolmap(Uint *symbolmap)
{
  Uint i;

  for(i=0; i<=UCHAR_MAX; i++)
  {
    symbolmap[i] = (Uint) UNDEFCHAR;
  }
  symbolmap[(Uint) 'a'] = UintConst(0);
  symbolmap[(Uint) 'A'] = UintConst(0);
  symbolmap[(Uint) 'c'] = UintConst(1);
  symbolmap[(Uint) 'C'] = UintConst(1);
  symbolmap[(Uint) 'g'] = UintConst(2);
  symbolmap[(Uint) 'G'] = UintConst(2);
  symbolmap[(Uint) 't'] = UintConst(3);
  symbolmap[(Uint) 'T'] = UintConst(3);
  symbolmap[(Uint) 'u'] = UintConst(3);
  symbolmap[(Uint) 'U'] = UintConst(3);
  for(i=0; DNAWILDCARDS[i] != '\0'; i++)
  {
    symbolmap[(Sint) DNAWILDCARDS[i]] = (Uint) WILDCARD;
  }
}

/*EE
  The following function initializes the alphabet \texttt{alpha}
  in the same way as \texttt{readsymbolmap}, if it would be 
  applied to a map file with the following content:
  \begin{alltt}
  aA
  cC
  gG
  tTuU
  nsywrkvbdhmNSYWRKVBDHM
  \end{alltt}
  If the argument \texttt{wildcard} is 0, then the wildcard characters
  in the last line are mapped to 4. Otherwise they are mapped to 
  the character \texttt{WILDCARD}, as defined in \texttt{chardef.h}
*/

void assignDNAalphabet(Alphabet *alpha)
{
  alpha->domainsize = (Uint) strlen(DNAALPHABETDOMAIN);
  alpha->mappedwildcards = (Uint) strlen(DNAWILDCARDS);
  memcpy(alpha->mapdomain,
         (Uchar *) DNAALPHABETDOMAIN,
         (size_t) alpha->domainsize);
  alpha->mapsize = MAPSIZEDNA;
  alpha->undefsymbol = (Uint) UNDEFCHAR;
  memcpy(alpha->characters,"acgt",(size_t) (MAPSIZEDNA-1));
  alpha->characters[WILDCARD] = (Uchar) DNAWILDCARDS[0];
  alpha->characters[MAPSIZEDNA-1] = (Uchar) DNAWILDCARDS[0];
  assignDNAsymbolmap(alpha->symbolmap);
}

static void assignproteinsymbolmap(Uint *symbolmap)
{
  Uint i;

  for(i=0; i<=UCHAR_MAX; i++)
  {
    symbolmap[i] = (Uint) UNDEFCHAR;
  }
  for(i=0; PROTEINUPPERAMINOACIDS[i] != '\0'; i++)
  {
    symbolmap[(Sint) PROTEINUPPERAMINOACIDS[i]] = i;
  }
  for(i=0; PROTEINWILDCARDS[i] != '\0'; i++)
  {
    symbolmap[(Sint) PROTEINWILDCARDS[i]] = (Uint) WILDCARD;
  }
}

/*EE
  The following function initializes the alphabet \texttt{alpha}
  in the same way as \texttt{readsymbolmap}, if it would be 
  applied to a map file with the following content:
  \begin{alltt}
  L
  V
  I
  F
  K
  R
  E
  D
  A
  G
  S
  T
  N
  Q
  Y
  W
  P
  H
  M
  C
  XUBZJO*-
  \end{alltt}
  If the argument \texttt{wildcard} is 0, then the wildcard characters
  in the last line are mapped to 20. Otherwise they are mapped to 
  the character \texttt{WILDCARD}, as defined in \texttt{chardef.h}
*/

void assignProteinalphabet(Alphabet *alpha)
{
  alpha->domainsize = (Uint) strlen(PROTEINALPHABETDOMAIN);
  alpha->mappedwildcards = (Uint) strlen(PROTEINWILDCARDS);
  memcpy(alpha->mapdomain,
         (Uchar *) PROTEINALPHABETDOMAIN,(size_t) alpha->domainsize);
  alpha->mapsize = MAPSIZEPROTEIN;
  alpha->undefsymbol = (Uint) UNDEFCHAR;
  memcpy(alpha->characters,PROTEINUPPERAMINOACIDS,(size_t) MAPSIZEPROTEIN-1);
  alpha->characters[WILDCARD] = (Uchar) PROTEINWILDCARDS[0];
  alpha->characters[MAPSIZEPROTEIN-1] = (Uchar) PROTEINWILDCARDS[0];
  assignproteinsymbolmap(alpha->symbolmap);
}

/*EE
  The following function scans the \texttt{text} of length 
  \texttt{textlen} and computes the list of all characters
  appearing in \texttt{text}. Suppose the list is sorted
  by the order of the ASCII alphabet. Then the \(i\)th
  character in this list is mapped to \(i\).
*/

void findalphabet(Alphabet *alpha, Uchar *text, Uint textlen,
                  Uint undefsymbol)
{
  Uint i;
  Uchar *tptr, cc;
  BOOL occurs[UCHAR_MAX+1] = {False};
 
  alpha->undefsymbol = undefsymbol;  
  for(i=0; i<=UCHAR_MAX; i++)
  {
    alpha->symbolmap[i] = UCHAR_MAX;
  }
  alpha->mapsize=0;
  for(tptr=text; tptr < text + textlen; tptr++)
  {
    cc = *tptr;
    if(!occurs[(Sint) cc])
    {
      occurs[(Sint) cc] = True;
    }
  }
  for(i=0; i<=UCHAR_MAX; i++)
  {
    if(occurs[i])
    {
      alpha->characters[alpha->mapsize] = (Uchar) i;
      alpha->symbolmap[i] = alpha->mapsize++;
    }
  }
  alpha->domainsize = alpha->mapsize;
  alpha->mappedwildcards = 0;
}

/*EE
  The following function determines the alphabet
  according to the information stored in 
  \texttt{inputalpha}. If there is a symbolmap file, then 
  alphabet is read from this file.
  If \texttt{dnafile} is true, the alphabet is initialized as the
  DNA alphabet.
  If \texttt{proteinfile} is true, then alphabet is initialized as the
  Protein alphabet.
*/

Sint determineAlphabet(Alphabet *alpha,Inputalpha *inputalpha)
{
  if(inputalpha->symbolmapfile != NULL)
  {
    if(readsymbolmap(alpha,
                     (Uint) UNDEFCHAR,
                     (Uint) WILDCARD,
                     inputalpha->symbolmapfile) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(inputalpha->dnafile)
    {
      assignDNAalphabet(alpha);
    } else
    {
      if(inputalpha->proteinfile)
      {
        assignProteinalphabet(alpha);
      } else
      {
        NOTSUPPOSED;
      }
    }
  }
  return 0;
}


/*EE
  Suppose the string \texttt{w} of length \texttt{wlen} 
  was transformed according to the alphabet \texttt{alpha}.
  The following function shows each character in \texttt{w}
  as the characters specified in the transformation.
  The output goes to the given file pointer.
 */

void vm_showsymbolstringgeneric(FILE *fpout,Alphabet *alpha,Uchar *w,Uint wlen)
{
  Uint i;
 
  for(i = 0; i < wlen; i++)
  {
    (void) putc((Fputcfirstargtype) alpha->characters[(Sint) w[i]],fpout);
  }
}

/*EE
  The following function is a special case of the previous
  function showing the output on stdout.
*/

void vm_showsymbolstring(Alphabet *alpha,Uchar *w,Uint wlen)
{
  vm_showsymbolstringgeneric(stdout,alpha,w,wlen);
}

/*EE
  The following function copies the alphabet \texttt{alpha2} into the alphabet 
  \texttt{alpha1}.
*/

void copyAlphabet(Alphabet *alpha1,const Alphabet *alpha2)
{
  Uint i;

  alpha1->domainsize = alpha2->domainsize;
  alpha1->mapsize = alpha2->mapsize;
  alpha1->mappedwildcards = alpha2->mappedwildcards;
  alpha1->undefsymbol = alpha2->undefsymbol;
  for(i=0; i<=UCHAR_MAX; i++)
  {
    alpha1->symbolmap[i] = alpha2->symbolmap[i];
    alpha1->characters[i] = alpha2->characters[i];
    alpha1->mapdomain[i] = alpha2->mapdomain[i];
  }
}


/*EE
  The following function checks if the given alphabet is the Protein
  alphabet with the aminoacids 
  A, C, D, E, F, G, H, I, K, L, M, N, P, Q, R, S, T, V, W, Y written in 
  lower or upper case.
*/

#define UNCAST(X) (*((const Uchar *) (X)))

static Qsortcomparereturntype comparechar(const void *a,const void *b)
{
  if(UNCAST(a) < UNCAST(b))
  {
    return -1;
  }
  if(UNCAST(a) > UNCAST(b))
  {
    return 1;
  }
  return 0;
}

static Uint removelowercaseproteinchars(Uchar *domainbuf,Alphabet *alpha)
{
  Uint i, j = 0;

  for(i=0; i< alpha->domainsize - alpha->mappedwildcards; i++)
  {
    if(isalnum(alpha->mapdomain[i]) && isupper(alpha->mapdomain[i]))
    {
      domainbuf[j++] = alpha->mapdomain[i];
    }
  }
  return j;
}

BOOL vm_isproteinalphabet(Alphabet *alpha)
{
  Alphabet proteinalphabet;
  Uint i, reduceddomainsize1, reduceddomainsize2;
  Uchar domainbuf1[UCHAR_MAX+1], domainbuf2[UCHAR_MAX+1];

  reduceddomainsize1 = removelowercaseproteinchars(&domainbuf1[0],alpha);
  assignProteinalphabet(&proteinalphabet);
  reduceddomainsize2 = removelowercaseproteinchars(&domainbuf2[0],
                                                   &proteinalphabet);
  if(reduceddomainsize1 == reduceddomainsize2)
  {
    qsort(&domainbuf1[0],(size_t) reduceddomainsize1,sizeof(char),
          (Qsortcomparefunction) comparechar);
    qsort(&domainbuf2[0],(size_t) reduceddomainsize2,sizeof(char),
          (Qsortcomparefunction) comparechar);
    for(i=0; i < reduceddomainsize2; i++)
    {
      if(domainbuf1[i] != domainbuf2[i])
      {
#ifdef DEBUG
        fprintf(stderr,"domainbuf1=%*.*s\n",
                       (Fieldwidthtype) reduceddomainsize1,
                       (Fieldwidthtype) reduceddomainsize1,
                       (char *) domainbuf1);
        fprintf(stderr,"domainbuf2=%*.*s\n",
                       (Fieldwidthtype) reduceddomainsize2,
                       (Fieldwidthtype) reduceddomainsize2,
                       (char *) domainbuf2);
        fprintf(stderr,"i=%lu: domainbuf1 = %c != %c = domainbuf2\n",
                        (Showuint) i,domainbuf1[i],domainbuf2[i]);
#endif
        return False;
      }
    }
    return True;
  }
#ifdef DEBUG
  fprintf(stderr,"reduceddomainsize1 = %lu != %lu = reduceddomainsize2\n",
                 (Showuint) reduceddomainsize1,
                 (Showuint) reduceddomainsize2);
#endif
  return False;
}

static BOOL checksymbolmap(Uint *testsymbolmap,
                           Uint *verifiedsymbolmap,
                           char *testcharacters)
{
  Uint i;
  Ctypeargumenttype cc1, cc2;

  for(i=0; testcharacters[i] != '\0'; i++)
  {
    cc1 = testcharacters[i];
    if(isupper(cc1))
    {
      cc2 = tolower(cc1);
    } else
    {
      if(islower(cc1))
      {
        cc2 = toupper(cc1);
      } else
      {
        fprintf(stderr,"checksymbolmap used for non-alphabet character %c\n",
                cc1);
        exit(EXIT_FAILURE);
      }
    }
    if(testsymbolmap[cc1] != verifiedsymbolmap[cc1] &&
       testsymbolmap[cc2] != verifiedsymbolmap[cc2])
    {
      return False;
    }
  }
  return True;
}

/*EE
  The following function checks if the given alphabet is the DNA
  alphabet with the bases A, C, G, T written in lower or upper case.
*/

BOOL vm_isdnaalphabet(Alphabet *alpha)
{
  if(vm_isproteinalphabet(alpha))
  {
    return False;
  }
  if(alpha->mapsize == MAPSIZEDNA)
  {
    Uint dnasymbolmap[UCHAR_MAX+1];

    assignDNAsymbolmap(&dnasymbolmap[0]);
    if(checksymbolmap(alpha->symbolmap,&dnasymbolmap[0],"acgt"))
    {
      return True;
    }
  }
  return False;
}

void initmappower(Uint *mappower,Uint numofchars,Uint prefixlength)
{
  Uint thepower = (Uint) pow((double) numofchars,(double) (prefixlength-1)),
       mapindex;

  mappower[0] = 0;
  for(mapindex = UintConst(1); mapindex < numofchars; mapindex++)
  {
    mappower[mapindex] = mappower[mapindex-1] + thepower;
  }
}

BOOL containswildcard(Uchar *seq,Uint len)
{
  Uchar *sptr;

  for(sptr=seq; sptr < seq + len; sptr++)
  {
    if(ISSPECIAL(*sptr))
    {
      return True;
    }
  }
  return False;
}


/*
  The folling functions parses an .al2 file
*/

static Sint parseal2file(Alphabet *alpha,const char *indexname)
{
  Uint i;
  FILE *fp;
  char *tmpfilename = COMPOSEFILENAME(indexname,"al2");

  fp = CREATEFILEHANDLE(tmpfilename,READMODE);
  FREESPACE(tmpfilename);
  if(fp != NULL)
  {
    Fgetcreturntype cc;

    alpha->undefsymbol = (Uint) UNDEFCHAR;
    for(i=0; i<=UCHAR_MAX; i++)
    {
      alpha->symbolmap[i] = (Uint) UNDEFCHAR;
    }
    alpha->mapsize = (Uint) fgetc(fp);
    if(alpha->mapsize == 0)
    {
      alpha->mapsize = (Uint) (UCHAR_MAX+1);
    }
    for(i=0; (cc = fgetc(fp)) != EOF; i++)
    {
      alpha->symbolmap[cc] = i;
      alpha->characters[i] = (Uchar) cc;
    }
    if(DELETEFILEHANDLE(fp) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    alpha->mapsize = 0;
  }
  return 0;
} 
                    
Sint mapalphabetifyoucan(BOOL *hasspecialsymbols,
                         Alphabet *alpha,const char *indexname)
{
  char *tmpfilename = COMPOSEFILENAME(indexname,"al1");

  if(readsymbolmap(alpha,
                   (Uint) UNDEFCHAR,
                   (Uint) WILDCARD,
                   tmpfilename) == 0)
  {
    FREESPACE(tmpfilename);
    *hasspecialsymbols = True;
  } else
  {
    FREESPACE(tmpfilename);
    if(parseal2file(alpha,indexname) != 0)
    {
      return (Sint) -1;
    }
    *hasspecialsymbols = False;
    alpha->domainsize = alpha->mapsize;
  }
  return 0;
}

//\Ignore{

void showsymbolmap(Alphabet *alpha)
{
  Uint i;
  Uchar cc;

  printf("# domainsize=%lu\n",(Showuint) alpha->domainsize);
  printf("# mapsize=%lu\n",(Showuint) alpha->mapsize);
  printf("# mappedwildcards=%lu\n",(Showuint) alpha->mappedwildcards);
  printf("# mapdomain=%*.*s\n",(Fieldwidthtype) alpha->domainsize,
                               (Fieldwidthtype) alpha->domainsize,
                               (char *) alpha->mapdomain);
  printf("# characters=");
  for(i = 0; i < alpha->mapsize; i++)
  {
    cc = verbosealphachar(alpha,i);
    if(INVISIBLE(cc))
    {
      printf("\\%lu",(Showuint) cc);
    } else
    {
      printf("%c",cc);
    }
  }
  (void) putchar('\n');
  for(i = 0; i <= UCHAR_MAX; i++)
  {
    if(alpha->symbolmap[i] != alpha->undefsymbol)
    {
      if(INVISIBLE(i))
      {
        printf("# %lu",(Showuint) i);
      } else
      {
        printf("# %c",(char) i);
      }
      printf("->%lu\n",(Showuint) alpha->symbolmap[i]);
    }
  }
}

//}

