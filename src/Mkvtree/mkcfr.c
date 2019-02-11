#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "divmodmul.h"
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "minmax.h"
#include "arraydef.h"
#include "spacedef.h"
#include "vnodedef.h"
#include "fhandledef.h"

#include "outindextab.pr"
#include "accvirt.pr"
#include "readvirt.pr"
#include "safescpy.pr"
#include "filehandle.pr"

/*
  This file contains the function makereversetable for the construction of 
  the reverse tables in the affix affix array data structure
*/

DEFINEEVALLCP

#define SUFFIXLEN(S)  (virtualtree->multiseq.totallength - (S))
#define SUFFIX(S)     (virtualtree->multiseq.sequence + (S))
#define RPREFIX(S)    (virtualtree->multiseq.sequence + \
                       (virtualtree->multiseq.totallength-1-(S)))

#define SEQOFFSET(V) V->multiseq.sequence

#define TEXTPOS(V,F,I)\
        ((F)? (SEQOFFSET(V)+I) : (SEQOFFSET(V)+(V)->multiseq.totallength-1-(I)))

#define COMP(I,LCPSTART,PDIR,TDIR)\
        if (PDIR) \
        {\
          if (TDIR)\
          {\
            COMPARE1(I,LCPSTART)\
          } else\
	  {\
            COMPARE2(I,LCPSTART)\
          }\
        } else\
        {\
          if (TDIR)\
          {\
            REVCOMPARE1(I,LCPSTART)\
          } else\
	  {\
            REVCOMPARE2(I,LCPSTART)\
          }\
        }

//pattern forward, text forward

#define COMPARE1(I,LCPSTART) \
        start = virtualtree->suftab[I];\
        slen = SUFFIXLEN(start);\
        s = SUFFIX(start);\
        len = MIN(wlen+leftstart,slen);\
        for(lcplen=LCPSTART; lcplen<len; lcplen++)\
        {\
          if(w[lcplen-leftstart] < s[lcplen])\
          {\
            retcode = (Sint) -1;\
            break;\
          }\
          if(w[lcplen-leftstart] > s[lcplen])\
          {\
            retcode = (Sint) 1;\
            break;\
          }\
        }\
        if(lcplen == len)\
        {\
          if(wlen+leftstart <= slen)\
          {\
            retcode = 0;\
          } else\
          {\
            retcode = (Sint) -1;\
          }\
        }


//pattern forward, text backward

#define COMPARE2(I,LCPSTART) \
	start = virtualtree->suftab[I];\
	slen = SUFFIXLEN(start);\
        s = RPREFIX(start);\
        len = MIN(wlen+leftstart,slen);\
        for(lcplen=LCPSTART; lcplen<len; lcplen++)\
	{\
	  if(w[lcplen-leftstart] < s[-lcplen])\
	  {\
            retcode = (Sint) -1;\
            break;\
          }\
          if(w[lcplen-leftstart] > s[-lcplen])\
          {\
            retcode = (Sint) 1;\
            break;\
          }\
        }\
        if(lcplen == len)\
        {\
          if(wlen+leftstart <= slen)\
          {\
            retcode = 0;\
          } else\
          {\
            retcode = (Sint) -1;\
	  }\
        }

//pattern backward, text forward

#define REVCOMPARE1(I,LCPSTART)\
        start = virtualtree->suftab[I];\
        slen = SUFFIXLEN(start);\
        s = SUFFIX(start);\
        len = MIN(wlen+leftstart,slen);\
        for(lcplen=LCPSTART; lcplen<len; lcplen++)\
        {\
          if(w[leftstart-lcplen] < s[lcplen])\
          {\
            retcode = (Sint) -1;\
            break;\
          }\
          if(w[leftstart-lcplen] > s[lcplen])\
          {\
            retcode = (Sint) 1;\
            break;\
          }\
        }\
        if(lcplen == len)\
        {\
          if(wlen+leftstart <= slen)\
          {\
            retcode = 0;\
          } else\
          {\
            retcode = (Sint) -1;\
          }\
        }
      
// pattern backward, text backward

