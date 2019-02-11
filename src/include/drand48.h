#ifndef DRAND48_H
#define DRAND48_H

/* only for Windows, on UNIX theses functions are defined in <stdlib.h> */
#ifdef _WIN32
void   srand48(long int seedval);
double drand48(void);
#endif

#endif
