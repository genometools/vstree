#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "optdesc.h"
#include "debugdef.h"
#include "intbits.h"
#include "errordef.h"
#include "virtualdef.h"
#include "chardef.h"
#include "genfile.h"
#include "fhandledef.h"
#include "programversion.h"
#include "drand48.h"

#include "accvirt.pr"
#include "procopt.pr"
#include "multiseq-adv.pr"
#include "reverse.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "distri.pr"
#include "safescpy.pr"
#include "multiseq.pr"

#ifndef NOLICENSEMANAGER
#include "licensemanager.h"
#endif

#define READPOSITIVEINT\
        READINTGENERIC(options[(Uint) optval].optname,\
                       readint,argc-1,<=,"positive")
#define READINT\
        READINTGENERIC(options[(Uint) optval].optname,\
                       readint,argc-1,<,"non-negative")

typedef enum 
{
  OPTMINLENGTH = 0,
  OPTMAXLENGTH,
  OPTSELECTNUM,      
  OPTSELECTRANGE,
  OPTSELECTSEQ,
  OPTVMATCHVERSION,
  OPTHELP,
  NUMOFOPTIONS
} Optionnumber;

typedef struct
{
  Uint randminlength,
       randmaxlength,
       randselectnum,
       rangefrom,
       rangeto,
       seqlen,
       seqnum,
       seqoffset;
  BOOL randdefined,
       rangedefined,
       seqlendefined;
  char indexname[PATH_MAX+1];
} Selectcallinfo;


static Sint parseseselect(Selectcallinfo *selectcallinfo,
                          const char **argv,
                          Argctype argc)
{
  Uint argnum;
  Optionnumbertype optval;
  Scaninteger readint;
  OptionDescription options[NUMOFOPTIONS];

  selectcallinfo->randdefined = False;
  selectcallinfo->rangedefined = False;
  selectcallinfo->seqlendefined = False;
  initoptions(&options[0],(Uint) NUMOFOPTIONS);
  ADDOPTION(OPTMINLENGTH,"-minlength",
            "specify the minimal length of the substrings to be selected");
  ADDOPTION(OPTMAXLENGTH,"-maxlength",
            "specify the maximal length of the substrings to be selected");
  ADDOPTION(OPTSELECTNUM,"-snum",
            "specify the number of substrings to be selected\n"
            "the previous three option must be used in combination");
  ADDOPTION(OPTSELECTRANGE,"-range",
            "specify the first and last position of the substring "
            "to be selected");
  ADDOPTION(OPTSELECTSEQ,"-seq",
            "specify length, number, and relative position\n"
            "of the substring to be selected");
  ADDOPTVMATCHVERSION;
  ADDOPTION(OPTHELP,"-help","this option");
  if(argc == 1)
  {
    ERROR1("missing options: %s displays the possible options",
            options[OPTHELP].optname);
    return (Sint) -1;
  }
  for(argnum = UintConst(1); argnum < (Uint) argc && ISOPTION(argv[argnum]); 
      argnum++)
  {
    optval = procoption(options,(Uint) NUMOFOPTIONS,argv[argnum]);
    if(optval < 0)
    {
      return (Sint) -2;
    }
    switch(optval)
    {
      case OPTHELP:
        showoptions(stdout,argv[0],options,(Uint) NUMOFOPTIONS);
        return (Sint) 1;
      case OPTVMATCHVERSION:
        return (Sint) 1;
      case OPTMINLENGTH:
        READPOSITIVEINT;
        selectcallinfo->randminlength = (Uint) readint;
        selectcallinfo->randdefined = True;
        break;
      case OPTMAXLENGTH:
        READPOSITIVEINT;
        selectcallinfo->randmaxlength = (Uint) readint;
        selectcallinfo->randdefined = True;
        break;
      case OPTSELECTNUM:
        READPOSITIVEINT;
        selectcallinfo->randselectnum = (Uint) readint;
        selectcallinfo->randdefined = True;
        break;
      case OPTSELECTRANGE:
        READINT;
        selectcallinfo->rangefrom = (Uint) readint;
        READPOSITIVEINT;
        selectcallinfo->rangeto = (Uint) readint;
        selectcallinfo->rangedefined = True;
        break;
      case OPTSELECTSEQ:
        READPOSITIVEINT;
        selectcallinfo->seqlen = (Uint) readint;
        READINT;
        selectcallinfo->seqnum = (Uint) readint;
        READINT;
        selectcallinfo->seqoffset = (Uint) readint;
        selectcallinfo->seqlendefined = True;
        break;
    }
  }
  OPTIONEXCLUDE(OPTSELECTRANGE,OPTMINLENGTH);
  OPTIONEXCLUDE(OPTSELECTRANGE,OPTMAXLENGTH);
  OPTIONEXCLUDE(OPTSELECTRANGE,OPTSELECTNUM);
  OPTIONEXCLUDE(OPTSELECTRANGE,OPTSELECTSEQ);
  OPTIONEXCLUDE(OPTSELECTSEQ,OPTMINLENGTH);
  OPTIONEXCLUDE(OPTSELECTSEQ,OPTMAXLENGTH);
  OPTIONEXCLUDE(OPTSELECTSEQ,OPTSELECTNUM);
  OPTIONIMPLY(OPTMINLENGTH,OPTMAXLENGTH);
  OPTIONIMPLY(OPTMAXLENGTH,OPTMINLENGTH);
  OPTIONIMPLY(OPTMINLENGTH,OPTSELECTNUM);
  OPTIONIMPLY(OPTSELECTNUM,OPTMINLENGTH);
  if(ISSET(OPTMAXLENGTH))
  {
    if(selectcallinfo->randminlength > selectcallinfo->randmaxlength)
    {
      ERROR2("argument to option %s must not be larger than argument "
             "to option %s",
             options[OPTMINLENGTH].optname,
             options[OPTMAXLENGTH].optname);
      return (Sint) -3;
    }
  }
  if(argnum < (Uint) (argc-1))
  {
    ERROR1("superfluous file argument \"%s\"",argv[argc-1]);
    return (Sint) -4;
  }
  if(argnum >= (Uint) argc)
  {
    ERROR0("missing indexname");
    return (Sint) -5;
  }
  if(safestringcopy(&selectcallinfo->indexname[0],
                    argv[argnum],
                    PATH_MAX) != 0)
  {
    return (Sint) -6;
  }
  return 0;
}

