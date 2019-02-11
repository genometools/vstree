#ifndef ASSERTION_H
#define ASSERTION_H

//\IgnoreLatex{

#include "functrace.h"

//}

/*
  This file defines a macro for so-called assertions.
  Assertions are a very helpful way to ensure that certain conditions in a
  program are true. Assertions prooved to be very helpful to develop reliable 
  software and often an assertion will lead to a failure which is easy to fix. 
  Whereas without them one would either get a segmentation fault a few 
  functions calls later which is much harder to track down and to fix or, even
  worse, simply a wrong result.
  Therefore, the time to write simple assertions is well spend and pays well
  off in saved debugging time and better software.

  Lets consider one function which usually performs one certain computation
  to understand what assertions are and how to write them.
  In a typical function one gets some input data, a computation is performed 
  and this computation produces some output data.
  Assertions can be applied in two different yet similar ways in such a 
  function. On the one hand assertions can be used to make the implicit 
  assumptions which are made on the input data during the computation explicit.   For example, if a function performs some calculations on a linked 
  list whereas it is implicitly assumed that the list contains at least one 
  element, one would explicitly write this down in the form of an assertion.
  Using the macro it would look like this:

  ASSERTION("list contains at least one element", list->numofelems > 0);

  This prevents strange behaviour of the functions due to incorrect function
  calls.

  On the other hand assertions can be used to check for consistency of the
  computed results. For example, if our function computes a probability one
  could use the assertion macro in a way like this after the computation:

  ASSERTION("result is a valid probability", result >= 0.0 && result <= 1.0); 

  If the assertion fails, it gives us valuable hints that something must have
  gone wrong during the calculation of the result.

  When writing assertions one should take care that the actual assertion 
  computation is not too expensive. This is not the case, if simple conditions
  are checked as in the two examples above. 
  But one can also call functions which return a boolean value depending on a 
  consistency check of their input data.
  One has to be very careful when doing so, that the computation is not
  too expensive (especially if you plan to include the assertions in the
  final version of the program, see discussion below).  
  And even more important, one has to take special care that such a function
  does not alter the data which is checked. This would lead to different
  results depending on whether the assertions are switched on or off, and 
  therefore cause a very nasty bug.

  It is a controversial issue whether assertions (which lead to an exit upon
  failure) should be compiled into the release version of a software.
  
  the pros are: 
  - the result is more reliable since it passed all internal assertions
  - the programm is better tested since it is usually used in different ways 
    and with different input data by the enduser in contrast to the developer
  - if the program fails, potentially you get feedback which helps you to fix
    an yet unknown bug

  the cons are:
  - the program as a whole may be less reliable, since failed assertions lead 
    to early exits
  - the program is slower because performing the checks takes computation time
  - if the programm aborts, it is inconvenient for the enduser and it is
    questionable if feedback is given

  Therefore, it is up to you what to do and depends on the ``weights'' you 
  give the different arguments. My personal preference is to compile assertions
  into the released version.
*/

/*
  This macro defines what happens after an assertion failed.
  If it is not defined before this file is included, the programm exits.
  If you just want the assertion message and no exit define an empty macro.
  If you want to output some additional information before the exit, like
  printing an email address where a bug can reported, you can also to this
  with a corresponding macro definition.
*/

#ifndef ASSERTION_HANDLING
#define ASSERTION_HANDLING	exit(EXIT_FAILURE)
#endif 

/*
  This is the actual assertion macro. It is only defined, if the code is 
  compiled with -DASSERTIONS. Thereby, you can switch assertions on and off.

  The first argument \texttt{M} is a string describing the assertion.
  For example, "the number of exons equals the number of introns plus 1".
  The second argument \texttt{C} is the actual assertion which should be true.
  In our example: numofexons == numofintrons + 1;
  This would yield to a macro call like this:  

  ASSERTION("the number of exons equals the number of introns plus 1"
           , numofexons == numofintrons + 1);

  That is, both \texttt{M} and \texttt{C} are formulated in a way which 
  reflects how it should be.
*/

#ifdef ASSERTIONS
#define ASSERTION(M,C)\
        if(!(C))\
        {\
          fprintf(stderr, "%s(): assertion failed: %s\n", FUNCTIONNAME, (M));\
          ASSERTION_HANDLING;\
        }
#else
#define ASSERTION(M,C)
#endif

#endif
