static Sint cmpmatches(Evalues *evalues,
                       Sint distance1,
                       Sint distance2,
                       Uint length11,
                       Uint length21,
                       __attribute__ ((unused)) Uint pos11,
                       __attribute__ ((unused)) Uint pos21)
{
  double identity1, identity2;
  Evaluetype evalue1, evalue2;

  if(evalues == NULL)
  {
    evalue1 = 0.0;
    evalue2 = 0.0;
  } else
  {
    evalue1 = incgetEvalue(evalues,1.0,distance1,length11);
    evalue2 = incgetEvalue(evalues,1.0,distance2,length21);
  }
  if(evalue1 < evalue2)
  {
    return (Sint) -1;
  }
  if(evalue1 > evalue2)
  {
    return (Sint) 1;
  }
  EVALIDENTITY(identity1,distance1,length11,length11);
  EVALIDENTITY(identity2,distance2,length21,length21);
  if(identity1 < identity2)
  {
    return (Sint) 1;
  }
  if(identity1 > identity2)
  {
    return (Sint) -1;
  }
  if(length11 < length21)
  {
    return (Sint) 1;
  }
  if(length11 > length21)
  {
    return (Sint) -1;
  }
  /*
  if(pos11 < pos21)
  {
    return (Sint) -1;
  }
  */
  return (Sint) 1;
}
