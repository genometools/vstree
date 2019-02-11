#!/usr/bin/env perl

use strict;
use warnings;
use Mkvcallstab;

my $countsystemcall = 0;

my $numofargs = scalar @ARGV;

if($numofargs ne 3)
{
  print STDERR "Usage: $0 <program> <DBdna> <DBprot>\n";
  exit 1;
}

my $program = $ARGV[0];
my $DBdna = $ARGV[1];
my $DBprot = $ARGV[2];

my $arglisttableref = makemkvtreeargumenttable($DBdna,$DBprot);

foreach my $argstring (@$arglisttableref)
{
  makesystemcall($program . " " . $argstring);
}

print STDERR "# number of calls was $countsystemcall\n";

sub makesystemcall
{
  my ($argstring) = @_;
  my @args = split(' ',$argstring);
  if(system(@args) == 0)
  {
    print STDERR "# success $argstring\n";
    $countsystemcall++;
  } else
  {
    print STDERR "system \"@args\" failed: errorcode $?\n";
    exit 1;
  }
}
