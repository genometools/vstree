#!/usr/bin/env perl

use strict;
use warnings;

my $numoffiles = scalar @ARGV;

for(my $i=0; $i<$numoffiles; $i++)
{
  my($inputfile) = $ARGV[$i];
  unless ( -e $inputfile)
  {
    print STDERR "$0: file \"$inputfile\" does not exist\n";
    exit 1;
  }
  unless ( open(INPUTFILEHANDLE, $inputfile) )
  {
    print STDERR "Cannot open file \"$inputfile\"";
    exit 1;
  }
  while (my $in = <INPUTFILEHANDLE>)
  {
    if($in =~ m/^\\EXECUTE\{([^\}]*)\}/)
    {
      my $argstring = $1;
      if($argstring =~ m/^(mkvtree|mkdna6idx)/)
      {
        runtheprogram($argstring);
      }
    }
  }
}

sub runtheprogram
{
  my($argstring) = @_;

  print $argstring, "\n";
  my($retcode) = system($argstring);
  $retcode = $? >> 8;
  if($retcode ne 0)
  {
    print STDERR "failure: $argstring\n";
    exit 1;
  }
}
