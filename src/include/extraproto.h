#ifndef EXTRAPROTO_H
#define EXTRAPROTO_H
/* This file is generated. Do not edit. */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
Uint ceillog2(Uint v);
void checkord(Uchar *tlast,Uchar **pos);
void checkordindex(Uchar *text,Uint last,Uint *pos);
void checkneighbors(Uchar *text,Uint last,Uint left,Uint right);
void showsuffixes(Uchar *t,Uint n,Uchar **pos);
int callsort(int argc,char *argv[],Uchar *(*sortit)(Uchar *,Uint,Uint *));
BOOL emptyline(char *s);
void makefailure(char *p, Uint m, Uint *failure);
void floorlog(Uint k, Uint n, Uint *m, Uint *power);
int pamscore(int nvalue,char a,char b);
void getAlpha (char *s, char *alpha, Uint *alphaSize);
void getUchars(Uchar *text, Uint textlen, Uchar *alpha, Uint *alphasize);
void printAlpha (char *s);
Uint makemap(Uchar *t, Uint n, Uint *map);
void getalphabet(Uchar *t, Uint n, Uchar *alphabet);
Uint makemapandalphabet(Uchar *t, Uint n,Uchar *alphabet,Uint *map);
double getmedian(int *count, int datapoints);
void itermedian(int datapoints);
void nextResult(Uint *result, Uint *r, Uint value);
Uint ceilprime(Uint value);
void quicksort (long *xs, long left, long right);
long longrandom(long range);
void initrandom(void);
void rstringm(char *alpha, long asize, long m, char *s);
long rstringregion(char *alpha, long asize, long minlen, long maxlen, char *s);
long rthreshold(long mink, long maxk);
int fileOpen(char *name, Uint *textlen, BOOL writefile);
caddr_t fileParts(int fd,Uint offset,Uint len,BOOL writemap);
void freetextspace(Uchar *text, Uint textlen);
caddr_t genfile2String(char *name, Uint *textlen,
                       BOOL writefile, BOOL writemap);
caddr_t file2String(char *name, Uint *textlen);
int initnewresultlist(void);
int storenextresult(Uint listid,Uint pos);
int showresultlist(Uint listid);
Uint *getresultlist(Uint listid,Uint *noofresults);
void wrapresultlist(Uint listid);
void showList (Uint *result, Uint len);
void showstr(FILE *fp,Uchar *tlast,Uchar *left,Uchar *right);
char *showchar(Uchar c);
double getvariance(short *count, int stretch, int datapoints, double avg);
#endif
