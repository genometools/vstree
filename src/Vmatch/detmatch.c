
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "errordef.h"
#include "maxfiles.h"
#include "spacedef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "genfile.h"
#include "fhandledef.h"
#include "match.h"
#include "matchtask.h"
#include "matchinfo.h"
#include "codondef.h"
#include "procmultiseq.h"
#ifdef VMATCHDB
#include "vmdbtypes.h"
#include "vmdbfunc.pr"
#include "parsedbms.pr"
#endif

#include "dstrdup.pr"
#include "splitargs.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"

#include "parsevm.pr"
#include "assigndig.pr"
#include "procargs.pr"
#include "mokay.pr"
#include "queryindex.pr"
#include "vmatch.pr"

#define LARGEHEADINGPREFIX  "# largeheading="
#define SMALLHEADINGPREFIX  "# smallheading="

#define ARGLINEERROR(S)\
        ERROR4("matchfile \"%s\", line %lu: argument line \"%s...\" %s",\
               matchfile,(Showuint) line,ARGUMENTLINEPREFIX,S)

#define CANNOTPARSE(S)\
        ERROR2("cannot parse matchfile \"%s\" %s",matchfile,S)

#define CANNOTPARSELINE(S)\
        ERROR3("matchfile \"%s\", line %lu: %s",matchfile,(Showuint) line,S)

#define PARSEINT(INP)\
        if(fscanf(fp,"%ld",&INP) != 1)\
        {\
          CANNOTPARSELINE("cannot parse integer");\
          return (Sint) -1;\
        }

#define ASSIGNPOSITIVE(U,I)\
        if((I) < 0)\
        {\
          ERROR3("matchfile \"%s\", line %lu: negative value %ld not allowed",\
                  matchfile,(Showuint) line,(Showsint) (I));\
          return (Sint) -2;\
        }\
        U = (Uint) I

#define SCANVALUE(FT,V)\
        while ((*matchline == ' ') && (*matchline != '\0'))\
        {\
          matchline++;\
        }\
        scanned = False;\
        if (sscanf(matchline,FT,&V) == 1 )\
        {\
           scanned = True;\
        }\
        while ((*matchline != ' ') && (*matchline != '\0'))\
        {\
          matchline++;\
        }

#define INCCOMMENTBUF 1024

#ifdef VMATCHDB
#define IBUFSIZE 1024
#define ERRORDB\
        ERROR0(dbmsbundle.lasterror());\
        dbmsbundle.disconnect();
#endif

static void matchcallinfo2matchinfo(void *minfo,void *mcinfo)
{
  Matchinfo *matchinfo = (Matchinfo *) minfo;
  Matchcallinfo *matchcallinfo = (Matchcallinfo *) mcinfo;
  Uint i;

  matchinfo->numberofqueryfiles = matchcallinfo->numberofqueryfiles;
  matchinfo->matchtask = matchcallinfo->matchtask;
  matchinfo->transnum = matchcallinfo->matchparam.transnum;
  matchinfo->xdropbelowscore = matchcallinfo->matchparam.xdropbelowscore;
  if(matchcallinfo->matchparam.dnasymbolmapdnavsprot == NULL)
  {
    matchinfo->dnasymbolmapdnavsprot = NULL;
  } else
  {
    ASSIGNDYNAMICSTRDUP(matchinfo->dnasymbolmapdnavsprot,
                        matchcallinfo->matchparam.dnasymbolmapdnavsprot);
  }
  for(i=0; i<matchinfo->numberofqueryfiles; i++)
  {
    ASSIGNDYNAMICSTRDUP(matchinfo->queryfiles[i],matchcallinfo->queryfiles[i]);
  }
  COPYOUTINFO(&matchinfo->outinfo,&matchcallinfo->outinfo);
  /*
    Do not copy mfargs, smallheading, and largeheading
  */
}

