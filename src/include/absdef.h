//\IgnoreLatex{

#ifndef ABSDEF_H
#define ABSDEF_H

//}

/*
  This file defines a macro for computing the absolute value of a number,
  if this is not already defined.
*/

#ifndef ABS
#define ABS(A)        (((A) < 0) ? -(A) : (A))
#define DOUBLEABS(A)  (((A) < 0.0) ? -(A) : (A))
#endif

//\IgnoreLatex{

#endif

//}
