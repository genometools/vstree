#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "queryext.h"
#include "vnodedef.h"

#include "qgram2code.c"

 void maxprefixmatchbinstepwithprevious(Virtualtree *virtualtree,Vnode *vnode,
                                        Vnode *branchfather,
                                        Uchar *query,Uint querylen);

 BOOL isbranching(Virtualtree *virtualtree,Vnode *vnode);

/*
  The following code fragment computes the integer code of the 
  current substring of length \texttt{prefixlength} of the query.
  If the substring contains a special symbol, the code is undefined.
*/

#define EVALUATECODE(VIRT,MAPPOWER)\
        bchar = (backptr < qsubstring) ? SEPARATOR : *backptr;\
        fchar = *(backptr + (VIRT)->prefixlength);\
        if(codeokay)\
        {\
          if(ISSPECIAL(fchar))\
          {\
            codeokay = False;\
          } else\
          {\
            if(numofchars == UintConst(4))\
            {\
              code = MULT4(code - MAPPOWER[(Uint) bchar]) |\
                     ((Uint) fchar);\
            } else\
            {\
              code = (code - MAPPOWER[(Uint) bchar])\
                     * numofchars;\
              code += (Uint) fchar;\
            }\
          }\
        } else\
        {\
          codeokay = qgram2code(&code,numofchars,\
                                (VIRT)->prefixlength,\
                                backptr+1);\
        }

#define RANKOFNEXTLEAF1(VIRT,LEAFRANK)\
        (VIRT)->stitab1[(VIRT)->suftab[LEAFRANK]+1]

#define LSFLEFT(VIRT,IND)  (VIRT)->lsftab[MULT2(IND)]
#define LSFRIGHT(VIRT,IND) (VIRT)->lsftab[MULT2(IND) + 1]

static Uint scanleft(Virtualtree *virtualtree,Uint offset,Uint i)
{ 
  Uint tmplcp, s;
  
  for(s=i; s > 0; s--)
  {
    EVALLCP(virtualtree,tmplcp,s);
    if(tmplcp < offset)
    {
      break;
    }
  }
  return s;
}

static BOOL canextendleft(Virtualtree *virtualtree,Uint offset,Uint i)
{ 
  Uint tmplcp;

  if(i > 0)
  {
    EVALLCP(virtualtree,tmplcp,i);
    if(tmplcp >= offset)
    {
      return True;
    }
  }
  return False;
}

static Uint scanright(Virtualtree *virtualtree,Uint offset,Uint i)
{                       
  Uint tmplcp, s;

  for(s=i+1; s <= virtualtree->multiseq.totallength; s++)
  {
    EVALLCP(virtualtree,tmplcp,s);
    if(tmplcp < offset)
    {
      break;
    }
  }
  return s-1;
}

static BOOL canextendright(Virtualtree *virtualtree,Uint offset,Uint i)
{                       
  Uint tmplcp;

  if(i < virtualtree->multiseq.totallength)
  {
    EVALLCP(virtualtree,tmplcp,i+1);
    if(tmplcp >= offset)
    {
      return True;
    }
  }
  return False;
}

#ifdef DEBUG
static void startatroot(Virtualtree *virtualtree,Uchar *qseptr,
                        Uint remaining,Vnode *vnode,PairUint *maxwit)
{
  vnode->offset = 0;
  vnode->left = 0;
  vnode->right = virtualtree->multiseq.totallength;
  findmaxprefixlen(virtualtree,vnode,qseptr,remaining,maxwit);
  vnode->offset = maxwit->uint0;
  vnode->left = scanleft(virtualtree,vnode->offset,maxwit->uint1);
  vnode->right = scanright(virtualtree,vnode->offset,maxwit->uint1);
}

static void checkvnode(Virtualtree *virtualtree,Uchar *qseptr,Uint remaining,
                       Vnode *vnode)
{
  Vnode tmpvnode;
  PairUint maxwit;
  
  startatroot(virtualtree,qseptr,remaining,&tmpvnode,&maxwit);
  if(tmpvnode.left != vnode->left)
  {
    fprintf(stderr,"tmpvnode.left = %lu != %lu = vnode->left\n",
                    (Showuint) tmpvnode.left,
                    (Showuint) vnode->left);
    exit(EXIT_FAILURE);
  }
  if(tmpvnode.right != vnode->right)
  {
    fprintf(stderr,"tmpvnode.right = %lu != %lu = vnode->right\n",
                    (Showuint) tmpvnode.right,
                    (Showuint) vnode->right);
    exit(EXIT_FAILURE);
  }
  if(tmpvnode.offset != vnode->offset)
  {
    fprintf(stderr,"tmpvnode.offset = %lu != %lu = vnode->offset\n",
                    (Showuint) tmpvnode.offset,(Showuint) vnode->offset);
    exit(EXIT_FAILURE);
  }
  DEBUG3(3,"(%lu,%lu,%lu)\n",(Showuint) vnode->offset,
                             (Showuint) vnode->left,
                             (Showuint) vnode->right);
}
#endif

