#ifndef TRIEINS_DEF_H
#define TRIEINS_DEF_H

#include "types.h"
#include "arraydef.h"
#include "encseq-def.h"

typedef struct
{
  Uint idx,
       startpos;
#ifdef DEBUG
  Uint64 ident;
#endif
} Suffixinfo;

typedef struct _Trienode
{
  Suffixinfo suffixinfo;
  struct _Trienode *firstchild, 
                   *rightsibling,
                   *parent;
  Uint depth;
} Trienode;

typedef Trienode * Trienodeptr;

DECLAREARRAYSTRUCT(Trienodeptr);

typedef struct
{
  Encodedsequence *encseqtable;
  Trienode *nodetable, 
           *root;
  Trienodeptr *unusedTrienodes;
  Uint numofindexes,
       nextunused,
       allocatedTrienode, 
       nextfreeTrienode;
} Trierep;

#endif
