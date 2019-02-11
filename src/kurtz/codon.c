
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "errordef.h"
#include "failures.h"
#include "codondef.h"
#include "alphadef.h"
#include "debugdef.h"

#include "alphabet.pr"

//}

/*EE
  This file implements the translation of DNA into protein w.r.t.\
  the genetic code.
*/

/*
  The number of translation schemes.
*/

#define NUMOFTRANSSCHEMES\
        ((Uint) (sizeof(schemetable)/sizeof(schemetable[0])))
#define SIZEOFTRANSRANGE\
        ((Uint) (sizeof(transnum2index)/sizeof(transnum2index[0])))

#define UNDEFTRANSNUM NUMOFTRANSSCHEMES

#define INCONSISTENT(BASE)\
        ERROR4("code=%lu with wildcard %c: inconsistent aminoacids %c and %c",\
                (Showuint) codeof2,wildcard,aa,newaa);\
        return AMINOACIDFAIL

#define ILLEGALCHAR(V)\
        ERROR3("illegal char %s='%c'(%lu)",#V,V,(Showuint) (V));\
        return AMINOACIDFAIL

#define STARTAMINO 'M'

/*
  The codes for the bases:
*/

#define CODONTCODE (UintConst(0))
#define CODONCCODE (UintConst(1))
#define CODONACODE (UintConst(2))
#define CODONGCODE (UintConst(3))

/*
  One bit for each base.
*/

#define TBIT ((Uchar) 1)
#define CBIT (((Uchar) 1) << 1)
#define ABIT (((Uchar) 1) << 2)
#define GBIT (((Uchar) 1) << 3)

/*
  The case distinction for all wildcards.
*/

#define CASEWILDCARD\
          case 'n':\
          case 'N':\
          case 's':\
          case 'S':\
          case 'y':\
          case 'Y':\
          case 'w':\
          case 'W':\
          case 'r':\
          case 'R':\
          case 'k':\
          case 'K':\
          case 'v':\
          case 'V':\
          case 'b':\
          case 'B':\
          case 'd':\
          case 'D':\
          case 'h':\
          case 'H':\
          case 'm':\
          case 'M'

/*
  The following type is used to store the information about different
  coding schemes. The string \texttt{aminos} specifies how to translate
  translate a codon into an amino acid.
  The encoding is such that the bases are weighted as follows:
  \begin{center}
  \begin{array}
  T &\to&0\\
  C &\to&1\\
  A &\to&2\\
  G &\to&3
  \end{array}
  \end{center}
  The first base of a codon is weighted by 16, the second by 4, and the 
  third by 1. This gives the lookup-index  for table \texttt{aminos}.
*/
      
typedef struct
{
  char *name;         // the name of the translation
  Uint identity;      // the identity number
  char *aminos,       // the amino acids in order of the 
       *startcodon;   // the start codons
} Translationscheme;

/* according to http://www.ncbi.nlm.nih.gov/Taxonomy/Utils/wprintgc.cgi */
static Translationscheme schemetable[] = {
  {"Standard",
   (Uint) 1,
   "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "---M---------------M---------------M----------------------------"},
  {"Vertebrate Mitochondrial",
   (Uint) 2,
   "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSS**VVVVAAAADDEEGGGG",
   "--------------------------------MMMM---------------M------------"},
  {"Yeast Mitochondrial",
   (Uint) 3,
   "FFLLSSSSYY**CCWWTTTTPPPPHHQQRRRRIIMMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Mold Mitochondrial; Protozoan Mitochondrial; Coelenterate Mitochondrial; Mycoplasma; Spiroplasma",
   (Uint) 4,
   "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "--MM---------------M------------MMMM---------------M------------"},
  {"Invertebrate Mitochondrial",
   (Uint) 5,
   "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSSSSVVVVAAAADDEEGGGG",
   "---M----------------------------MMMM---------------M------------"},
  {"Ciliate Nuclear; Dasycladacean Nuclear; Hexamita Nuclear",
   (Uint) 6,
   "FFLLSSSSYYQQCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Echinoderm Mitochondrial",
   (Uint) 9,
   "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNNKSSSSVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Euplotid Nuclear",
   (Uint) 10,
   "FFLLSSSSYY**CCCWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Bacterial",
   (Uint) 11,
   "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "---M---------------M------------MMMM---------------M------------"},
  {"Alternative Yeast Nuclear",
   (Uint) 12,
   "FFLLSSSSYY**CC*WLLLSPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "-------------------M---------------M----------------------------"},
  {"Ascidian Mitochondrial",
   (Uint) 13,
   "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSSGGVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Flatworm Mitochondrial",
   (Uint) 14,
   "FFLLSSSSYYY*CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNNKSSSSVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Blepharisma Macronuclear",
   (Uint) 15,
   "FFLLSSSSYY*QCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Chlorophycean Mitochondrial",
   (Uint) 16,
   "FFLLSSSSYY*LCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Trematode Mitochondrial",
   (Uint) 21,
   "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNNKSSSSVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Scenedesmus Obliquus Mitochondrial",
   (Uint) 22,
   "FFLLSS*SYY*LCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "-----------------------------------M----------------------------"},
  {"Thraustochytrium Mitochondrial",
   (Uint) 23,
   "FF*LSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
   "--------------------------------M--M---------------M------------"}
};

static Uint transnum2index[] =
{
  UNDEFTRANSNUM,
  UintConst(0),
  UintConst(1),
  UintConst(2),
  UintConst(3),
  UintConst(4),
  UintConst(5),
  UNDEFTRANSNUM,
  UNDEFTRANSNUM,
  UintConst(6),
  UintConst(7),
  UintConst(8),
  UintConst(9),
  UintConst(10),
  UintConst(11),
  UintConst(12),
  UintConst(13),
  UNDEFTRANSNUM,
  UNDEFTRANSNUM,
  UNDEFTRANSNUM,
  UNDEFTRANSNUM,
  UintConst(14),
  UintConst(15),
  UintConst(16),
};

Sint checktransnum(Uint transnum)
{
  if(transnum >= SIZEOFTRANSRANGE || transnum2index[transnum] == UNDEFTRANSNUM)
  {
    ERROR2("illegal translation table number %lu: must be number "
           "in the range [1,%lu] except for 7, 8, 17, 18, 19 and 20",
           (Showuint) transnum,(Showuint) (SIZEOFTRANSRANGE-1));
    return (Sint) -1;
  }
  return 0;
}

static Translationscheme *getschemetable(Uint transnum)
{
#ifdef DEBUG
  if(checktransnum(transnum) != 0)
  {
    fprintf(stderr,"%s\n",messagespace());
    exit(EXIT_FAILURE);
  }
#endif
  return schemetable + transnum2index[transnum];
}

char *transnum2name(Uint transnum)
{
  Translationscheme *scheme = getschemetable(transnum);

  return scheme->name;
}

void helptransnumorganism(Uint maxsize,char *sbuf)
{
  Uint idx, transnum;
  Sprintfreturntype sizehelp;

  if(NUMOFTRANSSCHEMES > (Uint) MAXTRANSLATIONTABLE)
  {
    fprintf(stderr,"NUMOFTRANSSCHEMES = %lu > %lu = MAXTRANSLATIONTABLE\n",
                    (Showuint) NUMOFTRANSSCHEMES,
                    (Showuint) MAXTRANSLATIONTABLE);
    exit(EXIT_FAILURE);
  }
  sizehelp = sprintf(sbuf,"perform six frame translation\n"
                          "specify codon translation table by a number "
                          "in the range [1,%lu] except for 7, 8, 17, 18, 19 "
                          "and 20; (default is %lu):\n",
                          (Showuint) (SIZEOFTRANSRANGE-1),
                          (Showuint) STANDARDTRANSLATIONTABLE);
  for(transnum = 0; transnum < SIZEOFTRANSRANGE; transnum++)
  {
    if(checktransnum(transnum) == 0)
    {
      idx = transnum2index[transnum];
      sizehelp += sprintf(sbuf+sizehelp,"%2lu %s%s",
                          (Showuint) schemetable[idx].identity,
                          schemetable[idx].name,
                          (transnum == SIZEOFTRANSRANGE - 1) ? "" : "\n");
      if(sizehelp >= (Sprintfreturntype) maxsize)
      {
        fprintf(stderr,"sizehelp = %ld >= %lu = maxsize\n",
              (Showsint) sizehelp,(Showuint) maxsize);
        exit(EXIT_FAILURE);
      }
    }
  }
}

/*
  The following table specifies a unsigned characters
  whose bits are set according to the characters encoded by a wildcard.
*/

static Uchar wbitsvector[] =
{
  0 /* 0 */,
  0 /* 1 */,
  0 /* 2 */,
  0 /* 3 */,
  0 /* 4 */,
  0 /* 5 */,
  0 /* 6 */,
  0 /* 7 */,
  0 /* 8 */,
  0 /* 9 */,
  0 /* 10 */,
  0 /* 11 */,
  0 /* 12 */,
  0 /* 13 */,
  0 /* 14 */,
  0 /* 15 */,
  0 /* 16 */,
  0 /* 17 */,
  0 /* 18 */,
  0 /* 19 */,
  0 /* 20 */,
  0 /* 21 */,
  0 /* 22 */,
  0 /* 23 */,
  0 /* 24 */,
  0 /* 25 */,
  0 /* 26 */,
  0 /* 27 */,
  0 /* 28 */,
  0 /* 29 */,
  0 /* 30 */,
  0 /* 31 */,
  0 /* 32 */,
  0 /* 33 */,
  0 /* 34 */,
  0 /* 35 */,
  0 /* 36 */,
  0 /* 37 */,
  0 /* 38 */,
  0 /* 39 */,
  0 /* 40 */,
  0 /* 41 */,
  0 /* 42 */,
  0 /* 43 */,
  0 /* 44 */,
  0 /* 45 */,
  0 /* 46 */,
  0 /* 47 */,
  0 /* 48 */,
  0 /* 49 */,
  0 /* 50 */,
  0 /* 51 */,
  0 /* 52 */,
  0 /* 53 */,
  0 /* 54 */,
  0 /* 55 */,
  0 /* 56 */,
  0 /* 57 */,
  0 /* 58 */,
  0 /* 59 */,
  0 /* 60 */,
  0 /* 61 */,
  0 /* 62 */,
  0 /* 63 */,
  0 /* 64 */,
  0 /* 65 */,
  CBIT | GBIT | TBIT         /* [cgt] */ /* b */,
  0 /* 67 */,
  ABIT | GBIT | TBIT         /* [agt] */ /* d */,
  0 /* 69 */,
  0 /* 70 */,
  0 /* 71 */,
  ABIT | CBIT | TBIT         /* [act] */ /* h */,
  0 /* 73 */,
  0 /* 74 */,
  GBIT | TBIT                /* [gt] */ /* k */,
  0 /* 76 */,
  ABIT | CBIT                /* [ac] */ /* m */,
  ABIT | CBIT | GBIT | TBIT  /* [acgt] */ /* n */,
  0 /* 79 */,
  0 /* 80 */,
  0 /* 81 */,
  ABIT | GBIT                /* [ag] */ /* r */,
  CBIT | GBIT                /* [cg] */ /* s */,
  0 /* 84 */,
  0 /* 85 */,
  ABIT | CBIT | GBIT         /* [acg] */ /* v */,
  ABIT | CBIT                /* [at] */ /* w */,
  0 /* 88 */,
  CBIT | TBIT                /* [ct] */ /* y */,
  0 /* 90 */,
  0 /* 91 */,
  0 /* 92 */,
  0 /* 93 */,
  0 /* 94 */,
  0 /* 95 */,
  0 /* 96 */,
  0 /* 97 */,
  CBIT | GBIT | TBIT         /* [cgt] */ /* B */,
  0 /* 99 */,
  ABIT | GBIT | TBIT         /* [agt] */ /* D */,
  0 /* 101 */,
  0 /* 102 */,
  0 /* 103 */,
  ABIT | CBIT | TBIT         /* [act] */ /* H */,
  0 /* 105 */,
  0 /* 106 */,
  GBIT | TBIT                /* [gt] */ /* K */,
  0 /* 108 */,
  ABIT | CBIT                /* [ac] */ /* M */,
  ABIT | CBIT | GBIT | TBIT  /* [acgt] */ /* N */,
  0 /* 111 */,
  0 /* 112 */,
  0 /* 113 */,
  ABIT | GBIT                /* [ag] */ /* R */,
  CBIT | GBIT                /* [cg] */ /* S */,
  0 /* 116 */,
  0 /* 117 */,
  ABIT | CBIT | GBIT         /* [acg] */ /* V */,
  ABIT | CBIT                /* [at] */ /* W */,
  0 /* 120 */,
  CBIT | TBIT                /* [ct] */ /* Y */,
  0 /* 122 */,
  0 /* 123 */,
  0 /* 124 */,
  0 /* 125 */,
  0 /* 126 */,
  0 /* 127 */,
  0 /* 128 */,
  0 /* 129 */,
  0 /* 130 */,
  0 /* 131 */,
  0 /* 132 */,
  0 /* 133 */,
  0 /* 134 */,
  0 /* 135 */,
  0 /* 136 */,
  0 /* 137 */,
  0 /* 138 */,
  0 /* 139 */,
  0 /* 140 */,
  0 /* 141 */,
  0 /* 142 */,
  0 /* 143 */,
  0 /* 144 */,
  0 /* 145 */,
  0 /* 146 */,
  0 /* 147 */,
  0 /* 148 */,
  0 /* 149 */,
  0 /* 150 */,
  0 /* 151 */,
  0 /* 152 */,
  0 /* 153 */,
  0 /* 154 */,
  0 /* 155 */,
  0 /* 156 */,
  0 /* 157 */,
  0 /* 158 */,
  0 /* 159 */,
  0 /* 160 */,
  0 /* 161 */,
  0 /* 162 */,
  0 /* 163 */,
  0 /* 164 */,
  0 /* 165 */,
  0 /* 166 */,
  0 /* 167 */,
  0 /* 168 */,
  0 /* 169 */,
  0 /* 170 */,
  0 /* 171 */,
  0 /* 172 */,
  0 /* 173 */,
  0 /* 174 */,
  0 /* 175 */,
  0 /* 176 */,
  0 /* 177 */,
  0 /* 178 */,
  0 /* 179 */,
  0 /* 180 */,
  0 /* 181 */,
  0 /* 182 */,
  0 /* 183 */,
  0 /* 184 */,
  0 /* 185 */,
  0 /* 186 */,
  0 /* 187 */,
  0 /* 188 */,
  0 /* 189 */,
  0 /* 190 */,
  0 /* 191 */,
  0 /* 192 */,
  0 /* 193 */,
  0 /* 194 */,
  0 /* 195 */,
  0 /* 196 */,
  0 /* 197 */,
  0 /* 198 */,
  0 /* 199 */,
  0 /* 200 */,
  0 /* 201 */,
  0 /* 202 */,
  0 /* 203 */,
  0 /* 204 */,
  0 /* 205 */,
  0 /* 206 */,
  0 /* 207 */,
  0 /* 208 */,
  0 /* 209 */,
  0 /* 210 */,
  0 /* 211 */,
  0 /* 212 */,
  0 /* 213 */,
  0 /* 214 */,
  0 /* 215 */,
  0 /* 216 */,
  0 /* 217 */,
  0 /* 218 */,
  0 /* 219 */,
  0 /* 220 */,
  0 /* 221 */,
  0 /* 222 */,
  0 /* 223 */,
  0 /* 224 */,
  0 /* 225 */,
  0 /* 226 */,
  0 /* 227 */,
  0 /* 228 */,
  0 /* 229 */,
  0 /* 230 */,
  0 /* 231 */,
  0 /* 232 */,
  0 /* 233 */,
  0 /* 234 */,
  0 /* 235 */,
  0 /* 236 */,
  0 /* 237 */,
  0 /* 238 */,
  0 /* 239 */,
  0 /* 240 */,
  0 /* 241 */,
  0 /* 242 */,
  0 /* 243 */,
  0 /* 244 */,
  0 /* 245 */,
  0 /* 246 */,
  0 /* 247 */,
  0 /* 248 */,
  0 /* 249 */,
  0 /* 250 */,
  0 /* 251 */,
  0 /* 252 */,
  0 /* 253 */,
  0 /* 254 */,
  0 /* 255 */
};

#ifdef DEBUG

static void checkwildcardbits(void)
{
  Uint i;
  Uchar wbits[UCHAR_MAX+1] = {0};

  wbits['r'] = ABIT | GBIT;                // [ag]
  wbits['R'] = ABIT | GBIT;                // [ag]
  wbits['y'] = CBIT | TBIT;                // [ct]
  wbits['Y'] = CBIT | TBIT;                // [ct]
  wbits['m'] = ABIT | CBIT;                // [ac]
  wbits['M'] = ABIT | CBIT;                // [ac]
  wbits['k'] = GBIT | TBIT;                // [gt]
  wbits['K'] = GBIT | TBIT;                // [gt]
  wbits['s'] = CBIT | GBIT;                // [cg]
  wbits['S'] = CBIT | GBIT;                // [cg]
  wbits['w'] = ABIT | CBIT;                // [at]
  wbits['W'] = ABIT | CBIT;                // [at]
  wbits['h'] = ABIT | CBIT | TBIT;         // [act]
  wbits['H'] = ABIT | CBIT | TBIT;         // [act]
  wbits['b'] = CBIT | GBIT | TBIT;         // [cgt]
  wbits['B'] = CBIT | GBIT | TBIT;         // [cgt]
  wbits['v'] = ABIT | CBIT | GBIT;         // [acg]
  wbits['V'] = ABIT | CBIT | GBIT;         // [acg]
  wbits['d'] = ABIT | GBIT | TBIT;         // [agt]
  wbits['D'] = ABIT | GBIT | TBIT;         // [agt]
  wbits['n'] = ABIT | CBIT | GBIT | TBIT;  // [acgt]
  wbits['N'] = ABIT | CBIT | GBIT | TBIT;  // [acgt]
  for(i=0; i<=UCHAR_MAX; i++)
  {
    if(wbits[i] != wbitsvector[i])
    {
      fprintf(stderr,"wbitslocal[%lu] = %lu != %lu = wbitsvector[%lu]\n",
                      (Showuint) i,(Showuint) wbits[i],
                      (Showuint) wbitsvector[i],(Showuint) i);
      exit(EXIT_FAILURE);
    }
  }
}

#endif

/*
  The following function tries to solve the case, where 
  the third base of a codon is a \texttt{wildcard}.
  It takes the code \texttt{codeof2} of the first two bases
  of a codon and checks if all characters encoded by \texttt{wildcard}
  lead to the same amino acid, when doing the translation.
  This means that the characters encoded by \texttt{wildcard} are 
  equivalent.
*/

static char equivalentbits(char *aminos,
                           Uint codeof2,
                           Uchar wildcard)
{
  Uchar bits = wbitsvector[(Sint) wildcard];
  char aa = 0, newaa;
  BOOL aaundefined = True;

  if(bits & TBIT)
  {
    aa = aminos[codeof2 + CODONTCODE];
    aaundefined = False;
  }
  if(bits & CBIT)
  {
    newaa = aminos[codeof2 + CODONCCODE];
    if(aaundefined)
    {
      aa = newaa;
      aaundefined = False;
    } else
    {
      if(aa != newaa)
      {
        INCONSISTENT('C');
      }
    }
  }
  if(bits & ABIT)
  {
    newaa = aminos[codeof2 + CODONACODE];
    if(aaundefined)
    {
      aa = newaa;
      aaundefined = False;
    } else
    {
      if(aa != newaa)
      {
        INCONSISTENT('A');
      }
    }
  }
  if(bits & GBIT)
  {
    newaa = aminos[codeof2 + CODONGCODE];
    if(aaundefined)
    {
      aa = newaa;
      aaundefined = False;
    } else
    {
      if(aa != newaa)
      {
        INCONSISTENT('G');
      }
    }
  }
  if(aaundefined)
  {
    return AMINOACIDFAIL;
  }
  return aa;
}

/*
  The following function finds the smallest character in the set
  of characters encoded by a wildcard. This set is given by the bit vector
  \texttt{bits}.
*/

static Uint smallestbase(Uchar bits)
{
  if(bits & TBIT)
  {
    return CODONTCODE;
  }
  if(bits & CBIT)
  {
    return CODONCCODE;
  }
  if(bits & ABIT)
  {
    return CODONACODE;
  }
  if(bits & GBIT)
  {
    return CODONGCODE;
  }
  NOTSUPPOSED;
  /*@ignore@*/
  return 0;
  /*@end@*/
}

/*
  The following function translate the codon consisting of the bases
  \texttt{c0}, \texttt{c1}, and \texttt{c2}. \texttt{aminos} is the
  corresponding line from table \texttt{schemetable}.
  If \texttt{coderet} is not \texttt{NULL}, the code of the computed amino
  acid is returned.
*/

static char codon2amino(char *aminos,BOOL forward,Uchar c0,Uchar c1,Uchar c2,
                        Uint *coderet)
{
  Uint code = 0;
  char aa;

  switch(c0)
  {
    case 'a':
    case 'A':
      if(forward)
      {
        code = (CODONACODE << 4);
      } else
      {
        code = 0;
      }
      break;
    case 'c':
    case 'C':
      if(forward)
      {
        code = (CODONCCODE << 4);
      } else
      {
        code = (CODONGCODE << 4);
      }
      break;
    case 'g':
    case 'G':
      if(forward)
      {
        code = (CODONGCODE << 4);
      } else
      {
        code = (CODONCCODE << 4);
      }
      break;
    case 't':
    case 'T':
    case 'u':
    case 'U':
      if(forward)
      {
        code = 0;
      } else
      {
        code = (CODONACODE << 4);
      }
      break;
    CASEWILDCARD: // delete this and the next line, to inform about wildcards
      code = (smallestbase(wbitsvector[(Sint) c0]) << 4);
      break;
    default: 
      ILLEGALCHAR(c0);
  }
  switch(c1)
  {
    case 'a':
    case 'A':
      if(forward)
      {
        code += (CODONACODE << 2);
      }
      break;
    case 'c':
    case 'C':
      if(forward)
      {
        code += (CODONCCODE << 2);
      } else
      {
        code += (CODONGCODE << 2);
      }
      break;
    case 'g':
    case 'G':
      if(forward)
      {
        code += (CODONGCODE << 2);
      } else
      {
        code += (CODONCCODE << 2);
      }
      break;
    case 't':
    case 'T':
    case 'u':
    case 'U':
      if(!forward)
      {
        code += (CODONACODE << 2);
      }
      break;
    CASEWILDCARD: // delete this and the next line, to inform about wildcards
      code += (smallestbase(wbitsvector[(Sint) c1]) << 2);
      break;
    default: 
      ILLEGALCHAR(c1);
  }
  switch(c2)
  {
    case 'a':
    case 'A':
      if(forward)
      {
        code += CODONACODE;
      }
      break;
    case 'c':
    case 'C':
      if(forward)
      {
        code += CODONCCODE;
      } else
      {
        code += CODONGCODE;
      }
      break;
    case 'g':
    case 'G':
      if(forward)
      {
        code += CODONGCODE;
      } else
      {
        code += CODONCCODE;
      }
      break;
    case 't':
    case 'T':
    case 'u':
    case 'U':
      if(!forward)
      {
        code += CODONACODE;
      }
      break;
    CASEWILDCARD:
      aa = equivalentbits(aminos,code,c2);
      if(aa == AMINOACIDFAIL)
      {
        // no unique aminoacid => choose smallest base and compute aminos
        // accordingly
        code += smallestbase(wbitsvector[(Sint) c2]);
      } else
      {
        if(coderet != NULL)
        {
          *coderet = code;
        } 
        return aa;
      }
      break;
    default: 
      ILLEGALCHAR(c2);
  }
  if(coderet != NULL)
  {
    *coderet = code;
  } 
  return aminos[code];
}

Sint findstartcodon(Uint transnum,Uchar *dnaseq,Uint dnaseqlen)
{
  Uint code = 0;
  Uchar *dnaptr;
  Translationscheme *scheme;

  scheme = getschemetable(transnum);
  for(dnaptr = dnaseq; dnaptr < dnaseq + dnaseqlen - 3; dnaptr++)
  {
    (void) codon2amino(scheme->aminos,True,*dnaptr,*(dnaptr+1),*(dnaptr+2),
                       &code);
    if(scheme->startcodon[code] == STARTAMINO)
    {
      return (Sint) (dnaptr - dnaseq);
    }
  }
  ERROR0("Cannot find start codon");
  return -1;
}

void outrubytranstable(Uint transnum)
{
  Uint i, j, k;
  char aa;
  Translationscheme *scheme;
  char *alphabet = "TCAG";

  scheme = getschemetable(transnum);
  printf("def codon2aa_%lu(codon)\n",(Showuint) transnum);
  printf("    genetic_code = {\n");
  for (i=0; i<(Uint) 4; i++)
  {
    for (j=0; j<(Uint) 4; j++)
    {
      for (k=0; k<(Uint) 4; k++)
      {
        aa = codon2amino(scheme->aminos,True,
                         alphabet[i],
                         alphabet[j],
                         alphabet[k],NULL);
        printf("    '%c%c%c' => '%c',\n",alphabet[i],
                                         alphabet[j],
                                         alphabet[k],aa);
      }
    }
  }
  printf("    }\n");
  printf("    if genetic_code.has_key?(codon)\n"
         "        return genetic_code[codon]\n"
         "    else\n"
         "        STDERR.print \"Bad codon \\\"#{codon}\\\"!!\\n\"\n"
         "        exit 1\n"
         "    end\n");
  printf("end\n");
}

/*EE
  The following function translates a DNA sequence \texttt{dnaseq} of length
  \texttt{dnaseqlen} into the corresponding protein sequence. The translation
  is in forward direction following the translation scheme number 
  \texttt{transnum} and the reading frame \texttt{frame} which must be
  0, 1, or 2.
  The space for \texttt{proteinseq} must have been allocated. In case of 
  failure, a negative error code is returned. In case of succes, the length
  of the protein sequence.
*/

Sint translateDNAforward(Uint transnum,Sint frame,Uchar *proteinseq,
                         Uchar *dnaseq,Uint dnaseqlen)
{
  Uchar *pptr, *dnaptr;
  char aa;
  Translationscheme *scheme;

#ifdef DEBUG
  checkwildcardbits();
#endif
  scheme = getschemetable(transnum);
  pptr = proteinseq;
  for(dnaptr = dnaseq+frame; 
      dnaptr < dnaseq + dnaseqlen - 2; 
      dnaptr += CODONLENGTH)
  {
    aa = codon2amino(scheme->aminos,True,*dnaptr,*(dnaptr+1),*(dnaptr+2),NULL);
    if(aa == AMINOACIDFAIL)
    {
      return (Sint) -2;
    }
    *pptr++ = (Uchar) aa;
  }
  return (Sint) (pptr - proteinseq);
}

/*EE
  The following function translates a DNA sequence \texttt{dnaseq} of length
  \texttt{dnaseqlen} into the corresponding protein sequence. The translation
  is in backward direction following the translation scheme number 
  \texttt{transnum} and the reading frame \texttt{frame} which must be
  0, $-1$, $-2$.
  The space for \texttt{proteinseq} must have been allocated. In case of 
  failure, a negative error code is returned. In case of succes, the length
  of the protein sequence.
*/

Sint translateDNAbackward(Uint transnum,Sint frame,Uchar *proteinseq,
                          Uchar *dnaseq,Uint dnaseqlen)
{
  Uchar *pptr, *dnaptr;
  char aa;
  Translationscheme *scheme;

#ifdef DEBUG
  checkwildcardbits();
#endif
  scheme = getschemetable(transnum);
  pptr = proteinseq;
  for(dnaptr = dnaseq + dnaseqlen - 1 + frame; 
      dnaptr >= dnaseq + 2; dnaptr -= CODONLENGTH)
  {
    aa = codon2amino(scheme->aminos,False,*dnaptr,*(dnaptr-1),*(dnaptr-2),NULL);
    if(aa == AMINOACIDFAIL)
    {
      return (Sint) -2;
    }
    *pptr++ = (Uchar) aa;
  }
  return (Sint) (pptr - proteinseq);
}


char extractaminoacid(BOOL forward,
                      Uint transnum,
                      Uchar *dnaseq,
                      Uint mmlength,
                      Uint start)
{
  char aa;
  Uchar *dnaptr;
  Translationscheme *scheme;

  scheme = getschemetable(transnum);
  if(forward)
  {
    dnaptr = dnaseq + start;
    aa = codon2amino(scheme->aminos,True,*dnaptr,*(dnaptr+1),*(dnaptr+2),NULL);
  } else
  {
    dnaptr = dnaseq + mmlength - 1 - start;
    aa = codon2amino(scheme->aminos,False,*dnaptr,*(dnaptr-1),*(dnaptr-2),NULL);
  }
  if(aa == AMINOACIDFAIL)
  {
    return AMINOACIDFAIL;
  }
  return aa;
}

#ifdef DEBUG

static Sint compareaminoacids(BOOL protvscodon,
                              Uint *symbolmap,
                              Uint undefsymbol,
                              Uchar aa,Uchar ab)
{
  Uint transaa, transab;

  transaa = symbolmap[(Uint) aa];
  if(transaa == undefsymbol)
  {
    ERROR1("symbolmap[%c] = undefsymbol",aa);
    return (Sint) -1;
  }
  transab = symbolmap[(Uint) ab];
  if(transab == undefsymbol)
  {
    ERROR1("symbolmap[%c] = undefsymbol",ab);
    return (Sint) -2;
  }
  if(transaa != transab)
  {
    if(protvscodon)
    {
      ERROR2("aminoacid %c in protein sequence does "
             "not match translated codon %c",aa,ab);
    } else
    {
      ERROR4("translated codon %c->%lu does not match translated codon %c->%lu",
              aa,(Showuint) transaa,ab,(Showuint) transab);
    }
    return (Sint) -3;
  }
  return 0;
}

Sint checkproteindnamatch(Alphabet *proteinalphabet,
                          Uint transnum,
                          BOOL forward,
                          Uchar *proteinseq,
                          Uint proteinmatchlength,
                          Uchar *dnaseq,
                          Uint dnamatchlength)
{
  Uchar *pptr, *dnaptr;
  char aa;
  Translationscheme *scheme;

  if(!vm_isproteinalphabet(proteinalphabet))
  {
    ERROR0("checkproteindnamatch must be called with proteinalphabet");
    return EXIT_SUCCESS;
  }
  scheme = getschemetable(transnum);
  if(forward)
  {
    for(dnaptr = dnaseq, pptr = proteinseq; 
        dnaptr+2<dnaseq+dnamatchlength; dnaptr+=CODONLENGTH, pptr++)
    {
      aa = codon2amino(scheme->aminos,True,*dnaptr,*(dnaptr+1),*(dnaptr+2),
                       NULL);
      if(aa == AMINOACIDFAIL)
      {
        return (Sint) -1;
      }
      if(pptr >= proteinseq + proteinmatchlength)
      {
        ERROR0("protein sequence is too short");
        return (Sint) -2; 
      }
      if(compareaminoacids(True,proteinalphabet->symbolmap,
                           proteinalphabet->undefsymbol,
                           *pptr,(Uchar) aa) != 0)
      {
        return (Sint) -3;
      }
    }
  } else
  {
    for(dnaptr = dnaseq + dnamatchlength - 1, pptr = proteinseq; 
        dnaptr >= dnaseq + 2; dnaptr -= CODONLENGTH, pptr++)
    {
      aa = codon2amino(scheme->aminos,False,*dnaptr,*(dnaptr-1),*(dnaptr-2),
                       NULL);
      if(aa == AMINOACIDFAIL)
      {
        return (Sint) -4;
      }
      if(pptr >= proteinseq + proteinmatchlength)
      {
        ERROR0("protein sequence is too short");
        return (Sint) -5; 
      }
      if(compareaminoacids(True,proteinalphabet->symbolmap,
                           proteinalphabet->undefsymbol,
                           *pptr,(Uchar) aa) != 0)
      {
        return (Sint) -6;
      }
    }
  }
  return 0;
}

Sint checkprotprotmatchondna(Alphabet *proteinalphabet,
                            Uint transnum,
                            BOOL leftforward,
                            BOOL rightforward,
                            Uchar *dnaseq1,
                            Uint dnamatchlength,
                            Uchar *dnaseq2)
{
  Uchar *dnaptr1, *dnaptr2;
  char aa1, aa2;
  Translationscheme *scheme;

  if(!vm_isproteinalphabet(proteinalphabet))
  {
    ERROR0("checkprotprotmatchondna must be called with proteinalphabet");
    return EXIT_SUCCESS;
  }
  scheme = getschemetable(transnum);
  if(leftforward)
  {
    dnaptr1 = dnaseq1;
  } else
  {
    dnaptr1 = dnaseq1 + dnamatchlength - 1;
  }
  if(rightforward)
  {
    dnaptr2 = dnaseq2; 
  } else
  {
    dnaptr2 = dnaseq2 + dnamatchlength - 1;
  }
  while(True)
  {
    if(leftforward)
    {
      if(dnaptr1 >= dnaseq1+dnamatchlength-2)
      {
        break;
      }
      aa1 = codon2amino(scheme->aminos,True,*dnaptr1,*(dnaptr1+1),*(dnaptr1+2),
                        NULL);
    } else
    {
      if(dnaptr1 < dnaseq1 + 2)
      {
        break;
      }
      aa1 = codon2amino(scheme->aminos,False,*dnaptr1,*(dnaptr1-1),*(dnaptr1-2),
                        NULL);
    }
    if(aa1 == AMINOACIDFAIL)
    {
      return (Sint) -1;
    }
    if(rightforward)
    {
      aa2 = codon2amino(scheme->aminos,True,*dnaptr2,*(dnaptr2+1),*(dnaptr2+2),
                        NULL);
    } else
    {
      aa2 = codon2amino(scheme->aminos,False,*dnaptr2,*(dnaptr2-1),*(dnaptr2-2),
                        NULL);
    }
    if(aa2 == AMINOACIDFAIL)
    {
      return (Sint) -2;
    }
    if(compareaminoacids(False,
                         proteinalphabet->symbolmap,
                         proteinalphabet->undefsymbol,
                         (Uchar) aa1,(Uchar) aa2) != 0)
    {
      return (Sint) -3;
    }
    if(leftforward)
    {
      dnaptr1 += CODONLENGTH;
    } else
    {
      dnaptr1 -= CODONLENGTH;
    }
    if(rightforward)
    {
      dnaptr2 += CODONLENGTH;
    } else
    {
      dnaptr2 -= CODONLENGTH;
    }
  }
  return 0;
}

Sint checkmultiproteinmatchondna(Uint transnum,
                                 Uint *symbolmap,
                                 Uchar *dnaseq,
                                 Uint dnamatchlength,
                                 Uint *positions,
                                 Uchar *forwardtable,
                                 Uint units)
{
  char aa0, aai;
  Uint mapaa0, mapaai;
  Uint unitnum, start;

  for(start = 0; start < dnamatchlength; start+= CODONLENGTH)
  {
    aa0 = extractaminoacid(forwardtable[0],
                           transnum,
                           dnaseq + positions[0],
                           dnamatchlength,
                           start);
    if(aa0 == AMINOACIDFAIL)
    {
      return (Sint) -1;
    }
    if(symbolmap == NULL)
    {
      mapaa0 = (Uint) aa0;
    } else
    {
      mapaa0 = symbolmap[(Uint) aa0];
    }
    for(unitnum=UintConst(1); unitnum<units; unitnum++)
    {
      aai = extractaminoacid(forwardtable[unitnum],
                             transnum,
                             dnaseq + positions[unitnum],
                             dnamatchlength,
                             start);
      if(aai == AMINOACIDFAIL)
      {
        return (Sint) -2;
      }
      if(symbolmap == NULL)
      {
        mapaai = (Uint) aai;
      } else
      {
        mapaai = symbolmap[(Uint) aai];
      }
      if(mapaa0 != mapaai)
      {
        ERROR6("aminoacid %c(=>%lu) in sequence 0 at position %lu does "
               "not match aminoacid %c(=>%lu) at position %lu",
               aa0,(Showuint) mapaa0,(Showuint) (positions[0] + start),
               aai,(Showuint) mapaai,(Showuint) (positions[unitnum] +start));
        return (Sint) -3;
      }
    }
  }
  return 0;
}

#endif