Sint matchquerysubstring0(Virtualtree *virtualtree,
                          Uint *mappower,
                          Uint searchlength,
                          Querymatchextendfunction extendleftright,
                          /*@unused@*/ BOOL onlyleftmaximal,
                          Uint qseqnum,
                          Uchar *qsubstring,
                          Uint qseqlen,
                          void *substringinfo)
{
  BOOL codeokay = False;
  Uchar bchar,
        fchar,
        *backptr;   // pointer to character to the left of q-gram
  Uint remaining,   // remaining length of query from current position
       numofchars,  // number of effective characters
       code = 0,    // integer code of current word of length prefixlength
       *bckptr;     // points into bcktab
  Vnode vnode;      // Interval with offset in suffix array
  PairUint maxwit;  // (maximal prefix in interval vnode,
                    //  suffix witnessing the maximal prefix)

  if(qseqlen < searchlength)
  {
    return 0;
  }
  DEBUG3(2,"matchsubagainstvirt %lu %lu %lu\n",
          (Showuint) qseqnum,
          (Showuint) qseqlen,
          (Showuint) searchlength);
  vnode.offset = virtualtree->prefixlength;
  numofchars = virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(virtualtree,mappower);
    if(codeokay)
    {
      bckptr = virtualtree->bcktab + MULT2(code);
      vnode.left = *bckptr;  // left boundary of bucket
      if((vnode.right = *(bckptr+1)) > vnode.left)  // right boundary of bucket
      {
        vnode.right--;  // adjust right boundary
        findmaxprefixlen(virtualtree,&vnode,
                         backptr+1,remaining,&maxwit); // calculate the interval
        if(maxwit.uint0 >= searchlength)
        { // maximal prefix is long enough
          DEBUG1(3,"leftchar=%lu\n",(Showuint) bchar);
          if(extendleftright(vnode.left,
                             vnode.right,
                             maxwit.uint0,
                             maxwit.uint1,
                             bchar,
                             vnode.left,
                             vnode.right,
                             substringinfo,
                             qsubstring,
                             backptr+1,
                             qseqlen,
                             qseqnum) != 0)
          {
            return (Sint) -1;
          }
        } 
      }
    } 
  }
  return 0;
}

Sint matchquerysubstring1(Virtualtree *virtualtree,
                          Uint *mappower,
                          Uint searchlength,
                          Querymatchextendfunction extendleftright,
                          /*@unused@*/ BOOL onlyleftmaximal,
                          Uint qseqnum,
                          Uchar *qsubstring,
                          Uint qseqlen,
                          void *substringinfo)
{
  BOOL codeokay = False;
  Uchar bchar,
        fchar,
        *backptr;   // pointer to character to the left of q-gram
  Uint remaining,   // remaining length of query from current position
       numofchars,  // number of effective characters
       code = 0,    // integer code of current word of length prefixlength
       tmpisodepth, // temporary isomoprhism depth    
       *bckptr;     // points into bcktab
  Vnode vnode;      // Interval with offset in suffix array
  PairUint maxwit;  // (maximal prefix in interval vnode,
                    //  suffix witnessing the maximal prefix)
  BOOL processnext = True;

  if(qseqlen < searchlength)
  {
    return 0;
  }
  DEBUG3(2,"matchsubagainstvirt %lu %lu %lu\n",
            (Showuint) qseqnum,
            (Showuint) qseqlen,
            (Showuint) searchlength);
  vnode.offset = virtualtree->prefixlength;
  numofchars = virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(virtualtree,mappower);
    if(codeokay && processnext)
    {
      bckptr = virtualtree->bcktab + MULT2(code);
      vnode.left = *bckptr;  // left boundary of bucket
      if((vnode.right = *(bckptr+1)) > vnode.left)  // right boundary of bucket
      {
        vnode.right--;  // adjust right boundary
        findmaxprefixlen(virtualtree,&vnode,
                         backptr+1,remaining,&maxwit); // calculate the interval
        tmpisodepth = virtualtree->isodepthtab[maxwit.uint1];
        if(maxwit.uint0 >= searchlength)
        { // maximal prefix is long enough
          DEBUG1(3,"leftchar=%lu\n",(Showuint) bchar);
          if(extendleftright(vnode.left,
                             vnode.right,
                             maxwit.uint0,
                             maxwit.uint1,
                             bchar,
                             vnode.left,
                             vnode.right,
                             substringinfo,
                             qsubstring,
                             backptr+1,
                             qseqlen,
                             qseqnum) != 0)
          {
            return (Sint) -1;
          }
          if(tmpisodepth < UCHAR_MAX && 
             searchlength > tmpisodepth)
          {
#ifdef COUNT
            skipsuccesscount++;
#endif
            processnext = False;
          }
        } else
        {
          if(tmpisodepth < UCHAR_MAX && maxwit.uint0 > tmpisodepth)
          {
#ifdef COUNT
            skipfailcount++;
#endif
            processnext = False;
          }
        }
      }
    } else
    {
      processnext = True;
    }
  }
#ifdef COUNT
  printf("# skipfailcount=%lu (%.2f)\n",
          (Showuint) skipfailcount,
          (double) skipfailcount/querylength);
  printf("# skipsuccesscount=%lu (%.2f)\n",
          (Showuint) skipsuccesscount,
          (double) skipsuccesscount/querylength);
#endif
  return 0;
}

#ifdef SHOWLINKDIFF
static void sumoflargescans(ArrayUint *distribution)
{
  Uint sumof = 0, i;

  for(i=0; i < distribution->nextfreeUint; i++)
  {
    sumof += (i+1) * distribution->spaceUint[i];
  }
  printf("sumoflargescans=%lu\n",(Showuint) sumof);
}
#endif

