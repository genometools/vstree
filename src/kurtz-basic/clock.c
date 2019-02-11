//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <time.h>
#include "types.h"

//}

/*
  This module implements function to measures the running time
  of programs or program parts.
*/

/*
  The following values store the the clockticks at start time 
  and stop time of the clock.
*/

static clock_t startclock, 
               stopclock;

/*EE
  The following function initializes the clock.
*/

void initclock(void)
{ 
  startclock = clock(); 
}

/*EE
  The following function delivers the time since the 
  clock was initialized. The time is reported in seconds
  as a floating point value.
*/

double getruntime(void)
{
   stopclock = clock();
   return (double) (stopclock-startclock) / (double) CLOCKS_PER_SEC;
}

/*EE
  The following function delivers the clock ticks betwenn 
  \texttt{startclock} to \texttt{stopclock}.
*/

Uint getclockticks(void)
{
   stopclock = clock();
   return (Uint) (stopclock-startclock);
}
