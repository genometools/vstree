#ifndef REDBLACKDEF_H
#define REDBLACKDEF_H

#include "types.h"

#define CHECKREDBLACKRETCODE\
        if(retcode < 0 || retcode == SintConst(1))\
        {\
          return retcode;\
        }

typedef enum
{
  preorder,
  postorder,
  endorder,
  leaf
} VISIT;
  
typedef void * Keytype;
#define Keytypeerror NULL

typedef Sint (*Dictcomparefunction)(const Keytype,const Keytype,void *);
typedef void (*Dictshowelem)(const Keytype,void *);
typedef Sint (*Dictaction)(const Keytype,VISIT,Uint,void *);
typedef void (*Freekeyfunction)(const Keytype,void *);
typedef BOOL (*Comparewithkey)(const Keytype,void *);

typedef struct
{
  Uint currentdictsize,                  // current size of the dictionary
       maxdictsize;                      // maximal size of the dictionary
  void *worstelement,                    // reference to worst key
       *root,                            // root of tree
       *lastcallinsertedelem,            // element inserted in last call
       *lastcalldeletedelem;             // element deleted in last call
} Dictmaxsize;

//\IgnoreLatex{

#endif

//}