Sint matchquerysubstring2(Virtualtree *virtualtree,
                          Uint *mappower,
                          Uint searchlength,
                          Querymatchextendfunction extendleftright,
                          BOOL onlyleftmaximal,
                          Uint qseqnum,
                          Uchar *qsubstring,
                          Uint qseqlen,
                          void *substringinfo)
{
  BOOL proceed,
       codeokay = False;
  Uchar fchar,
        bchar,
        rankval2,
        *backptr;  // pointer to character to the left of q-gram
  Uint bucketleft,
       remaining,
       rankleft, 
       rankright,
       code = 0,
       numofchars,
       *bckptr;
  Vnode vnode;      // Interval with offset in suffix array
  PairUint maxwit;  // (maximal prefix in interval vnode, suffix witnessing the
                    // maximal prefix)
#ifdef SHOWLINKDIFF
  ArrayUint distbucketdiffleft, distbucketdiffright;

  INITARRAY(&distbucketdiffleft,Uint);
  INITARRAY(&distbucketdiffright,Uint);
#endif
  vnode.offset = 0;
  numofchars = virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(virtualtree,mappower);
    if(!codeokay)
    {
      vnode.offset = 0;
    } else
    {
      bckptr = virtualtree->bcktab + MULT2(code);
      bucketleft = *bckptr;
      if(vnode.offset <= virtualtree->prefixlength ||
         ((rankval2 = RANKOFNEXTLEAF1(virtualtree,vnode.right))
            == UCHAR_MAX))
      {
        if(*(bckptr+1) > bucketleft)
        {
          vnode.right = *(bckptr+1) - 1;
          vnode.left = bucketleft;
          vnode.offset = virtualtree->prefixlength;
          findmaxprefixlen(virtualtree,&vnode,backptr+1,remaining,&maxwit);
          if(maxwit.uint0 > vnode.offset)
          {
            vnode.offset = maxwit.uint0;
            vnode.left = scanleft(virtualtree,vnode.offset,maxwit.uint1);
            vnode.right = scanright(virtualtree,vnode.offset,maxwit.uint1);
          }
          proceed = (maxwit.uint0 >= searchlength) 
                    ? True : False;
        } else
        {
          vnode.offset = maxwit.uint0 = 0;
          proceed = False;
        }
      } else
      {
        vnode.offset--;
        rankleft = scanleft(virtualtree,vnode.offset,
                            bucketleft + 
                            RANKOFNEXTLEAF1(virtualtree,vnode.left));
        rankright = scanright(virtualtree,vnode.offset,bucketleft + rankval2);
#ifdef SHOWLINKDIFF
        {
          Uint difference;
          
          difference = bucketleft + RANKOFNEXTLEAF1(virtualtree,
                                                    vnode.left) - rankleft;
          adddistribution(&distbucketdiffleft,difference);
          difference = rankright - (bucketleft + rankval2);
          adddistribution(&distbucketdiffright,difference);
        }
#endif
        if(rankright - rankleft == vnode.right - vnode.left)
        {
          vnode.left = rankleft;
          vnode.right = rankright;
          maxwit.uint0 = vnode.offset;
          maxwit.uint1 = bucketleft +
                         RANKOFNEXTLEAF1(virtualtree,maxwit.uint1);
          if(onlyleftmaximal)
          {
            if(maxwit.uint0 >= searchlength)
            {
              if(canextendleft(virtualtree,searchlength,vnode.left) ||
                 canextendright(virtualtree,searchlength,vnode.right))
              {
                proceed = True;
              } else
              {
                proceed = False;
              }
            } else
            {
              proceed = False;
            }
          } else
          {
            if(maxwit.uint0 >= searchlength)
            {
              proceed = True;
            } else
            {
              proceed = False;
            }
          }
#ifdef COUNT
            isomorphic++;
#endif
        } else
        {
          vnode.left = rankleft;
          vnode.right = rankright;
          findmaxprefixlen(virtualtree,&vnode,backptr+1,remaining,&maxwit);
          if(maxwit.uint0 > vnode.offset)
          {
            vnode.offset = maxwit.uint0;
            vnode.left = scanleft(virtualtree,vnode.offset,maxwit.uint1);
            vnode.right = scanright(virtualtree,vnode.offset,maxwit.uint1);
          }
          proceed = (maxwit.uint0 >= searchlength) 
                    ? True : False;
        }
      }
      DEBUGCODE(1,if(vnode.offset > 0)
                  {
                    checkvnode(virtualtree,backptr+1,remaining,&vnode);
                  });
      if(proceed)
      {
#undef MATCHINGSTATISTICS
#ifdef MATCHINGSTATISTICS
        printf("length=%lu,startpos1=%lu\n",
               (Showuint) vnode.offset,
               (Showuint) virtualtree->suftab[vnode.left]);
#else
        if(extendleftright(vnode.left,
                           vnode.right,
                           maxwit.uint0,
                           maxwit.uint1,
                           bchar,
                           0,
                           virtualtree->multiseq.totallength-1,
                           substringinfo,
                           qsubstring,
                           backptr+1,
                           qseqlen,
                           qseqnum) != 0)
        {
          return (Sint) -2;
        }
#endif
      }
    }
  }
