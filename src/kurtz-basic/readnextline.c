#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "arraydef.h"

/*EE
  The following functions read the next line from the given file pointer
  and stores it in the array \texttt{line}.
*/

Sint readnextline(FILE *fpin,ArrayUchar *line)
{
  Fgetcreturntype cc;

  while(True)
  {
    cc = getc(fpin);
    if(cc == EOF)
    {
      return (Sint) EOF;
    }
    if(cc == (Fgetcreturntype) '\n')
    {
      STOREINARRAY(line,Uchar,512,(Uchar) '\0');
      line->nextfreeUchar--;
      return 0;
    } 
    STOREINARRAY(line,Uchar,512,(Uchar) cc);
  }
}

Uint maximumlinelength(FILE *fpin)
{
  Sint retcode;
  ArrayUchar line;
  Uint maxvalue = 0;

  INITARRAY(&line,Uchar);
  while(True)
  {
    retcode = readnextline(fpin,&line);
    if(maxvalue < line.nextfreeUchar)
    {
      maxvalue = line.nextfreeUchar;
    }
    if(retcode == (Sint) EOF)
    {
      break;
    }
  }
  FREEARRAY(&line,Uchar);
  return maxvalue;
}
