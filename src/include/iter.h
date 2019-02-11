

#ifndef ITER_H
#define ITER_H
#ifdef ITERATIONS
#define ITER(N) ((N) < 10000) ? 1000 : (((N) < 100000) ? 100 : 10)
#else
#define ITER(N) 1
#endif
#endif