#ifdef SHOWLINKDIFF
  printf("distbucketdiffleft\n");
  showdistribution(stdout,&distbucketdiffleft);
  printf("distbucketdiffright\n");
  showdistribution(stdout,&distbucketdiffright);
  sumoflargescans(&distbucketdiffleft);
  sumoflargescans(&distbucketdiffright);
  FREEARRAY(&distbucketdiffleft,Uint);
  FREEARRAY(&distbucketdiffright,Uint);
#endif
#ifdef COUNT
  printf("isomorphic=%lu\n",(Showuint) isomorphic);
#endif
  return 0;
}

Sint matchquerysubstring3(Virtualtree *virtualtree,
                          Uint *mappower,
                          Uint searchlength,
                          Querymatchextendfunction extendleftright,
                          /*@unused@*/ BOOL onlyleftmaximal,
                          Uint qseqnum,
                          Uchar *qsubstring,
                          Uint qseqlen,
                          void *substringinfo)
{
#ifdef COUNT
  Uint scanops = 0,
       addscan,
       maxaddscan = 0,
       countscanleft = 0,
       countscanright = 0;
#endif
  BOOL proceed,
       codeokay = False;
  Uchar fchar,
        bchar,
        rankval2,
        *backptr;  // pointer to character to the left of q-gram
  Uint bucketleft,
       remaining,
       rankleft, 
       rankright,
       code = 0,
       numofchars,
       *bckptr;
  Vnode vnode,      // Interval with offset in suffix array
        branchingfather;

  vnode.offset = 0;
  numofchars = virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(virtualtree,mappower);
    if(!codeokay)
    {
      vnode.offset = 0;
    } else
    {
      bckptr = virtualtree->bcktab + MULT2(code);
      bucketleft = *bckptr;
      if(vnode.offset <= virtualtree->prefixlength ||
         ((rankval2 = RANKOFNEXTLEAF1(virtualtree,vnode.right))
            == UCHAR_MAX))
      {
        if(*(bckptr+1) > bucketleft)
        {
          vnode.right = *(bckptr+1) - 1;
          vnode.left = bucketleft;
          vnode.offset = virtualtree->prefixlength;
          maxprefixmatchbinstepwithprevious(virtualtree,&vnode,
                                            &branchingfather,backptr+1,remaining);
          proceed = (vnode.offset >= searchlength) 
                    ? True : False;
        } else
        {
          vnode.offset = 0;
          branchingfather.offset = 0;
          proceed = False;
        }
      } else
      {
        vnode.offset--;
        rankleft = scanleft(virtualtree,vnode.offset,
                            bucketleft + 
                            RANKOFNEXTLEAF1(virtualtree,
                                            vnode.left));
        rankright = scanright(virtualtree,vnode.offset,
                              bucketleft + rankval2);
        printf("link (%lu,%lu)->(%lu,%lu)\n",
               (Showuint) vnode.left,
               (Showuint) vnode.right,
               (Showuint) rankleft,
               (Showuint) rankright);
        {
          Uint lcpleft, lcpright, lsfleft, lsfright, homevalue;
          if(branchingfather.offset > virtualtree->prefixlength)
          {
            DELIVERHOME(virtualtree,
                        homevalue,
                        lcpleft,
                        lcpright,
                        branchingfather.left,
                        branchingfather.right);
          if((lsfleft = LSFLEFT(virtualtree,homevalue))
                      < UCHAR_MAX &&
             (lsfright = LSFRIGHT(virtualtree,homevalue))
                       < UCHAR_MAX)
          {
            lsfleft += bucketleft;
            lsfright += lsfleft;
            if(branchingfather.offset-1 < vnode.offset)
            {
              Vnode tmpvnode;
              tmpvnode.offset = branchingfather.offset-1;
              tmpvnode.left = lsfleft;
              tmpvnode.right = lsfright;
              maxprefixmatchbinstepwithprevious(virtualtree,
                                                &tmpvnode,
                                                &branchingfather,backptr+1,
                                                vnode.offset);
              lsfleft = tmpvnode.left;
              lsfright = tmpvnode.right;
            }
            if(lsfleft != rankleft)
            {
              fprintf(stderr,"lsfleft=%lu != %lu=rankleft\n",
                              (Showuint) lsfleft,
                              (Showuint) rankleft);
              exit(EXIT_FAILURE);
            }
            if(lsfright != rankright)
            {
              fprintf(stderr,"lsfright=%lu != %lu=rankright\n",
                              (Showuint) lsfright,
                              (Showuint) rankright);
              exit(EXIT_FAILURE);
            }
          }
          }
        }
#ifdef COUNT
        scanops++;
        addscan = bucketleft +
                  RANKOFNEXTLEAF1(virtualtree,vnode.left) 
                  - rankleft;
        if(addscan > maxaddscan)
        {
          maxaddscan = addscan;
        }
        countscanleft += addscan;
        addscan = rankright - (bucketleft + rankval2);
        countscanright += addscan;
        if(addscan > maxaddscan)
        {
          maxaddscan = addscan;
        }
#endif
        if(rankright - rankleft == vnode.right - vnode.left)
        {
          vnode.left = rankleft;
          vnode.right = rankright;
          if(vnode.offset >= searchlength)
          {
            if(canextendleft(virtualtree,searchlength,vnode.left) ||
               canextendright(virtualtree,searchlength,vnode.right))
            {
              proceed = True;
            } else
            {
              proceed = False;
            }
          } else
          {
            proceed = False;
          }
#ifdef COUNT
          isomorphic++;
#endif
        } else
        {
          vnode.left = rankleft;
          vnode.right = rankright;
          maxprefixmatchbinstepwithprevious(virtualtree,&vnode,
                                            &branchingfather,backptr+1,remaining);
          proceed = (vnode.offset >= searchlength) 
                    ? True : False;
        }
      }
      DEBUGCODE(1,if(vnode.offset > 0)\
                  {\
                    checkvnode(virtualtree,backptr+1,\
                               remaining,\
                               &vnode);\
                  }\
                );\
      if(proceed)
      {
        if(extendleftright(vnode.left,
                           vnode.right,
                           vnode.offset,
                           vnode.left,
                           bchar,
                           0,
                           virtualtree->multiseq.totallength-1,
                           substringinfo,
                           qsubstring,
                           backptr+1,
                           qseqlen,
                           qseqnum) != 0)
        {
          return (Sint) -2;
        }
      }
    }
  }
