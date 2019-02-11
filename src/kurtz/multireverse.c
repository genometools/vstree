#include "multidef.h"
#include "multiseq.pr"
#include "reverse.pr"

static Sint reversesinglesequence(/*@unused@*/ void *info,
                                  /*@unused@*/ Uint seqnum,
                                  Uchar *start,
                                  Uint len)
{
  reverseinplace(start,len);
  return 0;
}


Sint multiseqreverse(Multiseq *multiseq)
{
  return overallsequences(False,multiseq,NULL,reversesinglesequence);
}

static Sint complementsinglesequence(/*@unused@*/ void *info,
                                     /*@unused@*/ Uint seqnum,
                                     Uchar *start,
                                     Uint len)
{
  return onlycomplement(start,len);
}

Sint multiseqonlycomplement(Multiseq *multiseq)
{
  return overallsequences(False,multiseq,NULL,complementsinglesequence);
}

static Sint reversecomplementsinglesequence(/*@unused@*/ void *info,
                                            /*@unused@*/ Uint seqnum,
                                            Uchar *start,
                                            Uint len)
{
  return reversecomplementinplace(start,len);
}

Sint multiseqreversecomplement(Multiseq *multiseq)
{
  return overallsequences(False,multiseq,NULL,reversecomplementsinglesequence);
}
