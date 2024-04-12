//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "divmodmul.h"
#include "maxfiles.h"
#include "visible.h"
#include "absdef.h"
#include "fhandledef.h"
#include "errordef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "esafileend.h"
#include "chardef.h"
#include "arraydef.h"
#include "alphadef.h"
#include "cmp-tabdef.h"
#include "codondef.h"
#include "megabytes.h"

#include "dstrdup.pr"
#include "compfilenm.pr"
#include "multiseq.pr"
#include "detnumofcodes.pr"
#include "alphabet.pr"
#include "verbosealpha.pr"
#include "filehandle.pr"
#include "multiseq-adv.pr"
#include "cmpalpha.pr"
#include "checkonoff.pr"

//}

/*
  This file defines functions to read a virtual tree.
*/

/*
  The following macro abbreviates the call of the function 
  \texttt{statusbit}.
*/

#define STATUS(B,S)\
        showstatusbit(virtualtree->mapped,virtualtree->constructed,indexname,\
                      B,#S,largelcps,showverbose)

/*
  The following macro is an abbreviation of a call to the function 
  \texttt{freevirtualtree}.
*/

#define VIRTFREE(B,P)\
        if(freevirtualtable(virtualtree->mapped,virtualtree->constructed,\
                            B,P,#B) != 0)\
        {\
          return (Sint) -1;\
        }

/*
  The following three simple macros are used for the output of a virtual 
  tree in \LaTeX-format. \texttt{COUNTTAB} increments the variable
  \texttt{numoftabs} by the value substituted for \texttt{I}, whenever
  the flag substituted for \texttt{B} is set.
*/

#define COUNTTAB(B,I)\
        if(which & (B))\
        {\
          numoftabs += I;\
        }

#define ADDTAB(B)\
        if(which & (B))\
        {\
          if((B) == STI1TAB)\
          {\
            printf(" &\\STITABone");\
          } else\
          {\
            printf(" &\\%3.3s",#B);\
          }\
        }

#define ADDNEWCOMMAND(B)\
        if(which & (B))\
        {\
          if((B) == STI1TAB)\
          {\
            printf("\\newcommand{\\STITABone}[0]{\\mathsf{STI1}}\n");\
          } else\
          {\
            printf("\\newcommand{\\%3.3s}[0]{\\mathsf{%3.3s}}\n",\
                    #B,#B);\
          }\
        }

#define MAPSPECIFICTABLE(TAB,TYPE,TABBIT,END,NUMITEMS)\
        if(demand & (TABBIT))\
        {\
          tmpfilename = COMPOSEFILENAME(indexname,#END);\
          virtualtree->TAB = (TYPE *) CREATEMEMORYMAP(tmpfilename,False,\
                                                       &numofbytes);\
          IFNULLRETURN(virtualtree->TAB);\
          EXPECTED(indexsize,numofbytes,tmpfilename,\
                   (NUMITEMS) * (Uint) sizeof(TYPE),(Sint) -1);\
          virtualtree->mapped |= (TABBIT);\
          FREESPACE(tmpfilename);\
        }

#define MAXFILENAMESUFFIXSIZE 4

static char *vstreetabsuffixes[] =
{
  "tis",
  "ois",
  "des",
  "ssp",
  "bwt",
  "suf",
  "lcp",
  "bck",
  "sti1",
  "skp",
  "cld",
  "cld1",
  "iso",
  "sti",
  "cfr",
  "crf",
  "lsf"
};

static char *vstreeextratabsuffixes[] =
{
  "sds",
  "llv",
  "al1",
  "al2",
  PROJECTFILESUFFIX
};

/*
  The following function looks up a table number in
  table \texttt{vstreetabsuffixes}.
*/

static char *lookupvstreetabsuffix(Uint tabnum)
{
  if(tabnum >= (Uint) sizeof(vstreetabsuffixes)/
               (Uint) sizeof(vstreetabsuffixes[0]))
  {
    fprintf(stderr,"lookupvstreetabsuffixes(%lu) is undefined\n",
            (Showuint) tabnum);
    exit(EXIT_FAILURE);
  }
  return vstreetabsuffixes[tabnum];
}

#ifdef DEBUG

static void computevstreetabsuffix(char *buffer,char *tabname)
{
  Uint i;

  for(i=0; /* Nothing */; i++)
  {
    if(i == (Uint) (MAXFILENAMESUFFIXSIZE-1))
    {
      if(tabname[i] == '1')
      {
        buffer[i++] = '1';
      }
      buffer[i] = '\0';
      return;
    } 
    if(!isupper((Ctypeargumenttype) tabname[i]))
    {
      fprintf(stderr,"tabname[%lu] = %c is not lower case\n",
                      (Showuint) i,tabname[i]);
      exit(EXIT_FAILURE);
    }
    buffer[i] = tolower((Ctypeargumenttype) tabname[i]);
  }
}

static void checktablesuffixes(Uint tabnum, char *tabname)
{
  char suffixspace[MAXFILENAMESUFFIXSIZE+1];

  computevstreetabsuffix(&suffixspace[0],tabname);
  if(strcmp(&suffixspace[0],lookupvstreetabsuffix(tabnum)) != 0)
  {
    fprintf(stderr,"failure: %s != %s\n",&suffixspace[0],
                       lookupvstreetabsuffix(tabnum));
    exit(EXIT_FAILURE);
  }
}

#define CHECKTABLESUFFIXES(TNUM)\
        checktablesuffixes((Uint) TNUM, #TNUM)

static void checkalltablesuffixes(void)
{
  CHECKTABLESUFFIXES(TISTABNUM);
  CHECKTABLESUFFIXES(OISTABNUM);
  CHECKTABLESUFFIXES(DESTABNUM);
  CHECKTABLESUFFIXES(SSPTABNUM);
  CHECKTABLESUFFIXES(BWTTABNUM);
  CHECKTABLESUFFIXES(SUFTABNUM);
  CHECKTABLESUFFIXES(LCPTABNUM);
  CHECKTABLESUFFIXES(BCKTABNUM);
  CHECKTABLESUFFIXES(STI1TABNUM);
  CHECKTABLESUFFIXES(SKPTABNUM);
  CHECKTABLESUFFIXES(CLDTABNUM);
  CHECKTABLESUFFIXES(CLD1TABNUM);
  CHECKTABLESUFFIXES(ISOTABNUM);
  CHECKTABLESUFFIXES(STITABNUM);
  CHECKTABLESUFFIXES(CFRTABNUM);
  CHECKTABLESUFFIXES(CRFTABNUM);
  CHECKTABLESUFFIXES(LSFTABNUM);
}

#endif

static void showvstreedemand(Uint demand)
{
  Uint tabbit, tabnum;

  for(tabnum=0; tabnum< (Uint) NUMBEROFVTABS; tabnum++)
  {
    if(tabnum == (Uint) TISTABNUM)
    {
      tabbit = TISTAB;
    } else
    {
      tabbit = MAKETABBIT(tabnum);
    }
    if(demand & tabbit)
    {
      printf("# demand includes %stab\n",lookupvstreetabsuffix(tabnum));
    }
  }
}

/*EE
  The following function checks if an index file for a virtual tree exists
  If so, then \texttt{True} is returned. Otherwise, \texttt{False}.
*/

BOOL checkifvstreetabfileexists(const char *indexname,const char *suffix)
{
  BOOL retval; 
  char *tmpfilename = COMPOSEFILENAME(indexname,suffix);

  retval = filealreadyexists(tmpfilename);
  FREESPACE(tmpfilename);
  return retval;
}

static Uint whatsthere2demand(const char *indexname)
{
  Uint tabnum, demand = 0;
  char *vstreetabsuffix;

  for(tabnum=0; tabnum< (Uint) NUMBEROFVTABS; tabnum++)
  {
    vstreetabsuffix = lookupvstreetabsuffix(tabnum);
    if(checkifvstreetabfileexists(indexname,vstreetabsuffix))
    {
      printf("# found table %s.%s\n",indexname,vstreetabsuffix);
      if(tabnum == (Uint) TISTABNUM)
      {
        demand |= TISTAB;
      } else
      {
        demand |= MAKETABBIT(tabnum);
      }
    } 
  }
  return demand;
}

Sint removeexistingindexfile(Showverbose showverbose,char *filename)
{
  if(unlink(filename) == -1)
  {
    ERROR1("cannot remove file \"%s\"",filename);
    return (Sint) -1;
  }
  if(showverbose != NULL)
  {
    char sbuf[64+PATH_MAX+MAXFILENAMESUFFIXSIZE+1];
    sprintf(sbuf,"remove file \"%s\"",filename);
    showverbose(sbuf);
  }
  return 0;
}

Sint removeexistingindex(Showverbose showverbose,char *indexname)
{
  char *vstreetabsuffix, *tmpfilename;
  Uint tabnum;

  for(tabnum=0; tabnum< (Uint) NUMBEROFVTABS; tabnum++)
  {
    vstreetabsuffix = lookupvstreetabsuffix(tabnum);
    tmpfilename = COMPOSEFILENAME(indexname,vstreetabsuffix);
    if(filealreadyexists(tmpfilename))
    {
      if(removeexistingindexfile(showverbose,tmpfilename) != 0)
      {
        return (Sint) -1;
      }
    }
    FREESPACE(tmpfilename);
  }
  for(tabnum = 0; tabnum < (Uint) sizeof(vstreeextratabsuffixes)/
                           (Uint) sizeof(vstreeextratabsuffixes[0]); tabnum++)
  { 
    tmpfilename = COMPOSEFILENAME(indexname,vstreeextratabsuffixes[tabnum]);
    if(filealreadyexists(tmpfilename))
    {
      if(removeexistingindexfile(showverbose,tmpfilename) != 0)
      {
        return (Sint) -1;
      }
    }
    FREESPACE(tmpfilename);
  }
  return 0;
}

/*
  The following function checks if the given bit \texttt{tabbit} (one of the
  bits in \texttt{BASICTABS | EXTRATABS}) is either set in the argument
  \texttt{mapped} or the argument \texttt{constructed}. These argument
  tell if the corresponding table is mapped or constructed. 
  The argument \texttt{indexname} is the name of the virtual tree index.
  \texttt{bitname} is \texttt{tabbit} as a string constant. For example,
  if \texttt{tabbit} is \texttt{TISTAB}, then \texttt{bitname} is 
  \texttt{\symbol{34}tis\symbol{34}}. \texttt{largelcps} is the 
  true iff there is at least one lcp value \(\geq 255\). \texttt{showverbose}
  is the function showing the buffered information line.
*/

static void showstatusbit(Uint mapped,Uint constructed,const char *indexname,
                          Uint tabbit,const char *bitname,BOOL largelcps,
                          Showverbose showverbose)
{
  char sbuf[PATH_MAX+30];

  if(mapped & tabbit)
  {
    if(tabbit == LCPTAB)
    {
      sprintf(sbuf,"%s.lcp read",indexname);
      showverbose(sbuf);
      if(largelcps)
      {
        sprintf(sbuf,"%s.llv read",indexname);
        showverbose(sbuf);
      }
    } else
    {
      if(tabbit == DESTAB)
      {
        sprintf(sbuf,"%s.des read",indexname);
        showverbose(sbuf);
        sprintf(sbuf,"%s.sds read",indexname);
        showverbose(sbuf);
      } else
      {
        sprintf(sbuf,"%s.%s read",indexname,bitname);
        showverbose(sbuf);
      }
    }
  } else
  {
    if(constructed & tabbit)
    {
      if(tabbit == LCPTAB)
      {
        showverbose("lcptab constructed");
        if(largelcps)
        {
          showverbose("llvtab constructed");
        }
      } else
      {
        if(tabbit == DESTAB)
        {
          showverbose("destab constructed");
          showverbose("sdstab constructed");
        } else
        {
          sprintf(sbuf,"%stab constructed",bitname);
          showverbose(sbuf);
        }
      }
    }
  }
}

/*
  The following function frees the table referenced by \texttt{tab}.
  \texttt{tabbit} is the corresponding bit. \texttt{bitname} is the name
  of \texttt{tabbit}, see above.
*/

static Sint freevirtualtable(Uint mapped,Uint constructed,Uint tabbit,
                             void *tab,char *bitname)
{
  DEBUG1(2,"# freevirtualtable: %s ",bitname);
  if(mapped & tabbit)
  {
    DEBUG0(2,"was mapped => unmap\n");
    if(DELETEMEMORYMAP(tab) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(constructed & tabbit)
    {
      DEBUG0(2,"was constructed => freespace\n");
      FREESPACE(tab);
    } else
    {
      DEBUG0(2,"neither mapped nor constructed\n");
    }
  }
  return 0;
}

/*EE
  The following function shows the status of \texttt{virtualtree}
  via the function \texttt{showverbose}. The name of the
  index is \texttt{indexname}. If anything went wrong, then
  a negative error code is returned. Otherwise the error code is 0.
*/

Sint showvirtualtreestatus(Virtualtree *virtualtree,
                           const char *indexname,
                           Showverbose showverbose)
{
  BOOL largelcps;
  Sprintfreturntype start;
  Uint i,
       startseq = 0;
  char sbuf[PATH_MAX+200+1];

  if(showverbose == NULL)
  {
    return 0;
  }
  if(virtualtree->rcmindex)
  {
    sprintf(sbuf,"index \"%s\" is reverse complement index",indexname);
    showverbose(sbuf);
  }
  if(virtualtree->sixframeindex != NOTRANSLATIONSCHEME)
  {
    sprintf(sbuf,"index \"%s\" is six frame index for translation table %lu",
                  indexname,(Showuint) virtualtree->sixframeindex);
    showverbose(sbuf);
  }
  for(i=0; i<virtualtree->multiseq.totalnumoffiles; i++)
  {
    sprintf(sbuf,"file=%s %lu %lu",
           virtualtree->multiseq.allfiles[i].filenamebuf,
           (Showuint) virtualtree->multiseq.allfiles[i].filelength,
           (Showuint) ((i == virtualtree->multiseq.totalnumoffiles - 1) ? 
                       virtualtree->multiseq.totallength-startseq : 
                       virtualtree->multiseq.filesep[i]-startseq));
    showverbose(sbuf);
    if(i != virtualtree->multiseq.totalnumoffiles - 1)
    {
      startseq = virtualtree->multiseq.filesep[i]+1;
    } 
  }
  start = sprintf(sbuf,"databaselength=%lu",
                  (Showuint) DATABASELENGTH(&virtualtree->multiseq));
  if(virtualtree->multiseq.numofsequences > UintConst(1))
  {
    start += sprintf(sbuf+start," (including %lu separator%s)",
                     (Showuint) (virtualtree->multiseq.numofsequences - 1),
                     (virtualtree->multiseq.numofsequences == UintConst(2)) 
                           ? "" : "s");
  }
  showverbose(sbuf);
  if(virtualtree->multiseq.numofsequences > UintConst(1))
  {
    ExtremeAverageSequences extreme;
    if(calculateseqparm(&virtualtree->multiseq,
                        &extreme) != 0)
    {
      return (Sint) -1;
    }
    sprintf(sbuf,"sequence lengths: minimal=%lu, maximal=%lu, average=%.2f",
                  (Showuint) extreme.minlength,
                  (Showuint) extreme.maxlength,
                  extreme.averageseqlen);
    showverbose(sbuf);
  }
  if(HASINDEXEDQUERIES(&virtualtree->multiseq))
  {
    sprintf(sbuf,"querylength=%lu",
            (Showuint) virtualtree->multiseq.totalquerylength);
    showverbose(sbuf);
  }
  if(virtualtree->alpha.mapsize > 0)
  {
    verbosealphabet(&virtualtree->alpha,showverbose);
  }
  if(virtualtree->largelcpvalues.nextfreePairUint > 0)
  {
    largelcps = True;
  } else
  {
    largelcps = False;
  }
  STATUS(TISTAB,tis);
  STATUS(OISTAB,ois);
  STATUS(BWTTAB,bwt);
  STATUS(SUFTAB,suf);
  STATUS(LCPTAB,lcp);
  STATUS(ISOTAB,iso);
  STATUS(STITAB,sti);
  STATUS(STI1TAB,sti1);
  STATUS(CLDTAB,cld);
  STATUS(SKPTAB,skp);
  STATUS(BCKTAB,bck);
  STATUS(DESTAB,des);
  STATUS(SSPTAB,ssp);
  STATUS(CFRTAB,afx);
  STATUS(CRFTAB,afx);
  STATUS(LSFTAB,lsf);
  return 0;
}

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

Sint showvirtualtreestatusonstdout(Virtualtree *virtualtree,
                                   char *indexname)
{
  return showvirtualtreestatus(virtualtree,indexname,showonstdout);

}

Sint pumpthroughcache(Virtualtree *virtualtree,Uint demand)
{
  __attribute__ ((unused)) Uint uivalue = 0;
  Uint i;
  __attribute__ ((unused)) Uchar cvalue = 0;

  if(demand == 0)
  {
    ERROR0("pumpthroughcache requires to specify at least one table");
    return (Sint) -1;
  }
  if(demand & TISTAB)
  {
    for(i=0; i<virtualtree->multiseq.totallength; i++)
    {
      cvalue += virtualtree->multiseq.sequence[i];
    }
  }
  if(demand & OISTAB)
  {
    for(i=0; i<virtualtree->multiseq.totallength; i++)
    {
      cvalue += virtualtree->multiseq.originalsequence[i];
    }
  }
  if(demand & BWTTAB)
  {
    for(i=0; i<=virtualtree->multiseq.totallength; i++)
    {
      cvalue += virtualtree->bwttab[i];
    }
  }
  if(demand & LCPTAB)
  {
    for(i=0; i<=virtualtree->multiseq.totallength; i++)
    {
      cvalue += virtualtree->lcptab[i];
    }
    for(i=0; i<virtualtree->largelcpvalues.nextfreePairUint;i++)
    {
      uivalue += virtualtree->largelcpvalues.spacePairUint[i].uint0;
      uivalue += virtualtree->largelcpvalues.spacePairUint[i].uint1;
    }
  }
  if(demand & SUFTAB)
  {
    for(i=0; i<=virtualtree->multiseq.totallength; i++)
    {
      uivalue += virtualtree->suftab[i];
    }
  }
  if(demand & SKPTAB)
  {
    for(i=0; i<=virtualtree->multiseq.totallength; i++)
    {
      uivalue += virtualtree->skiptab[i];
    }
  }
  if(demand & BCKTAB)
  {
    for(i=0; i < UintConst(2) * virtualtree->numofcodes; i++)
    {
      uivalue += virtualtree->bcktab[i];
    }
  }
  if(demand & ISOTAB)
  {
    for(i=0; i<virtualtree->multiseq.totallength; i++)
    {
      cvalue += virtualtree->isodepthtab[i];
    }
  }
  if(demand & STITAB)
  {
    for(i=0; i<=virtualtree->multiseq.totallength; i++)
    {
      uivalue += virtualtree->stitab[i];
    }
  }
  if(demand & STI1TAB)
  {
    for(i=0; i <= virtualtree->multiseq.totallength; i++)
    {
      cvalue += virtualtree->stitab1[i];
    }
  }
  if(demand & CLDTAB)
  {
    for(i=0; i <= virtualtree->multiseq.totallength; i++)
    {
      cvalue += virtualtree->cldtab[i].up;
      cvalue += virtualtree->cldtab[i].down;
      cvalue += virtualtree->cldtab[i].nextlIndex;
      cvalue += virtualtree->cldtab1[i];
    }
  }
  if(demand & (CFRTAB | CRFTAB))
  {
    for(i=0; i < virtualtree->multiseq.totallength; i++)
    {
      uivalue += virtualtree->afxtab[i];
    }
  }
  if(demand & LSFTAB)
  {
    for(i=0; i < UintConst(2) * (virtualtree->multiseq.totallength+1); i++)
    {
      cvalue += virtualtree->lsftab[i];
    }
  }
  return 0;
}

/*
  The following function decodes the longest value from the end of
  the bwttable;
*/

/*
static Uint decodelongest(Uint totallength,Uchar *bwttab)
{
  Uint longest, j, i = totallength+1;

  longest = (Uint) bwttab[i];
  for(j=UintConst(1); j < (Uint) sizeof(Uint); j++)
  {
    longest |= (((Uint) bwttab[j+i]) << MULT8(j));
  }
  DEBUG1(2,"# longest=%lu\n",(Showuint) longest);
  return longest;
}
*/


Sint mapskptab(Virtualtree *virtualtree,const char *indexname)
{
  Uint numofbytes;
  __attribute__ ((unused)) Uint indexsize = 0;
  char *tmpfilename = COMPOSEFILENAME(indexname,"skp");

  virtualtree->skiptab = (Uint *) CREATEMEMORYMAP(tmpfilename,False,
                                                  &numofbytes);
  if(virtualtree->skiptab == NULL)
  {
    FREESPACE(tmpfilename);
    return (Sint) -1;
  }
  EXPECTED(indexsize,numofbytes,tmpfilename,
           (virtualtree->multiseq.totallength + 1) * (Uint) sizeof(Uint),
           (Sint) -1);
  virtualtree->mapped |= SKPTAB;
  FREESPACE(tmpfilename);
  return 0;
}

/*EE
  The following function makes a virtual tree empty. The function 
  should be called for any virtual tree before using it.
*/

void makeemptyvirtualtree(Virtualtree *virtualtree)
{
  virtualtree->bwttab = NULL;
  virtualtree->lcptab = NULL;
  virtualtree->isodepthtab = NULL;
  virtualtree->suftab = NULL;
  virtualtree->skiptab = NULL;
  virtualtree->afxtab = NULL;
  virtualtree->bcktab = NULL;
  virtualtree->stitab = NULL;
  virtualtree->stitab1 = NULL;
  virtualtree->cldtab = NULL;
  virtualtree->cldtab1 = NULL;
  virtualtree->lsftab = NULL;
  virtualtree->longest.defined = False;
  initmultiseq(&virtualtree->multiseq);
  virtualtree->multiseq.originalsequence = NULL;
  INITARRAY(&virtualtree->largelcpvalues,PairUint);
  virtualtree->specialsymbols = False;
  virtualtree->rcmindex = False;
  virtualtree->sixframeindex = NOTRANSLATIONSCHEME;
  virtualtree->numofcodes = 0;
  virtualtree->maxbranchdepth = 0;
  virtualtree->mapped = virtualtree->constructed = 0;
  virtualtree->prefixlength = 0;
  initmultiseqfileinfo(&virtualtree->multiseq);
#ifdef COUNT
  virtualtree->callgetexception = virtualtree->llvcachehit = 0;
#endif
  virtualtree->llvcachemin = virtualtree->llvcachemax = NULL;
}

/*EE
  The following function maps the different tables comprising the 
  virtual tree with name \texttt{indexname} according to the bits
  specified in \texttt{demand}. If the demand cannot be fully
  be satisfied, then a negative error code is returned and 
  a corresponding error message is throws.
*/

#define SETDEMAND(TAB)\
        if(demand & (TAB))\
        {\
          demand##TAB = True;\
        } else\
        {\
          demand##TAB = False;\
        }

Sint mapvirtualtreeifyoucan(Virtualtree *virtualtree,
                            const char *indexname,
                            Uint demand)
{
  Uint numofchars = 0, numofbytes, indexsize = 0;
  char *tmpfilename;
  BOOL demandTISTAB, demandOISTAB, demandDESTAB, demandSSPTAB;
  Sint retcode;

#ifdef DEBUG
  checkalltablesuffixes();
#endif
  makeemptyvirtualtree(virtualtree);
  if(strlen(indexname) >= PATH_MAX)
  {
    ERROR1("indexname \"%s\" too long",indexname);
    return (Sint) -1;
  }
  if(parseprojectfile(&virtualtree->multiseq,
                      &virtualtree->longest,
                      &virtualtree->prefixlength,
                      &virtualtree->largelcpvalues,
                      &virtualtree->maxbranchdepth,
                      &virtualtree->rcmindex,
                      &virtualtree->sixframeindex,
                      indexname) != 0)
  {
    return (Sint) -2;
  }
  if(mapalphabetifyoucan(&virtualtree->specialsymbols,
                         &virtualtree->alpha,indexname) != 0)
  {
    return (Sint) -3;
  }
  if(virtualtree->specialsymbols)
  {
    numofchars = virtualtree->alpha.mapsize - 1;
  } else
  {
    numofchars = virtualtree->alpha.mapsize;
  }
  virtualtree->numofcodes = vm_determinenumofcodes(numofchars,
                                                   virtualtree->prefixlength);
  virtualtree->mapped = virtualtree->constructed = 0;
  SETDEMAND(TISTAB);
  SETDEMAND(OISTAB);
  SETDEMAND(DESTAB);
  SETDEMAND(SSPTAB);
  if(mapmultiseqifyoucan(&indexsize,
                         &virtualtree->multiseq,
                         indexname,
                         demandTISTAB,
                         demandOISTAB,
                         demandDESTAB,
                         demandSSPTAB) != 0)
  {
    return (Sint) -4;
  }
  virtualtree->constructed = 0;
  virtualtree->mapped = (demand & SEQUENCETABS);
  MAPSPECIFICTABLE(bwttab,Uchar,BWTTAB,bwt,
                   virtualtree->multiseq.totallength + 1);
  MAPSPECIFICTABLE(suftab,Uint,SUFTAB,suf,
                   virtualtree->multiseq.totallength + 1);
  MAPSPECIFICTABLE(lcptab,Uchar,LCPTAB,lcp,
                   virtualtree->multiseq.totallength + 1);
  if(demand & LCPTAB)  // the rest
  {
    if(virtualtree->largelcpvalues.nextfreePairUint > 0)
    {
      tmpfilename = COMPOSEFILENAME(indexname,"llv");
      virtualtree->largelcpvalues.spacePairUint 
        = (PairUint *) CREATEMEMORYMAP(tmpfilename,False,&numofbytes);
      IFNULLRETURN(virtualtree->largelcpvalues.spacePairUint);
      EXPECTED(indexsize,numofbytes,tmpfilename,
               virtualtree->largelcpvalues.nextfreePairUint * 
               (Uint) sizeof(PairUint),(Sint) -1);
      FREESPACE(tmpfilename);
    }
  }
  if(demand & BCKTAB)   // generate special error message
  {
    if(virtualtree->prefixlength == 0)
    {
      ERROR1("cannot open file %s.bck: "
             "use option -pl when constructing the index",
             indexname);
      return (Sint) -21;
    }
  }
  MAPSPECIFICTABLE(bcktab,Uint,BCKTAB,bck,
                   UintConst(2) * virtualtree->numofcodes);
  MAPSPECIFICTABLE(stitab1,Uchar,STI1TAB,sti1,
                   virtualtree->multiseq.totallength+1);
  if(demand & SKPTAB)  // GENERIC
  {
    if(mapskptab(virtualtree,indexname) != 0)
    {
      return (Sint) -18;
    }
    indexsize += (virtualtree->multiseq.totallength + 1) * (Uint) sizeof(Uint);
  }
  MAPSPECIFICTABLE(cldtab,Childinfo,CLDTAB,cld,
                   virtualtree->multiseq.totallength+1);
  MAPSPECIFICTABLE(cldtab1,Uchar,CLD1TAB,cld1,
                   virtualtree->multiseq.totallength+1);
  MAPSPECIFICTABLE(isodepthtab,Uchar,ISOTAB,iso,
                   virtualtree->multiseq.totallength);
  MAPSPECIFICTABLE(stitab,Uint,STITAB,sti,
                   virtualtree->multiseq.totallength+1);
  if((demand & CFRTAB) && (demand & CRFTAB))
  {
    ERROR0("cannot map cfrtab and crftab at the same time");
    return (Sint) -32;
  }
  MAPSPECIFICTABLE(afxtab,Uint,CFRTAB,cfr,
                   virtualtree->multiseq.totallength);
  MAPSPECIFICTABLE(afxtab,Uint,CRFTAB,crf,
                   virtualtree->multiseq.totallength);
  MAPSPECIFICTABLE(lsftab,Uchar,LSFTAB,lsf,
                   UintConst(2) * virtualtree->multiseq.totallength+1);
  retcode = checkenvvaronoff("VMATCHSHOWTIMESPACE");
  if(retcode == (Sint) -1)
  {
    return (Sint) -33;
  }
  if(retcode != 0)
  {
    printf("# mapped index of size %.2f\n",MEGABYTES(indexsize));
  }
  return 0;
}

Sint mapallofvirtualtreewhatsthere(Virtualtree *virtualtree,
                                   const char *indexname)
{
  Uint demand = whatsthere2demand(indexname);

  showvstreedemand(demand);
  if(mapvirtualtreeifyoucan(virtualtree,indexname,demand) != 0)
  {
    return (Sint) -1;
  }
  if(pumpthroughcache(virtualtree,demand) < 0)
  {
    return (Sint) -3;
  }
  return 0;
}

/*EE
  The following function additionally maps the original sequence
  into secondary memory and returns a pointer to it. If this is not 
  possible, then \texttt{NULL} is returned.
*/

/*@null@*/ Uchar *getoriginal(char *indexname)
{
  Uchar *orig;
  Uint numofbytes;
  char *tmpfilename = COMPOSEFILENAME(indexname,"ois");

  orig = (Uchar *) CREATEMEMORYMAP(tmpfilename,False,&numofbytes);
  FREESPACE(tmpfilename);
  return orig;
}


/*EE
  The following function frees the space allocated or mapped for
  a virtual tree, except for the multiple sequence.
  If something went wrong, then a negative error
  code is returned. If not, then the return code is 0.
*/

Sint freevirtualtreemain(Virtualtree *virtualtree)
{
  VIRTFREE(BWTTAB,virtualtree->bwttab);
  VIRTFREE(SUFTAB,virtualtree->suftab);
  VIRTFREE(LCPTAB,virtualtree->lcptab);
  VIRTFREE(SKPTAB,virtualtree->skiptab);
  VIRTFREE(BCKTAB,virtualtree->bcktab);
  VIRTFREE(ISOTAB,virtualtree->isodepthtab);
  VIRTFREE(STITAB,virtualtree->stitab);
  VIRTFREE(STI1TAB,virtualtree->stitab1);
  VIRTFREE(CLDTAB,virtualtree->cldtab);
  VIRTFREE(CLDTAB,virtualtree->cldtab1);
  VIRTFREE(CFRTAB,virtualtree->afxtab);
  VIRTFREE(CRFTAB,virtualtree->afxtab);
  VIRTFREE(LSFTAB,virtualtree->lsftab);
  if(virtualtree->largelcpvalues.nextfreePairUint > 0)
  {
    if(virtualtree->mapped & LCPTAB)
    {
      if(virtualtree->largelcpvalues.nextfreePairUint > 0)
      {
        if(DELETEMEMORYMAP(virtualtree->largelcpvalues.spacePairUint) != 0)
        {
          return (Sint) -1;
        }
      }
    } else
    {
      if(virtualtree->constructed & LCPTAB)
      {
        FREEARRAY(&virtualtree->largelcpvalues,PairUint);
      }
    }
  }
  virtualtree->mapped = virtualtree->constructed = 0;
  return 0;
}

/*EE
  The following function frees the space allocated or mapped for
  a virtual tree, including the multiple sequence.
  If something went wrong, then a negative error
  code is returned. If not, then the return code is 0.
*/

Sint freevirtualtree(Virtualtree *virtualtree)
{
  DEBUG0(2,"# freevirtualtree\n");
#ifdef COUNT
  printf("callgetexception=%lu\n",(Showuint) virtualtree->callgetexception);
  printf("llvcachehit=%lu\n",(Showuint) virtualtree->llvcachehit);
#endif
  if(freevirtualtreemain(virtualtree) != 0)
  {
    return (Sint) -1;
  }
  freemultiseq(&virtualtree->multiseq);
  return 0;
}

static void simplyshowthestring(char *s)
{
  printf("%s",s);
}

/*EE
  The following function transforms a integer encoding a 
  string of length \texttt{prefixlen} over an alphabet with
  \texttt{numofchars} characters into the original string 
  w.r.t.\ to the list of \texttt{characters}.
*/

void integercode2string(void (*processstring)(char *),
                        Uint code,
                        Uint numofchars,
                        Uint prefixlen,
                        Uchar *characters)
{
  char sspace[64+1];
  Sint i;
  Uint cc, tmpcode = code;

  sspace[prefixlen] = '\0';
  for(i=(Sint) (prefixlen-1); i>=0; i--)
  {
    cc = tmpcode % numofchars;
    sspace[i] = (char) characters[cc];
    tmpcode = (tmpcode - cc) / numofchars;
  }
  processstring(sspace);
}

static Uint getsepnum(Uchar *w,Uint wlen,Uint i)
{
  Uint j, sepcount = 0;

  for(j=0; j<wlen; j++)
  {
    if(j==i)
    {
      break;
    }
    if(w[j] == SEPARATOR)
    {
      sepcount++;
    }
  }
  return sepcount;
}

static void showtexcharacter(Uchar *w,Uint wlen,Uchar *characters,Uint i)
{
  Uchar tic;

  tic = w[i];
  if(tic == SEPARATOR)
  {
    printf("%lu",(Showuint) getsepnum(w,wlen,i));
  } else
  {
    if(i == wlen)
    {
      (void) putchar((Fputcfirstargtype) ' ');
    } else
    {
      (void) putchar((Fputcfirstargtype) characters[tic]);
    }
  }
}

static void showtexsymbolstring(Multiseq *multiseq,Alphabet *alpha,
                                Uint startpos,Uint showlength)
{
  Uint i;
 
  for(i = startpos; i < startpos + showlength; i++)
  {
    showtexcharacter(multiseq->sequence,
                     multiseq->totallength,
                     &alpha->characters[0],i);
  }
}

/*EE
  The following function shows the virtual tree as a table in \LaTeX-format.
  The bits in the argument \texttt{which} tell which of the tables 
  is to be shown.
*/

Sint virtual2tex(Uint which,BOOL bckhz,
                 BOOL showstring,Virtualtree *virtualtree)
{
  Uint i, idx, numoftabs = 0;
  Uchar oic;

  printf("\\documentclass[12pt]{article}\n");

  ADDNEWCOMMAND(OISTAB);
  ADDNEWCOMMAND(TISTAB);
  ADDNEWCOMMAND(SUFTAB);
  ADDNEWCOMMAND(LCPTAB);
  ADDNEWCOMMAND(SKPTAB);
  ADDNEWCOMMAND(BWTTAB);     
  ADDNEWCOMMAND(ISOTAB); 
  ADDNEWCOMMAND(STITAB);
  ADDNEWCOMMAND(STI1TAB);  
  ADDNEWCOMMAND(CLDTAB);  
  ADDNEWCOMMAND(BCKTAB);  
  ADDNEWCOMMAND(CFRTAB);
  ADDNEWCOMMAND(CRFTAB);
  ADDNEWCOMMAND(LSFTAB);
  if(showstring & !(which & SUFTAB))
  {
    printf("\\newcommand{\\SUF}[0]{\\mathsf{SUF}}\n");
  }
  printf("\\begin{document}\n");

/*
  count the number numoftabs of tables to be shown
*/

  COUNTTAB(OISTAB,1);
  COUNTTAB(TISTAB,1);
  COUNTTAB(SUFTAB,1);
  COUNTTAB(LCPTAB,1);
  COUNTTAB(SKPTAB,1);
  COUNTTAB(BWTTAB,1);    
  COUNTTAB(ISOTAB,1); 
  COUNTTAB(STITAB,1);  
  COUNTTAB(STI1TAB,1); 
  COUNTTAB(CLDTAB,3); 
  COUNTTAB(CFRTAB,1); 
  COUNTTAB(CRFTAB,1); 
  COUNTTAB(LSFTAB,2); 
  if(showstring)
  {
    numoftabs++;
  }

  printf("\\[\n");
  if(numoftabs > 0)
  {
    printf(" \\begin{array}[t]{*{%lu}{|r}|%c|}\\hline\n i",
               (Showuint) numoftabs,
               showstring ? 'l' : 'r');

/*
  add each table to the header line of the \LaTeX-array
*/

    ADDTAB(OISTAB);
    ADDTAB(TISTAB);
    ADDTAB(SUFTAB);
    ADDTAB(LCPTAB);
    ADDTAB(SKPTAB);
    ADDTAB(BWTTAB);     
    ADDTAB(ISOTAB); 
    ADDTAB(STITAB);
    ADDTAB(STI1TAB);
    if(which & CLDTAB)
    {
      printf(" &\\multicolumn{3}{c|}{\\CLD}");
    }
    ADDTAB(CFRTAB);
    ADDTAB(CRFTAB);
    if(which & LSFTAB)
    {
      printf(" &\\multicolumn{2}{c|}{\\LSF}");
    }
    if(showstring)
    {
      printf(" &S_{\\SUF[i]}");
    }
    printf(" \\\\\\hline\\hline\n");
    for(i=0; i<=virtualtree->multiseq.totallength; i++)
    {
      printf(" %lu",(Showuint) i);
      if(which & OISTAB)
      {
        printf(" &");
        oic = virtualtree->multiseq.originalsequence[i];
        if(oic == SEPARATOR)
        {
          printf("%lu",
                  (Showuint) getsepnum(virtualtree->multiseq.originalsequence,
                  virtualtree->multiseq.totallength,
                  i));
        } else
        {
          if(i == virtualtree->multiseq.totallength)
          {
            (void) putchar((Fputcfirstargtype) ' ');
          } else
          {
            (void) putchar((Fputcfirstargtype) oic);
          }
        }
      }
      if(which & TISTAB)
      {
        printf(" &");
        showtexcharacter(virtualtree->multiseq.sequence,
                         virtualtree->multiseq.totallength,
                         &virtualtree->alpha.characters[0],
                         i);
      }
      if(which & SUFTAB)
      {
        printf(" &%lu",(Showuint) virtualtree->suftab[i]);
      }
      if(which & LCPTAB)
      {
        if(i == 0)
        {
          printf(" &      ");
        } else
        {
          printf(" &%lu",(Showuint) virtualtree->lcptab[i]);
        }
      }
      if(which & SKPTAB)
      {
        printf(" &%lu",(Showuint) (1+virtualtree->skiptab[i]));
      }
      if(which & BWTTAB)
      {
        if(!virtualtree->longest.defined)
        {
          ERROR0("longest is not defined");
          return (Sint) -1;
        }
        if(virtualtree->longest.uintvalue == i)
        {
          printf(" &          ");
        } else
        {
          printf(" &\\texttt{");
          showtexcharacter(virtualtree->multiseq.sequence,
                           virtualtree->multiseq.totallength,
                           &virtualtree->alpha.characters[0],
                           virtualtree->suftab[i]-1);
          printf("}");
        }
      }
      if(which & ISOTAB)
      {
        printf(" &");
        if(i < virtualtree->multiseq.totallength)
        {
          printf("%lu",(Showuint) virtualtree->isodepthtab[i]);
        }
      }
      if(which & STITAB)
      {
        printf(" &%lu",(Showuint) virtualtree->stitab[i]);
      }
      if(which & STI1TAB)
      {
        printf(" &%lu",(Showuint) virtualtree->stitab1[i]);
      }
      if(which & CLDTAB)
      {
        if(virtualtree->cldtab[i].up != UNDEFCHILDVALUE)
        {
          printf(" &%lu",(Showuint) (i-virtualtree->cldtab[i].up));
        } else
        {
          printf(" &    ");
        }
        if(virtualtree->cldtab[i].down != UNDEFCHILDVALUE)
        {
          printf(" &%lu",(Showuint) (i+virtualtree->cldtab[i].down));
        } else
	{
	  printf(" &    ");
	}
        if(virtualtree->cldtab[i].nextlIndex != UNDEFCHILDVALUE)
        {
          printf(" &%lu",(Showuint) (i+virtualtree->cldtab[i].nextlIndex));
        } else
        {
          printf(" &    ");
        }
      }
      if(which & (CFRTAB | CRFTAB))
      {
        printf(" &%lu",(Showuint) virtualtree->afxtab[i]);
      }
      if(which & LSFTAB)
      {
        printf(" &%lu &%lu",
                 (Showuint) virtualtree->lsftab[MULT2(i)],
                 (Showuint) virtualtree->lsftab[MULT2(i)+1]);
      }
      if(showstring)
      {
        Uint showlen, reallen;
        printf(" &\\texttt{");
        reallen = virtualtree->multiseq.totallength - virtualtree->suftab[i];
        showlen = reallen;
        if(showlen > UintConst(10))
        {
          Uint maxlcp = virtualtree->lcptab[i];
          
          if(i < virtualtree->multiseq.totallength && 
             virtualtree->lcptab[i+1] > maxlcp)
          {
            maxlcp = virtualtree->lcptab[i+1];
          }
          if(showlen >  maxlcp)
          {
            showlen = (Uint) maxlcp + UintConst(1);
          }
        }
        showtexsymbolstring(&virtualtree->multiseq,
                            &virtualtree->alpha,
                            virtualtree->suftab[i],
                            showlen);
        if(showlen == reallen)
        {
          printf("\\symbol{36}}\n");
        } else
        {
          printf("...}\n");
        }
      }
      printf(" \\\\\\hline\n");
    }
    printf(" \\end{array}\n");
  }
  if(which & BCKTAB)
  {
    if(numoftabs > 0)
    {
      printf("&");
    }
    if(bckhz)
    {
      printf(" \\begin{array}{|l*{%lu}{|c}|}\\hline\n",
              (Showuint) virtualtree->numofcodes);
      printf(" w&");
      for(idx = 0, i=0; i <virtualtree->numofcodes; i++)
      {
        printf(" \\texttt{");
        integercode2string(simplyshowthestring,
                           i,
                           virtualtree->alpha.mapsize-1,
                           virtualtree->prefixlength,
                           &virtualtree->alpha.characters[0]);
        printf("}%s",(i == virtualtree->numofcodes - 1) ?  
                     "\\\\\\hline\n" : "&");
      }
      printf("\\BCK[\\varphi(w)]&");
      for(idx=0, i=0; i <virtualtree->numofcodes; idx+=2, i++)
      {
        if(virtualtree->bcktab[idx+1] > virtualtree->bcktab[idx])
        {
          printf("(%lu,%lu)",(Showuint) virtualtree->bcktab[idx],
                             (Showuint) (virtualtree->bcktab[idx+1]-1));
        } else
        {
          printf("(1,0)");
        }
        printf("%s",(i == virtualtree->numofcodes - 1) ?  
                    " \\\\\\hline\n" : "&");
      }
    } else
    {
      printf(" \\begin{array}[t]{|l|c|}\\hline\n");
      printf(" w&\\BCK[\\varphi(w)]\\\\\\hline\\hline\n");
      for(idx=0, i=0; i <virtualtree->numofcodes; idx+=2, i++)
      {
        printf(" \\texttt{");
        integercode2string(simplyshowthestring,
                           i,
                           virtualtree->alpha.mapsize-1,
                           virtualtree->prefixlength,
                           &virtualtree->alpha.characters[0]);
        printf("}&");
        if(virtualtree->bcktab[idx+1] > virtualtree->bcktab[idx])
        {
          printf("(%lu,%lu)",(Showuint) virtualtree->bcktab[idx],
                             (Showuint) (virtualtree->bcktab[idx+1]-1));
        } else
        {
          printf("(1,0)");
        }
        printf(" \\\\\\hline\n");
      }
    }
    printf(" \\end{array}\n");
  }
  printf("\\]\n\\end{document}\n");
  return 0;
}

Sint verifysuffixofindexname(const char *indexname,const char *suffix)
{
  size_t indexnamelen = strlen(indexname),
          suffixlen = strlen(suffix);

  if(indexnamelen < suffixlen+1 || 
     strcmp(indexname + (indexnamelen-suffixlen),suffix) != 0)
  {
    ERROR2("indexname \"%s\" does not have suffix \"%s\"",
           indexname,suffix);
    return (Sint) -1;
  }
  return 0;
}

static Sint cutoffsuffixofindexname(char *indexname,char *suffix)
{
  size_t indexnamelen = strlen(indexname),
         suffixlen = strlen(suffix);

  if(indexnamelen < suffixlen+1 || 
     strcmp(indexname + (indexnamelen-suffixlen),suffix) != 0)
  {
    ERROR2("indexname \"%s\" does not have suffix \"%s\"",
           indexname,suffix);
    return (Sint) -1;
  }
  indexname[indexnamelen-suffixlen] = '\0';
  return 0;
}

Sint prepareforsixframeselfmatch(Virtualtree *dnavirtualtree,
                                 const char *indexname,
                                 BOOL maporig,
                                 Showverbose showverbose)
{
  Uint dnademand = TISTAB | DESTAB;
  char *dnaindexname;

  if(verifysuffixofindexname(indexname,SIXFRAMESUFFIX) != 0)
  {
    return (Sint) -1;
  }
  ASSIGNDYNAMICSTRDUP(dnaindexname,indexname);
  if(cutoffsuffixofindexname(dnaindexname,SIXFRAMESUFFIX) != 0)
  {
    return (Sint) -2;
  }
#ifdef DEBUG
  dnademand |= OISTAB;
#endif
  if(maporig)
  {
    dnademand |= OISTAB;
  }
  if(mapvirtualtreeifyoucan(dnavirtualtree,
                            dnaindexname,
                            dnademand) != 0)
  {
    return (Sint) -4;
  }
  if(showverbose != NULL)
  {
    if(showvirtualtreestatus(dnavirtualtree,
                             dnaindexname,
                             showverbose) != 0)
    {
      return (Sint) -5;
    }
  }
  FREESPACE(dnaindexname);
  return 0;
}

Sint checkshowstringmultiple(Uint showstring)
{
  if(showstring > 0)
  {
    if((showstring % CODONLENGTH) != 0)
    {
      ERROR1("argument to option -s must be multiple of %lu",
             (Showuint) CODONLENGTH);
      return (Sint) -1;
    }
  }
  return 0;
}

//\Ignore{

#ifdef DEBUG

#define COMPAREVIRTUALINT(INT)\
        if(virtualtree1->INT != virtualtree2->INT)\
        {\
          fprintf(stderr,"%s.%s: %lu != %lu\n","compareVirtualtree",#INT,\
                  (Showuint) virtualtree1->INT,(Showuint) virtualtree2->INT);\
          exit(EXIT_FAILURE);\
        }\
        printf("# %s.%s=%lu\n","compareVirtualtree",#INT,\
                                (Showuint) virtualtree1->INT)

#define COMPAREVIRTUALBOOL(BVAL)\
        if(virtualtree1->BVAL != virtualtree2->BVAL)\
        {\
          fprintf(stderr,"%s.%s: %lu != %lu\n","compareVirtualtree",#BVAL,\
                  (Showuint) virtualtree1->BVAL,(Showuint) virtualtree2->BVAL);\
          exit(EXIT_FAILURE);\
        }\
        printf("# %s.%s=%lu\n","compareVirtualtree",#BVAL,\
               (Showuint) virtualtree1->BVAL)

void comparePairUinttab(char *tag,PairUint *tab1,PairUint *tab2,Uint len)
{
  Uint i;

  for(i=0; i < len; i++)
  {
    if(tab1[i].uint0 != tab2[i].uint0)
    {
      fprintf(stderr,"(%s.uint0)[%lu]: %lu != %lu\n",tag,
              (Showuint) i,
              (Showuint) tab1[i].uint0,
              (Showuint) tab2[i].uint0);
      exit(EXIT_FAILURE);
    }
    if(tab1[i].uint1 != tab2[i].uint1)
    {
      fprintf(stderr,"(%s.uint1)[%lu]: %lu != %lu\n",tag,
              (Showuint) i,
              (Showuint) tab1[i].uint1,
              (Showuint) tab2[i].uint1);
      exit(EXIT_FAILURE);
    }
  }
  printf("# %s of length %lu identical\n",tag,(Showuint) len);
}


void comparebwttab(Virtualtree *virtualtree1,Virtualtree *virtualtree2)
{
  Uint i;

  if(virtualtree1->bwttab == NULL && virtualtree2->bwttab == NULL)
  {
    printf("# comparebwttab: bwttab has length 0\n");
  } else
  {
    if(virtualtree1->bwttab == NULL)
    {
      fprintf(stderr,"comparebwttab: bwttab1 is NULL but bwttab2 not\n");
      exit(EXIT_FAILURE);
    } 
    if(virtualtree2->bwttab == NULL)
    {
      fprintf(stderr,"comparebwttab: bwttab2 is NULL but bwttab1 not\n");
      exit(EXIT_FAILURE);
    }
    if(!virtualtree1->longest.defined)
    {
      fprintf(stderr,"virtualtree1->longest is undefined\n");
      exit(EXIT_FAILURE);
    }
    if(!virtualtree2->longest.defined)
    {
      fprintf(stderr,"virtualtree2->longest is undefined\n");
      exit(EXIT_FAILURE);
    }
    COMPAREVIRTUALINT(longest.uintvalue);
    for(i=0; i < virtualtree1->multiseq.totallength+1; i++)
    {
      if(i != virtualtree1->longest.uintvalue && 
         virtualtree1->bwttab[i] != virtualtree2->bwttab[i])
      {
        fprintf(stderr,"comparebwttab[%lu]: %lu != %lu\n",(Showuint) i,
                        (Showuint) virtualtree1->bwttab[i],
                        (Showuint) virtualtree2->bwttab[i]);
        exit(EXIT_FAILURE);
      }
    }
    printf("# comparebwttab: bwttab of length %lu identical\n",
            (Showuint) (virtualtree1->multiseq.totallength+1));
  }
}

static void compareChildinfo(char *tag,Childinfo *tab1,Childinfo *tab2,Uint len)
{
  Uint i;

  if(tab1 == NULL || tab2 == NULL)
  {
    printf("# %s is NULL\n",tag);
  } else
  {
    if(tab1 == NULL)
    {
      fprintf(stderr,"%s1 is NULL but %s2 not\n",tag,tag);
      exit(EXIT_FAILURE);
    }
    if(tab2 == NULL)
    {
      fprintf(stderr,"%s2 is NULL but %s1 not\n",tag,tag);
      exit(EXIT_FAILURE);
    }
    for(i=0; i < len; i++)
    {
      if(tab1[i].nextlIndex != tab2[i].nextlIndex)
      {
        fprintf(stderr,"%s[%lu].nextlIndex: %lu != %lu\n",tag,
                                        (Showuint) i,
                                        (Showuint) tab1[i].nextlIndex,
                                        (Showuint) tab2[i].nextlIndex);
        exit(EXIT_FAILURE);
      }
      if(tab1[i].down != tab2[i].down)
      {
        fprintf(stderr,"%s[%lu].down: %lu != %lu\n",
                tag,
                (Showuint) i,(Showuint) tab1[i].down,
                (Showuint) tab2[i].down);
        exit(EXIT_FAILURE);
      }
      if(tab1[i].up != tab2[i].up)
      {
        fprintf(stderr,"%s[%lu].up: %lu != %lu\n",
                tag,
                (Showuint) i,(Showuint) tab1[i].up,(Showuint) tab2[i].up);
        exit(EXIT_FAILURE);
      }
    }
    printf("# %s of length %lu identical\n",tag,(Showuint) len);
  }
}

void compareVirtualtree(Virtualtree *virtualtree1,Virtualtree *virtualtree2)
{
  compareMultiseq(&virtualtree1->multiseq,&virtualtree2->multiseq);
  if(compareAlphabet(&virtualtree1->alpha,&virtualtree2->alpha) != 0)
  {
    fprintf(stderr,"%s\n",messagespace());
    exit(EXIT_FAILURE);
  }
/*
  if((virtualtree1->mapped | virtualtree1->constructed) !=
     (virtualtree2->mapped | virtualtree2->constructed))
  {
    fprintf(stderr,"compareVirtualtree: different mapped | constructed\n");
    fprintf(stderr,"virtualtree1:\n");
    if(showvirtualtreestatus(virtualtree1,"(null)",showonstderr) != 0)
    {
      fprintf(stderr,"%s\n",messagespace());
      exit(EXIT_FAILURE);
    }
    fprintf(stderr,"virtualtree2:\n");
    if(showvirtualtreestatus(virtualtree2,"(null)",showonstderr) != 0)
    {
      fprintf(stderr,"%s\n",messagespace());
      exit(EXIT_FAILURE);
    }
  }
  printf("# mapped | constructed are identical\n");
*/
  comparebwttab(virtualtree1,virtualtree2);
  compareUinttab("comparevirtual.suftab",virtualtree1->suftab,
                  virtualtree2->suftab,
                  virtualtree1->multiseq.totallength+1);
  compareUinttab("comparevirtual.skiptab",virtualtree1->skiptab,
                  virtualtree2->skiptab,
                  virtualtree1->multiseq.totallength+1);
  COMPAREVIRTUALINT(numofcodes);
  compareUinttab("comparevirtual.bcktab",virtualtree1->bcktab,
                  virtualtree2->bcktab,
                  UintConst(2) * virtualtree1->numofcodes);
  compareUinttab("comparevirtual.stitab",virtualtree1->stitab,
                  virtualtree2->stitab,
                  virtualtree1->multiseq.totallength+1);
  compareUchartab("comparevirtual.stitab1",virtualtree1->stitab1,
                  virtualtree2->stitab1,
                  virtualtree1->multiseq.totallength+1);
  compareUchartab("comparevirtual.lcptab",virtualtree1->lcptab,
                  virtualtree2->lcptab,
                  virtualtree1->multiseq.totallength+1);
  compareUchartab("comparevirtual.isodepthdepth",virtualtree1->isodepthtab,
                  virtualtree2->isodepthtab,
                  virtualtree1->multiseq.totallength);
  compareChildinfo("comparevirtual.cldtab",virtualtree1->cldtab,
                    virtualtree2->cldtab,
                    virtualtree1->multiseq.totallength+1);
  COMPAREVIRTUALINT(largelcpvalues.nextfreePairUint);
  comparePairUinttab("comparevirtual.largelcptab",
                     virtualtree1->largelcpvalues.spacePairUint,
                     virtualtree2->largelcpvalues.spacePairUint,
                     virtualtree1->largelcpvalues.nextfreePairUint);
  COMPAREVIRTUALBOOL(specialsymbols);
  COMPAREVIRTUALINT(maxbranchdepth);
  COMPAREVIRTUALINT(prefixlength);
  printf("# comparevirtualtrees: okay\n");
}
#endif

//}