#ifdef COUNT
  printf("isomorphic=%lu\n",(Showuint) isomorphic);
  printf("scanops=%lu (%.2f)\n",(Showuint) scanops,(double) scanops/qseqlen);
  printf("countscanright=%lu (%.2f)\n",(Showuint) countscanright,
                                       (double) countscanright/qseqlen);
  printf("maxaddscan = %lu\n",(Showuint) maxaddscan);
#endif
  return 0;
}

static BOOL getlsfvalue(Virtualtree *virtualtree,Uint bucketleft,
                        Vnode *branchingfather)
{
  Uint homevalue, lcpleft, lcpright;

  DELIVERHOME(virtualtree,homevalue,lcpleft,lcpright,
              branchingfather->left,branchingfather->right);
  if((branchingfather->left = LSFLEFT(virtualtree,homevalue)) == UCHAR_MAX)
  {
    return False;
  }
  if((branchingfather->right = LSFRIGHT(virtualtree,homevalue)) == UCHAR_MAX)
  {
    return False;
  }
  branchingfather->offset--;
  branchingfather->left += bucketleft;
  branchingfather->right += branchingfather->left;
  return True;
}

#undef CHECKCANONICAL
#ifdef CHECKCANONICAL

static void showvnode(FILE *fp,Vnode *vnode)
{
  fprintf(fp,"(%lu,%lu,%lu)",(Showuint) vnode->offset,
                             (Showuint) vnode->left,
                             (Showuint) vnode->right);
}

static void compareVnodes(Vnode *vnode1,Vnode *vnode2)
{
  if(vnode1->offset != vnode2->offset)
  {
    fprintf(stderr,"check vnode1=");
    showvnode(stderr,vnode1);
    fprintf(stderr," and   vnode2=");
    showvnode(stderr,vnode2);
    fprintf(stderr,"\n");
    fprintf(stderr,"offset incorrect: vnode1=%lu!=%lu=vnode2\n",
                                      (Showuint) vnode1->offset,
                                      (Showuint) vnode2->offset);
    exit(EXIT_FAILURE);
  }
  if(vnode1->left != vnode2->left)
  {
    fprintf(stderr,"check vnode1=");
    showvnode(stderr,vnode1);
    fprintf(stderr," and   vnode2=");
    showvnode(stderr,vnode2);
    fprintf(stderr,"\n");
    fprintf(stderr,"left: vnode1=%lu!=%lu=vnode2\n",
                          (Showuint) vnode1->left,
                          (Showuint) vnode2->left);
    exit(EXIT_FAILURE);
  }
  if(vnode1->right != vnode2->right)
  {
    fprintf(stderr,"check vnode1=");
    showvnode(stderr,vnode1);
    fprintf(stderr," and   vnode2=");
    showvnode(stderr,vnode2);
    fprintf(stderr,"\n");
    fprintf(stderr,"right: vnode1=%lu!=%lu=vnode2\n",
                           (Showuint) vnode1->right,
                           (Showuint) vnode2->right);
    exit(EXIT_FAILURE);
  }
}

#endif