static Sint analyzeargline(Argctype callargc,
                           const char **callargv,
                           const char *matchfile,
                           FILE *outfp,
                           char *commentlinebuffer,
                           Uint linelen,
                           Matchinfo *matchinfo,
                           BOOL ignoreshowoptions,
                           BOOL withinputsequences,
                           Showverbose showverbose)
{
  Argctype vargc;
  char *vargv[MAXNUMBEROFFILES + MAXNUMOFARGS];
  Matchcallinfo localmatchcallinfo;
  Uint demand; 
  BOOL storedesc, maporig, didreadqueryfromindex;

  vargv[0] = "";
  if(splitargstring(commentlinebuffer,
                    linelen,
                    (Uint) (MAXNUMBEROFFILES + MAXNUMOFARGS),
                    &vargc,
                    &vargv[1]) != 0)
  {
    return (Sint) -1;
  }
  if(parsevmatchargs(True,vargc,(const char * const*) vargv,
                     NULL,  // showverbose
                     NULL,  // outfp
                     NULL,
                     &localmatchcallinfo) != 0)
  {
    return (Sint) -2;
  }
  /*
    in case the source of data is independend of made showoptions
    we ignore them here. vmatchdb gets its match data directly from
    C data structures not from output made by vmatch. Furthermore
    Vmatchdb will assemble its own representations of matches to analyze.
  */
  if (!ignoreshowoptions)
  {
    if(localmatchcallinfo.outinfo.showdesc.defined)
    {
      CANNOTPARSE("with descriptions: use sequence numbers for output");
      return (Sint) -3;
    }
    if(localmatchcallinfo.outinfo.showstring > 0)
    {
      CANNOTPARSE("with sequence content of the matches");
      return (Sint) -4;
    }
    if(localmatchcallinfo.matchtask & TASKPREINFO)
    {
      CANNOTPARSE("containing only the number of matches");
      return (Sint) -6;
    }
    if(localmatchcallinfo.outinfo.showmode & SHOWNODIST)
    {
      CANNOTPARSE("without distance information");
      return (Sint) -7;
    }
    if(localmatchcallinfo.outinfo.showmode & SHOWNOEVALUE)
    {
      CANNOTPARSE("without Evalues");
      return (Sint) -7;
    }
    if(localmatchcallinfo.outinfo.showmode & SHOWFILE)
    {
      CANNOTPARSE("with filename");
      return (Sint) -8;
    }
  }
  if(outfp != NULL)
  {
    char *rawargs;
    if(savethearguments(&rawargs,
                        False,
                        vargc,
                        (const char * const*) vargv,
                        localmatchcallinfo.indexormatchfile) != 0)
    {
      return (Sint) -9;
    }
    if(showargumentline(NULL,
                        outfp,
                        rawargs,
                        callargc,
                        callargv) != 0)
    {
      return (Sint) -10;
    }
    FREESPACE(rawargs);
    if(savethearguments(&localmatchcallinfo.mfargs,
                        True,
                        vargc,
                        (const char * const*) vargv,
                        &localmatchcallinfo.indexormatchfile[0]) != 0)
    {
      return (Sint) -11;
    }
  }
  matchcallinfo2matchinfo((void *) matchinfo,(void *) &localmatchcallinfo);
  demand = DESTAB | SSPTAB;
  if(withinputsequences)
  {
    demand = demand | TISTAB | OISTAB;
  }
  if(mapvirtualtreeifyoucan(&matchinfo->virtualtree,
                            &localmatchcallinfo.indexormatchfile[0],
                            demand) != 0)
  {
    return (Sint) -12;
  }
  if(matchinfo->numberofqueryfiles == 0 && 
     matchinfo->virtualtree.sixframeindex != NOTRANSLATIONSCHEME)
  {
    matchinfo->transnum = matchinfo->virtualtree.sixframeindex;
  }
  assignvirtualdigits(&matchinfo->outinfo.digits,
                      &matchinfo->virtualtree.multiseq);
  if(showverbose != NULL)
  {
    if(showvirtualtreestatus(&matchinfo->virtualtree,
                             &localmatchcallinfo.indexormatchfile[0],
                             showverbose) != 0)
    {
      return (Sint) -13;
    }
    if(matchinfo->numberofqueryfiles == 0)
    {
      if(matchinfo->matchtask & TASKSUPER)
      {
        showverbose("read supermaximal self matches");
      } else
      {
        if(matchinfo->matchtask & TASKMUM)
        {
          showverbose("read maximal unique matches");
        } else
        {
          showverbose("read self matches");
        }
      }
    } else
    {
      showverbose("read query matches");
    }
  }
  storedesc = True;
  maporig = withinputsequences;
  if(initoutinfostruct(&matchinfo->outinfo,
                       NULL,
                       matchinfo->numberofqueryfiles,
                       &localmatchcallinfo.indexormatchfile[0],
                       storedesc,
                       matchinfo->transnum,
                       NULL,
                       NULL,
                       matchinfo->dnasymbolmapdnavsprot,
                       (const char * const*) &matchinfo->queryfiles[0],
                       maporig,
                       &matchinfo->virtualtree,
                       &matchinfo->dnavirtualtree,
                       &matchinfo->queryvirtualtree,
                       &didreadqueryfromindex,
                       showverbose) != 0)
  {
    return (Sint) -14;
  }
  if(freeMatchcallinfo(&localmatchcallinfo) != 0)
  {
    return (Sint) -15;
  }
  return 0;
}

