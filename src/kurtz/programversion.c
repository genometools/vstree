#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "types.h"
#include "vmrelease.h"

static void showgenericprogramversion(FILE *fp,
                                      char *program,
                                      char *version,
                                      char *compiledate,
                                      char *cflags)
{
  fprintf(fp, "%s (Vmatch) %s (%s)\n", program, version, compiledate);
  fprintf(fp, "libvmatch:\n");
  fprintf(fp, "Copyright (c) 2000-2016 LScSA-Software GmbH\n\n");
#ifndef NOLICENSEMANAGER
  fprintf(fp, "libzlm:\n");
  fprintf(fp, "Copyright (c) 2013-2016 Wikena GmbH\n\n");
#endif
  fprintf(fp, "Email: kurtz@zbh.uni-hamburg.de\n\n");
  fprintf(fp, "Compile flags: %s\n\n", cflags);
  fprintf(fp, "Vmatch is provided on an AS IS basis. The developers do not "
              "warrant its validity\nof performance, efficiency, or "
              "suitability, nor do they warrant that Vmatch is\n"
              "free from errors. All warranties, including without limitation, "
              "any warranty or\nmerchantability or fitness for a particular "
              "purpose, are hereby excluded.\n");
}

void showprogramversion(FILE *fp,char *program)
{
  showgenericprogramversion(fp,
                            program,
                            VSTREEVERSION,
                            VSTREECOMPILEDATE,
                            VSTREECFLAGS);
}