Sint matchquerysubstring4(Virtualtree *virtualtree,
                          Uint *mappower,
                          Uint searchlength,
                          Querymatchextendfunction extendleftright,
                          /*@unused@*/ BOOL onlyleftmaximal,
                          Uint qseqnum,
                          Uchar *qsubstring,
                          Uint qseqlen,
                          void *substringinfo)
{
  BOOL proceed,
       codeokay = False;
  Uchar fchar,
        bchar,
        *backptr;  // pointer to character to the left of q-gram
  Uint bucketleft,
       remaining,
       code = 0,
       numofchars,
       offsetbuffer,
       *bckptr;
  Vnode vnode,      // Interval with offset in suffix array
        branchingfather;

  vnode.offset = 0;
  branchingfather.offset = 0;
  numofchars = virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(virtualtree,mappower);
    if(!codeokay)
    {
      vnode.offset = 0;
      branchingfather.offset = 0;
    } else
    {
      bckptr = virtualtree->bcktab + MULT2(code);
      bucketleft = *bckptr;
      if(vnode.offset <= virtualtree->prefixlength ||
         branchingfather.offset <= virtualtree->prefixlength ||
         !getlsfvalue(virtualtree,bucketleft,&branchingfather))
      {
        if(*(bckptr+1) > bucketleft)
        {
          vnode.right = *(bckptr+1) - 1;
          vnode.left = bucketleft;
          vnode.offset = virtualtree->prefixlength;
          if(isbranching(virtualtree,&vnode))
          {
            branchingfather = vnode;
          } else
          {
            branchingfather.offset = 0;
          }
          maxprefixmatchbinstepwithprevious(virtualtree,&vnode,
                                            &branchingfather,backptr+1,
                                            remaining);
          proceed = (vnode.offset >= searchlength) 
                    ? True : False;
        } else
        {
          vnode.offset = 0;
          branchingfather.offset = 0;
          proceed = False;
        }
      } else
      {
        vnode.offset--;
        if(branchingfather.offset < vnode.offset)
        {
          /* 
             the following step reaches level vnode.offset 
             it should reach this level in time proportional
             to the number of branching intervals visited.
          */
          offsetbuffer = vnode.offset;
          vnode = branchingfather;
          maxprefixmatchbinstepwithprevious(virtualtree,
                                            &vnode,&branchingfather,
                                            backptr+1,offsetbuffer);
        } else
        {
          vnode.left = branchingfather.left;
          vnode.right = branchingfather.right;
        }
        maxprefixmatchbinstepwithprevious(virtualtree,
                                          &vnode,&branchingfather,
                                          backptr+1,remaining);
        proceed = (vnode.offset >= searchlength) 
                  ? True : False;
#ifdef CHECKCANONICAL
        {
          if(*(bckptr+1) > bucketleft)
          {
            Vnode tmpvnode;
            tmpvnode.right = *(bckptr+1) - 1;
            tmpvnode.left = bucketleft;
            tmpvnode.offset = virtualtree->prefixlength;
            maxprefixmatchbinstep(virtualtree,
                                  &tmpvnode,backptr+1,remaining);
            compareVnodes(&vnode,&tmpvnode);
          }
        }
#endif
      }
      if(proceed)
      {
#ifdef MATCHINGSTATISTICS
        printf("%lu %lu\n",
               (Showuint) vnode.offset,
               (Showuint) virtualtree->suftab[vnode.left]
              );
#else
        if(extendleftright(vnode.left,
                           vnode.right,
                           vnode.offset,
                           vnode.left,
                           bchar,
                           0,
                           virtualtree->multiseq.totallength-1,
                           substringinfo,
                           qsubstring,
                           backptr+1,
                           qseqlen,
                           qseqnum) != 0)
        {
          return (Sint) -1;
        }
#endif
      }
    }
  }
  return 0;
}

Sint matchquerysubstring5(Virtualtree *virtualtree,
                          Uint *mappower,
                          Uint searchlength,
                          Querymatchextendfunction extendleftright,
                          /*@unused@*/ BOOL onlyleftmaximal,
                          Uint qseqnum,
                          Uchar *qsubstring,
                          Uint qseqlen,
                          void *substringinfo)
{
  BOOL codeokay = False;
  Uchar bchar,
        fchar,
        *backptr;   // pointer to character to the left of q-gram
  Uint remaining,   // remaining length of query from current position
       numofchars,  // number of effective characters
       code;        // integer code of current word of length prefixlength
  Vnode vnode;      // Interval with offset in suffix array
  PairUint maxwit;  // (maximal prefix in interval vnode,
                    //  suffix witnessing the maximal prefix)

  if(qseqlen < searchlength)
  {
    return 0;
  }
  DEBUG3(2,"matchsubagainstvirt %lu %lu %lu\n",
          (Showuint) qseqnum,
          (Showuint) qseqlen,
          (Showuint) searchlength);
  vnode.offset = 0;
  numofchars = virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(virtualtree,mappower);
    if(codeokay)
    {
      vnode.left = 0;
      vnode.right = virtualtree->multiseq.totallength-1;
      findmaxprefixlen(virtualtree,&vnode,
                       backptr+1,remaining,&maxwit); // calculate the interval
      if(maxwit.uint0 >= searchlength)
      { // maximal prefix is long enough
        DEBUG1(3,"leftchar=%lu\n",(Showuint) bchar);
        if(extendleftright(vnode.left,
                           vnode.right,
                           maxwit.uint0,
                           maxwit.uint1,
                           bchar,
                           vnode.left,
                           vnode.right,
                           substringinfo,
                           qsubstring,
                           backptr+1,
                           qseqlen,
                           qseqnum) != 0)
        {
          return (Sint) -1;
        } 
      }
    } 
  }
  return 0;
}