static Sint analyzematchline(const char *matchfile,
                             const char *matchline,
                             Uint line,
                             Matchinfo *matchinfo,
                             Matchparam *selectmatchparam)
{
  Scaninteger length1, length2, position1, position2, seqnum1, seqnum2, 
              relpos1, relpos2, score, distance;
  char modechar;
  StoreMatch *mptr;
  PairUint pos;
  Multiseq *ms2;
  double identity;
  BOOL scanned;

  SCANVALUE("%ld",length1);
  if(!scanned)
  {
    return (Sint) 1;
  }
  GETNEXTFREEINARRAY(mptr,&matchinfo->matchtab,StoreMatch,128);
  ASSIGNPOSITIVE(mptr->Storelength1,length1);
  if(matchinfo->outinfo.showmode & SHOWABSOLUTE)
  {
    SCANVALUE("%ld",position1);
    if(scanned)
    {
      ASSIGNPOSITIVE(mptr->Storeposition1,position1);
      if(pos2pospair(matchinfo->outinfo.outvms,
                     &pos,
                     mptr->Storeposition1) != 0)
      {
        return (Sint) -1;
      }
      mptr->Storeseqnum1 = pos.uint0;
      mptr->Storerelpos1 = pos.uint1;
    } else
    {
      CANNOTPARSELINE("cannot read position1");
      return (Sint) -2;
    }
  } else
  {
    SCANVALUE("%ld",seqnum1);
    if (scanned)
    {
      SCANVALUE("%ld",relpos1);
    }
    if(scanned)
    {
      ASSIGNPOSITIVE(mptr->Storeseqnum1,seqnum1);
      ASSIGNPOSITIVE(mptr->Storerelpos1,relpos1);
      if(findfirstpos(matchinfo->outinfo.outvms,
                      mptr->Storeseqnum1,
                      &mptr->Storeposition1) != 0)
      {
        return (Sint) -3;
      }
      mptr->Storeposition1 += mptr->Storerelpos1;
    } else
    {
      CANNOTPARSELINE("cannot read seqnum1 and relpos1");
      return (Sint) -4;
    }
  }
  SCANVALUE("%c",modechar);
  if(scanned)
  {
    if(modechar != DIRECTCHAR &&
       modechar != PALINDROMICCHAR &&
       modechar != PPFWDFWDCHAR &&
       modechar != PPFWDREVCHAR &&
       modechar != PPREVFWDCHAR &&
       modechar != PPREVREVCHAR)
    {
      CANNOTPARSELINE(MODECHARERRMS);
      return (Sint) -5;
    }
  } else
  {
    CANNOTPARSELINE("cannot read modechar");
    return (Sint) -6;
  }
  if(matchinfo->numberofqueryfiles == 0)
  {
    ms2 = matchinfo->outinfo.outvms;
    switch(modechar)
    {
      case DIRECTCHAR:
        mptr->Storeflag = 0;
        break;
      case PALINDROMICCHAR:
        mptr->Storeflag = FLAGPALINDROMIC | FLAGSELFPALINDROMIC;
        break;
      case PPFWDFWDCHAR:
        mptr->Storeflag = 0;
        break;
      case PPFWDREVCHAR:
        mptr->Storeflag = FLAGPPRIGHTREVERSE;
        break;
      case PPREVFWDCHAR:
        mptr->Storeflag = FLAGPPLEFTREVERSE;
        break;
      case PPREVREVCHAR:
        mptr->Storeflag = FLAGPPLEFTREVERSE | FLAGPPRIGHTREVERSE;
        break;
    }
  } else
  {
    ms2 = matchinfo->outinfo.outqms;
    switch(modechar)
    {
      case DIRECTCHAR:
        mptr->Storeflag = FLAGQUERY;
        break;
      case PALINDROMICCHAR:
        mptr->Storeflag = FLAGPALINDROMIC | FLAGQUERY;
        break;
      case PPFWDFWDCHAR:
        mptr->Storeflag = FLAGQUERY;
        break;
      case PPFWDREVCHAR:
        mptr->Storeflag = FLAGQUERY | FLAGPPRIGHTREVERSE;
        break;
      case PPREVFWDCHAR:
        mptr->Storeflag = FLAGQUERY | FLAGPPLEFTREVERSE;
        break;
      case PPREVREVCHAR:
        mptr->Storeflag = FLAGQUERY | FLAGPPLEFTREVERSE | FLAGPPRIGHTREVERSE;
        break;
    }
  }
  if(matchinfo->xdropbelowscore != UNDEFXDROPBELOWSCORE)
  {
    mptr->Storeflag |= SETFLAGXDROP(matchinfo->xdropbelowscore);
  } 
  if(matchinfo->transnum != NOTRANSLATIONSCHEME)
  {
    mptr->Storeflag |= MAKEFLAGCODONMATCH(matchinfo->transnum);
  }
  SCANVALUE("%ld",length2);
  if(scanned)
  {
    ASSIGNPOSITIVE(mptr->Storelength2,length2);
  } else
  {
    CANNOTPARSELINE("cannot read length2");
    return (Sint) -7;
  }
  if(matchinfo->numberofqueryfiles == 0)
  {
    ms2 = matchinfo->outinfo.outvms;
  } else
  {
    ms2 = matchinfo->outinfo.outqms;
  }
  if(matchinfo->outinfo.showmode & SHOWABSOLUTE)
  {
    SCANVALUE("%ld",position2);
    if(scanned)
    {
      ASSIGNPOSITIVE(mptr->Storeposition2,position2);
      if(pos2pospair(ms2,&pos,mptr->Storeposition2) != 0)
      {
        return (Sint) -8;
      }
      mptr->Storeseqnum2 = pos.uint0;
      mptr->Storerelpos2 = pos.uint1;
    } else
    {
      CANNOTPARSELINE("cannot read position2");
      return (Sint) -9;
    }
  } else
  {
    SCANVALUE("%ld",seqnum2);
    if (scanned)
    {
      SCANVALUE("%ld",relpos2);
    }
    if(scanned)
    {
      ASSIGNPOSITIVE(mptr->Storeseqnum2,seqnum2);
      ASSIGNPOSITIVE(mptr->Storerelpos2,relpos2);
      if(findfirstpos(ms2,
                      mptr->Storeseqnum2,
                      &mptr->Storeposition2) != 0)
      {
        return (Sint) -10;
      }
      mptr->Storeposition2 += mptr->Storerelpos2;
    } else
    {
      CANNOTPARSELINE("cannot read seqnum2 and relpos2");
      return (Sint) -11;
    }
  }
  SCANVALUE("%ld",distance);
  if(!scanned)
  {
    CANNOTPARSELINE("cannot read distance");
    return (Sint) -12;
  }
  mptr->Storedistance = (Sint) distance;
  SCANVALUE("%lf",mptr->StoreEvalue);
  if(!scanned)
  {
    CANNOTPARSELINE("cannot read Evalue");
    return (Sint) -13;
  }
  if(!(matchinfo->outinfo.showmode & SHOWNOSCORE))
  {
    SCANVALUE("%ld",score);
    if(!scanned)
    {
      CANNOTPARSELINE("cannot read score");
    }
  }
  if(!(matchinfo->outinfo.showmode & SHOWNOIDENTITY))
  {
    SCANVALUE("%lf",identity);
    if(!scanned)
    {
      CANNOTPARSELINE("cannot read identity score");
    }
    if(identity == 0.0)
    {
      mptr->Storeflag |= FLAGSCOREMATCH;
    }
  }
  mptr->idnumber = matchinfo->matchtab.nextfreeStoreMatch - 1;
  if(selectmatchparam != NULL &&
     !matchokay(mptr->Storelength1,
                mptr->Storeposition1,
                mptr->Storelength2,
                mptr->Storeposition2,
                mptr->Storedistance,
                mptr->StoreEvalue,
                mptr->Storeflag,
                selectmatchparam))
  {
    matchinfo->matchtab.nextfreeStoreMatch--;
  }
  return 0;
}

