
#include <string.h>
#include "errordef.h"
#include "vmcldef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "outinfo.h"
#include "intbits.h"
#include "genfile.h"
#include "fhandledef.h"

#include "multiseq-adv.pr"

#include "procargs.pr"
#include "echomatch.pr"
#include "cluster.pr"
#include "filehandle.pr"

static Sint clshowseqnum(void *info,Uint seqnum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;
  
  fprintf(clusterinfo->outinfo.outfp," %lu",(Showuint) seqnum);
  return 0;
}

static Sint clshowdesc(void *info,Uint seqnum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;
  
  fprintf(clusterinfo->outinfo.outfp,"  ");
  if(clusterinfo->clusterparms.nonredundantfile != NULL)
  {
    fprintf(clusterinfo->outinfo.outfp,"%lu: ",(Showuint) seqnum);
  }
  if(clusterinfo->virtualmultiseq->descspace.spaceUchar == NULL)
  {
    fprintf(clusterinfo->outinfo.outfp,"sequence%lu",(Showuint) seqnum);
  } else
  {
    if(clusterinfo->outinfo.showdesc.defined)
    {
      echothedescription(clusterinfo->outinfo.outfp,
                         &clusterinfo->outinfo.showdesc,
			 clusterinfo->virtualmultiseq,
                         seqnum);
    } else
    {
      echothedescription(clusterinfo->outinfo.outfp,
                         &clusterinfo->defaultshowdesc,
                         clusterinfo->virtualmultiseq,
                         seqnum);
    }
  }
  fprintf(clusterinfo->outinfo.outfp,"\n");
  return 0;
}

static Sint clshowclusternum(void *info,Uint cnum,/*@unused@*/ Uint csize)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  fprintf(clusterinfo->outinfo.outfp,"%lu: ",(Showuint) cnum);
  return 0;
}

static Sint clshownewline(void *info,/*@unused@*/ Uint cnum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  fprintf(clusterinfo->outinfo.outfp,"\n");
  return 0;
}

static Sint clshowclusternumwithnewline(void *info,Uint cnum,
                                        /*@unused@*/ Uint csize)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  fprintf(clusterinfo->outinfo.outfp,"%lu:\n",(Showuint) cnum);
  return 0;
}

