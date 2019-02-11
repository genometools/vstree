//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "minmax.h"
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "failures.h"
#include "alphadef.h"
#include "alignment.h"
#include "xdropdef.h"
#include "assertion.h"
#include "codondef.h"
#include "scoredef.h"
#include "chardef.h"
#include "xmlindent.h"

#include "codon.pr"
#include "reverse.pr"

//}

#define OUTCHAR(C)         (void) genputc((Fputcfirstargtype) (C),outfp)
#define OUTEMPTY           if(!(flag & SHOWALIGNMENTNOSUBJECTANDQUERY))\
                           {\
                             genprintf(outfp,PREEMPTY);\
                           }

#define OUTSUBJECT         if(!(flag & SHOWALIGNMENTNOSUBJECTANDQUERY))\
                           {\
                             genprintf(outfp,"%s: ",PRESUBJECT);\
                           }

#define OUTQUERY           if(!(flag & SHOWALIGNMENTNOSUBJECTANDQUERY))\
                           {\
                             genprintf(outfp,"%s: ",PREQUERY);\
                           }

#define OUTSTRINGNUM(N,S)  genprintf(outfp,"%*lu",(Ctypeargumenttype) (N),\
                                                  (Showuint) (S))

#define CHECK_FOR_IDENTICAL_SUBSTRING_OF_LENGTH_0(EOPPTR)\
        if((EOPPTR) == 0)\
        {\
          fprintf(stderr,"identical substring of length 0 not allowed\n");\
          exit(EXIT_FAILURE);\
        }

#define CHECK_IF_PROTEINEOP\
        if(!proteineop)\
        {\
          fprintf(stderr, "protein edit operation %hu used as normal one\n"\
                 , eop);\
        }

#define TRANSLATE_DNA_INTO_CODON\
        if(translateDNAforward(translationschemenumber, 0, &codon, dna,\
                               (Uint) CODONLENGTH) != SintConst(1))\
        {\
          fprintf(stderr,"%s\n", messagespace());\
          exit(EXIT_FAILURE);\
        }

#define GENOMICDNAPTR_STILL_VALID\
        ASSERTION("genomicdnaptr still valid"\
                 , genomicdnaptr < aftergenomicdnaline)

#define GENOMICPROTEINPTR_STILL_VALID\
        ASSERTION("genomicproteinptr still valid"\
                 , genomicproteinptr < aftergenomicproteinline)

#define REFERENCEPROTEINPTR_STILL_VALID\
        ASSERTION("referenceproteinptr still valid"\
                 , referenceproteinptr < afterreferenceproteinline)

#define EQUAL_AMINO_ACID_CHAR	'|'
#define POSITIVE_SCORE_CHAR	'+'
#define ZERO_SCORE_CHAR		'.'
#define NEGATIVE_SCORE_CHAR	' '

#define SHOW_MULTI_EDIT_OPERATION(E)\
        GEN_XML_INDENT;\
        if(proteineop)\
        {\
          genprintf(outfp, "<Protein_eop_type>");\
          genprintf(outfp, E);\
          genprintf(outfp, "</Protein_eop_type>\n");\
        }\
        else\
        {\
          genprintf(outfp, "<DNA_eop_type>");\
          genprintf(outfp, E);\
          genprintf(outfp, "</DNA_eop_type>\n");\
        }

#define SHOW_MULTI_EDIT_OPERATION_LENGTH(L)\
        GEN_XML_INDENT;\
        if(proteineop)\
        {\
          genprintf(outfp, "<Protein_eop_length>");\
          genprintf(outfp, "%lu", (Showuint) (L));\
          genprintf(outfp, "</Protein_eop_length>\n");\
        }\
        else\
        {\
          genprintf(outfp, "<DNA_eop_length>");\
          genprintf(outfp, "%lu", (Showuint) (L));\
          genprintf(outfp, "</DNA_eop_length>\n");\
        }

Eoptype determine_eop_type(Editoperation eop, BOOL proteineop)
{
  Editoperation maxlen = proteineop ? (Editoperation) MAXIDENTICALLENGTH_PROTEIN
                                    : (Editoperation) MAXIDENTICALLENGTH;

  if(eop & maxlen)
  {
    switch(eop & ~maxlen)
    {
      case 0:
        return EOP_TYPE_MATCH;
      case DELETIONEOP:
        return EOP_TYPE_INTRON;
      case DELETION_WITH_1_GAP_EOP:
        CHECK_IF_PROTEINEOP;
        return EOP_TYPE_INTRON_WITH_1_BASE_LEFT;
      case DELETION_WITH_2_GAPS_EOP:
        CHECK_IF_PROTEINEOP;
        return EOP_TYPE_INTRON_WITH_2_BASES_LEFT;
      DEFAULTFAILURE;
    }
  } 
  else
  {
    switch(eop)
    {
      case MISMATCHEOP:
        return EOP_TYPE_MISMATCH;
      case DELETIONEOP:
        return EOP_TYPE_DELETION;
      case INSERTIONEOP:
        return EOP_TYPE_INSERTION;
      case MISMATCH_WITH_1_GAP_EOP:
        return EOP_TYPE_MISMATCH_WITH_1_GAP;
      case MISMATCH_WITH_2_GAPS_EOP:
        CHECK_IF_PROTEINEOP;
        return EOP_TYPE_MISMATCH_WITH_2_GAPS;
      case DELETION_WITH_1_GAP_EOP: 
        CHECK_IF_PROTEINEOP;
        return EOP_TYPE_DELETION_WITH_1_GAP;
      case DELETION_WITH_2_GAPS_EOP: 
        CHECK_IF_PROTEINEOP;
        return EOP_TYPE_DELETION_WITH_2_GAPS;
      default:
        fprintf(stderr, "illegal edit operation %hu\n", eop);
        exit(EXIT_FAILURE);
    }
  }
}

Uint determine_eop_length(Editoperation eop, BOOL proteineop)
{
  Editoperation maxlen = proteineop ? (Editoperation) MAXIDENTICALLENGTH_PROTEIN
                                    : (Editoperation) MAXIDENTICALLENGTH;

  if(eop & maxlen)
  {
    return (Uint) eop & maxlen;
  } 
  else
  {
    return UintConst(1);
  }
}

static void showoneeditopgeneric(GENFILE *outfp, Editoperation eop, 
                                 BOOL proteineop, 
                                 BOOL xmlout, Uint indentlevel,
                                 BOOL nexteopisdefined, Editoperation nexteop,
                                 Uint *consecutive_eop_length)
{
  Uint eop_length;

  // this code is necessary to show consecutive edit operations of the same
  // type as one edit operation in the XML output 
  if(xmlout)
  {
    if(nexteopisdefined &&
       determine_eop_type(eop, proteineop) == 
       determine_eop_type(nexteop, proteineop))
    {
      // store length of this eop
      *consecutive_eop_length += determine_eop_length(eop, proteineop);
      // return, this consecutive eop is shown later
      return;
    }
    else
    {
      // store total length of this consecutive eop for output
      eop_length = *consecutive_eop_length + 
                   determine_eop_length(eop, proteineop);
      // reset
      *consecutive_eop_length = 0; 
    }
  }
  else
  {
    eop_length = determine_eop_length(eop, proteineop);
  }

  switch(determine_eop_type(eop, proteineop))
  {
    case EOP_TYPE_MATCH:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("match");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "(M %lu)", (Showuint) eop_length);
      }
      break;
    case EOP_TYPE_INTRON:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("intron");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "(Intron%s %lu)", proteineop ? "(0)" : ""
                 , (Showuint) eop_length);
      }
      break;
    case EOP_TYPE_INTRON_WITH_1_BASE_LEFT:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("intron_with_1_base_left");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "(Intron(1) %lu)", (Showuint) eop_length); 
      }
      break;
    case EOP_TYPE_INTRON_WITH_2_BASES_LEFT:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("intron_with_2_bases_left");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "(Intron(2) %lu)", (Showuint) eop_length);
      }
      break;
    case EOP_TYPE_MISMATCH:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("mismatch");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        (void) genputc('R',outfp);
      }
      break;
    case EOP_TYPE_DELETION:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("deletion");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        (void) genputc('D',outfp);
      }
      break;
    case EOP_TYPE_INSERTION:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("insertion");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        (void) genputc('I',outfp);
      }
      break;
    case EOP_TYPE_MISMATCH_WITH_1_GAP:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("mismatch_with_1_gap");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "R1");
      }
      break;
    case EOP_TYPE_MISMATCH_WITH_2_GAPS:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("mismatch_with_2_gaps");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "R2");
      }
      break;
    case EOP_TYPE_DELETION_WITH_1_GAP:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("deletion_with_1_gap");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "D1");
      }
      break;
    case EOP_TYPE_DELETION_WITH_2_GAPS:
      if(xmlout)
      {
        SHOW_MULTI_EDIT_OPERATION("deletion_with_2_gaps");
        SHOW_MULTI_EDIT_OPERATION_LENGTH(eop_length);
      }
      else
      {
        genprintf(outfp, "D2");
      }
      break;
    DEFAULTFAILURE;
  }
  if(!xmlout)
  {
    (void) genputc(' ', outfp);
  }
}

void showoneeditop(FILE *fp, Editoperation eop)
{
  GENFILE genfile;

  genfile.genfilemode  = STANDARD;
  genfile.fileptr.file = fp;

  showoneeditopgeneric(&genfile, eop, False, False, 0, False, 0, NULL);
}

void showonednaorproteineditop(FILE *fp, Editoperation eop, BOOL proteineop)
{
  GENFILE genfile;

  genfile.genfilemode  = STANDARD;
  genfile.fileptr.file = fp;

  showoneeditopgeneric(&genfile, eop, proteineop, False, 0, False, 0, NULL);
}


/*
  This is the generic function to show edit operations for DNA and protein
  edit operations. They can be shown either in plain text or in XML format.
*/
void showeditopsgeneric(Editoperation *al, Uint lenalg, BOOL proteineops, 
                        BOOL xmlout, Uint indentlevel, GENFILE *outfp)
{
  Sint i;
  Uint consecutive_eop_length = 0;

  if(xmlout)
  {
    GEN_XML_INDENT;
    if(proteineops)
    {
      genprintf(outfp, "<Protein_eops>\n");
    }
    else
    {
      genprintf(outfp, "<DNA_eops>\n");
    }
  }

  for(i=(Sint) (lenalg-1); i>= 0; i--)
  {
    if(i > 0)
    {
      showoneeditopgeneric(outfp, al[i], proteineops, xmlout, indentlevel + 1
                          , True // the next eop is defined
                          , al[i-1]
                          , &consecutive_eop_length);
    }
    else
    {
      showoneeditopgeneric(outfp, al[i], proteineops, xmlout, indentlevel + 1
                          , False // the next eop is defined
                          , 0
                          , &consecutive_eop_length);

    }
  }

  if(xmlout)
  {
    GEN_XML_INDENT;
    if(proteineops)
    {
      genprintf(outfp, "</Protein_eops>\n");
    }
    else
    {
      genprintf(outfp, "</DNA_eops>\n");
    }

  }
  else
  {
    (void) genputc('\n', outfp);
  }
}

