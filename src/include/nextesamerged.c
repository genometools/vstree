static Sint nextesamergedsuflcptabvalues(Uint *lcptabvalue,
                                         Indexedsuffix *indexedsuffix,
                                         Emissionmergedesa *emmesa)
{
  if(emmesa->buf.nextaccessidx >= emmesa->buf.nextstoreidx)
  {
    if(emmesa->numofentries == 0)
    {
      return 0;
    }
    if(stepdeleteandinsertothersuffixes(emmesa) != 0)
    {
      return (Sint) -1;
    }
    if(emmesa->buf.nextstoreidx == 0)
    {
      return 0;
    }
    emmesa->buf.nextaccessidx = 0;
  }
  *indexedsuffix = emmesa->buf.suftabstore[emmesa->buf.nextaccessidx];
  if(emmesa->buf.nextaccessidx < emmesa->buf.nextstoreidx-1 ||
     !emmesa->buf.lastpage)
  {
    *lcptabvalue = emmesa->buf.lcptabstore[emmesa->buf.nextaccessidx];
    emmesa->buf.nextaccessidx++;
    return (Sint) 1;
  }
  return 0;
}
