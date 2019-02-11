#include "types.h"
#include "debugdef.h"
#include "arraydef.h"
#include "redblackdef.h"

#include "redblack.pr"

typedef struct hnode_t
{
  Uint symbol, count;
  struct hnode_t *leftchild, 
                 *rightchild;
} Huffmannode;

typedef struct
{
  Huffmannode *roothuffmantree;
  Uint insertednodes, largestsymbol,
       totalbits, totalnumofvalues;
  void *rbhuffroot;
} Huffmaninfo;

static Sint cmpHuffmannode(const Keytype keya,
                           const Keytype keyb,
                           /*@unused@*/ void *info)
{
  Huffmannode *h1 = (Huffmannode *) keya,
              *h2 = (Huffmannode *) keyb;
  
  if(h1->count < h2->count)
  {
     return (Sint) -1;
  }
  if(h1->count > h2->count)
  {
    return (Sint) 1;
  }
  if(h1->symbol < h2->symbol)
  {
    return (Sint) -1;
  }
  if(h1->symbol > h2->symbol)
  {
    return (Sint) 1;
  }
  return 0;
}

static void initialhuffmaninsert(Huffmaninfo *huffmaninfo,
                                 const ArrayUint *distribution)
{
  Huffmannode *huffptr;
  BOOL nodecreated;
  Uint i; 

  huffmaninfo->rbhuffroot = NULL;
  huffmaninfo->insertednodes = 0;
  for(i = 0; i < distribution->nextfreeUint; i++)
  {
    if(distribution->spaceUint[i] > 0)
    {
      huffptr = malloc(sizeof(Huffmannode));
      if(huffptr == NULL)
      {
        fprintf(stderr,"cannot allocate space for initial Huffmannode\n");
        exit(EXIT_FAILURE);
      }
      huffptr->count = distribution->spaceUint[i];
      huffptr->symbol = i;
      huffptr->leftchild = NULL;
      huffptr->rightchild = NULL;
      (void) redblacktreesearch ((Keytype) huffptr,
                                 &nodecreated,
                                 &huffmaninfo->rbhuffroot,
                                 cmpHuffmannode,
                                 NULL);
      huffmaninfo->insertednodes++;
      huffmaninfo->largestsymbol = i;
    }
  }
}

static Sint makehuffmantree(Huffmaninfo *huffmaninfo)
{
  Huffmannode *n1, *n2, *newnode = NULL;
  Uint i, currentsymbol = huffmaninfo->largestsymbol + 1;
  BOOL nodecreated;

  if(huffmaninfo->insertednodes == 0)
  {
    huffmaninfo->roothuffmantree = NULL;
    return (Sint) 0;
  }
  if(huffmaninfo->insertednodes == UintConst(1))
  {
    huffmaninfo->roothuffmantree
      = (Huffmannode *) extractrootkey(huffmaninfo->rbhuffroot);
  } else
  {
    for(i=0; i<huffmaninfo->insertednodes-1; i++)
    {
      n1 = redblacktreeminimumkey (huffmaninfo->rbhuffroot);
      NOTSUPPOSEDTOBENULL(n1);
      if(redblacktreedelete (n1,&huffmaninfo->rbhuffroot,
                             cmpHuffmannode,NULL) != 0)
      {
        return (Sint) -1;
      }
      n2 = redblacktreeminimumkey (huffmaninfo->rbhuffroot);
      NOTSUPPOSEDTOBENULL(n2);
      if(redblacktreedelete (n2,&huffmaninfo->rbhuffroot,
                             cmpHuffmannode,NULL) != 0)
      {
        return (Sint) -1;
      }
      newnode = malloc(sizeof(Huffmannode));
      if(newnode == NULL)
      {
        fprintf(stderr,"cannot allocate space for Huffmannode in trees\n");
        exit(EXIT_FAILURE);
      }
      newnode->count = n1->count + n2->count;
      if(n1->count < n2->count)
      {
        newnode->leftchild = n2;
        newnode->rightchild = n1;
      } else
      {
        newnode->leftchild = n1;
        newnode->rightchild = n2;
      }
      newnode->symbol = currentsymbol++;
      (void) redblacktreesearch (newnode,
                                 &nodecreated,
                                 &huffmaninfo->rbhuffroot,
                                 cmpHuffmannode,NULL);
    }
    huffmaninfo->roothuffmantree = (Huffmannode *) newnode;
  }
  return 0;
}

#ifdef DEBUG
static void showsinglehuffmancode(Uint length,Uint code)
{
  if(length > 0)
  {
    Uint leftbit;

    for(leftbit = UintConst(1) << (length-1);
        leftbit != 0;
        leftbit >>= 1)
    {
      (void) putchar((code & leftbit) ? '1' : '0');
    }
  }
}
#endif

static void recurseextractallhuffmancodes(Huffmaninfo *huffmaninfo,
                                          Uint length,Uint code,
                                          Huffmannode *hnode)
{
  if(hnode->leftchild == NULL)
  {
#ifdef DEBUG
    printf("symbol %lu, count %lu, codelength %lu: ",
             (Showuint) hnode->symbol,
             (Showuint) hnode->count,
             (Showuint) length);
    showsinglehuffmancode(length,code);
    printf("\n");
#endif
    huffmaninfo->totalbits += length * hnode->count;
    huffmaninfo->totalnumofvalues += hnode->count;
  } else
  {
    recurseextractallhuffmancodes(huffmaninfo,length+1,
                                  code << 1,hnode->leftchild);
    recurseextractallhuffmancodes(huffmaninfo,length+1,
                                  (code << 1) | UintConst(1),
                                  hnode->rightchild);
  }
}

static void extractallhuffmancodes(Huffmaninfo *huffmaninfo)
{
  if(huffmaninfo->roothuffmantree != NULL)
  {
    recurseextractallhuffmancodes(huffmaninfo,0,0,huffmaninfo->roothuffmantree);
  }
}

static void recursedestroyhufftree(Huffmannode *hnode)
{
  if(hnode->leftchild != NULL)
  {
    recursedestroyhufftree(hnode->leftchild);
  } 
  if(hnode->rightchild != NULL)
  {
    recursedestroyhufftree(hnode->rightchild);
  }
  free(hnode);
}

Sint huffmanencoding(Uint *totalbits,
                     Uint *totalnumofvalues,
                     const ArrayUint *distribution)
{
  Huffmaninfo huffmaninfo; 

  initialhuffmaninsert(&huffmaninfo,distribution);
  if(makehuffmantree(&huffmaninfo) != 0)
  {
    return (Sint) -1;
  }
  huffmaninfo.totalbits = 0;
  huffmaninfo.totalnumofvalues = 0;
  extractallhuffmancodes(&huffmaninfo);
  if(huffmaninfo.rbhuffroot != NULL)
  {
    recursedestroyhufftree((Huffmannode *) 
                           extractrootkey(huffmaninfo.rbhuffroot));
    free(huffmaninfo.rbhuffroot);
  }
  huffmaninfo.rbhuffroot = NULL;
  *totalbits = huffmaninfo.totalbits;
  *totalnumofvalues = huffmaninfo.totalnumofvalues;
  return 0;
}
