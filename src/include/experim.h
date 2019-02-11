#ifndef EXPERIM_H
#define EXPERIM_H
#ifndef ITER
#define ITER 1
#endif
#define SHOWTIME(P)\
        printf("TIME %s %s %.2f\n",P,filename,getruntime()/(double) ITER)

#ifdef NOSPACEBOOKKEEPING
#define SHOWSPACE(P) /* Nothing */
#define SHOWMMSPACE(P) /* Nothing */
#else
#define SHOWSPACE(P)\
        printf("SPACE %s %s %lu\n",P,filename,(Showuint) getspacepeak())

#define SHOWMMSPACE(P)\
        printf("MMSPACE %s %s %lu\n",P,filename,(Showuint) mmgetspacepeak());

#endif

#define ITERATE(S)   { Uint i; for(i=0; i<ITER; i++) { S } }

#endif
