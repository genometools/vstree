
#include "debugdef.h"
#include "vplugin-interface.h"
#include "vmotif-data.h"

static Sint init(/*@unused@*/ void *data)
{
#ifdef DEBUG
  if(getdebuglevel() == 0)
  {
    DEBUGLEVELSET;
  }
#endif /* DEBUG */
  DEBUG0(1,"# call vmotifinit\n");
  return 0;
}

static Sint adddemand(void *data)
{
  Vmotifdata *vmotifdata = (Vmotifdata *) data;

  vmotifdata->includedemand = 0;
  vmotifdata->excludedemand = 0;
  DEBUG0(1,"# call vmotifadddemand\n");
  return 0;
}

static Sint parse(void *data)
{
  Vmotifdata *vmotifdata = (Vmotifdata *) data;
  Uint filenum;

  DEBUG0(1,"# call vmotifparse\n");
  printf("# progname=%s\n",vmotifdata->progname);
  printf("# indexname=%s\n",vmotifdata->indexname);
  printf("# numberofqueryfiles=%lu\n",
         (Showuint) vmotifdata->numberofqueryfiles);
  for(filenum=0; filenum<vmotifdata->numberofqueryfiles; filenum++)
  {
    printf("# queryfile=\"%s\"\n",vmotifdata->queryfiles[filenum]);
  }
  printf("# forceonline=%s\n",SHOWBOOL(vmotifdata->forceonline));
  return 0;
}

static Sint search(void *data)
{
  Vmotifdata *vmotifdata = (Vmotifdata *) data;
  Match match;

  DEBUG0(1,"# call vmotifsearch\n");
  match.flag = FLAGCOMPLETEMATCH | FLAGQUERY | FLAGREGEXPMATCH; 
  match.distance = 0;
  match.position1 = 0;
  match.length1 = UintConst(20);
  match.position1 = 0;
  match.length2 = UintConst(20);
  match.seqnum2 = 0;
  match.relpos2 = 0;
  if(vmotifdata->processfinal(vmotifdata->voidMatchstate,&match) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint wrap(/*@unused@*/ void *data)
{
  DEBUG0(1,"# call vmotifwrap\n");
  return 0;
}

char vplugingetinterface(Uchar ptrsize, Uchar ifacesize, Vplugininterface *iface)
{
  VPLUGINCHECKSIZES(ptrsize,ifacesize);
  iface->vplugininit=init;
  iface->vpluginadddemand=adddemand;
  iface->vpluginparse=parse;
  iface->vpluginsearch=search;
  iface->vpluginwrap=wrap;
  return 0;
}
