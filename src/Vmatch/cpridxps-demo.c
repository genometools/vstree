
#include "debugdef.h"
#include "vplugin-interface.h"
#include "cpridx-data.h"

static Sint init(/*@unused@*/ void *data)
{
#ifdef DEBUG
  if(getdebuglevel() == 0)
  {
    DEBUGLEVELSET;
  }
#endif /* DEBUG */
  DEBUG0(1,"# call cpridxpsinit\n");
  return 0;
}

static Sint adddemand(void *data)
{
  Cpridxpatsearchdata *cpridxpsdata = (Cpridxpatsearchdata *) data;

  cpridxpsdata->includedemand = 0;
  cpridxpsdata->excludedemand = 0;
  DEBUG0(1,"# call cpridxpsadddemand\n");
  return 0;
}

static Sint parse(/*@unused@*/ void *data)
{
  DEBUG0(1,"# call cpridxpsparse\n");
  return 0;
}

static Sint search(void *data)
{
  Cpridxpatsearchdata *cpridxpsdata = (Cpridxpatsearchdata *) data;
  Match match;

  DEBUG0(1,"# call cpridxpssearch\n");
  printf("search for pattern number %lu of length %lu\n",
           (Showuint) cpridxpsdata->seqnum2,
           (Showuint) cpridxpsdata->plen);
  match.flag = FLAGCOMPLETEMATCH | FLAGQUERY;
  match.distance = 0;
  match.position1 = 0;
  match.length1 = UintConst(20);
  match.position1 = 0;
  match.length2 = UintConst(30);
  match.seqnum2 = 0;
  match.relpos2 = 0;
  if(cpridxpsdata->processfinal(cpridxpsdata->voidMatchstate,&match) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint wrap(/*@unused@*/ void *data)
{
  DEBUG0(1,"# call cpridxpswrap\n");
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
