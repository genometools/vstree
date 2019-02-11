

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"
#include "spacedef.h"
#include "debugdef.h"
#include "errordef.h"

#define MAXIT 150    /* Maximum number of iterations used in calculating K */

static Sint gcd (Sint a, Sint b)
{
  Sint c;

  if (b < 0)
  {
    b = -b;
  }
  if (b > a)
  {
    c = a;
    a = b;
    b = c;
  }
  for (; b > 0; b = c)
  {
    c = a % b;
    a = b;
  }
  return a;
}

static Sint karlinpp(Sint low, Sint high, double *pr, 
                     double *lambda, double *K)
{
  Sint i, j, range, lo, hi, first, last;
  double upval, Sumval, av, sum, *p, *P, *ptrP, *ptr1, *ptr2, *ptr1e, newval,
         Ktmp;

  /* Check that scores and their associated probabilities are valid     */

  if (low >= 0)
  {
    ERROR1("Lowest score %ld must be negative",(Showsint) low);
    return (Sint) -1;
  }
  for (i = range = high - low; i > -low && !pr[i]; --i)
    /* Nothing */ ;
  if (i <= -low)
  {
    ERROR0("A positive score must be possible");
    return (Sint) -2;
  }
  for (sum = 0.0, i = 0; i <= range; sum += pr[i++])
  {
    if (pr[i] < 0.0)
    {
      ERROR1("Negative probability %.2f not allowed",pr[i]);
      return (Sint) -3;
    }
  }
  if (sum < 0.99995 || sum > 1.00005)
  {
    DEBUG1(3,"Probabilities sum to %.4f. Normalizing.\n", sum);
  }
  ALLOCASSIGNSPACE(p,NULL,double,(Uint) (range+1));
  if(p == NULL)
  {
    return (Sint) -4;
  }
  for (Sumval = (double) low, i = 0; i <= range; ++i)
  {
    Sumval += i * (p[i] = pr[i] / sum);
  }
  if(Sumval >= 0.0)
  {
    ERROR1("Invalid (non-negative) expected score:  %.3f", Sumval);
    return (Sint) -5;
  }

  /* Calculate the parameter lambda */

  upval = 0.5;
  do
  {
    upval *= 2;
    ptr1 = p;
    for (sum = 0.0, i = low; i <= high; ++i)
    {
      sum += *ptr1++ * exp (upval * i);
    }
  } while (sum < 1.0);
  for (*lambda = 0.0, j = 0; j < (Sint) 25; ++j)
  {
    newval = (*lambda + upval) / 2.0;
    ptr1 = p;
    for (sum = 0.0, i = low; i <= high; ++i)
    {
      sum += *ptr1++ * exp (newval * i);
    }
    if (sum > 1.0)
    {
      upval = newval;
    } else
    {
      *lambda = newval;
    }
  }

  /* Calculate the pamameter K */

  ptr1 = p;
  for (av = 0.0, i = low; i <= high; ++i)
  {
    av += *ptr1++ * i * exp (*lambda * i);
  }
  if (low == (Sint) -1 || high == (Sint) 1)
  {
    *K = (high == (Sint) 1) ? av : Sumval * Sumval / av;
    *K *= 1.0 - exp (-*lambda);
    free (p);
    return 0;  /* Parameters calculated successfully */
  }
  Sumval = 0.0;
  lo = 0;
  hi = 0;
  ALLOCASSIGNSPACE(P,NULL,double,(Uint) (MAXIT * range + 1));
  if(P == NULL)
  {
    return (Sint) -6;
  }
  for (*P = 1.0, sum = 1.0, j = (Sint) 1; 
       j <= (Sint) MAXIT && sum > 0.00001; Sumval += sum /= j++)
  {
    first = last = range;
    for (ptrP = P + (hi += high) - (lo += low); ptrP >= P; *ptrP-- = sum)
    {
      ptr1 = ptrP - first;
      ptr1e = ptrP - last;
      ptr2 = p + first;
      for (sum = 0.0; ptr1 >= ptr1e;)
      {
        sum += *ptr1-- * *ptr2++;
      }
      if (first)
      {
        --first;
      }
      if (((Sint) (ptrP - P)) <= range)
      {
        --last;
      }
    }
    for (sum = 0.0, i = lo; i; ++i)
    {
      sum += *++ptrP * exp (*lambda * i);
    }
    for (; i <= hi; ++i)
    {
      sum += *++ptrP;
    }
  }
  if (j > (Sint) MAXIT)
  {
    ERROR0("Value for K may be too large due to insufficient iterations");
    return (Sint) -7;
  }
  for (i = low; !p[i - low]; ++i)
    /* Nothing */ ;
  for (j = -i; i < high && j > (Sint) 1;)
  {
    if (p[++i - low] != 0.0)
    {
      j = gcd (j, i);
    }
  }
  Ktmp = (double) (j * exp (-2 * Sumval));
  *K = Ktmp / (av * (1.0 - exp (-*lambda * j)));

  FREESPACE(P);
  FREESPACE(p);
  return 0;  /* Parameters calculated successfully */
}

Sint karlinunitcostpp(double *lambda, double *K)
{
  double pr[] = {0.75, 0.0, 0.0, 0.25};
  
  return karlinpp((Sint) -1,(Sint) +2,&pr[0],lambda,K);
}

double significance (double lambda,double K,double multiplier,Sint score)
{
  double y;

  y = -lambda * score;
  y = K * multiplier * exp (y);
  return exp (-y);
}
