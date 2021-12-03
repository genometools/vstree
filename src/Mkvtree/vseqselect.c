#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef _WIN32
#include <regex.h>
#endif
#include "types.h"
#include "optdesc.h"
#include "debugdef.h"
#include "intbits.h"
#include "errordef.h"
#include "chardef.h"
#include "virtualdef.h"
#include "genfile.h"
#include "fhandledef.h"
#include "programversion.h"
#include "drand48.h"

#include "dstrdup.pr"
#include "accvirt.pr"
#include "procopt.pr"
#include "multiseq-adv.pr"
#include "readvirt.pr"
#include "safescpy.pr"
#include "filehandle.pr"

#define MAXPART 0.5    // maximum of randomselnum/numofsequences resp.
                       // randomsellength/numofsequences

#define MAXREGEXPSIZE 255

#define READPOSITIVEINT    READINTGENERIC(options[(Uint) optval].optname,\
                                          readint,argc-1,<=,"positive")

typedef enum 
{
  OPTMINLENGTH = 0,
  OPTMAXLENGTH,
  OPTRANDOMNUM,      
  OPTRANDOMLENGTH,      
  OPTSEQNUM,
#ifdef WITHREGEXP
  OPTMATCHDESC,
#endif
  OPTVMATCHVERSION,
  OPTHELP,
  NUMOFOPTIONS
} Optionnumber;

typedef struct
{
  BOOL minlengthdefined,
       maxlengthdefined;
  Uint minlength,
       maxlength,
       randomselnum,
       randomsellength;
  char *seqnumfile,
#ifdef WITHREGEXP
       *seqdescpattern,
#endif
       indexname[PATH_MAX+1];
} Selectcallinfo;


static Sint parseseselect(Selectcallinfo *selectcallinfo,
                          const char **argv,
                          Argctype argc)
{
  Optionnumbertype optval;
  Uint argnum;
  Scaninteger readint;
  OptionDescription options[NUMOFOPTIONS];

  selectcallinfo->minlengthdefined = False;
  selectcallinfo->maxlengthdefined = False;
  selectcallinfo->randomselnum = 0;
  selectcallinfo->randomsellength = 0;
#ifdef WITHREGEXP
  selectcallinfo->seqdescpattern = NULL;
#endif
  selectcallinfo->seqnumfile = NULL;
  initoptions(&options[0],(Uint) NUMOFOPTIONS);
  ADDOPTION(OPTMINLENGTH,"-minlength","specify the minimal length of the sequences to be selected");
  ADDOPTION(OPTMAXLENGTH,"-maxlength","specify the maximal length of the sequences to be selected");
  ADDOPTION(OPTRANDOMNUM,"-randomnum","specify the number of random sequences to be selected");
  ADDOPTION(OPTRANDOMLENGTH,"-randomlength","specify the minimal total length of the random sequences\nto be selected");
  ADDOPTION(OPTSEQNUM,"-seqnum","select the sequences with numbers given in filename");
#ifdef WITHREGEXP
  ADDOPTION(OPTMATCHDESC,"-matchdesc","select all sequences whose description contains a substring\nmatching the given pattern");
#endif
  ADDOPTVMATCHVERSION;
  ADDOPTION(OPTHELP,"-help","this option");
  if(argc == 1)
  {
    ERROR1("missing options: %s displays the possible options",
            options[OPTHELP].optname);
    return (Sint) -1;
  }
  for(argnum = UintConst(1); 
      argnum < (Uint) argc && ISOPTION(argv[argnum]);
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
        selectcallinfo->minlength = (Uint) readint;
        selectcallinfo->minlengthdefined = True;
        break;
      case OPTMAXLENGTH:
        READPOSITIVEINT;
        selectcallinfo->maxlength = (Uint) readint;
        selectcallinfo->maxlengthdefined = True;
        break;
      case OPTRANDOMNUM:
        READPOSITIVEINT;
        selectcallinfo->randomselnum = (Uint) readint;
        break;
      case OPTRANDOMLENGTH:
        READPOSITIVEINT;
        selectcallinfo->randomsellength = (Uint) readint;
        break;
#ifdef WITHREGEXP
      case OPTMATCHDESC:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        ASSIGNDYNAMICSTRDUP(selectcallinfo->seqdescpattern,argv[argnum]);
        break;
#endif
      case OPTSEQNUM:
        argnum++;
        CHECKMISSINGARGUMENTWITHOUTLAST;
        ASSIGNDYNAMICSTRDUP(selectcallinfo->seqnumfile,argv[argnum]);
        break;
    }
  }
  OPTIONEXCLUDE(OPTRANDOMNUM,OPTRANDOMLENGTH);
  OPTIONEXCLUDE(OPTSEQNUM,OPTMINLENGTH);
  OPTIONEXCLUDE(OPTSEQNUM,OPTMAXLENGTH);
  OPTIONEXCLUDE(OPTSEQNUM,OPTRANDOMNUM);
  OPTIONEXCLUDE(OPTSEQNUM,OPTRANDOMLENGTH);
