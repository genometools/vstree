
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "errordef.h"
#include "debugdef.h"
#include "spacedef.h"
#include "qualint.h"

#include "dstrdup.pr"

#define ERRORLPARAM\
        ERROR5("argument \"%s\" of option %s must be positive number "\
               "possibly followed by character %c or %c; if the number is "\
               "followed by character %c, then it must be <= 100",\
               lparam,option,BESTCHARACTER,\
               PERCENTAWAYCHARACTER,\
               PERCENTAWAYCHARACTER);

Sint parsequalifiedinteger(Qualifiedinteger *qualint,
                           const char *option,
                           const char *lparam)
{
  Scaninteger readint;
  Uint i;
  char *lparamcopy;
  
  ASSIGNDYNAMICSTRDUP(lparamcopy,lparam);
  for(i=0; lparamcopy[i] != '\0'; i++)
  {
    if(!isdigit((Ctypeargumenttype) lparamcopy[i]) && 
       lparamcopy[i] != BESTCHARACTER &&
       lparamcopy[i] != PERCENTAWAYCHARACTER)
    {
      ERRORLPARAM;
      return (Sint) -1;
    }
  }
  if(i == 0)
  {
    ERRORLPARAM;
    return (Sint) -2;
  }
  if(lparamcopy[i-1] == BESTCHARACTER)
  {
    lparamcopy[i-1] = '\0';
    qualint->qualtag = Qualbestof;
  } else
  {
    if(lparamcopy[i-1] == PERCENTAWAYCHARACTER)
    {
      lparamcopy[i-1] = '\0';
      qualint->qualtag = Qualpercentaway;
    } else
    {
      qualint->qualtag = Qualabsolute;
    }
  }
  if(sscanf(lparamcopy,"%ld",&readint) != 1 || readint <= 0)
  {
    ERRORLPARAM;
    return (Sint) -3;
  }
  if(qualint->qualtag == Qualpercentaway || qualint->qualtag == Qualbestof)
  {
    if(readint > (Scaninteger) 100)
    {
      ERRORLPARAM;
      return (Sint) -4;
    }
  }
  qualint->integervalue = (Uint) readint;
  FREESPACE(lparamcopy);
  return 0;
}