/*

static Sint skipline(FILE *fp)
{
  Fgetcreturntype inputchar;

  while(True)
  {
    inputchar = fgetc(fp);
    if(inputchar == EOF)
    {
      return 1;
    }
    if(inputchar == '\n')  // skip the line
    {
      break;
    }
  }
  return 0;
}
*/

#define WRAPUPRESOURCES\
        if(DELETEFILEHANDLE(matchfp) != 0)\
        {\
          return (Sint) -1;\
        }\
        FREEARRAY(&commentlinebuffer,char)

static Sint readMatchinfofromfile(Argctype callargc,
                                  const char **callargv,
                                  FILE *outfp,
                                  Matchinfo *matchinfo,
                                  const char *matchfile,
                                  Matchparam *selectmatchparam,
                                  BOOL withinputsequences,
                                  Showverbose showverbose)
{
  FILE *matchfp;
  Sint ret;
  Fgetcreturntype inputchar;
  Uint line = UintConst(1),
       argprefixlen = (Uint) strlen(ARGUMENTLINEPREFIX),
       largeheadingprefixlen = (Uint) strlen(LARGEHEADINGPREFIX),
       smallheadingprefixlen = (Uint) strlen(SMALLHEADINGPREFIX);
  Arraychar commentlinebuffer;

  INITARRAY(&commentlinebuffer,char);
  matchfp = CREATEFILEHANDLE(matchfile,READMODE);
  if(matchfp == NULL)
  {
    return (Sint) -1;
  }
  inputchar = fgetc(matchfp);
  if(inputchar == EOF)
  {
    ERROR1("empty matchfile \"%s\"",matchfile);
    WRAPUPRESOURCES;
    return (Sint) -2;
  }
  (void) ungetc(inputchar,matchfp);
  while(True)
  {
    inputchar = fgetc(matchfp);
    if(inputchar == EOF)
    {
      break;
    }
    (void) ungetc(inputchar,matchfp);
    if(inputchar == '#')
    {
      commentlinebuffer.nextfreechar = 0;
      while((inputchar = fgetc(matchfp)) != '\n')
      {
        STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,(char) inputchar);
      }
      STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,'\0');
      if(commentlinebuffer.nextfreechar >= argprefixlen &&
         strncmp(commentlinebuffer.spacechar,ARGUMENTLINEPREFIX,
                 (size_t) argprefixlen) == 0)
      {
        if(matchinfo->mfargs != NULL)
        {
          ARGLINEERROR("already occurred");
          WRAPUPRESOURCES;
          return (Sint) -3;
        }
        ASSIGNDYNAMICSTRDUP(matchinfo->mfargs,
                            commentlinebuffer.spacechar + argprefixlen);
        ret = analyzeargline(callargc,
                             callargv,
                             matchfile,
                             outfp,
                             commentlinebuffer.spacechar + argprefixlen,
                             commentlinebuffer.nextfreechar - argprefixlen,
                             matchinfo,
                             False,
                             withinputsequences,
                             showverbose);
        if(ret < 0)
        {
          WRAPUPRESOURCES;
          return (Sint) -4;
        }
        commentlinebuffer.nextfreechar = 0;
        if(ret == (Sint) 1)
        {
          break;
        }
      } else
      {
        if(commentlinebuffer.nextfreechar >= largeheadingprefixlen &&
           strncmp(commentlinebuffer.spacechar,LARGEHEADINGPREFIX,
                   (size_t) largeheadingprefixlen) == 0)
        {
          if(matchinfo->largeheading != NULL)
          {
            CANNOTPARSELINE("largeheading already occurred");
            return (Sint) -5;
          }
          ASSIGNDYNAMICSTRDUP(matchinfo->largeheading,
                              commentlinebuffer.spacechar+
                              largeheadingprefixlen);
          if(outfp != NULL)
          {
            fprintf(outfp,"%s%s\n",LARGEHEADINGPREFIX,matchinfo->largeheading);
          }
        } else
        {
          if(commentlinebuffer.nextfreechar >= smallheadingprefixlen &&
             strncmp(commentlinebuffer.spacechar,SMALLHEADINGPREFIX,
                     (size_t) smallheadingprefixlen) == 0)
          {
            if(matchinfo->smallheading != NULL)
            {
              CANNOTPARSE("smallheading already occurred");
              WRAPUPRESOURCES;
              return (Sint) -6;
            }
            ASSIGNDYNAMICSTRDUP(matchinfo->smallheading,
                                commentlinebuffer.spacechar+
                                smallheadingprefixlen);
            if(outfp != NULL)
            {
              fprintf(outfp,"%s%s\n",SMALLHEADINGPREFIX,
                      matchinfo->smallheading);
            }
          }
        }
      }
    } else
    {
      if(matchinfo->mfargs == NULL)
      {
        ARGLINEERROR("is missing");
        WRAPUPRESOURCES;
        return (Sint) -7;
      }
      commentlinebuffer.nextfreechar = 0;
      while(True)
      {
        inputchar = fgetc(matchfp);
        if(inputchar == EOF)
        {
         commentlinebuffer.nextfreechar = 0;
         (void) ungetc(inputchar,matchfp);
          break;
        }
        if(inputchar == '\n')
        {
          STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,'\0');
          break;
        }
        STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,inputchar);
      }
      if (commentlinebuffer.nextfreechar > 0)
      {      
        ret = analyzematchline(matchfile,
                               commentlinebuffer.spacechar,
                               line,
                               matchinfo,
                               selectmatchparam);
        if(ret < 0)
        {
          WRAPUPRESOURCES;
          return (Sint) -8;
        }
      }
    }
    line++;
  }
  FREEARRAY(&commentlinebuffer,char);
  WRAPUPRESOURCES;
  if(matchinfo->mfargs == NULL)
  {
    ARGLINEERROR("is missing");
    return (Sint) -9;
  }
  return 0;
}

