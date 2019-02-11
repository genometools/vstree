#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include "types.h"
#include "debugdef.h"
#include "megabytes.h"

/*EE
  The following function delivers the space limit of the machine 
  in megabytes. This only works if the variable
  \texttt{WITHSYSCONF} is defined. This is currently the case for
  Linux and Solaris.
*/

#ifdef WITHSYSCONF
#ifdef DEBUG
void showmemsize(void)
{
  Sint pagesize = (Sint) sysconf((Sysconfargtype) _SC_PAGESIZE);
  Sint physpages = (Sint) sysconf((Sysconfargtype) _SC_PHYS_PAGES);

  DEBUG1(1,"# pagesize = %ld\n",(Showsint) pagesize);
  DEBUG1(1,"# number of physical pages = %ld\n",(Showsint) physpages);
  DEBUG1(1,"# memory size = %.0f MB\n",MEGABYTES(pagesize * physpages));
}
#endif /* WITHSYSCONF */
#endif