/*
  This is the specialized function for DNA edit operations in plain text format.
*/
void showeditops(Editoperation *al, Uint lenalg, FILE *outfp)
{
  GENFILE genfile;

  genfile.genfilemode  = STANDARD;
  genfile.fileptr.file = outfp;

  showeditopsgeneric(al, lenalg, False, False, 0, &genfile);
}

//\Ignore{
#define SHORTINTRONINFOINCREASE		8

#ifdef DEBUG
static void verifyalignmentgeneric(char *file,
                                   Uint line,
                                   Editoperation *al,
                                   Uint lenalg,
                                   Uint ulen,
                                   Uint vlen,
                                   Xdropscore mustscore,
                                   Xdropscore matchscore,
                                   Xdropscore mismatchscore,
                                   Xdropscore indelscore)
{
  Uint i, ilen = 0, jlen = 0;
  Xdropscore scoresum = 0;
  Editoperation eop;
 
  DEBUG5(2,"%s,%lu: verifyalignment of len %lu, ulen=%lu, vlen=%lu\n",
            file,
            (Showuint) line,
            (Showuint) lenalg,
            (Showuint) ulen,
            (Showuint) vlen);
  for(i=0; i<lenalg; i++)
  {
    eop = al[i];
    DEBUGCODE(3,showoneeditop(getdbgfp(), eop));
    DEBUG0(3,"\n");
    if(eop & MAXIDENTICALLENGTH)
    {
      ilen += (eop & MAXIDENTICALLENGTH);
      jlen += (eop & MAXIDENTICALLENGTH);
      scoresum += (eop & MAXIDENTICALLENGTH) * matchscore;
    } else
    {
      if(eop == MISMATCHEOP)
      {
        ilen++;
        jlen++;
        scoresum += mismatchscore;
      } else
      {
        if(eop == DELETIONEOP)
        {
          ilen++;
          scoresum += indelscore;
        } else
        {
          if(eop == INSERTIONEOP)
          {
            jlen++;
            scoresum += indelscore;
          } else
          {
            fprintf(stderr,"file %s, line %lu\n",file,(Showuint) line);
            fprintf(stderr,"Illegal edit operation %hu\n",eop);
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  }
  if(ilen != ulen)
  {
    fprintf(stderr,"file %s, line %lu\n",file,(Showuint) line);
    fprintf(stderr,"verifygenericalignment failed: ilen=%lu != %lu=ulen\n",
            (Showuint) ilen,
            (Showuint) ulen);
    exit(EXIT_FAILURE);
  }
  if(jlen != vlen)
  {
    fprintf(stderr,"file %s, line %lu\n",file,
            (Showuint) line);
    fprintf(stderr,"verifygenericalignment failed: jlen=%lu != %lu=vlen\n",
            (Showuint) jlen,
            (Showuint) vlen);
    exit(EXIT_FAILURE);
  }
  if(scoresum != mustscore)
  {
    fprintf(stderr,"file %s, line %lu\n",file,
            (Showuint) line);
    fprintf(stderr,"verifygenericalignment failed: "
                   "scoresum=%ld != %ld=mustscore\n",
                   (Showsint) scoresum,(Showsint) mustscore);
    exit(EXIT_FAILURE);
  }
}

void verifyxdropalignment(char *file,
                          Uint line,
                          Editoperation *al,
                          Uint lenalg,
                          Uint ulen,
                          Uint vlen,
                          Xdropscore mustscore)
{
  verifyalignmentgeneric(file,
                         line,
                         al,
                         lenalg,
                         ulen,
                         vlen,
                         mustscore,
                         MATCHSCORE,
                         MISMATCHSCORE,
                         INDELSCORE);
}

void verifyedistalignment(char *file,
                          Uint line,
                          Editoperation *al,
                          Uint lenalg,
                          Uint ulen,
                          Uint vlen,
                          Xdropscore mustscore)
{
  verifyalignmentgeneric(file,
                         line,
                         al,
                         lenalg,
                         ulen,
                         vlen,
                         mustscore,
                         0,
                         (Xdropscore) 1,
                         (Xdropscore) 1);
}
#endif

//}


/*EE
  This file implements functions to formatting and showing an alignment. 
  The alignment is formatted in two steps. In the first step we
  take the list of edit operations and compute the two lines of the alignment 
  with inserted abstract gap symbols for insertions and deletions. 
  The characters are however stored as integers in the range [0..alphasize-1]. 
  In the second phase the two lines are formatted such that they fit
  on the given linewidth. Additionally, the position offset for the
  line are shown on the right. Moreover, if the given linewidth contains 
  at least one mismatch or indel, this is shown as an extra line 
  marking the corresponding columns by the symbol 
  \texttt{\symbol{34}\symbol{33}\symbol{34}}.
*/

/*
  The following has been included:
*/

#define NUMWIDTH          12            // width of position right of alignment

void reverseAlignment(Editoperation *alignment,Uint numofeditops)
{
  Editoperation *front, *back, tmp;

  for(front = alignment, back = alignment + numofeditops - 1; 
      front < back; front++, back--)
  {
    tmp = *front;
    *front = *back;
    *back = tmp;
  }
}

static Uint calctotallengthofintron(Uint *numofeopsinintron,
                                    Editoperation *intronstart, 
                                    Editoperation *alignmentstart)
{
  Editoperation *eopptr;
  Uint totalintronlength = 0;
  BOOL breakforloop = False;

  *numofeopsinintron = 0;

  for(eopptr = intronstart; eopptr >= alignmentstart; eopptr--)
  {
    switch(*eopptr)
    {
      case MISMATCHEOP:
      case DELETIONEOP:
      case INSERTIONEOP:
        breakforloop = True;
        break;
      default:
        switch(*eopptr & ~MAXIDENTICALLENGTH)
        {
          case 0:                               // match
            breakforloop = True;
            break;
          case DELETIONEOP:                     // intron
            (*numofeopsinintron)++;
            totalintronlength += *eopptr & MAXIDENTICALLENGTH;
            break;
        }
    }
    if(breakforloop)
    {
      break; 
    }
  }

  return totalintronlength;
}

/*
  The following function implements the first step mentioned above, i.e.\
  it fills the \texttt{firstlinecompare} and the \texttt{secondlinecompare}
  of the alignment.
*/

#define USEQ(I) (useq[forward ? (I) : (ulen-1-(I))])
#define VSEQ(J) (vseq[forward ? (J) : (vlen-1-(J))])

static Uint fillthelines(BOOL forward,
                         Uchar *firstline,Uchar *secondline,
                         Uchar *useq,Uint ulen,
                         Uchar *vseq,Uint vlen,
                         Editoperation *alignment, Uint lenalg, 
                         Uint linewidth, Uint showintronmaxlen, 
                         ArrayShortintroninfo *shortintroninfo)
{
  Editoperation *eopptr;
  Uint l, intronlength, totalintronlength, numofeopsinintron, restlength, end,
       completeshortintronlen = 0, i = 0, j = 0;
  Uchar *fptr = firstline, *sptr = secondline;
  Shortintroninfo oneshortintron;

/*
  first phase: construct first line of alignment
*/
  for(eopptr = alignment+lenalg-1; eopptr >= alignment; eopptr--)
  {
    switch(*eopptr)
    {
      case MISMATCHEOP:
      case DELETIONEOP:
        *fptr++ = USEQ(i);
        i++;
        break;
      case INSERTIONEOP:
        *fptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        break;
      default:
        switch(*eopptr & ~MAXIDENTICALLENGTH)
        {
        case 0:                               // match
          CHECK_FOR_IDENTICAL_SUBSTRING_OF_LENGTH_0(*eopptr);
          for(l=0; l< (Uint) *eopptr; l++)
          {
            *fptr++ = USEQ(i);
            i++;
          }
          break;
        case DELETIONEOP:                     // intron
          if(showintronmaxlen > 0)
          {
            // compute total intron length
            totalintronlength = calctotallengthofintron(&numofeopsinintron,
                                                        eopptr, alignment);

            // compute the rest length which is necessary to fill the current
            // line
            restlength = (Uint) (fptr - firstline) % linewidth;
            if(restlength != 0)
            {
              ASSERTION("restlength is strictly smaller than linewidth"
                       , restlength < linewidth);
              restlength = linewidth - restlength; 
            }

            // check if introducing a ``short'' intron here is necessary
            if(totalintronlength >=
               showintronmaxlen + restlength + 2 * linewidth )
            {
               // introduce ``short'' intron
               
               // write the beginning of the intron in the conventional way
               for(l = 0; l < restlength + linewidth; l++)
               {
                 *fptr++ = USEQ(i); 
                 i++;
               }

               // store short intron information if necessary
               if(shortintroninfo != NULL)
               {
                 oneshortintron.start  = (Uint) (fptr - firstline + 
                                                 completeshortintronlen);
                 end = oneshortintron.start + 
                       ((totalintronlength - restlength - 2 * linewidth)
                        / linewidth) * linewidth - 1; 
                 oneshortintron.length  = end - oneshortintron.start + 1;
                 completeshortintronlen += oneshortintron.length;

                 STOREINARRAY(shortintroninfo, Shortintroninfo, 
                              SHORTINTRONINFOINCREASE, oneshortintron);
               }
 
               // add length of short intron to i 
               i += ((totalintronlength - restlength - 2 * linewidth) 
                     / linewidth) * linewidth;

               // write the end  of the intron in the conventional way
               for(l = 0; 
                   l < linewidth + 
                       ((totalintronlength - restlength - 2 * linewidth) % 
                        linewidth); 
                   l++)
               {
                 *fptr++ = USEQ(i); 
                 i++;
               }

               // place eopptr to the last multi editoperation of this intron
               // because the intron has already been completely processed
               eopptr -= (numofeopsinintron - 1);
            
               // intron is already processed, therefore continue
               continue;
            }
          }

          intronlength = *eopptr & MAXIDENTICALLENGTH;
          for(l = 0; l < intronlength; l++)
          {
            *fptr++ = USEQ(i); 
            i++;
          }
        break;
        DEFAULTFAILURE;
        }
    }
  }
/*
  first phase: construct second line of alignment
*/
  for(eopptr = alignment+lenalg-1; eopptr >= alignment; eopptr--)
  {
    switch(*eopptr)
    {
      case MISMATCHEOP:
      case INSERTIONEOP:
        *sptr++ = VSEQ(j);
        j++;
        break;
      case DELETIONEOP:
        *sptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        break;
      default:
        switch(*eopptr & ~MAXIDENTICALLENGTH)
        {
        case 0:                               // match
          CHECK_FOR_IDENTICAL_SUBSTRING_OF_LENGTH_0(*eopptr);
          for(l=0; l< (Uint) *eopptr; l++)
          {
            *sptr++ = VSEQ(j);
            j++;
          }
          break;
        case DELETIONEOP:                     // intron
          if(showintronmaxlen > 0)
          {
            // compute total intron length
            totalintronlength = calctotallengthofintron(&numofeopsinintron,
                                                        eopptr, alignment);

            // compute the rest length which is necessary to fill the current
            // line
            restlength = (Uint) (sptr - secondline) % linewidth;
            if(restlength != 0)
            {
              ASSERTION("restlength is strictly smaller than linewidth"
                       , restlength < linewidth);
              restlength = linewidth - restlength;
            }

            // check if introducing a ``short'' intron here is necessary
            if(totalintronlength >=
               showintronmaxlen + restlength + 2 * linewidth )
            { 
               // write the rest of the intron in the conventional way
               for(l = 0; 
                   l < restlength + 2 * linewidth + 
                       ((totalintronlength - restlength - 2 * linewidth) %
                        linewidth); 
                   l++)
               {
                 *sptr++ = (Uchar) ABSTRACTINTRONSYMBOL; 
               }

               // place eopptr to the last multi editoperation of this intron
               // because the intron has already been completely processed
               eopptr -= (numofeopsinintron - 1);

               // intron is already processed, therefore continue
               continue;
            }
          }

          intronlength = *eopptr & MAXIDENTICALLENGTH;
          for(l=0; l<intronlength; l++)
          {
            *sptr++ = (Uchar) ABSTRACTINTRONSYMBOL;
          }
          break;
        DEFAULTFAILURE;
        }
    }
  }

  return (Uint) (sptr-secondline);
}

static void match_mismatch_genomicdnaline(Uchar **genomicdnaptr,
                                          BOOL 
                                          *processing_intron_with_1_base_left,
                                          BOOL
                                          *processing_intron_with_2_bases_left,
                                          Uchar *genseqorig,
                                          Uint *genseqindex)
{
  if(*processing_intron_with_2_bases_left)
  {
    // this means we are after an intron with 2 bases left
    // this bases have already been shown, therefore we only show 1 more
    *processing_intron_with_2_bases_left = False;
    **genomicdnaptr = genseqorig[(*genseqindex)++];
    (*genomicdnaptr)++;
  }
  else
  {
    if(*processing_intron_with_1_base_left)
    {
      // this means we are after an intron with 1 base left
      // this base has already been shown, therefore we only show 2 more
      *processing_intron_with_1_base_left = False;
    }
    else
    {
      **genomicdnaptr = genseqorig[(*genseqindex)++];
      (*genomicdnaptr)++;
    }
    **genomicdnaptr = genseqorig[(*genseqindex)++];
    (*genomicdnaptr)++;
    **genomicdnaptr = genseqorig[(*genseqindex)++];
    (*genomicdnaptr)++;
  }
}

/*
  The following function is used to construct the genomic DNA line of
  protein alignments.
*/

static Uint construct_genomic_dna_line(Uchar *genomicdnaline, 
#ifdef ASSERTIONS
                                       Uint lengthofgenomicdnaline,
#endif
                                       Uchar *genseqorig,
                                       Editoperation *alignment, 
                                       Uint lenalg,
                                       Uint showintronmaxlen)
{
  Editoperation *eopptr; 
  Eoptype eoptype;
  Uint eoplength, l,
       i = 0;
  Uchar *genomicdnaptr = genomicdnaline;
#ifdef ASSERTIONS
  Uchar *aftergenomicdnaline = genomicdnaline + lengthofgenomicdnaline;
#endif
  BOOL processing_intron_with_1_base_left  = False,
       processing_intron_with_2_bases_left = False;

  for(eopptr = alignment + lenalg - 1; eopptr >= alignment; eopptr--)
  {
    eoptype   = determine_eop_type(*eopptr,   True /* proteineop*/ );
    eoplength = determine_eop_length(*eopptr, True /* proteineop*/ );

    ASSERTION("we are not processing two intron types at the same time"
             , !(processing_intron_with_1_base_left &&
                 processing_intron_with_2_bases_left));

    switch(eoptype)
    {
      case EOP_TYPE_DELETION:
        ASSERTION("we are not processing with 1 base left here"
                 , !processing_intron_with_1_base_left);
        ASSERTION("we are not processing with 2 bases left here"
                 , !processing_intron_with_2_bases_left);
        /*@fallthrough@*/
      case EOP_TYPE_MISMATCH:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        match_mismatch_genomicdnaline(&genomicdnaptr,
                                      &processing_intron_with_1_base_left,
                                      &processing_intron_with_2_bases_left,
                                      genseqorig,
                                      &i);
        break;
      case EOP_TYPE_INSERTION:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        ASSERTION("we are not processing with 1 base left here"
                 , !processing_intron_with_1_base_left);
        ASSERTION("we are not processing with 2 bases left here"
                 , !processing_intron_with_2_bases_left);
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        break;
      case EOP_TYPE_MISMATCH_WITH_1_GAP:
      case EOP_TYPE_DELETION_WITH_1_GAP:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        ASSERTION("we are not processing with 1 base left here"
                 , !processing_intron_with_1_base_left);
        ASSERTION("we are not processing with 2 bases left here"
                 , !processing_intron_with_2_bases_left);
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = genseqorig[i++];
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = genseqorig[i++];
        break;
      case EOP_TYPE_MISMATCH_WITH_2_GAPS:
      case EOP_TYPE_DELETION_WITH_2_GAPS:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        ASSERTION("we are not processing with 1 base left here"
                 , !processing_intron_with_1_base_left);
        ASSERTION("we are not processing with 2 bases left here"
                 , !processing_intron_with_2_bases_left);
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = genseqorig[i++];
        GENOMICDNAPTR_STILL_VALID;
        *genomicdnaptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        break;
      case EOP_TYPE_MATCH:
        for(l=0; l < eoplength; l++)
        {
          match_mismatch_genomicdnaline(&genomicdnaptr,
                                        &processing_intron_with_1_base_left,
                                        &processing_intron_with_2_bases_left,
                                        genseqorig,
                                        &i);
        }
        break;
      case EOP_TYPE_INTRON_WITH_2_BASES_LEFT:
        if(!processing_intron_with_2_bases_left)
        {
          processing_intron_with_2_bases_left = True;
          GENOMICDNAPTR_STILL_VALID;
          *genomicdnaptr++ = genseqorig[i++]; 
          GENOMICDNAPTR_STILL_VALID;
          *genomicdnaptr++ = genseqorig[i++]; 
        }
        // skip the next case statement and process this intron
        goto process_intron;
      case EOP_TYPE_INTRON_WITH_1_BASE_LEFT:
        if(!processing_intron_with_1_base_left)
        {
          processing_intron_with_1_base_left = True;
          GENOMICDNAPTR_STILL_VALID;
          *genomicdnaptr++ = genseqorig[i++]; 
        }
        /*@fallthrough@*/
      case EOP_TYPE_INTRON:
      process_intron: 
        if(showintronmaxlen > 0)
        {
          NOTIMPLEMENTED;
          /*
            // compute total intron length
            totalintronlength = calctotallengthofintron(&numofeopsinintron,
                                                        eopptr, alignment);

            // compute the rest length which is necessary to fill the current
            // line
            restlength = (Uint) (genomicdnaptr - genomicdnaline) % linewidth;
            if(restlength != 0)
            {
              ASSERTION("restlength is strictly smaller than linewidth"
                       , restlength < linewidth);
              restlength = linewidth - restlength; 
            }

            // check if introducing a ``short'' intron here is necessary
            if(totalintronlength >=
               showintronmaxlen + restlength + 2 * linewidth )
            {
               // introduce ``short'' intron
               
               // write the beginning of the intron in the conventional way
               for(l = 0; l < restlength + linewidth; l++)
               {
                 GENOMICDNAPTR_STILL_VALID;
                 *genomicdnaptr++ = genseqorig[i]; 
                 i++;
               }

               // store short intron information if necessary
               if(shortintroninfo != NULL)
               {
                 oneshortintron.start  = (Uint) (genomicdnaptr - 
                                                 genomicdnaline + 
                                                 completeshortintronlen);
                 end = oneshortintron.start + 
                       ((totalintronlength - restlength - 2 * linewidth)
                        / linewidth) * linewidth - 1; 
                 oneshortintron.length  = end - oneshortintron.start + 1;
                 completeshortintronlen += oneshortintron.length;

                 STOREINARRAY(shortintroninfo, Shortintroninfo, 
                              SHORTINTRONINFOINCREASE, oneshortintron);
               }
 
               // add length of short intron to i 
               i += ((totalintronlength - restlength - 2 * linewidth) 
                     / linewidth) * linewidth;

               // write the end  of the intron in the conventional way
               for(l = 0; 
                   l < linewidth + 
                       ((totalintronlength - restlength - 2 * linewidth) % 
                        linewidth); 
                   l++)
               {
                 GENOMICDNAPTR_STILL_VALID;
                 *genomicdnaptr++ = genseqorig[i]; 
                 i++;
               }

               // place eopptr to the last multi editoperation of this intron
               // because the intron has already been completely processed
               eopptr -= (numofeopsinintron - 1);
            
               // intron is already processed, therefore continue
               continue;
            }
            */
        }

        for(l = 0; l < eoplength; l++)
        {
          GENOMICDNAPTR_STILL_VALID;
          *genomicdnaptr++ = genseqorig[i++]; 
        }
        break;
      DEFAULTFAILURE;
    }
  }

  ASSERTION("we are not processing with 1 base left here"
           , !processing_intron_with_1_base_left);
  ASSERTION("we are not processing with 2 bases left here"
           , !processing_intron_with_2_bases_left);

  return (Uint) (genomicdnaptr - genomicdnaline);
}

static void match_mismatch_genomicproteinline(Uchar **genomicproteinptr,
                                          BOOL 
                                          *processing_intron_with_1_base_left,
                                          BOOL 
                                          *processing_intron_with_2_bases_left,
                                          Uchar *genseqorig,
                                          DefinedUchar *first_base_left,
                                          DefinedUchar *second_base_left,
                                          Uchar *dummyptr,
                                          Uint *genseqindex,
                                          Uint translationschemenumber)
{
  Uchar codon, dna[CODONLENGTH];

  if(*processing_intron_with_2_bases_left)
  {
    *processing_intron_with_2_bases_left = False;
    ASSERTION("first base left is defined"
             , first_base_left->defined);
    ASSERTION("second base left is defined"
             , second_base_left->defined);
    ASSERTION("dummy pointer is defined", dummyptr != NULL);
    dna[0] = first_base_left->ucharvalue;
    dna[1] = second_base_left->ucharvalue;
    dna[2] = genseqorig[(*genseqindex)++];
    first_base_left->defined = False;
    second_base_left->defined = False;
    TRANSLATE_DNA_INTO_CODON;
    // set dummy pointer
    *dummyptr = codon;
    // show second blank here
    **genomicproteinptr = (Uchar) ' ';
    (*genomicproteinptr)++;
  }
  else
  {
    if(*processing_intron_with_1_base_left)
    {
      *processing_intron_with_1_base_left = False;
      ASSERTION("first base left is defined"
               , first_base_left->defined);
      dna[0] = first_base_left->ucharvalue;
      first_base_left->defined = False;
    }
    else
    {
      dna[0] = genseqorig[(*genseqindex)++];
    }
    dna[1] = genseqorig[(*genseqindex)++];
    dna[2] = genseqorig[(*genseqindex)++];
    TRANSLATE_DNA_INTO_CODON;
    (**genomicproteinptr) = (Uchar) ' ';
    (*genomicproteinptr)++;
    (**genomicproteinptr) = codon;
    (*genomicproteinptr)++;
    (**genomicproteinptr) = (Uchar) ' ';
    (*genomicproteinptr)++;
  }
}

/*
  The following function is used to construct the genomic protein line of
  protein alignments.
*/

static Uint construct_genomic_protein_line(Uchar *genomicproteinline,
#ifdef ASSERTIONS
                                           Uint lengthofgenomicproteinline,
#endif
                                           Uchar *genseqorig,
                                           Editoperation *alignment, 
                                           Uint lenalg,
                                           /*@unused@*/ Uint showintronmaxlen,
                                           Uint translationschemenumber)
{
  Editoperation *eopptr; 
  Eoptype eoptype;
  Uint eoplength, l,
       i = 0;
  Uchar *dummyptr          = NULL,
        *genomicproteinptr = genomicproteinline;
  DefinedUchar first_base_left = {0,False},
               second_base_left = {0,False};
#ifdef ASSERTIONS
  Uchar *aftergenomicproteinline   = genomicproteinline + 
                                     lengthofgenomicproteinline;
#endif
  BOOL processing_intron_with_1_base_left  = False,
       processing_intron_with_2_bases_left = False;

  first_base_left.defined = False;
  second_base_left.defined = False;

  for(eopptr = alignment + lenalg - 1; eopptr >= alignment; eopptr--)
  {
    eoptype   = determine_eop_type(*eopptr,   True /* proteineop*/ );
    eoplength = determine_eop_length(*eopptr, True /* proteineop*/ );

    ASSERTION("we are not processing two intron types at the same time"
             , !(processing_intron_with_1_base_left &&
                 processing_intron_with_2_bases_left));

    DEBUG2(2, "%s(): i=%lu\n", FUNCTIONNAME, (Showuint) i);
    /*@ignore@*/
    DEBUGCODE(2,{fprintf(getdbgfp(), "%s(): ", FUNCTIONNAME);
                showonednaorproteineditop(getdbgfp(), *eopptr, True);
                (void) putc('\n', getdbgfp());});
    /*@end@*/

    switch(eoptype)
    {
      case EOP_TYPE_MISMATCH:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        match_mismatch_genomicproteinline(&genomicproteinptr,
                                          &processing_intron_with_1_base_left,
                                          &processing_intron_with_2_bases_left,
                                          genseqorig,
                                          &first_base_left,
                                          &second_base_left,
                                          dummyptr,
                                          &i,
                                          translationschemenumber);
        break;
      case EOP_TYPE_MISMATCH_WITH_1_GAP:
      case EOP_TYPE_DELETION_WITH_1_GAP:
        // skip two characters in genomic dna (fall through case!)
        i++;
        /*@fallthrough@*/
      case EOP_TYPE_MISMATCH_WITH_2_GAPS:
      case EOP_TYPE_DELETION_WITH_2_GAPS:
        // skip one characters in genomic dna (fall through case!)
        i++;
        /*@fallthrough@*/
      case EOP_TYPE_DELETION:
      case EOP_TYPE_INSERTION:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        ASSERTION("we are not processing with 1 base left here"
                 , !processing_intron_with_1_base_left);
        ASSERTION("we are not processing with 2 bases left here"
                 , !processing_intron_with_2_bases_left);
        GENOMICPROTEINPTR_STILL_VALID;
        *genomicproteinptr++ = (Uchar) ' ';
        GENOMICPROTEINPTR_STILL_VALID;
        *genomicproteinptr++ = (Uchar) ' ';
        GENOMICPROTEINPTR_STILL_VALID;
        *genomicproteinptr++ = (Uchar) ' ';
        break;
      case EOP_TYPE_MATCH:
        for(l = 0; l < eoplength; l++)
        {
          match_mismatch_genomicproteinline(&genomicproteinptr,
                                           &processing_intron_with_1_base_left,
                                           &processing_intron_with_2_bases_left,
                                           genseqorig,
                                           &first_base_left,
                                           &second_base_left,
                                           dummyptr,
                                           &i,
                                           translationschemenumber);
        }
        break;
      case EOP_TYPE_INTRON_WITH_2_BASES_LEFT:
        if(!processing_intron_with_2_bases_left)
        {
          processing_intron_with_2_bases_left = True;
          // save the first two bases
          first_base_left.ucharvalue  = genseqorig[i++];
          first_base_left.defined = True;
          second_base_left.ucharvalue= genseqorig[i++];
          second_base_left.defined = True;
          // print first blank here already
          GENOMICPROTEINPTR_STILL_VALID;
          *genomicproteinptr++ = (Uchar) ' ';
          // save dummy pointer
          GENOMICPROTEINPTR_STILL_VALID;
          dummyptr = genomicproteinptr++;
        }
        // skip the next case statement and process this intron
        goto process_intron;
      case EOP_TYPE_INTRON_WITH_1_BASE_LEFT:
        if(!processing_intron_with_1_base_left)
        {
          processing_intron_with_1_base_left = True;
          first_base_left.ucharvalue = genseqorig[i++];
          first_base_left.defined = True;
        }
        /*@fallthrough@*/
      case EOP_TYPE_INTRON:
      process_intron:
        for(l = 0; l < eoplength; l++)
        {
          GENOMICPROTEINPTR_STILL_VALID;
          *genomicproteinptr++ = (Uchar) ' ';
          i++;
        }
        break;
      DEFAULTFAILURE; 
    }
  }

  ASSERTION("we are not processing with 1 base left here"
           , !processing_intron_with_1_base_left);
  ASSERTION("we are not processing with 2 bases left here"
           , !processing_intron_with_2_bases_left);

  return (Uint) (genomicproteinptr - genomicproteinline);
}

static void match_mismatch_referenceproteinline(Uchar **referenceproteinptr,
                                          BOOL 
                                          *processing_intron_with_1_base_left,
                                          BOOL 
                                          *processing_intron_with_2_bases_left,
                                          Uchar *refseqorig,
                                          Uint *refseqindex)
{
  if(*processing_intron_with_2_bases_left)
  {
    *processing_intron_with_2_bases_left = False;
    **referenceproteinptr = (Uchar) ' ';
    (*referenceproteinptr)++;
  }
  else
  {
    if(*processing_intron_with_1_base_left)
    {
      *processing_intron_with_1_base_left = False;
    }
    else
    {
      **referenceproteinptr = (Uchar) ' ';
      (*referenceproteinptr)++;
    }
    **referenceproteinptr = refseqorig[(*refseqindex)++];
    (*referenceproteinptr)++;
    **referenceproteinptr = (Uchar) ' ';
    (*referenceproteinptr)++;
  }
}

/*
  The following function is used to construct the genomic protein line of
  protein alignments.
*/

static Uint construct_reference_protein_line(Uchar *referenceproteinline,
#ifdef ASSERTIONS
                                             Uint lengthofreferenceproteinline,
#endif
                                             Uchar *refseqorig,
                                             Editoperation *alignment, 
                                             Uint lenalg,
                                             /*@unused@*/ 
                                             Uint showintronmaxlen)
{
  Editoperation *eopptr; 
  Eoptype eoptype;
  Uint eoplength, l,
       i = 0;
  Uchar *referenceproteinptr = referenceproteinline;
#ifdef ASSERTIONS
  Uchar *afterreferenceproteinline = referenceproteinline +
                                     lengthofreferenceproteinline;
#endif
  BOOL processing_intron_with_1_base_left  = False,
       processing_intron_with_2_bases_left = False;

  for(eopptr = alignment + lenalg - 1; eopptr >= alignment; eopptr--)
  {
    eoptype   = determine_eop_type(*eopptr,   True /* proteineop*/ );
    eoplength = determine_eop_length(*eopptr, True /* proteineop*/ );

    ASSERTION("we are not processing two intron types at the same time"
             , !(processing_intron_with_1_base_left &&
                 processing_intron_with_2_bases_left));

    switch(eoptype)
    {
      case EOP_TYPE_MISMATCH_WITH_1_GAP:
      case EOP_TYPE_MISMATCH_WITH_2_GAPS:
      case EOP_TYPE_INSERTION:
        ASSERTION("we are not processing with 1 base left here"
                 , !processing_intron_with_1_base_left);
        ASSERTION("we are not processing with 2 bases left here"
                 , !processing_intron_with_2_bases_left);
        /*@fallthrough@*/
      case EOP_TYPE_MISMATCH:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        match_mismatch_referenceproteinline(&referenceproteinptr,
                                           &processing_intron_with_1_base_left,
                                           &processing_intron_with_2_bases_left,
                                           refseqorig,
                                           &i);
        break;
      case EOP_TYPE_DELETION:
      case EOP_TYPE_DELETION_WITH_1_GAP:
      case EOP_TYPE_DELETION_WITH_2_GAPS:
        ASSERTION("editoperation has length 1", eoplength == UintConst(1));
        ASSERTION("we are not processing with 1 base left here"
                 , !processing_intron_with_1_base_left);
        ASSERTION("we are not processing with 2 bases left here"
                 , !processing_intron_with_2_bases_left);
        REFERENCEPROTEINPTR_STILL_VALID;
        *referenceproteinptr++ = (Uchar) ' ';
        REFERENCEPROTEINPTR_STILL_VALID;
        *referenceproteinptr++ = (Uchar) ABSTRACTGAPSYMBOL;
        REFERENCEPROTEINPTR_STILL_VALID;
        *referenceproteinptr++ = (Uchar) ' ';
        break;
      case EOP_TYPE_MATCH:
        for(l = 0; l < eoplength; l++)
        {
          match_mismatch_referenceproteinline(&referenceproteinptr,
                                           &processing_intron_with_1_base_left,
                                           &processing_intron_with_2_bases_left,
                                           refseqorig,
                                           &i);
        }
        break;
      case EOP_TYPE_INTRON_WITH_2_BASES_LEFT:
        if(!processing_intron_with_2_bases_left)
        {
          processing_intron_with_2_bases_left = True;
          REFERENCEPROTEINPTR_STILL_VALID;
          *referenceproteinptr++ = (Uchar) ' ';
          REFERENCEPROTEINPTR_STILL_VALID;
          *referenceproteinptr++ = refseqorig[i++];
        }
        // skip the next case statement and process this intron
        goto process_intron;
      case EOP_TYPE_INTRON_WITH_1_BASE_LEFT:
        if(!processing_intron_with_1_base_left)
        {
          processing_intron_with_1_base_left = True;
          REFERENCEPROTEINPTR_STILL_VALID;
          *referenceproteinptr++ = (Uchar) ' ';
        }
        /*@fallthrough@*/
      case EOP_TYPE_INTRON:
      process_intron:
        for(l = 0; l < eoplength; l++)
        {
          REFERENCEPROTEINPTR_STILL_VALID;
          *referenceproteinptr++ = (Uchar) ABSTRACTINTRONSYMBOL; 
        }
        break;
      DEFAULTFAILURE; 
    }
  }

  ASSERTION("we are not processing with 1 base left here"
           , !processing_intron_with_1_base_left);
  ASSERTION("we are not processing with 2 bases left here"
           , !processing_intron_with_2_bases_left);

  return (Uint) (referenceproteinptr - referenceproteinline);
}

static Uint filltheproteinlines(Uchar *genomicdnaline, 
                                Uchar *genomicproteinline, 
                                Uchar *referenceproteinline,
#ifdef ASSERTIONS
                                Uint lengthofgenomicdnaline,
                                Uint lengthofgenomicproteinline,
                                Uint lengthofreferenceproteinline,
#endif
                                Uchar *genseqorig,
                                /*@unused@*/ Uint genseqlen, 
                                Uchar *refseqorig,
                                /*@unused@*/ Uint refseqlen,
                                Editoperation *alignment,
                                Uint lenalg,
                                /*@unused@*/ Uint linewidth,
                                Uint showintronmaxlen, 
                                /*@unused@*/ ArrayShortintroninfo 
                                             *shortintroninfo,
                                Uint translationschemenumber)
{
  Uint genomicdnalinelen,
       genomicproteinlinelen,
       referenceproteinlinelen;

  // change: delete this and implement the showintronmaxlen feature for proteins
  showintronmaxlen = 0;

  // construct genomic DNA line of alignment
  genomicdnalinelen = construct_genomic_dna_line(genomicdnaline,
#ifdef ASSERTIONS
                                                 lengthofgenomicdnaline,
#endif
                                                 genseqorig,
                                                 alignment,
                                                 lenalg,
                                                 showintronmaxlen);

  // construct genomic protein line of alignment
  genomicproteinlinelen = 
    construct_genomic_protein_line(genomicproteinline,
#ifdef ASSERTIONS
                                   lengthofgenomicproteinline,
#endif
                                   genseqorig,
                                   alignment,
                                   lenalg,
                                   showintronmaxlen,
                                   translationschemenumber);

  // construct reference protein line of alignment
  referenceproteinlinelen = 
    construct_reference_protein_line(referenceproteinline,
#ifdef ASSERTIONS
                                     lengthofreferenceproteinline,
#endif
                                     refseqorig,
                                     alignment,
                                     lenalg,
                                     showintronmaxlen);

  DEBUG2(2, "%s(): genomic dna line length=%lu\n", FUNCTIONNAME
        , (Showuint) (genomicdnalinelen));
  DEBUG2(2, "%s(): genomic protein line length=%lu\n", FUNCTIONNAME
        , (Showuint) (genomicproteinlinelen));
  DEBUG2(2, "%s(): reference protein line length=%lu\n", FUNCTIONNAME
        , (Showuint) (referenceproteinlinelen));

  // all three lines have the same length
  if (genomicdnalinelen != genomicproteinlinelen)
  {
    fprintf(stderr,"line 1 and line 2 do not have the same length\n");
    exit(EXIT_FAILURE);
  }
  if (genomicproteinlinelen != referenceproteinlinelen)
  {
    fprintf(stderr,"line 2 and line 3 have the same length\n");
    exit(EXIT_FAILURE);
  }
  return referenceproteinlinelen;
}

static Uchar wildcardimplosion(Uchar c, Uint flag, Alphabet *alpha)
{
  if((flag & SHOWALIGNMENTWILDCARDIMPLOSION) && 
     (alpha->symbolmap[c] == (Uint) WILDCARD))
  {
    if(islower(c))
    {
      return (Uchar) tolower(alpha->characters[alpha->mapsize - 1]);
    }
    else
    {
      return (Uchar) toupper(alpha->characters[alpha->mapsize - 1]);
    }
  }
  else
  {
    return c;
  }
}


/*
  The following function formats a sequence \texttt{s} of length \texttt{len}.
  Each character code is shown as the corresponding character w.r.t.
  the given alphabet. Each abstract gap symbol is shown as a 
  \texttt{\symbol{34}-\symbol{34}}. The output goes to the file pointer
  \texttt{outfp}.
*/

static void formatseqwithgaps(GENFILE *outfp,Uchar *sorig,Uint len, 
                              Uint flag, Uint *insertioncount,
                              BOOL countproteininsertions, Alphabet *alpha)
{
  Uint i;

  for(i=0; i<len; i++)
  {
    if((flag & SHOWALIGNMENTTENNERBLOCKS) && (i != 0) && (i%10 == 0))
    {
      OUTCHAR(' ');
    }
    if(sorig[i] == (Uchar) ABSTRACTGAPSYMBOL)
    {
      OUTCHAR(CONCRETEGAPSYMBOL);
      if(insertioncount != NULL)
      {
        if(countproteininsertions)
        {
          *insertioncount += CODONLENGTH;
        }
        else
        {
          (*insertioncount)++;
        }
      }
    } 
    else if(sorig[i] == (Uchar) ABSTRACTINTRONSYMBOL)
    {
      OUTCHAR(CONCRETEINTRONSYMBOL);
      if(insertioncount != NULL)
      {
        (*insertioncount)++;
      }
    }
    else
    {
      if(flag & SHOWALIGNMENTFORCEUPPER)
      {
        (void) genputc((Fputcfirstargtype) 
                       wildcardimplosion((Uchar) toupper(sorig[i]),flag,alpha), 
                       outfp);
      }
      else if(flag & SHOWALIGNMENTFORCELOWER)
      {
        (void) genputc((Fputcfirstargtype) 
                       wildcardimplosion((Uchar) tolower(sorig[i]),flag,alpha), 
                       outfp);
      }
      else
      {
        (void) genputc((Fputcfirstargtype) 
                       wildcardimplosion(sorig[i],flag,alpha), outfp);
      }
    }
  }
}

static void fancyformatseqwithgaps(GENFILE *outfp,Uchar *scompare,
                                   Uchar *tcompare,Uchar *sorig,Uint len)
{
  Uint i;
  Uchar a, b;
  BOOL errormode = False;

  OUTCHAR(NORMALCHAR);
  for(i=0; i<len; i++)
  {
    a = scompare[i];
    b = tcompare[i];
    if(a != b || a == (Uchar) ABSTRACTGAPSYMBOL || a == (Uchar) WILDCARD)
    {
      if(!errormode)
      {
        OUTCHAR(ERRORCHAR);
        errormode = True;
      }
    } else
    {
      if(errormode)
      {
        OUTCHAR(NORMALCHAR);
        errormode = False;
      }
    }
    if(a == (Uchar) ABSTRACTGAPSYMBOL)
    {
      OUTCHAR(CONCRETEGAPSYMBOL);
    } 
    else if(sorig[i] == (Uchar) ABSTRACTINTRONSYMBOL)
    {
      OUTCHAR(CONCRETEINTRONSYMBOL);
    }
    else
    {
      OUTCHAR(sorig[i]);
    }
  }
}

/*
  For the alignment lines \texttt{firstlinecompare} and 
  \texttt{secondlinecompare}, both of length \texttt{len}, the following 
  function shows a line 
  with the symbol \texttt{\symbol{34}\symbol{33}\symbol{34}} in each column 
  corresponding to a mismatch or an indel. The output goes to the file 
  pointer \texttt{outfp}.
*/

#define CHECKORIGEQUAL(OKAY,A,B)\
        if((A) == (B))\
        {\
          OKAY = True;\
        } else\
        {\
          if(islower((Ctypeargumenttype) (A)))\
          { \
            OKAY = ((A) == (Uchar) tolower(B)) ? True : False;\
          } else\
          {\
            OKAY = ((A) == (Uchar) toupper((B))) ? True : False;\
          }\
        }

static void showeditopline(Uint flag, GENFILE *outfp,
                           Uchar *firstlinecompare,
                           Uchar *secondlinecompare,
                           Uchar *firstlineorig,
                           Uchar *secondlineorig,
                           Uint len)
{
  Uint i;
  Uchar acompare, bcompare, aorig, borig;
  BOOL charequal, charinline = False;
  BOOL equal = (flag & SHOWALIGNMENTEQUAL) ? True : False;

  for(i=0; i<len; i++)
  {
    acompare = firstlinecompare[i];
    bcompare = secondlinecompare[i];
    if(equal)
    {
      if(acompare == bcompare && acompare != (Uchar) ABSTRACTGAPSYMBOL 
                              && acompare != (Uchar) WILDCARD)
      {
        charinline = True;
        break;
      }
    } else
    {
      if(acompare != bcompare || acompare == (Uchar) ABSTRACTGAPSYMBOL 
                              || acompare == (Uchar) WILDCARD)
      {
        charinline = True;
        break;
      }
    }
    aorig = firstlineorig[i];
    borig = secondlineorig[i];
    CHECKORIGEQUAL(charequal,aorig,borig);
    if(equal)
    {
      if(!charequal)
      {
        charinline = True;
        break;
      }
    } else
    {
      if(!charequal)
      {
        charinline = True;
        break;
      }
    }
  }
  if(charinline)
  {
    OUTEMPTY;
    for(i=0; i<len; i++)
    {
      if((flag & SHOWALIGNMENTTENNERBLOCKS) && (i != 0) && (i%10 == 0)) 
      {
        OUTCHAR(' ');
      }

      acompare = firstlinecompare[i];
      bcompare = secondlinecompare[i];
      if(equal)
      {
        if(acompare == bcompare && acompare != (Uchar) ABSTRACTGAPSYMBOL 
                                && acompare != (Uchar) WILDCARD)
        {
          OUTCHAR('|');
        } else
        {
          aorig = firstlineorig[i];
          borig = secondlineorig[i];
          CHECKORIGEQUAL(charequal,aorig,borig);
          if(charequal)
          {
            OUTCHAR('=');
          } else
          {
            OUTCHAR(' ');
          }
        }
      } else
      {
        if(acompare != bcompare || acompare == (Uchar) ABSTRACTGAPSYMBOL 
                                || acompare == (Uchar) WILDCARD)
        {
          OUTCHAR('!');
        } else
        {
          aorig = firstlineorig[i];
          borig = secondlineorig[i];
          CHECKORIGEQUAL(charequal,aorig,borig);
          if(charequal)
          {
            OUTCHAR(' ');
          } else
          {
            OUTCHAR('=');
          }
        }
      }
    }
    OUTCHAR('\n');
  }
}

static void showeditoplineprotein(/*@unused@*/ Uint flag, GENFILE *outfp,
                                  Uchar *genomicproteinline,
                                  Uchar *referenceproteinline,
                                  Uint len, Scorematrix *scorematrix,
                                  Alphabet *scorematrixalphabet)
{
  Uint i;
  Uchar genchar, refchar;
  SCORE score;

  for(i = 0; i < len; i++)
  {
    genchar = genomicproteinline[i];
    refchar = referenceproteinline[i];

    ASSERTION("an empty refchar implies an emtpy genchar"
             , !(refchar == ' ' && genchar != ' '));

    // output blank if necessary
    if((flag & SHOWALIGNMENTTENNERBLOCKS) && (i != 0) && (i%10 == 0)) 
    {
      OUTCHAR(' ');
    }

    // handle the ``empty case''
    if(genchar == ' ')
    {
      OUTCHAR(' ');
    }
    else if(genchar == refchar)
    {
      OUTCHAR(EQUAL_AMINO_ACID_CHAR);
    } 
    else
    {
      // output character depending on amino acid substitution score of the two
      // characters
      // change: handle output of stop codons appropriately
      DEBUG3(3, "%s(): genchar=%c, refchar=%c\n", FUNCTIONNAME, genchar
            , refchar);

      // determine score
      score = GETSCORE(scorematrix
                      , scorematrixalphabet->symbolmap[genchar]
                      , scorematrixalphabet->symbolmap[refchar]);
      // output corresponding character depending on score
      if(score > 0)
      {
        OUTCHAR(POSITIVE_SCORE_CHAR);
      }
      else if(score == 0)
      {
        OUTCHAR(ZERO_SCORE_CHAR);
      }
      else // score < 0
      {
        OUTCHAR(NEGATIVE_SCORE_CHAR); 
      }
    }
  }
}

/*
  The following function formats an alignment given by the
  \texttt{firstlinecompare} and the \texttt{secondlinecompare}. 
  \texttt{numofcols} is the number of columns in the alignment.
  \texttt{linewidth} is the width according to which the alignment
  is formatted. \texttt{startfirst} and \texttt{startsecond} are
  the starting positions of the aligned sequences.
  The output goes to the file pointer \texttt{outfp}.
*/

static void formatalignment(GENFILE *outfp,Uint flag,
                            Uchar *firstlinecompare,Uchar *secondlinecompare,
                            Uchar *firstlineorig,Uchar *secondlineorig,
                            Uint numofcols,Uint linewidth,
                            Uint startfirst,Uint startsecond, Uint totalulen,
                            Uint totalvlen, Alphabet *alpha, 
                            ArrayShortintroninfo *shortintroninfo)
{
  Uint len, i = 0;
  Uint tennerblocksadjustment = 0,
       firstinsertioncount    = 0, 
       secondinsertioncount   = 0,
       currentshortintroninfo = 0,
       completeshortintronlen = 0;
  Sint j, numofblanks;
  Uint shortintronstart,
       shortintronend,
       shortintronlength;

  while(True)
  {
    OUTSUBJECT;
 
    if(flag & SHOWALIGNMENTTENNERBLOCKS)
    {
      if((numofcols - i) < linewidth)
      {
        len = numofcols -i;
        tennerblocksadjustment = (linewidth - len - 1)/10;
      }
      else
      {
        len = linewidth; 
      }
    }
    else
    {
      len = MIN(numofcols - i,linewidth);
    }
    formatseqwithgaps(outfp,firstlineorig+i,len,flag,&firstinsertioncount,
                      False,alpha);
    if(flag & SHOWALIGNMENTREVERSESUBJECTPOS)
    {
      OUTSTRINGNUM(NUMWIDTH+linewidth-len+tennerblocksadjustment, 
                   totalulen + 1 - (i+startfirst+len-firstinsertioncount+
                                    completeshortintronlen));
    }
    else
    {
      OUTSTRINGNUM(NUMWIDTH+linewidth-len+tennerblocksadjustment, 
                   i+startfirst+len-firstinsertioncount+completeshortintronlen);
    }
    OUTCHAR('\n');
    showeditopline(flag,outfp,firstlinecompare+i,secondlinecompare+i,
                   firstlineorig+i,secondlineorig+i,len);
    if(flag & SHOWALIGNMENTSELFCOMPARISON)
    {
      OUTSUBJECT;
    } else
    {
      OUTQUERY;
    }
    formatseqwithgaps(outfp,secondlineorig+i,len,flag,&secondinsertioncount,
                      False,alpha);

    if((!(flag & SHOWALIGNMENTSELFCOMPARISON) && 
         (flag & SHOWALIGNMENTREVERSEQUERYPOS)) ||
        ((flag & SHOWALIGNMENTSELFCOMPARISON) &&
         (flag & SHOWALIGNMENTREVERSESUBJECTPOS)))
   {
    OUTSTRINGNUM(NUMWIDTH+linewidth-len+tennerblocksadjustment, 
                 totalvlen + 1 - (i+startsecond+len-secondinsertioncount));
   }
   else
   {
    OUTSTRINGNUM(NUMWIDTH+linewidth-len+tennerblocksadjustment, 
                 i+startsecond+len-secondinsertioncount);
   }

    OUTCHAR('\n');
   
    if(flag & SHOWALIGNMENTNOSUBJECTANDQUERY)
    {
      OUTCHAR('\n');
    }

    i += len;
    if(i >= numofcols)
    {
      break;
    }
    OUTCHAR('\n');

    // take care of short introns, if some remain
    if(currentshortintroninfo < shortintroninfo->nextfreeShortintroninfo)
    {
      ASSERTION("current short intron starts after or at current position" 
               , shortintroninfo->spaceShortintroninfo[currentshortintroninfo]
                 .start >= i + completeshortintronlen);
      if(shortintroninfo->spaceShortintroninfo[currentshortintroninfo].start 
         == i + completeshortintronlen)
      {
        // output short intron
        shortintronstart = (flag & SHOWALIGNMENTREVERSESUBJECTPOS) 
                           ? totalulen + 1 - 
                             (i+startfirst-firstinsertioncount+
                              completeshortintronlen+1)
                           : i+startfirst-firstinsertioncount +
                             completeshortintronlen+1;
        shortintronend   = (flag & SHOWALIGNMENTREVERSESUBJECTPOS) 
                           ? totalulen + 1 - 
                             (i+startfirst-firstinsertioncount+
                              completeshortintronlen+
                              shortintroninfo->spaceShortintroninfo
                              [currentshortintroninfo].length)
                           : i+startfirst-firstinsertioncount+
                             completeshortintronlen+
                             shortintroninfo->spaceShortintroninfo
                             [currentshortintroninfo].length;
        shortintronlength = shortintroninfo->spaceShortintroninfo
                            [currentshortintroninfo].length;

        numofblanks  = (Sint) linewidth - 33;
        numofblanks -= floor(log10((double) shortintronstart))+1;
        numofblanks -= floor(log10((double) shortintronend))+1;
        numofblanks -= floor(log10((double) shortintronlength))+1; 
        if(flag & SHOWALIGNMENTTENNERBLOCKS)
        {
          numofblanks += (linewidth / 10 ) - 1;
        }

        genprintf(outfp, "// intron part %lu %lu (%lu n) not shown"
                 , (Showuint) shortintronstart
                 , (Showuint) shortintronend 
                 , (Showuint) shortintronlength);

        for(j = 0; j < numofblanks; j++)
        {
          OUTCHAR(' ');
        }

        genprintf(outfp, "//\n\n");
        genprintf(outfp, "//");
        for(j = SintConst(2); j < (Sint) linewidth - 2; j++)
        {
          if((flag & SHOWALIGNMENTTENNERBLOCKS) && (j != 0) && (j%10 == 0))
          {
            OUTCHAR(' ');
          }
          OUTCHAR(CONCRETEINTRONSYMBOL);
        }
        genprintf(outfp, "//\n\n\n");

        // update short intron adjustment
        completeshortintronlen += shortintroninfo->spaceShortintroninfo
                                 [currentshortintroninfo].length;
        // increase current intron
        currentshortintroninfo++;
      }
    }
  } 
  OUTCHAR('\n');
}

/*
  The following function formats an alignment given by the
  \texttt{firstlinecompare} and the \texttt{secondlinecompare} using 
  a fancy output format including some special characters denoting 
  formats. These special characters can later be interpreted in 
  a special way. \texttt{numofcols} is the number of columns in the alignment.
  \texttt{linewidth} is the width according to which the alignment
  is formatted. \texttt{startfirst} and \texttt{startsecond} are
  the starting positions of the aligned sequences.
  The output goes to the file pointer \texttt{outfp}.
*/

static void fancyformatalignment(GENFILE *outfp,Uint flag,
                                 Uchar *firstlinecompare,
                                 Uchar *secondlinecompare,
                                 Uchar *firstlineorig,
                                 Uchar *secondlineorig,
                                 Uint numofcols,
                                 Uint linewidth,Uint startfirst,
                                 Uint startsecond)
{
  Uint len, i = 0;

  while(True)
  {
    OUTCHAR(HEADERCHAR);
    OUTSUBJECT;
    len = MIN(numofcols - i,linewidth);
    fancyformatseqwithgaps(outfp,firstlinecompare+i,secondlinecompare+i,
                                 firstlineorig+i,len);
    OUTCHAR(NUMBERCHAR);
    OUTSTRINGNUM(NUMWIDTH+linewidth-len,i+startfirst+len);
    OUTCHAR(NORMALCHAR);
    OUTCHAR('\n');
    OUTCHAR(HEADERCHAR);
    if(flag & SHOWALIGNMENTSELFCOMPARISON)
    {
      OUTSUBJECT;
    } else
    {
      OUTQUERY;
    }
    fancyformatseqwithgaps(outfp,secondlinecompare+i,firstlinecompare+i,
                                 secondlineorig+i,len);
    OUTCHAR(NUMBERCHAR);
    OUTSTRINGNUM(NUMWIDTH+linewidth-len,i+startsecond+len);
    OUTCHAR(NORMALCHAR);
    OUTCHAR('\n');
    i += len;
    if(i >= numofcols)
    {
      break;
    }
    OUTCHAR('\n');
  } 
}

static void formatproteinalignment(GENFILE *outfp, 
                                   Uint flag,
                                   Uchar *genomicdnaline, 
                                   Uchar *genomicproteinline, 
                                   Uchar *referenceproteinline, 
                                   Uint numofcols,
                                   Uint linewidth, 
                                   Uint startfirst, 
                                   Uint startsecond,
                                   Uint totalulen, 
                                   Alphabet *alpha,
                                   Scorematrix *scorematrix,
                                   Alphabet *scorematrixalphabet)
{
  Uint len, codon_remainder, i = 0,
       tennerblocksadjustment = 0,
       genomicdnainsertioncount = 0,
       referenceproteininsertioncount = 0,
       completeshortintronlen = 0;

  DEBUG2(2, "%s(): numofcols=%lu\n", FUNCTIONNAME, (Showuint) numofcols);

  while(True)
  {
    OUTSUBJECT;

    if(flag & SHOWALIGNMENTTENNERBLOCKS)
    {
      if((numofcols - i) < linewidth)
      {
        len = numofcols - i;
        tennerblocksadjustment = (linewidth - len - 1)/10;
      }
      else
      {
        len = linewidth;
      }
    }
    else
    {
      len = MIN(numofcols - i,linewidth);
    }

    formatseqwithgaps(outfp, genomicdnaline + i, len, flag,
                      &genomicdnainsertioncount, False, alpha);
    if(flag & SHOWALIGNMENTREVERSESUBJECTPOS)
    {
      OUTSTRINGNUM(NUMWIDTH+linewidth-len+tennerblocksadjustment, 
                   totalulen + 1 - (i+startfirst+len-genomicdnainsertioncount+
                                    completeshortintronlen));
    }
    else
    {
      OUTSTRINGNUM(NUMWIDTH+linewidth-len+tennerblocksadjustment, 
                   i+startfirst+len-genomicdnainsertioncount+
                   completeshortintronlen);
    }
    OUTCHAR('\n');
    formatseqwithgaps(outfp, genomicproteinline + i, len, flag, NULL, False,
                      alpha);
    OUTCHAR('\n');
    showeditoplineprotein(flag, outfp, genomicproteinline + i,
                          referenceproteinline + i, len, scorematrix,
                          scorematrixalphabet);
    OUTCHAR('\n');
    formatseqwithgaps(outfp, referenceproteinline + i, len, flag,
                      &referenceproteininsertioncount, True, alpha);

    // this is necessary for a correct output of protein positions
    if((i+len-referenceproteininsertioncount) % CODONLENGTH ==
        UintConst(2))
    {
      codon_remainder = UintConst(1);
    }
    else 
    {
      codon_remainder = 0;
    }
    OUTSTRINGNUM(NUMWIDTH+linewidth-len+tennerblocksadjustment, 
                 startsecond+
                 ((i+len-referenceproteininsertioncount) / CODONLENGTH)+
                 codon_remainder);
    OUTCHAR('\n');

    if(flag & SHOWALIGNMENTNOSUBJECTANDQUERY)
    {
      OUTCHAR('\n');
    }

    i += len;
    if(i >= numofcols)
    {
      break;
    }
    OUTCHAR('\n');
  }
  OUTCHAR('\n');
}

Uint fillthethreealignmentlines(Uchar **firstline,
                                Uchar **secondline,
                                Uchar **thirdline,
                                Uint linewidth,
                                Editoperation *alignment,
                                Uint lenalg,
                                ArrayShortintroninfo *shortintroninfo,
                                Uint indelcount,
                                Uchar *genseqorig,
                                Uint genseqlen,
                                Uchar *refseqorig,
                                Uint refseqlen,
                                Uint showintronmaxlen,
                                Uint translationschemenumber)
{
  Uint lengthofgenomicdnaline,
       lengthofgenomicproteinline,
       lengthofreferenceproteinline;

  // set the lenght of the three output lines
  lengthofgenomicdnaline       = genseqlen + indelcount;
  lengthofgenomicproteinline   = genseqlen + indelcount;
  lengthofreferenceproteinline = refseqlen * CODONLENGTH + indelcount;

  // alloc space for the three output lines
  ALLOCASSIGNSPACE(*firstline, NULL, Uchar, lengthofgenomicdnaline);
  ALLOCASSIGNSPACE(*secondline, NULL, Uchar, lengthofgenomicproteinline);
  ALLOCASSIGNSPACE(*thirdline, NULL, Uchar, lengthofreferenceproteinline);

  // fill the output lines
  return filltheproteinlines(*firstline
                            , *secondline
                            , *thirdline
#ifdef ASSERTIONS
                            , lengthofgenomicdnaline
                            , lengthofgenomicproteinline
                            , lengthofreferenceproteinline
#endif
                            , genseqorig
                            , genseqlen
                            , refseqorig
                            , refseqlen
                            , alignment
                            , lenalg
                            , linewidth
                            , showintronmaxlen
                            , shortintroninfo
                            , translationschemenumber);
}


void showalignmentprotein(Uint flag, 
                          GENFILE *outfp, 
                          Uint linewidth,
                          Editoperation *alignment, 
                          Uint lenalg,
                          Uint indelcount, 
                          /*@unused@*/ Uchar *genseqcompare,
                          Uchar *genseqorig, 
                          Uint genseqlen, 
                          /*@unused@*/ Uchar *refseqcompare,
                          Uchar *refseqorig, 
                          Uint refseqlen, 
                          Uint startfirst, 
                          Uint startsecond, 
                          Uint totalulen, 
                          Uint showintronmaxlen, 
                          Alphabet *alpha,
                          Uint translationschemenumber,
                          Scorematrix *scorematrix,
                          Alphabet *scorematrixalphabet)
{
  Uint numofcols;
  Uchar *genomicdnaline,
        *genomicproteinline,
        *referenceproteinline;
  ArrayShortintroninfo shortintroninfo;
 
  // init
  INITARRAY(&shortintroninfo, Shortintroninfo);

  numofcols = fillthethreealignmentlines(&genomicdnaline
                                        , &genomicproteinline
                                        , &referenceproteinline
                                        , linewidth
                                        , alignment
                                        , lenalg
                                        , &shortintroninfo
                                        , indelcount
                                        , genseqorig
                                        , genseqlen
                                        , refseqorig
                                        , refseqlen
                                        , showintronmaxlen
                                        , translationschemenumber);

  // output the three lines in a formated fashion
  formatproteinalignment(outfp, flag, genomicdnaline, genomicproteinline,
                         referenceproteinline, numofcols, linewidth, startfirst,
                         startsecond, totalulen, alpha, scorematrix,
                         scorematrixalphabet);

  // free
  FREEARRAY(&shortintroninfo, Shortintroninfo);
  FREESPACE(genomicdnaline);
  FREESPACE(genomicproteinline);
  FREESPACE(referenceproteinline);
}

Uint fillthetwoalignmentlines(BOOL forward,
                              Uchar **firstline, 
                              Uchar **secondline,
                              Uchar *useq,
                              Uint ulen, 
                              Uchar *vseq,
                              Uint vlen,
                              Editoperation *alignment,
                              Uint lenalg, 
                              Uint linewidth,
                              Uint showintronmaxlen, 
                              ArrayShortintroninfo *shortintroninfo,
                              Uint indelcount)
{
  ALLOCASSIGNSPACE(*firstline, NULL, Uchar, ulen + indelcount);
  ALLOCASSIGNSPACE(*secondline, NULL, Uchar, vlen + indelcount);

  return fillthelines(forward, *firstline, *secondline, useq, ulen, vseq, vlen,
                      alignment, lenalg, linewidth, showintronmaxlen,
                      shortintroninfo);
}

/*EE
  The following function shows the \texttt{alignment} on 
  the file pointer \texttt{outfp}. \texttt{lenalg} is the length
  of the alignment and \texttt{indelcount} is an upper bound on the number 
  of insertions and deletions. If you do not exactly know this number then
  choose a value which is larger, e.g.\ MAX(ulen,vlen). The aligned 
  sequences are \texttt{useqcompare} and \texttt{vseqcompare} of 
  length \texttt{ulen} and \texttt{vlen}. The pointers 
  \texttt{useqorig} and \texttt{vseqorig} refer to the original versions
  of the sequences.
  \texttt{startfirst} and \texttt{startsecond} are
  the starting positions of the aligned sequences.
  \texttt{linewidth} is the width the alignment is formatted for,
  The bit vector \texttt{flag} can hold the following bits:
  \begin{itemize}
  \item
  \texttt{SHOWALIGNMENTFORWARD} is \texttt{True} iff the alignment to 
  show is for the sequence in forward direction.
  \item
  \texttt{SHOWALIGNMENTFANCY} is \texttt{True} iff the alignment is to 
  be shown in fancy mode. This is mainly used in the graphic user 
  interface.
  \item
  \texttt{SHOWALIGNMENTSELFCOMPARISON} is \texttt{True} iff the 
  alignment derived from a self-comparison.
  \item
  \texttt{SHOWALIGNMENTEQUAL} is \texttt{True} iff equal characters 
  are reported between the matching sequences rather than unequal.
  \item
  \texttt{SHOWALIGNMENTNOSUBJECTANDQUERY} is \texttt{True} iff no
  subject and query identifiers are shown before the aligned sequences.
  \item
  \texttt{SHOWALIGNMENTTENNERBLOCKS} is \texttt{True} iff the aligned sequeces
  are divided into blocks of ten bases. That is, every ten bases a blank
  is inserted into both sequences.
  \item
  \texttt{SHOWALIGNMENTFORCEUPPER} is \texttt{True} iff the characters 
  of both aligned sequences are shown as upper case characters.
  \item
  \texttt{SHOWALIGNMENTFORCELOWER} is \texttt{True} iff the characters 
  of both aligned sequences are shown as lower case characters.
  \item
  \texttt{SHOWALIGNMENTREVERSESUBJECTPOS} is \texttt{True} iff the positions
  of the reverse complement strand of the subject sequence are shown.
  The total length of the subject sequence is given by the variable
  \texttt{totalulen}.
  \item
  \texttt{SHOWALIGNMENTREVERSEQUERYPOS} is \texttt{True} iff the positions
  of the reverse complement strand of the query sequence are shown.
  The total length of the query sequence is given by the variable
  \texttt{totalvlen}.
  \item   
  \texttt{SHOWALIGNMENTWILDCARDIMPLOSION} is \texttt{True} iff all wildcard
  characters are replaced by the first wildcard character. The corresponding
  alphabet is given by the argument \texttt{alpha}.
  E.g., if \texttt{alpha} contains the wildcards N, S, Y, W, R, K, V, B, D, H, 
  and M, they will all be shown as N.
  \end{itemize}
  If the argument \texttt{showintronmaxlen} is set to 0, introns are showed
  completely. Otherwise introns larger than \texttt{showintronmaxlen} are only
  shown partially.
*/

void showalignmentgeneric(Uint flag,
                          GENFILE *outfp,
                          Uint linewidth,
                          Editoperation *alignment,
                          Uint lenalg,
                          Uint indelcount,
                          Uchar *useqcompare, 
                          Uchar *useqorig,
                          Uint ulen,
                          Uchar *vseqcompare, 
                          Uchar *vseqorig,
                          Uint vlen,
                          Uint startfirst, 
                          Uint startsecond,
                          Uint totalulen, 
                          Uint totalvlen,
                          Uint showintronmaxlen,
                          Alphabet *alpha)
{
  Uchar *firstlinecompare, *secondlinecompare, *firstlineorig, *secondlineorig;
  Uint numofcolscompare, numofcolsorig;
  BOOL forward = (flag & SHOWALIGNMENTFORWARD) ? True : False,
       fancy = (flag & SHOWALIGNMENTFANCY) ? True : False;
  ArrayShortintroninfo shortintroninfo;

  if((flag & SHOWALIGNMENTFORCEUPPER) && (flag & SHOWALIGNMENTFORCELOWER))
  {
    fprintf(stderr, "flags SHOWALIGNMENTFORCEUPPER and SHOWALIGNMENTFORCELOWER "
                    "cannot be used together\n");
    exit(EXIT_FAILURE);
  }

  // some checks to make sure that fancy is not used in conjunction with flags
  // which are not supported by the function for fancy formatting 
  if(fancy && (flag & SHOWALIGNMENTNOSUBJECTANDQUERY))
  {
    fprintf(stderr, "it is not possible to set fancy to True and use the flag"
                    "SHOWALIGNMENTNOSUBJECTANDQUERY at the same time\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && (flag &SHOWALIGNMENTTENNERBLOCKS))
  {
    fprintf(stderr, "it is not possible to set fancy to True and use the flag"
                    "SHOWALIGNMENTTENNERBLOCKS at the same time\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && (flag & SHOWALIGNMENTFORCEUPPER))
  {
    fprintf(stderr, "it is not possible to set fancy to True and use the flag"
                    "SHOWALIGNMENTFORCEUPPER at the same time\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && (flag & SHOWALIGNMENTFORCELOWER))
  {
    fprintf(stderr, "it is not possible to set fancy to True and use the flag"
                    "SHOWALIGNMENTFORCELOWER at the same time\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && (flag & SHOWALIGNMENTREVERSESUBJECTPOS))
  {
    fprintf(stderr, "it is not possible to set fancy to True and use the flag"
                    "SHOWALIGNMENTREVERSESUBJECTPOS at the same time\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && (flag & SHOWALIGNMENTREVERSEQUERYPOS))
  {
    fprintf(stderr, "it is not possible to set fancy to True and use the flag"
                    "SHOWALIGNMENTREVERSEQUERYPOS at the same time\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && (flag & SHOWALIGNMENTWILDCARDIMPLOSION))
  {
    fprintf(stderr, "it is not possible to set fancy to True and use the flag"
                    "SHOWALIGNMENTWILDCARDIMPLOSION at the same time\n");
    exit(EXIT_FAILURE);
  }
  if(fancy && showintronmaxlen)
  {
    fprintf(stderr, "it is not possible to set fancy to True and "
                    "showintronmaxlen to a value larger than 0\n");
    exit(EXIT_FAILURE);
  }

  INITARRAY(&shortintroninfo, Shortintroninfo);

  DEBUG1(2,"showalignmentgeneric of length %lu\n",(Showuint) lenalg);

  numofcolscompare = fillthetwoalignmentlines(forward,
                                              &firstlinecompare, 
                                              &secondlinecompare,
                                              useqcompare, ulen,
                                              vseqcompare, vlen,
                                              alignment, lenalg,
                                              linewidth, showintronmaxlen,
                                              &shortintroninfo, indelcount);
  numofcolsorig    = fillthetwoalignmentlines(forward,
                                              &firstlineorig,
                                              &secondlineorig,
                                              useqorig, ulen,
                                              vseqorig, vlen,
                                              alignment, lenalg,
                                              linewidth, showintronmaxlen,
                                              NULL, indelcount);
  if (numofcolscompare != numofcolsorig)
  {
    fprintf(stderr,"numofcols are not equal\n");
    exit(EXIT_FAILURE);
  }

  if(!forward)
  {
    reverseinplace(firstlinecompare,numofcolscompare);
    reverseinplace(secondlinecompare,numofcolscompare);
    reverseinplace(firstlineorig,numofcolscompare);
    reverseinplace(secondlineorig,numofcolscompare);
  }
  if(fancy)
  {
    fancyformatalignment(outfp,flag,
                         firstlinecompare,secondlinecompare,
                         firstlineorig,secondlineorig,
                         numofcolscompare,linewidth,startfirst,startsecond);
  } else
  {
    formatalignment(outfp,flag,
                    firstlinecompare,secondlinecompare,
                    firstlineorig,secondlineorig,
                    numofcolscompare,linewidth,startfirst,startsecond,
                    totalulen, totalvlen, alpha, &shortintroninfo);
  }
  FREESPACE(firstlinecompare);
  FREESPACE(secondlinecompare);
  FREESPACE(firstlineorig);
  FREESPACE(secondlineorig);
  FREEARRAY(&shortintroninfo, Shortintroninfo);
}

/*
  The following function has the same functionality as the showalignmentgeneric
  function except for the flags \texttt{SHOWALIGNMENTREVERSESUBJECTPOS},
  \texttt{SHOWALIGNMENTREVERSEQUERYPOS}, and 
  \texttt{SHOWALIGNMENTWILDCARDIMPLOSION}.
  This flags cannot be used with this function. The generic function has
  to be used in such cases.
*/

void showalignment(Uint flag,
                   FILE *outfp,
                   Uint linewidth,
                   Editoperation *alignment,
                   Uint lenalg,
                   Uint indelcount,
                   Uchar *useqcompare,
                   Uchar *useqorig,
                   Uint ulen,
                   Uchar *vseqcompare,
                   Uchar *vseqorig,
                   Uint vlen,
                   Uint startfirst,
                   Uint startsecond)
{
  GENFILE genfile;

  if(flag & SHOWALIGNMENTREVERSESUBJECTPOS)
  {
    fprintf(stderr, "the flag SHOWALIGNMENTREVERSESUBJECTPOS cannot be used"
                    "with the showalignment function\n");
    fprintf(stderr, "use the showalignmentgeneric function instead\n");
    exit(EXIT_FAILURE);
  }
  if(flag & SHOWALIGNMENTREVERSEQUERYPOS)
  {
    fprintf(stderr, "the flag SHOWALIGNMENTREVERSEQUERYPOS cannot be used"
                    "with the showalignment function\n");
    fprintf(stderr, "use the showalignmentgeneric function instead\n");
    exit(EXIT_FAILURE);
  }
  if(flag & SHOWALIGNMENTWILDCARDIMPLOSION)
  {
    fprintf(stderr, "the flag SHOWALIGNMENTWILDCARDIMPLOSION cannot be used"
                    "with the showalignment function\n");
    fprintf(stderr, "use the showalignmentgeneric function instead\n");
    exit(EXIT_FAILURE);
  }

  genfile.genfilemode  = STANDARD;
  genfile.fileptr.file = outfp;

  showalignmentgeneric(flag, 
                       &genfile, 
                       linewidth, 
                       alignment, 
                       lenalg, 
                       indelcount,
                       useqcompare, 
                       useqorig, 
                       ulen, 
                       vseqcompare, 
                       vseqorig,
                       vlen,
                       startfirst, 
                       startsecond, 
                       0, 
                       0,
                       0,
                       NULL);
}

/*EE
  The following function displays a alignment by applying the
  different functions in \texttt{fancydisplay} to the parts
  of the \texttt{alignment} substring marked accordingly.
*/

void fancyformatting(Fancydisplay *fancydisplay,Uchar *alignment,Uint len)
{
  Uchar *aptr, newcharmode, charmode = POSITIONCHAR;
  Uint showlen = 0;

  for(aptr = alignment; aptr < alignment + len; aptr++)
  {
    if(*aptr >= POSITIONCHAR)
    {
      if(*aptr > NORMALCHAR)  // larger than largest character
      {
        fprintf(stderr,"char %lu not allowed in alignment\n",
                (Showuint) *aptr);
        exit(EXIT_FAILURE);
      }
      newcharmode = *aptr;
      *aptr = (Uchar) '\0';
      if(showlen > 0)
      {
        switch(charmode)
        {
	   case POSITIONCHAR: 
                DEBUG1(2,"POSITIONCHAR \"%s\"\n",(char *) (aptr - showlen));
                fancydisplay->positionchar(fancydisplay->info,
                                           (char *) (aptr-showlen),
                                           (Sint) showlen);
                break;
	   case NUMBERCHAR: 
                DEBUG1(2,"NUMBERCHAR \"%s\"\n",(char *) (aptr - showlen));
                fancydisplay->numberchar(fancydisplay->info,
                                         (char *) (aptr-showlen),
                                         (Sint) showlen);
                break;
	   case HEADERCHAR: 
                DEBUG1(2,"HEADERCHAR \"%s\"\n",(char *) (aptr - showlen));
                fancydisplay->headerchar(fancydisplay->info,
                                         (char *) (aptr-showlen),
                                         (Sint) showlen);
                break;
	   case ERRORCHAR: 
                DEBUG1(2,"ERRORCHAR \"%s\"\n",(char *) (aptr - showlen));
                fancydisplay->errorchar(fancydisplay->info,
                                        (char *) (aptr-showlen),
                                        (Sint) showlen);
                break;
	   case NORMALCHAR: 
                DEBUG1(2,"NORMALCHAR \"%s\"\n",(char *) (aptr - showlen));
                fancydisplay->normalchar(fancydisplay->info,
                                         (char *) (aptr-showlen),
                                         (Sint) showlen);
                break;
        }
      }
      charmode = newcharmode;
      showlen = 0;
    } else
    {
      showlen++;
    }
  }
  fancydisplay->normalchar(fancydisplay->info,"\n\n", SintConst(2));
}


