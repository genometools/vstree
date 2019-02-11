#!/usr/bin/env perl

use strict;
use warnings;

my $foundinclude = 0;

while(my $line = <STDIN>)
{
  if($foundinclude)
  {
    print $line;
  } else
  {
    if($line =~ /^#include /)
    {
      $foundinclude = 1;
      printf("#ifdef HAVE_CONFIG_H\n");
      printf("#include <config.h>\n");
      printf("#endif /* HAVE_CONFIG_H */\n\n");
    }
    print $line;
  }
}

exit 0;