#define REVCOMPARE2(I,LCPSTART)\
	start = virtualtree->suftab[I];\
	slen = SUFFIXLEN(start);\
        s = RPREFIX(start);\
        len = MIN(wlen+leftstart,slen);\
        for(lcplen=LCPSTART; lcplen<len; lcplen++)\
	{\
	  if(w[leftstart-lcplen] < s[-lcplen])\
	  {\
            retcode = (Sint) -1;\
            break;\
          }\
          if(w[leftstart-lcplen] > s[-lcplen])\
          {\
            retcode = (Sint) 1;\
            break;\
          }\
        }\
        if(lcplen == len)\
        {\
          if(wlen+leftstart <= slen)\
          {\
            retcode = 0;\
          } else\
          {\
            retcode = (Sint) -1;\
          }\
        }

static void showonstdout(char *s)
{
  printf("%s\n",s);
}

/*
  The following function determines the left and right border of the lcp-
  interval representing the pattern \texttt{w} of length \texttt{wlen}
  in the virtual suffix tree \texttt{virtualtree} by binary search.
  The start interval of the search is given by \texttt{vnode->left}
  and {vnode->right}, 
  the result is given by an update of \texttt{vnode->left} and 
  \texttt{vnode->right}.
  \texttt{pdir} indicates the orientation of the pattern (1 means forward,
  0 means backward).
  \texttt{tdir} indicates the orientation of the text (1 means forward, 
  0 means backward).
*/  
 
static BOOL mmsearchvstree(Virtualtree *virtualtree,Vnode *vnode, Uint lcontext,
		           Uint matchlength, Uchar *w,Uint wlen,
                           BOOL pdir, BOOL tdir)
{
  Uint leftstart, left, leftsave, mid, right, start, lcplen, slen, 
       len, lpref, rpref;
  Uchar *s;
  Sint retcode = 0;

  leftstart = lcontext+matchlength;
  leftsave = left = vnode->left;
  
  right = vnode->right;

  COMP(left,leftstart,pdir,tdir);
 
  if(retcode > 0)
  {
    lpref = lcplen;

    COMP(right,leftstart,pdir,tdir);
    if(retcode > 0)
    {
      return False;
    } else
    {
      rpref = lcplen;
      while(right > left + 1)
      {
        mid = DIV2(left+right);
	COMP(mid,MIN(lpref,rpref),pdir,tdir);
        if(retcode <= 0)
        {
          right = mid;
          rpref = lcplen;
        } else
        {
          left = mid;
          lpref = lcplen;
        }
      }
      vnode->left = right;
    }
  }

  left = leftsave;
  right = vnode->right;

  COMP(left,leftstart,pdir,tdir);

  if(retcode < 0)
  {
    return False;
  } else
  {
    lpref = lcplen;
    COMP(right,leftstart,pdir,tdir);

    if(retcode >= 0)
    {
      vnode->right = right;
    } else
    {
      rpref = lcplen;
      while(right > left + 1)
      {
        mid = DIV2(left+right);
        COMP(mid,MIN(lpref,rpref),pdir,tdir);
        if(retcode >= 0)
        {
          left = mid;
          lpref = lcplen;
        } else
        {
          right = mid;
          rpref = lcplen;
        }
      }
      vnode->right = left;
    }
  }

  if (vnode->left>vnode->right) return False;

  return True;
}

#define PUSH(P)  STOREINARRAY(&lcpstack,PairUint,storesize,P)
#define POP   lcpstack.spacePairUint[--lcpstack.nextfreePairUint]

/*EE
  The following function returns the home of the interval [left,right]
  in the data structure sufarray/ given data structure sufarray
*/

static Uint gethome(Virtualtree *vtree, Uint left, Uint right)
{
  if (evallcp(vtree,left) >= evallcp(vtree, right+1))
    return(left);
  else
    return(right);
}

/*EE
  The following function generates a table revtab (incl. memory allocation)
  (type: Uint,size:sarray.totallength) so that revtab[gethome(I)] is set
  correctly for every lcp-interval I of \texttt{suftree}.
  The result of makereversetab is a pointer to the new table.
*/