#ifdef WITHREGEXP
  OPTIONEXCLUDE(OPTSEQNUM,OPTMATCHDESC);
  OPTIONEXCLUDE(OPTRANDOMNUM,OPTMATCHDESC);
#endif
  // XXXX add some more exclusion
  if(argnum < (Uint) (argc-1))
  {
    ERROR1("superfluous file argument \"%s\"",argv[argc-1]);
    return (Sint) -5;
  }
  if(argnum >= (Uint) argc)
  {
    ERROR0("missing indexname");
    return (Sint) -6;
  }
  if(safestringcopy(&selectcallinfo->indexname[0],argv[argnum],
                    PATH_MAX) != 0)
  {
    return (Sint) -7;
  }
  if(selectcallinfo->minlengthdefined && 
     selectcallinfo->maxlengthdefined)
  {
    if(selectcallinfo->minlength > selectcallinfo->maxlength)
    {
      ERROR2("argument to option %s must not be larger than argument "
             "to option %s",
             options[OPTMINLENGTH].optname,
             options[OPTMAXLENGTH].optname);
      return (Sint) -8;
    }
  }
  return 0;
}


static Sint countsatisfy(Multiseq *multiseq,
                         BOOL minlengthdefined,
                         Uint minlength,
                         BOOL maxlengthdefined,
                         Uint maxlength,
                         Uint *satisfynum,Uint *satisfylength)
{
  Uint snum, len, count = 0, sumlength = 0;
  PairUint range;

  for(snum=0; snum < multiseq->numofsequences; snum++)
  {
    if(findboundaries(multiseq,snum,&range) != 0)
    {
      return (Sint) -1;
    }
    len = range.uint1 - range.uint0 + 1;
    if((!minlengthdefined || len >= minlength) && 
       (!maxlengthdefined || len <= maxlength))
    {
      sumlength += len;
      count++;
    }
  }
  *satisfynum = count;
  *satisfylength = sumlength;
  return 0;
}