/*

#define MAXCASES 6
#define CHECKCANONICAL


static Sint matchsubagainstvirtspeedup4(void *info,Uint qseqnum,
                                        Uchar *qsubstring,Uint qseqlen)
{
  BOOL proceed,
       codeokay = False;
  Uchar fchar,
        bchar,
        XXrankval2,
        *backptr;  // pointer to character to the left of q-gram
  Uint bucketleft,
       remaining,
       XXrankleft, 
       XXrankright,
       code,
       numofchars,
       *bckptr;
  Vnode vnode,      // Interval with offset in suffix array
        branchingfather;
  Substringinfo *substringinfo = (Substringinfo *) info;
  Uint branchingfather = 0;
  Uint countcases[MAXCASES] = {0};

  vnode.offset = 0;
  branchingfather.offset = 0;
  numofchars = substringinfo->virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= substringinfo->searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(substringinfo->virtualtree,substringinfo->mappower);
    if(!codeokay)
    {
      vnode.offset = 0;
      branchingfather.offset = 0;
      countcases[0]++;
    } else
    {
      bckptr = substringinfo->virtualtree->bcktab + MULT2(code);
      bucketleft = *bckptr;
      if(vnode.offset <= substringinfo->virtualtree->prefixlength ||
         ((XXrankval2 = RANKOFNEXTLEAF1(substringinfo->virtualtree,vnode.right))
          == UCHAR_MAX))
      {
        if(*(bckptr+1) > bucketleft)
        {
          vnode.right = *(bckptr+1) - 1;
          vnode.left = bucketleft;
          vnode.offset = substringinfo->virtualtree->prefixlength;
          if(isbranching(substringinfo->virtualtree,&vnode))
          {
            branchingfather = vnode;
          } else
          {
            branchingfather.offset = 0;
            countcases[1]++;
          }
          maxprefixmatchbinstepwithprevious(substringinfo->virtualtree,&vnode,
                                            &branchingfather,backptr+1,
                                            remaining);
          proceed = (vnode.offset >= substringinfo->searchlength) 
                    ? True : False;
        } else
        {
          countcases[2]++;
          vnode.offset = 0;
          branchingfather.offset = 0;
          proceed = False;
        }
      } else
      {
        vnode.offset--;
        // here the constant time lookup happens
        if(branchingfather.offset > substringinfo->virtualtree->prefixlength)
        {
          Uint homevalue, lcpleft, lcpright, lsfleft, lsfright;
          DELIVERHOME(substringinfo->virtualtree,homevalue,
                      lcpleft,lcpright,branchingfather.left,
                      branchingfather.right);
          if((lsfleft = LSFLEFT(substringinfo->virtualtree,homevalue))
                      < UCHAR_MAX &&
             (lsfright = LSFRIGHT(substringinfo->virtualtree,homevalue))
                       < UCHAR_MAX)
          {
            lsfleft += bucketleft;
            lsfright += lsfleft;
            branchingfather.offset--;
            branchingfather.left = lsfleft;
            branchingfather.right = lsfright;
            if(branchingfather.offset < vnode.offset)
            {
              Vnode tmpvnode = branchingfather;
              maxprefixmatchbinstepwithprevious(substringinfo->virtualtree,
                                                &tmpvnode,&branchingfather,
                                                backptr+1,vnode.offset);
            }
          } else
          {
            countcases[3]++;
            branchingfather.offset = 0;
          }
        } else
        {
          countcases[4]++;
          branchingfather.offset = 0;
        }
        // here it ends
        XXrankleft = scanleft(substringinfo->virtualtree,vnode.offset,
                            bucketleft + 
                            RANKOFNEXTLEAF1(substringinfo->virtualtree,
                                            vnode.left));
        XXrankright = scanright(substringinfo->virtualtree,vnode.offset,
                              bucketleft + XXrankval2);
        if(XXrankright - XXrankleft == vnode.right - vnode.left)
        {
          vnode.left = XXrankleft;
          vnode.right = XXrankright;
          if(vnode.offset >= substringinfo->searchlength)
          {
            proceed = canextendleft(substringinfo->virtualtree,
                                    substringinfo->searchlength,vnode.left) ||
                      canextendright(substringinfo->virtualtree,
                                     substringinfo->searchlength,vnode.right);
          } else
          {
            proceed = False;
          }
        } else
        {
          vnode.left = XXrankleft;
          vnode.right = XXrankright;
          maxprefixmatchbinstepwithprevious(substringinfo->virtualtree,&vnode,
                                            &branchingfather,backptr+1,
                                            remaining);
          proceed = (vnode.offset >= substringinfo->searchlength) 
                    ? True : False;
        }
      }
#ifdef CHECKCANONICAL
      if(vnode.offset > 0)
      {
#ifdef SHOWCANONICAL
        printf("vnode=(%lu,%lu,%lu)\n",
                (Showuint) vnode.offset,
                (Showuint) vnode.left,
                (Showuint) vnode.right);
        if(branchingfather.offset == 0)
        {
          printf("branchingfather=root\n");
        } else
        {
          printf("branchingfather=(%lu,%lu,%lu)\n",
                (Showuint) branchingfather.offset,
                (Showuint) branchingfather.left,
                (Showuint) branchingfather.right);
        }
#endif
        if(branchingfather.offset == 0)
        {
          branchingfather++;
        } else
        {
          Vnode basenode, lastbranch;

#ifdef SHOWCANONICAL
          if(isbranching(substringinfo->virtualtree,&vnode))
          {
            compareVnodes(&vnode,&branchingfather);
          }
          if(branchingfather.offset > substringinfo->virtualtree->prefixlength)
          {
            if(!isbranching(substringinfo->virtualtree,&branchingfather))
            {
              fprintf(stderr,"branchingfather is not branching\n");
              exit(EXIT_FAILURE);
            }
          }
#endif
          basenode.offset = substringinfo->virtualtree->prefixlength;
          basenode.left = bucketleft;
          basenode.right = *(bckptr+1) - 1;
          lastbranch.offset = 0;
          maxprefixmatchbinstepwithprevious(substringinfo->virtualtree,
                                            &basenode,
                                            &lastbranch,backptr+1,remaining);
          if(lastbranch.offset > substringinfo->virtualtree->prefixlength)
          {
            compareVnodes(&lastbranch,&branchingfather);
            countcases[5]++;
          }
        }
      }
#endif
      // this goes in again
      if(proceed)
      {
        if(substringinfo->extendleftright(vnode.offset,
                                          vnode.left,
                                          bchar,
                                          0,
                                          substringinfo->virtualtree->
                                                         multiseq.totallength-1,
                                          substringinfo,
                                          qsubstring,
                                          backptr+1,
                                          qseqlen,
                                          qseqnum) != 0)
        {
          return (Sint) -2;
        }
      }
    }
  }
  printf("branchingfather in bucket = %lu",(Showuint) branchingfatherinbucket);
  printf("(%.2f)\n",(double) branchingfatherinbucket/qseqlen);
  {
    Uint casenum;
    for(casenum=0; casenum < MAXCASES; casenum++)
    {
      printf("case %lu: %lu ",(Showuint) casenum,
                              (Showuint) countcases[casenum]);
      printf("(%.2f)\n",(double) countcases[casenum]/qseqlen);
    }
  }
  return 0;
}

static Sint matchsubagainstvirtspeedup4(void *info,Uint qseqnum,
                                        Uchar *qsubstring,Uint qseqlen)
{
  BOOL proceed,
       usebucket,
       codeokay = False;
  Uchar fchar,
        bchar,
        *backptr;  // pointer to character to the left of q-gram
  Uint bucketleft,
       remaining,
       lsfleft, 
       lsfright,
       lcpleft,
       lcpright,
       homevalue,
       code,
       numofchars,
       *bckptr;
  Vnode vnode;      // Interval with offset in suffix array
  Substringinfo *substringinfo = (Substringinfo *) info;

  vnode.offset = 0;
  numofchars = substringinfo->virtualtree->alpha.mapsize - 1;
  for(backptr = qsubstring-1,
      remaining = qseqlen;
      remaining >= substringinfo->searchlength;
      backptr++, remaining--)
  {
    EVALUATECODE(substringinfo->virtualtree,substringinfo->mappower);
    if(!codeokay)
    {
      vnode.offset = 0;
    } else
    {
      bckptr = substringinfo->virtualtree->bcktab + MULT2(code);
      bucketleft = *bckptr;
      lsfleft = lsfright = UCHAR_MAX;
      usebucket = True;
      if(vnode.offset > substringinfo->virtualtree->prefixlength)
      {
        DELIVERHOME(substringinfo->virtualtree,homevalue,
                    lcpleft,lcpright,vnode.left,vnode.right);
        if((lsfleft = LSFLEFT(substringinfo->virtualtree,homevalue))
                    < UCHAR_MAX &&
           (lsfright = LSFRIGHT(substringinfo->virtualtree,homevalue))
                     < UCHAR_MAX)
        {
          lsfleft += bucketleft;
          lsfright += lsfleft;
          usebucket = False;
          printf("link (%lu,%lu)->(%lu,%lu)\n",
                 (Showuint) vnode.left,
                 (Showuint) vnode.right,
                 (Showuint) lsfleft,
                 (Showuint) lsfright);
        }
      }
      if(usebucket)
      {
        if(*(bckptr+1) > bucketleft)
        {
          vnode.right = *(bckptr+1) - 1;
          vnode.left = bucketleft;
          vnode.offset = substringinfo->virtualtree->prefixlength;
          maxprefixmatchmm(substringinfo->virtualtree,&vnode,
                         backptr+1,remaining);
          proceed = (vnode.offset >= substringinfo->searchlength) 
                    ? True : False;
        } else
        {
          vnode.offset = 0;
          proceed = False;
        }
      } else
      {
        vnode.offset--;
        if(lsfright - lsfleft == vnode.right - vnode.left)
        {
          vnode.left = lsfleft;
          vnode.right = lsfright;
          if(vnode.offset >= substringinfo->searchlength)
          {
            proceed = canextendleft(substringinfo->virtualtree,
                                    substringinfo->searchlength,vnode.left) ||
                      canextendright(substringinfo->virtualtree,
                                     substringinfo->searchlength,vnode.right);
          } else
          {
            proceed = False;
          }
#ifdef COUNT
          isomorphic++;
#endif
        } else
        {
          vnode.left = lsfleft;
          vnode.right = lsfright;
          maxprefixmatchmm(substringinfo->virtualtree,&vnode,
                         backptr+1,remaining);
          proceed = (vnode.offset >= substringinfo->searchlength) 
                    ? True : False;
        }
      }
      DEBUGCODE(1,if(vnode.offset > 0) checkvnode(substringinfo->virtualtree,backptr+1,remaining,
                             &vnode));
      if(proceed)
      {
        if(substringinfo->extendleftright(vnode.offset,
                                          vnode.left,
                                          bchar,
                                          0,
                                          substringinfo->virtualtree->
                                                         multiseq.totallength-1,
                                          substringinfo,
                                          qsubstring,
                                          backptr+1,
                                          qseqlen,
                                          qseqnum) != 0)
        {
          return (Sint) -2;
        }
      }
    }
  }
#ifdef COUNT
  printf("isomorphic=%lu\n",(Showuint) isomorphic);
#endif
  return 0;
}
*/