#ifdef VMATCHDB
static Sint readMatchinfofromdb(Argctype callargc,
                                char **callargv,
                                FILE *outfp,
                                Matchinfo *matchinfo,
                                char *runidstr,
                                Matchparam *selectmatchparam,
                                Databaseparms *dbparms,
                                BOOL withinputsequences,
                                Showverbose showverbose)
{
  Sint ret;
  Scaninteger readint;
  Uint i,count;
  Arraychar commentlinebuffer;
  ArrayMatchrecord matches;
  char matchline[IBUFSIZE+1];
  Runrecord run;
  Matchrecord match;
  DbmsBundle dbmsbundle;

  /*
    getting pointers to database functions
  */
  if (openDbmsBundle(dbparms->sharedlibname,&dbmsbundle) != 0)
  {
    freedbmsparms(dbparms);
    return (Sint) -1;
  }

  /*
    initializing the record so it can be used as a template for searching
  */
  dbmsbundle.runbundle.initrec(&run);

  /*
    getting runid
  */
  if(sscanf(runidstr,"%ld",&readint) != 1 || readint <= 0)
  {
    ERROR1("invalid runid given \"%s\"",runidstr);
    freedbmsparms(dbparms);
    return (Sint) -1;
  }
  run.runid = (Uint) readint;

  /*
    connecting to a database
  */
  if (dbmsbundle.connect (dbparms) != 0)
  {
    ERRORDB;
    freedbmsparms(dbparms);
    return (Sint) -2;
  }
  freedbmsparms(dbparms);

  /*
    check: first we count the runs
  */
  if (dbmsbundle.runbundle.countrecs (&run,&count) < 0)
  {
    ERRORDB;
    return (Sint) -4;
  }
  if (count == 0)
  {
    dbmsbundle.disconnect();
    ERROR1("cannot find a run with id \"%s\"",runidstr);
    return (Sint) -5;
  }

  /*
    now loading this very one run
  */
  if (dbmsbundle.runbundle.loadrec (&run) < 0)
  {
    ERRORDB;
    return (Sint) -6;
  }

  /*
    assembling and analyzing argument line
  */
  INITARRAY(&commentlinebuffer,char);
  for (i=0; i<strlen((char *)run.params); i++)
  {
    STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,(char) run.params[i]);
  }
  STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,' ');
  for (i=0; i<strlen((char *)run.indexname); i++)
  {
    STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,(char) run.indexname[i]);
  }
  STOREINARRAY(&commentlinebuffer,char,INCCOMMENTBUF,'\0');
  dbmsbundle.runbundle.freerec(&run);
  ASSIGNDYNAMICSTRDUP(matchinfo->mfargs,commentlinebuffer.spacechar);
  ret = analyzeargline(callargc,
                       callargv,
                       runidstr,
                       outfp,
                       commentlinebuffer.spacechar,
                       commentlinebuffer.nextfreechar,
                       matchinfo,
                       True,
                       withinputsequences,
                       showverbose);
  FREEARRAY(&commentlinebuffer,char);
  if(ret < 0)
  {
    return (Sint) -7;
  }

  /*
    loading all matches which belong to our run
  */
  dbmsbundle.matchbundle.initrecs(&matches);
  dbmsbundle.matchbundle.initrec(&match);
  if(sscanf(runidstr,"%ld",&readint) != 1)
  {
    ERROR1("invalid runid given \"%s\"",runidstr);
    freedbmsparms(dbparms);
    return (Sint) -8;
  }
  match.runid = (Uint) readint;
  if (dbmsbundle.matchbundle.loadrecs (NULL,&match,&matches) < 0)
  {
    ERRORDB;
    return (Sint) -9;
  }

  /*
    only disconnecting. checking for spaceleaks later
  */
  dbmsbundle.disconnect();

  /*
    iterating and reading matches
  */
  for (i=0; i<matches.nextfreeMatchrecord; i++)
  {
    sprintf(matchline,"%ld %ld %ld %c %ld %ld %ld %ld %.2e %ld %.2f",
                      (Showsint) matches.spaceMatchrecord[i].length1,
                      (Showsint) matches.spaceMatchrecord[i].seqnum1,
                      (Showsint) matches.spaceMatchrecord[i].relpos1,
                      (matches.spaceMatchrecord[i].ispalindromic == 'Y') 
                             ? 'P' : 'D',
                      (Showsint) matches.spaceMatchrecord[i].length2,
                      (Showsint) matches.spaceMatchrecord[i].seqnum2,
                      (Showsint) matches.spaceMatchrecord[i].relpos2,
                      (Showsint) matches.spaceMatchrecord[i].distance,
                      matches.spaceMatchrecord[i].evalue,
                      (Showsint) matches.spaceMatchrecord[i].score,
                      matches.spaceMatchrecord[i].identity);
    ret = analyzematchline("Database",
                           matchline,
                           i,
                           matchinfo,
                           selectmatchparam);
    if(ret < 0)
    {
      dbmsbundle.matchbundle.freerecs(&matches);
      return (Sint) -10;
    }
  }
  dbmsbundle.matchbundle.freerecs(&matches);
  dbmsbundle.checkspaceleak ();
  return 0;
}
#endif

