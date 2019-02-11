
/* This file is generated. Do not edit. */
 Sint vmatmaxout4(VMATMAXOUTTYPE);    // ALPHABETSIZE
 Sint vmatmaxcount4(VMATMAXCOUNTTYPE);  // ALPHABETSIZE
 Sint vmatmaxout12(VMATMAXOUTTYPE);    // ALPHABETSIZE
 Sint vmatmaxcount12(VMATMAXCOUNTTYPE);  // ALPHABETSIZE
 Sint vmatmaxout21(VMATMAXOUTTYPE);    // ALPHABETSIZE
 Sint vmatmaxcount21(VMATMAXCOUNTTYPE);  // ALPHABETSIZE
 Sint vmatmaxout31(VMATMAXOUTTYPE);    // ALPHABETSIZE
 Sint vmatmaxcount31(VMATMAXCOUNTTYPE);  // ALPHABETSIZE
 Sint vmatmaxout42(VMATMAXOUTTYPE);    // ALPHABETSIZE
 Sint vmatmaxcount42(VMATMAXCOUNTTYPE);  // ALPHABETSIZE
 Sint vmatmaxout53(VMATMAXOUTTYPE);    // ALPHABETSIZE
 Sint vmatmaxcount53(VMATMAXCOUNTTYPE);  // ALPHABETSIZE
 Sint vmatmaxout66(VMATMAXOUTTYPE);    // ALPHABETSIZE
 Sint vmatmaxcount66(VMATMAXCOUNTTYPE);  // ALPHABETSIZE

Sint vmatmaxoutgeneric(Virtualtree *virtualtree,
                       Uint numberofprocessors,
                       Uint searchlength,
                       /*@unused@*/ void *repeatgapspec,
                       void *outinfo,
                       Outputfunction output)
{
  MAXOUTCASE(4);    // ALPHABETSIZE
  MAXOUTCASE(12);    // ALPHABETSIZE
  MAXOUTCASE(21);    // ALPHABETSIZE
  MAXOUTCASE(31);    // ALPHABETSIZE
  MAXOUTCASE(42);    // ALPHABETSIZE
  MAXOUTCASE(53);    // ALPHABETSIZE
  MAXOUTCASE(66);    // ALPHABETSIZE
  return 0;
}

Sint vmatmaxcountgeneric(Virtualtree *virtualtree,
                         Uint numberofprocessors,
                         Uint searchlength,
                         ArrayUint *counttab)
{
  MAXCOUNTCASE(4);    // ALPHABETSIZE
  MAXCOUNTCASE(12);    // ALPHABETSIZE
  MAXCOUNTCASE(21);    // ALPHABETSIZE
  MAXCOUNTCASE(31);    // ALPHABETSIZE
  MAXCOUNTCASE(42);    // ALPHABETSIZE
  MAXCOUNTCASE(53);    // ALPHABETSIZE
  MAXCOUNTCASE(66);    // ALPHABETSIZE
  return 0;
}