static Sint makereversetable(Uint *revtab,Virtualtree *suftree,
                             Virtualtree *preftree, BOOL sufflag)
{
  Uint i, lcpval, left, right, maxlcp=0,
       storesize = UintConst(100),
       textlength = suftree->multiseq.totallength-1;
                   // this field is not allowed
                   // always use multiseq.totallength
  Vnode vnode;
  PairUint lcppair;
   
  ArrayPairUint lcpstack;
  INITARRAY(&lcpstack,PairUint);
  
  lcppair.uint0=0;
  lcppair.uint1=0;
  PUSH(lcppair);
  for (i=UintConst(1);i<=textlength+UintConst(1);i++)
  {
    if (evallcp(suftree,i)>maxlcp)
    {
      lcppair.uint0=i-1;
      maxlcp=evallcp(suftree,i);
      lcppair.uint1=maxlcp;
      PUSH(lcppair);
    }
    else
    {
      left=0;
      right=i-1;
      while (evallcp(suftree,i)<maxlcp)
      {
	lcppair=POP;
	//borders of lcp-interval:
	left=lcppair.uint0;
	lcpval=lcppair.uint1;

	//always start with complete preftree-interval: [0,totallength]
	vnode.left=0;
	vnode.right=suftree->multiseq.totallength-1;
	vnode.offset=0;

	if (!mmsearchvstree(preftree,&vnode,0,0,
			    TEXTPOS(suftree,sufflag,
				    suftree->suftab[left]+lcpval-1),
			    lcpval,sufflag ? False : True,
                                   sufflag ? False : True))
        {
	  ERROR2("String not found while constructing REVTAB. interval borders [%ld %ld]", 
                 (Showsint) vnode.left,
                 (Showsint) vnode.right);
          return (Sint) -1;
        }
	revtab[gethome(suftree,left,right)]= vnode.left;
	maxlcp=lcpstack.spacePairUint[lcpstack.nextfreePairUint-1].uint1;
      }	
      if (evallcp(suftree,i)>maxlcp)
      {
	lcppair.uint0=left;
	maxlcp=evallcp(suftree,i);
	lcppair.uint1=maxlcp;
	PUSH(lcppair);
      }      
    }
  }
  FREEARRAY(&lcpstack,PairUint);
  return 0;
}

MAINFUNCTION
{
  Virtualtree fvirt, rvirt;
  Uint *revtab;
  char findexname[PATH_MAX+1], 
       rindexname[PATH_MAX+4+1];

  DEBUGLEVELSET;

  VSTREECHECKARGNUM(2,"indexname");
  if(safestringcopy(&findexname[0],argv[1],PATH_MAX) != 0)
  {
    STANDARDMESSAGE;
  }
  if(mapvirtualtreeifyoucan(&fvirt,&findexname[0],
                            TISTAB | SUFTAB | LCPTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(showvirtualtreestatus(&fvirt,&findexname[0],showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  if(safestringcopy(&rindexname[0],argv[1],PATH_MAX) != 0)
  {
    STANDARDMESSAGE;
  }
  strcat(&rindexname[0],".rev");
  if(mapvirtualtreeifyoucan(&rvirt,&rindexname[0],
                            SUFTAB | LCPTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(showvirtualtreestatus(&rvirt,&rindexname[0],showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  ALLOCASSIGNSPACE(revtab,NULL,Uint,fvirt.multiseq.totallength);
  rvirt.multiseq.sequence = fvirt.multiseq.sequence;
  if(makereversetable(revtab,&fvirt,&rvirt,True) != 0)
  {
    STANDARDMESSAGE;
  }
  if(outindextab(&findexname[0],"cfr",(void *) revtab,(Uint) sizeof(Uint),
     fvirt.multiseq.totallength) != 0)
  {
    STANDARDMESSAGE;
  }
  if(makereversetable(revtab,&rvirt,&fvirt,False) != 0)
  {
    STANDARDMESSAGE;
  }
  if(outindextab(&rindexname[0],"crf",(void *) revtab,(Uint) sizeof(Uint),
                 fvirt.multiseq.totallength) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(revtab);
  if(freevirtualtree(&fvirt) != 0)
  {
    STANDARDMESSAGE;
  }
  rvirt.multiseq.sequence = NULL;
  if(freevirtualtree(&rvirt) != 0)
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