static Sint makestorematchtable(void *info,
                                /*@unused@*/ Multiseq *virtualmultiseq,
                                /*@unused@*/ Multiseq *queryvirtualtree,
                                StoreMatch *storematch)
{
  Matchinfo *matchinfo = (Matchinfo *) info;
  StoreMatch *ptr;

  GETNEXTFREEINARRAY(ptr,&matchinfo->matchtab,StoreMatch,1024);
  ptr->Storeflag = storematch->Storeflag;
  ptr->Storedistance = storematch->Storedistance;
  ptr->Storeposition1 = storematch->Storeposition1;
  ptr->Storelength1 = storematch->Storelength1;
  ptr->Storeposition2 = storematch->Storeposition2;
  ptr->Storelength2 = storematch->Storelength2;
  ptr->Storeseqnum2 = storematch->Storeseqnum2;
  ptr->Storerelpos2 = storematch->Storerelpos2;
  ptr->StoreEvalue = storematch->StoreEvalue;
  ptr->Storeseqnum1 = storematch->Storeseqnum1;
  ptr->Storerelpos1 = storematch->Storerelpos1;
  return 0;
}

/*EE
  The following function determines the matchinformation according to
  the given parameters. The parameters can have three different forms:
  \begin{itemize}
  \item
  If \texttt{dbparms} is not \texttt{NULL}, then \texttt{matchfile}
  points to a string representation of a database key used to identify
  stored data produced by a run of \texttt{vmatch}.
  \texttt{dbparms} contains options to connect to a database.
  See the corresponding manual. If something went wrong when connecting
  to the database or when the data retrieved from the database is corrupt,
  the function returns a negative error code.
  \item
  If \texttt{dbparms} is \texttt{NULL} and \texttt{matchfile} is not
  \texttt{NULL}, then \texttt{matchfile} points to a path of a matchfile.
  If the path is not valid, or the matchfile is corrupt, then a negative
  error code is returned.
  \item
  If \texttt{dbparms} and \texttt{matchfile} is \texttt{NULL}, then
  \texttt{argv} must be an argument vector of length \texttt{argc}
  containing valid options of the program \texttt{vmatch}.
  See the corresponding manual. If the options are not valid or
  something went wrong when computing the matches, the function 
  returns a negative error code.
  \end{itemize}
  In all cases, the matches computed or parsed from the matchfile or
  retrieved from database are stored in table \texttt{matchinfo->matchtab}.
  Further more, the remaining components of \texttt{matchinfo} are
  initialized properly.
*/

