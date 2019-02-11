static Sint nextesastreamsuflcptabvalues(Uint *lcptabvalue,
                                         Uint *suftabvalue,
                                         Esastream *esastream)
{
  Sint retval;
  Uchar smalltmplcpvalue;
  
  retval = readnextUchar(&smalltmplcpvalue,&esastream->lcptabstream);
  if(retval < 0)
  {
    return (Sint) -1;
  }
  if(retval == 0)
  {
    return 0;
  }
  if(smalltmplcpvalue == UCHAR_MAX)
  {
    PairUint exceptionpair;

    retval = readnextPairUint(&exceptionpair,&esastream->llvtabstream);
    if(retval < 0)
    {
      return (Sint) -2;
    }
    if(retval == 0)
    {
      ERROREOF("llvtab");
      return (Sint) -3;
    }
    *lcptabvalue = exceptionpair.uint1;
  } else
  {
    *lcptabvalue = (Uint) smalltmplcpvalue;
  }
  retval = readnextUint(suftabvalue,&esastream->suftabstream);
  if(retval < 0)
  {
    return (Sint) -4; 
  }
  if(retval == 0)
  {
    ERROREOF("suftab");
    return (Sint) -5;
  }
  return (Sint) 1;
}