static void showsubseq(Alphabet *alpha,Uchar *w,Uint wlen)
{
  Uint i;

  for(i = 0; i < wlen; i++)
  {
    (void) putchar((Fputcfirstargtype) alpha->characters[(Uint) w[i]]);
  }
}

static void showsubsequenceslengthdistribution(ArrayUint *distribution)
{
  Uint i;

  for(i=0; i<distribution->nextfreeUint; i++)
  {
    if(distribution->spaceUint[i] > 0)
    {
      printf("# %lu subsequences of length %lu\n",
              (Showuint) distribution->spaceUint[i],
              (Showuint) i);
    }
  }
}

static Sint selectrange(char *indexname,
                        Multiseq *multiseq,
                        Uint rangefromseqnum,
                        Uint rangefrom,
                        Uint rangeto)
{
  Uint i, laststart, currentseqnum;
  Showdescinfo showdesc;

  if(rangeto >= multiseq->totallength)
  {
    ERROR2("rangeto = %lu is too large, must be smaller than %lu",
            (Showuint) rangeto,
            (Showuint) multiseq->totallength);
    return (Sint) -1;
  }
  laststart = rangefrom;
  currentseqnum = rangefromseqnum;
  ASSIGNDEFAULTSHOWDESC(&showdesc);
  for(i=rangefrom; i<=rangeto; i++)
  {
    if(multiseq->originalsequence[i] == SEPARATOR)
    {
      printf(">");
      echothedescription(stdout,&showdesc,multiseq,currentseqnum);
      printf(" %s [%lu,%lu]\n",
             indexname,
             (Showuint) laststart,
             (Showuint) (i-1));
      currentseqnum++;
      if(formatseq(stdout,
                   0,
                   NULL,
                   0,
                   DEFAULTLINEWIDTH,
                   multiseq->originalsequence+laststart,
                   i-laststart) != 0)
      {
        return (Sint) -2;
      }
      laststart = i+1;
    }
  }
  printf(">");
  echothedescription(stdout,&showdesc,multiseq,currentseqnum);
  printf(" %s [%lu,%lu]\n",
         indexname,
         (Showuint) laststart,
         (Showuint) rangeto);
  if(formatseq(stdout,
               0,
               NULL,
               0,
               DEFAULTLINEWIDTH,
               multiseq->originalsequence+laststart,
               rangeto-laststart+1) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

static Sint selectsubsequences(Alphabet *alpha,
                               Multiseq *multiseq,
                               Selectcallinfo *selectcallinfo)
{
  Uint subseqcount = 0, j, subseqlength, start;
  BOOL special;
  Uchar *subseqspace;
  ArrayUint distribution;

  INITARRAY(&distribution,Uint);
  if(multiseq->totallength <= selectcallinfo->randmaxlength)
  {
    ERROR1("argument to option -maxlength must be smaller than total "
           "length of sequence which is %lu",
           (Showuint) multiseq->totallength);
    return (Sint) -1;
  }
  srand48(42349421);
  ALLOCASSIGNSPACE(subseqspace,NULL,Uchar,selectcallinfo->randmaxlength);
  while(subseqcount < selectcallinfo->randselectnum)
  {
    if(selectcallinfo->randminlength == selectcallinfo->randmaxlength)
    {
      subseqlength = selectcallinfo->randminlength;
    } else
    {
      subseqlength = (Uint) (selectcallinfo->randminlength + 
                            (drand48() * 
                              (double) (selectcallinfo->randmaxlength-
                                        selectcallinfo->randminlength+1)));
    }
    start = (Uint) (drand48() * 
                    (double) (multiseq->totallength-subseqlength));
    if(start > multiseq->totallength - subseqlength)
    {
      fprintf(stderr,"Not enough characters left\n");
      exit(EXIT_FAILURE);
    }
    special = False;
    for(j=0; j<subseqlength; j++)
    {
      subseqspace[j] = multiseq->sequence[start+j];
      if(ISSPECIAL(subseqspace[j]))
      {
        special = True;
        break;
      }
    }
    if(!special)
    {
      adddistribution(&distribution,subseqlength);
      if(subseqcount & 1)
      {
        reverseinplace(subseqspace,subseqlength);
      }
      showsubseq(alpha, subseqspace, subseqlength);
      (void) putchar('\n');
      subseqcount++;
    }
  }
  showsubsequenceslengthdistribution(&distribution);
  FREESPACE(subseqspace);
  FREEARRAY(&distribution,Uint);
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  Sint ret;
  Selectcallinfo selectcallinfo;
#ifndef NOLICENSEMANAGER
  LmLicense *license;
  if (!(license = lm_license_new_vmatch(argv[0])))
    return EXIT_FAILURE;
#endif

  DEBUGLEVELSET;

#ifndef NOLICENSEMANAGER
  CALLSHOWPROGRAMVERSIONWITHLICENSE("vsubseqselect", license);
#else
  CALLSHOWPROGRAMVERSION("vsubseqselect");
#endif
  ret = parseseselect(&selectcallinfo,argv,argc);
  if(ret == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  if(ret < 0)
  {
    STANDARDMESSAGE;
  }
  if(mapvirtualtreeifyoucan(&virtualtree,
                            &selectcallinfo.indexname[0],
                            SSPTAB | TISTAB | OISTAB | DESTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(selectcallinfo.rangedefined)
  {
    Sint rangefromseqnum;
    
    rangefromseqnum = getseqnum(&virtualtree.multiseq,
                                selectcallinfo.rangefrom);
    if(rangefromseqnum < 0)
    {
      STANDARDMESSAGE;
    }
    if(selectrange(&selectcallinfo.indexname[0],
                   &virtualtree.multiseq,
                   (Uint) rangefromseqnum,
                   selectcallinfo.rangefrom,
                   selectcallinfo.rangeto) != 0)
    {
      STANDARDMESSAGE;
    }
  } else
  {
    if(selectcallinfo.seqlendefined)
    {
      PairUint range;
      Uint start;

      if(findboundaries(&virtualtree.multiseq,
                        selectcallinfo.seqnum,
                        &range) != 0)
      {
        STANDARDMESSAGE;
      }
      start = range.uint0 + selectcallinfo.seqoffset;
      if(selectrange(&selectcallinfo.indexname[0],
                     &virtualtree.multiseq,
                     selectcallinfo.seqnum,
                     start,
                     start+selectcallinfo.seqlen-1) != 0)
      {
        STANDARDMESSAGE;
      }
    } else
    {
      if(selectsubsequences(&virtualtree.alpha,
                            &virtualtree.multiseq,
                            &selectcallinfo) != 0)
      {
        STANDARDMESSAGE;
      }
    }
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
#ifndef NOLICENSEMANAGER
  lm_license_delete(license);
#endif
  return EXIT_SUCCESS;
}