static Sint clopenfile(void *info,Uint cnum,Uint csize)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  sprintf(&clusterinfo->dbmatchfileoutname[0],"%s.%lu.%lu.match",
          clusterinfo->clusterparms.dbclusterfilenameprefix,
          (Showuint) csize,
          (Showuint) cnum);
  clusterinfo->dbmatchfileoutptr 
    = CREATEFILEHANDLE(&clusterinfo->dbmatchfileoutname[0],WRITEMODE);
  if(clusterinfo->dbmatchfileoutptr == NULL)
  {
    return (Sint) -1;
  }
  if(showargumentline(NULL,
                      clusterinfo->dbmatchfileoutptr,
                      clusterinfo->args,
                      0,
                      NULL) != 0)
  {
    return (Sint) -2;
  }
  if(clusterinfo->outinfo.showstring > 0)
  {
    sprintf(&clusterinfo->dbfnafileoutname[0],"%s.%lu.%lu.fna",
            clusterinfo->clusterparms.dbclusterfilenameprefix,
            (Showuint) csize,
            (Showuint) cnum);
    clusterinfo->dbfnafileoutptr 
      = CREATEFILEHANDLE(&clusterinfo->dbfnafileoutname[0],WRITEMODE);
    if(clusterinfo->dbfnafileoutptr == NULL)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

static Sint clformatseq(Sequenceclusterinfo *clusterinfo,Uint seqnum)
{
  PairUint range;

  if(findboundaries(clusterinfo->virtualmultiseq,seqnum,&range) != 0)
  {
    return (Sint) -1;
  }
  if(formatseq(clusterinfo->dbfnafileoutptr,
               0,
               NULL,
               0,
               (clusterinfo->outinfo.showstring > 0) 
                 ? (clusterinfo->outinfo.showstring & MAXLINEWIDTH)
                 : DEFAULTLINEWIDTH,
               clusterinfo->virtualmultiseq->originalsequence + range.uint0,
               range.uint1 - range.uint0 + 1) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

static Sint clshowsequence(void *info,Uint seqnum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  if(clusterinfo->outinfo.showstring > 0)
  {
    fprintf(clusterinfo->dbfnafileoutptr,">");
    echothedescription(clusterinfo->dbfnafileoutptr,
                       &clusterinfo->defaultshowdesc,
                       clusterinfo->virtualmultiseq,
                       seqnum);
    fprintf(clusterinfo->dbfnafileoutptr,"\n");
    if(clformatseq(clusterinfo,seqnum) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint clclosefile(void *info,/*@unused@*/ Uint cnum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  if(DELETEFILEHANDLE(clusterinfo->dbmatchfileoutptr) != 0)
  {
    return (Sint) -1;
  }
  if(clusterinfo->outinfo.showstring > 0)
  {
    if(DELETEFILEHANDLE(clusterinfo->dbfnafileoutptr) != 0)
    {
      return (Sint) -2;
    }
  }
  return 0;
}

static Sint clshowedge(void *info1,/*@unused@*/ void *info2,Uint edgenum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info1;

  if(echomatch2file(clusterinfo->dbmatchfileoutptr,
                    False,
                    clusterinfo->outinfo.showmode,
                    &clusterinfo->outinfo.showdesc,
                    0,
                    &clusterinfo->outinfo.digits,
                    clusterinfo->virtualmultiseq,
                    NULL,
                    NULL, // virtualalpha
                    clusterinfo->matchtab.spaceStoreMatch + edgenum) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint clcmpsequencelength(void *cmpinfo,Uint elem1,Uint elem2)
{
  PairUint range;
  Uint len1, len2;
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) cmpinfo;

  if(findboundaries(clusterinfo->virtualmultiseq,elem1,&range) != 0)
  {
    return (Sint) -2;
  }
  len1 = range.uint1 - range.uint0 + 1;
  if(findboundaries(clusterinfo->virtualmultiseq,elem2,&range) != 0)
  {
    return (Sint) -2;
  }
  len2 = range.uint1 - range.uint0 + 1;
  /*
  printf("elem1=%lu,len1=%lu,elem2=%lu,len2=%lu\n",(unsigned long) elem1,
                                                   (unsigned long) len1,
                                                   (unsigned long) elem2,
                                                   (unsigned long) len2);
  */
  if (len1 < len2)
  {
    return (Sint) -1; 
  }
  if (len1 > len2)
  {
    return (Sint) 1;
  }
  return 0;
}

static Sint clshowfastastart(void *info,/*@unused@*/ Uint clnum,
                             /*@unused@*/ Uint csize)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  clusterinfo->firstelemincluster = True;
  fprintf(clusterinfo->dbfnafileoutptr,">");
  return 0;
}

static Sint clshowfirstsequence(void *info,Uint seqnum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  if(clusterinfo->firstelemincluster)
  {
    if(clusterinfo->outinfo.showdesc.defined)
    {
      echothedescription(clusterinfo->dbfnafileoutptr,
                         &clusterinfo->outinfo.showdesc,
                         clusterinfo->virtualmultiseq,seqnum);
    } else
    {
      echothedescription(clusterinfo->dbfnafileoutptr,
                         &clusterinfo->defaultshowdesc,
                         clusterinfo->virtualmultiseq,seqnum);
    }
    fprintf(clusterinfo->dbfnafileoutptr,"\n");
    if(clformatseq(clusterinfo,seqnum) != 0)
    {
      return (Sint) -1;
    }
  }
  clusterinfo->firstelemincluster = False;
  return 0;
}

static Sint clshowsinglesequence(void *info,Uint seqnum)
{
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) info;

  fprintf(clusterinfo->dbfnafileoutptr,">");
  echothedescription(clusterinfo->dbfnafileoutptr,
                     &clusterinfo->defaultshowdesc,
                     clusterinfo->virtualmultiseq,seqnum);
  fprintf(clusterinfo->dbfnafileoutptr,"\n");
  if(clformatseq(clusterinfo,seqnum) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static BOOL sufficientoverlap(Uint matchlength,Uint seqlen,Uint percentage)
{
  Uint sufflen;

  sufflen = seqlen * percentage/100;
  return (matchlength >= sufflen) ? True : False;
}

Sint initvmcluster(Sequenceclusterinfo *clusterinfo,
                   Clusterparms *clusterparms,
                   Outinfo *mcoutinfo,
                   char *args,
                   Multiseq *virtualmultiseq)
{
  clusterinfo->clusterparms = *clusterparms;

  COPYOUTINFO(&clusterinfo->outinfo,mcoutinfo);
  INITARRAY(&clusterinfo->matchtab,StoreMatch);
  clusterinfo->virtualmultiseq = virtualmultiseq;
  clusterinfo->args = args;
  ASSIGNDEFAULTSHOWDESC(&clusterinfo->defaultshowdesc);
  if(virtualmultiseq->numofsequences == UintConst(1))
  {
    ERROR0("option -dbcluster only possible for index "
           "with at least two sequences");
    return (Sint) -1;
  }
  if(HASINDEXEDQUERIES(virtualmultiseq))
  {
    ERROR0("option -dbcluster requires index without query sequences");
    return (Sint) -2;
  }
  initClusterSet(&clusterinfo->clusterset,virtualmultiseq->numofsequences);
  return 0;
}

void showclusterparms(Clusterparms *clusterparms,Uint userdefinedleastlength,
                      Uint identity,Uint leastscore)
{
  printf("dbcluster=(%lu,%lu)\n",(Showuint) clusterparms->dbclpercsmall,
                                 (Showuint) clusterparms->dbclperclarge);
  printf("dbclminsize=%lu\n",(Showuint) clusterparms->dbclminsize);
  if(clusterparms->dbclmaxsize == DBCLMAXSIZE)
  {
    printf("dbclmaxsize=oo\n");
  } else
  {
    printf("dbclmaxsize=%lu\n",(Showuint) clusterparms->dbclmaxsize);
  }
  if(clusterparms->dbclusterfilenameprefix == NULL)
  {
    printf("dbclusterfilenameprefix=NONE\n");
  } else
  {
    printf("dbclusterfilenameprefix=%s\n",
            clusterparms->dbclusterfilenameprefix);
  }
  if(clusterparms->nonredundantfile == NULL)
  {
    printf("nonredundantfile=NONE\n");
  } else
  {
    printf("nonredundantfile=%s\n",clusterparms->nonredundantfile);
  }
  printf("userdefinedleastlength=%lu\n",(Showuint) userdefinedleastlength);
  printf("identity=%lu\n",(Showuint) identity);
  printf("leastscore=%lu\n",(Showuint) leastscore);
}

Sint addvmcluster(void *showmatchinfo,
                  Multiseq *virtualmultiseq,
                  /*@unused@*/ Multiseq *multiseq,
                  StoreMatch *storematch)
{
  Uint seqlensmall, seqlenlarge, tmp, mlengthmin;
  PairUint range;
  StoreMatch *mspaceptr;
  Sequenceclusterinfo *clusterinfo = (Sequenceclusterinfo *) showmatchinfo;

  if(storematch->Storeseqnum1 == storematch->Storeseqnum2)
  {
    return 0;
  }
  if(findboundaries(virtualmultiseq,
                    storematch->Storeseqnum1,
                    &range) != 0)
  {
    return (Sint) -1;
  }
  seqlensmall = range.uint1 - range.uint0 + 1;
  if(findboundaries(virtualmultiseq,storematch->Storeseqnum2,
                    &range) != 0)
  {
    return (Sint) -2;
  }
  seqlenlarge = range.uint1 - range.uint0 + 1;
  if(seqlensmall > seqlenlarge)
  {
    tmp = seqlensmall;
    seqlensmall = seqlenlarge;
    seqlenlarge = tmp;
  }
  if(storematch->Storelength1 < storematch->Storelength2)
  {
    mlengthmin = storematch->Storelength1;
  } else
  {
    mlengthmin = storematch->Storelength2;
  }
  if(sufficientoverlap(mlengthmin,seqlensmall,
                       (Uint) clusterinfo->clusterparms.dbclpercsmall) &&
     sufficientoverlap(mlengthmin,seqlenlarge,
                       (Uint) clusterinfo->clusterparms.dbclperclarge))
  {
    if(clusterinfo->clusterparms.dbclusterfilenameprefix != NULL)
    {
      GETNEXTFREEINARRAY(mspaceptr,&clusterinfo->matchtab,StoreMatch,1024);
      ASSIGNSTOREMATCH(mspaceptr,storematch);
    }
    if(linkcluster(&clusterinfo->clusterset,storematch->Storeseqnum1,
                                            storematch->Storeseqnum2) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

Sint processvmcluster(Sequenceclusterinfo *clusterinfo,
                      Showverbose showverbose)
{
  clusterSizedistribution(&clusterinfo->clusterset);
  if(clusterinfo->clusterparms.nonredundantfile == NULL && 
     !clusterinfo->outinfo.showdesc.defined)
  {
    if(showClusterSet(&clusterinfo->clusterset,0,
                      MAXCLUSTERSIZE(&clusterinfo->clusterset),
                      (void *) clusterinfo,clshowclusternum,
                      (void *) clusterinfo,clshowseqnum,
                      (void *) clusterinfo,clshownewline,
                      NULL,NULL) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(showClusterSet(&clusterinfo->clusterset,
                      0,
                      MAXCLUSTERSIZE(&clusterinfo->clusterset),
                      (void *) clusterinfo,clshowclusternumwithnewline,
                      (void *) clusterinfo,clshowdesc,
                      NULL,NULL,
                      NULL,NULL) != 0)
    {
      return (Sint) -2;
    }
  }
  if(clusterinfo->clusterparms.dbclusterfilenameprefix != NULL)
  {
    Uint edgenum, dbclmaxsize;

    if(showverbose != NULL)
    {
      char sbuf[256+2*PATH_MAX+1];
      if(clusterinfo->outinfo.showstring > 0)
      {
        sprintf(sbuf,"output cluster information to files \"%s.s.i.fna\" "
                     "and \"%s.s.i.match\" where i is the cluster number " 
                     "and s is the size of the cluster",
                clusterinfo->clusterparms.dbclusterfilenameprefix,
                clusterinfo->clusterparms.dbclusterfilenameprefix);
      } else
      {
        sprintf(sbuf,"output cluster information to files \"%s.s.i.match\" "
                     "where i is the cluster number and s is the "
                     "size of the cluster",
                clusterinfo->clusterparms.dbclusterfilenameprefix);
      } 
      showverbose(sbuf);
      if(clusterinfo->clusterparms.dbclmaxsize != DBCLMAXSIZE)
      {
        sprintf(sbuf,"only clusters of size between %lu and %lu are reported",
                      (Showuint) clusterinfo->clusterparms.dbclminsize,
                      (Showuint) clusterinfo->clusterparms.dbclmaxsize);
        showverbose(sbuf);
      }
    }
    for(edgenum=0; edgenum < clusterinfo->matchtab.nextfreeStoreMatch;
                   edgenum++)
    {
      if(addClusterEdge(&clusterinfo->clusterset,
                        clusterinfo->matchtab.spaceStoreMatch[edgenum].
                                             Storeseqnum1,
                        clusterinfo->matchtab.spaceStoreMatch[edgenum].
                                             Storeseqnum2,edgenum) != 0)
      {
        return (Sint) -3;
      }
    }
    clusterinfo->dbfnafileoutptr = NULL;
    clusterinfo->dbmatchfileoutptr = NULL;
    if(clusterinfo->clusterparms.dbclmaxsize == DBCLMAXSIZE)
    {
      dbclmaxsize = clusterinfo->clusterset.numofelems;
    } else
    {
      dbclmaxsize = clusterinfo->clusterparms.dbclmaxsize;
    }
    if(showClusterSet(&clusterinfo->clusterset,
                      clusterinfo->clusterparms.dbclminsize,
                      dbclmaxsize,
                      (void *) clusterinfo,clopenfile,
                      (void *) clusterinfo,clshowsequence,
                      (void *) clusterinfo,clclosefile,
                      NULL,clshowedge) != 0)
    {
      return (Sint) -4;
    }
    if(clusterinfo->matchtab.nextfreeStoreMatch != 
       clusterinfo->clusterset.numofedges)
    {
      ERROR2("number %lu of stored matches differs from number %lu of edges used for clustering",
            (Showuint) clusterinfo->matchtab.nextfreeStoreMatch,
            (Showuint) clusterinfo->clusterset.numofedges);
      return (Sint) -5;
    }
    if(clusterinfo->outinfo.showstring > 0 && 
       clusterinfo->clusterparms.dbclminsize == UintConst(1))
    {
      if(clusterinfo->clusterparms.nonredundantfile == NULL)
      {
        sprintf(&clusterinfo->dbfnafileoutname[0],
                "%s.single.fna",
                clusterinfo->clusterparms.dbclusterfilenameprefix);
        clusterinfo->dbfnafileoutptr 
          = CREATEFILEHANDLE(&clusterinfo->dbfnafileoutname[0],WRITEMODE);
        if(clusterinfo->dbfnafileoutptr == NULL)
        {
          return (Sint) -6;
        }
        if(showSingletonSet(&clusterinfo->clusterset,(void *) clusterinfo,
                            clshowsinglesequence) != 0)
        {
          return (Sint) -7;
        }
        if(DELETEFILEHANDLE(clusterinfo->dbfnafileoutptr) != 0)
        {
          return (Sint) -8;
        }
        clusterinfo->dbfnafileoutptr = NULL;
      }
    }
  }
  if(clusterinfo->clusterparms.nonredundantfile != NULL)
  {
    clusterinfo->dbfnafileoutptr 
      = CREATEFILEHANDLE(clusterinfo->clusterparms.nonredundantfile,WRITEMODE);
    if(clusterinfo->dbfnafileoutptr == NULL)
    {
      return (Sint) -9;
    }
    if(showClusterSetwithmaxelem(&clusterinfo->clusterset,0,
                                 MAXCLUSTERSIZE(&clusterinfo->clusterset),
                                 (void *) clusterinfo,clshowfastastart,
                                 (void *) clusterinfo,clcmpsequencelength,
                                 (void *) clusterinfo,clshowfirstsequence,
                                 (void *) clusterinfo,NULL,
                                 NULL,NULL) != 0)
    {
      return (Sint) -10;
    }
    if(showSingletonSet(&clusterinfo->clusterset,
                        (void *) clusterinfo,clshowsinglesequence) != 0)
    {
      return (Sint) -11;
    }
    if(DELETEFILEHANDLE(clusterinfo->dbfnafileoutptr) != 0)
    {
      return (Sint) -12;
    }
    clusterinfo->dbfnafileoutptr = NULL;
  }
  return 0;
}

void freevmcluster(Sequenceclusterinfo *clusterinfo)
{
  freeClusterSet(&clusterinfo->clusterset);
  FREEARRAY(&clusterinfo->matchtab,StoreMatch);
  FREESPACE(clusterinfo->clusterparms.dbclusterfilenameprefix);
  FREESPACE(clusterinfo->clusterparms.nonredundantfile);
}