Sint determineMatchinfo(FILE *outfp,
                        Matchinfo *matchinfo,
                        Argctype argc,
                        const char **argv,
                        const char *matchfile,
                        Matchparam *selectmatchparam,
                        /*@unused@*/ void *dbparminfo,
                        BOOL withinputsequences,
                        Showverbose showverbose)
{
#ifdef VMATCHDB
  Databaseparms *dbparms = (Databaseparms *) dbparminfo;
#endif
  INITARRAY(&matchinfo->matchtab,StoreMatch);
  matchinfo->mfargs = NULL;
  matchinfo->largeheading = NULL;
  matchinfo->smallheading = NULL;
  makeemptyvirtualtree(&matchinfo->virtualtree);
  makeemptyvirtualtree(&matchinfo->queryvirtualtree);
  makeemptyvirtualtree(&matchinfo->dnavirtualtree);
  makeemptyvirtualtree(&matchinfo->sixframeofqueryvirtualtree);

#ifdef VMATCHDB
  if (dbparms != NULL && dbparms->usedatabase)
  {
    if(readMatchinfofromdb(argc,
                           argv,
                           outfp,
                           matchinfo,
                           matchfile,
                           selectmatchparam,
                           dbparms,
                           withinputsequences,
                           showverbose) != 0)
    {
      return (Sint) -1;
    }
    return 0; 
  }
#endif
  if(matchfile != NULL)
  {
    if(readMatchinfofromfile(argc,
                             argv,
                             outfp,
                             matchinfo,
                             matchfile,
                             selectmatchparam,
                             withinputsequences,
                             showverbose) != 0)
    {
      return (Sint) -2;
    }
    return 0; 
  }

  if(callvmatch(argc,
                argv,
                (void *) matchinfo,
                matchcallinfo2matchinfo,
                "makestorematchtable",
                makestorematchtable,
                showverbose,
                NULL,
                NULL,
                &matchinfo->virtualtree,
                &matchinfo->queryvirtualtree,
                &matchinfo->sixframeofqueryvirtualtree,
                &matchinfo->dnavirtualtree) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

/*EE
  The following function frees the space for a record of type 
  \texttt{matchinfo}.
*/

Sint freeMatchinfo(Matchinfo *matchinfo)
{
  Uint i;

  if(freevirtualtree(&matchinfo->virtualtree) != 0)
  {
    return (Sint) -1;
  }
  if(freevirtualtree(&matchinfo->queryvirtualtree) != 0)
  {
    return (Sint) -2;
  }
  if(freevirtualtree(&matchinfo->sixframeofqueryvirtualtree) != 0)
  {
    return (Sint) -3;
  }
  if(freevirtualtree(&matchinfo->dnavirtualtree) != 0)
  {
    return (Sint) -4;
  }
  for(i=0; i<matchinfo->numberofqueryfiles; i++)
  {
    FREESPACE(matchinfo->queryfiles[i]);
  }
  FREEARRAY(&matchinfo->matchtab,StoreMatch);
  FREESPACE(matchinfo->dnasymbolmapdnavsprot);
  FREESPACE(matchinfo->largeheading);
  FREESPACE(matchinfo->smallheading);
  FREESPACE(matchinfo->mfargs);
  return 0;
}