static Sint showminmax(Multiseq *multiseq,
                       BOOL minlengthdefined,
                       Uint minlength,
                       BOOL maxlengthdefined,
                       Uint maxlength)
{
  Uint snum;

  for(snum=0; snum < multiseq->numofsequences; snum++)
  {
    if(processsequence(multiseq,minlengthdefined,
                                minlength,
                                maxlengthdefined,
                                maxlength,snum) < 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint showrandomselnum(Multiseq *multiseq,
                            Selectcallinfo *selectcallinfo)
{
  Uint count = 0, seqnum, *occurtab, satisfynum, satisfylength;
  Sint ret;

  if(selectcallinfo->randomselnum > 
     (Uint) (MAXPART * multiseq->numofsequences))
  {
    ERROR3("You can not randomly select more than %lu percent "
           "of the sequences. That is, the maximal number of the sequences "
           "you can select for index \"%s\" is %lu",
           (Showuint) (MAXPART * 100),
           &selectcallinfo->indexname[0],
           (Showuint) (MAXPART * multiseq->numofsequences));
    return (Sint) -1;
  }
  ret = countsatisfy(multiseq,
                     selectcallinfo->minlengthdefined,
                     selectcallinfo->minlength,
                     selectcallinfo->maxlengthdefined,
                     selectcallinfo->maxlength,
                     &satisfynum,&satisfylength);
  if(ret < 0)
  {
    return (Sint) -2;
  }
  if(satisfynum < selectcallinfo->randomselnum)
  {
    ERROR0("There are not enough sequences satisfying the selection criteria");
    return (Sint) -3;
  }
  INITBITTAB(occurtab,multiseq->numofsequences);
  srand48(42349421);
  while(count < selectcallinfo->randomselnum)
  {
    seqnum = (Uint) (drand48() * (double) multiseq->numofsequences);
    if(!ISIBITSET(occurtab,seqnum))
    {
      ret = processsequence(multiseq,
                            selectcallinfo->minlengthdefined,
                            selectcallinfo->minlength,
                            selectcallinfo->maxlengthdefined,
                            selectcallinfo->maxlength,
                            seqnum);
      if(ret < 0)
      {
        return (Sint) -4;
      } else
      {
        SETIBIT(occurtab,seqnum);
        if(ret > 0)
        {
          fprintf(stderr,
                  "# %4lu: select sequence with number %lu (length %ld)\n",
                  (Showuint) count,(Showuint) seqnum,(Showsint) ret);
          count++;
        } 
      }
    }
  }
  FREESPACE(occurtab);
  return 0;
}

static Sint showrandomsellength(Multiseq *multiseq,
                                Selectcallinfo *selectcallinfo)
{
  Uint sumlength = 0, count = 0, seqnum, *occurtab, satisfynum, satisfylength;
  Sint ret;

  if(selectcallinfo->randomsellength > 
     (Uint) (MAXPART * multiseq->totallength))
  {
    ERROR3("You can not randomly select more than %lu percent of the "
           "total sequence. That is, the maximal length of the sequences "
           " you can select for index \"%s\" is %lu",
           (Showuint) (MAXPART * 100),
           &selectcallinfo->indexname[0],
           (Showuint) (MAXPART * multiseq->totallength));
    return (Sint) -1;
  }
  ret = countsatisfy(multiseq,
                     selectcallinfo->minlengthdefined,
                     selectcallinfo->minlength,
                     selectcallinfo->maxlengthdefined,
                     selectcallinfo->maxlength,
                     &satisfynum,
                     &satisfylength);
  if(ret < 0)
  {
    return (Sint) -2;
  }
  if(satisfylength < selectcallinfo->randomsellength)
  {
    ERROR0("There are not enough sequences satisfying the selection criteria");
    return (Sint) -3;
  }
  INITBITTAB(occurtab,multiseq->numofsequences);
  srand48(42349421);
  while(sumlength < selectcallinfo->randomsellength)
  {
    seqnum = (Uint) (drand48() * (double) multiseq->numofsequences);
    if(!ISIBITSET(occurtab,seqnum))
    {
      ret = processsequence(multiseq,
                            selectcallinfo->minlengthdefined,
                            selectcallinfo->minlength,
                            selectcallinfo->maxlengthdefined,
                            selectcallinfo->maxlength,
                            seqnum);
      if(ret < 0)
      {
        return (Sint) -4;
      } else
      {
        SETIBIT(occurtab,seqnum);
        if(ret > 0)
        {
          fprintf(stderr,"# %4lu: select sequence with number %lu (length %ld)\n",
                    (Showuint) count,(Showuint) seqnum,(Showsint) ret);
          sumlength += ((Uint) ret);
          count++;
        } 
      }
    }
  }
  FREESPACE(occurtab);
  return 0;
}

static Sint showseqnumfromfile(Multiseq *multiseq,char *seqnumfile)
{
  FILE *fp;
  Uint linenum = UintConst(1);
  Scaninteger readint;

  DEBUG1(1,"showseqnumfromfile(%s)\n",seqnumfile);
  fp = CREATEFILEHANDLE(seqnumfile,READMODE);
  if(fp == NULL)
  {
    return (Sint) -1;
  }
  while(fscanf(fp,"%ld",&readint) == 1)
  {
    DEBUG1(1,"found position %ld\n",(Showsint) readint);
    if(readint < 0)
    {
      ERROR3("file \"%s\", line %lu contains a negative number %ld",
             seqnumfile,
             (Showuint) linenum,
             (Showsint) readint);
      return (Sint) -2;
    }
    if(processsequence(multiseq,
                       False,
                       0,
                       False,
                       0,(Uint) readint) < 0)
    {
      return (Sint) -3;
    }
    linenum++;
  }
  if(DELETEFILEHANDLE(fp) != 0)
  {
    return (Sint) -4;
  }
  return 0;
}

#ifdef WITHREGEXP

void seterror(Sint code);

static void printregerror(Regexreturntype errcode, regex_t *regex)
{
  char *errbuf;
  size_t i;

  i=regerror(errcode,regex,(char *)NULL,(size_t)0);
  ALLOCASSIGNSPACE(errbuf,NULL,char,(Uint)i);
  (void)regerror(errcode,regex,errbuf,i);
  ERROR1("%s",errbuf);
  seterror(1);
  FREESPACE(errbuf);
}

static Sint compileregexp(regex_t *regex, char *seqdescpattern)
{
  Regexreturntype errcode;

  errcode=regcomp(regex,seqdescpattern,REG_EXTENDED|REG_NEWLINE);
  if(errcode != 0)
  {
    printregerror(errcode,regex);
    regfree(regex);
    return (Sint) -1;
  }
  else
  {
    return 0;
  }
}

static Sint matchseqdescpattern(Multiseq *multiseq,char *seqdescpattern,
                                BOOL minlengthdefined,
                                Uint minlength,
                                BOOL maxlengthdefined,
                                Uint maxlength)
{
  char *descriptions;
  Uint pos=0, deslen, dbseqnum=0;
  regex_t regex;
  regmatch_t pmatch;
  Sint ret=0;
  Regexreturntype errcode=0;
  
  if(compileregexp(&regex,seqdescpattern))
  {
    return (Sint) -1;
  }
  
  deslen=multiseq->descspace.nextfreeUchar;
  ALLOCASSIGNSPACE(descriptions,NULL,char,deslen+1);
  memcpy(descriptions,multiseq->descspace.spaceUchar,(size_t)deslen);
  descriptions[deslen]='\0';
  
  while(errcode == 0 && pos < deslen)
  {
    errcode=regexec(&regex,&descriptions[pos],(size_t)1,&pmatch,0);
    if(errcode == 0)
    {
      if(pmatch.rm_so != pmatch.rm_eo)
      {
        /* find sequence number */
        for(++dbseqnum;
            multiseq->startdesc[dbseqnum] <= ((Uint)pmatch.rm_so+pos);
            ++dbseqnum)
        {
          /* nothing */
        }
        
        /* found non-empty match */
        if((ret=processsequence(multiseq,minlengthdefined,
                                         minlength,
                                         maxlengthdefined,
                                         maxlength,
                                         dbseqnum-1)) < 0)
        {
          break;
        }
        
        /* jump over rest of sequence */
        pos=multiseq->startdesc[dbseqnum];
      }
      else
      {
        /* just in case... */
        ++pos;
      }
    }
    else 
    {
      if(errcode != REG_NOMATCH)
      {
        printregerror(errcode,&regex);
        ret=-2;
      }
    }
  }
  regfree(&regex);
  FREESPACE(descriptions);
  return ret;
}
#endif

MAINFUNCTION
{
  Virtualtree virtualtree;
  Sint ret;
  Selectcallinfo selectcallinfo;

  DEBUGLEVELSET;
  CALLSHOWPROGRAMVERSION("vseqselect");
  ret = parseseselect(&selectcallinfo,argv,argc);
  if(ret == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  if(ret < 0)
  {
    STANDARDMESSAGE;
  }
  if(mapvirtualtreeifyoucan(&virtualtree,&selectcallinfo.indexname[0],
                            SSPTAB | OISTAB | DESTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(selectcallinfo.randomselnum > 0)
  {
    if(showrandomselnum(&virtualtree.multiseq,&selectcallinfo) != 0)
    {
      STANDARDMESSAGE;
    }
  } else
  {
    if(selectcallinfo.randomsellength > 0)
    {
      if(showrandomsellength(&virtualtree.multiseq,&selectcallinfo) != 0)
      {
        STANDARDMESSAGE;
      }
    } else
    {
#ifdef WITHREGEXP
      if(selectcallinfo.seqdescpattern != NULL)
      {
        if(matchseqdescpattern(&virtualtree.multiseq,
                               selectcallinfo.seqdescpattern,
                               selectcallinfo.minlengthdefined,
                               selectcallinfo.minlength,
                               selectcallinfo.maxlengthdefined,
                               selectcallinfo.maxlength) != 0)
        {
          STANDARDMESSAGE;
        }
      } else
#endif
      {
        if(selectcallinfo.seqnumfile != NULL)
        {
          if(showseqnumfromfile(&virtualtree.multiseq,
                                selectcallinfo.seqnumfile) != 0)
          {
            STANDARDMESSAGE;
          } 
        } else
        {
          if(selectcallinfo.minlengthdefined || 
             selectcallinfo.maxlengthdefined)
          {
            if(showminmax(&virtualtree.multiseq,
                          selectcallinfo.minlengthdefined,
                          selectcallinfo.minlength,
                          selectcallinfo.maxlengthdefined,
                          selectcallinfo.maxlength) != 0)
            {
              STANDARDMESSAGE;
            }
          }
        }
      }
    }
  }
  if(selectcallinfo.seqnumfile != NULL)
  {
    FREESPACE(selectcallinfo.seqnumfile);
  }
#ifdef WITHREGEXP
  if(selectcallinfo.seqdescpattern != NULL)
  {
    FREESPACE(selectcallinfo.seqdescpattern);
  }
#endif
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
