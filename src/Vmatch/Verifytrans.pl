#!/usr/bin/env perl

use strict;
use warnings;

my $argcount = scalar @ARGV;
my $transfile;

if ($argcount eq 1)
{
  $transfile = $ARGV[0];
} else
{
  print "Usage: $0 <transfile>\n";
  exit 1;
}

unless ( -e $transfile)
{
  print STDERR "file \"$transfile\" does not exist\n";
  exit 1;
}

unless(open(TRANSFP, $transfile))
{
  print STDERR "Cannot open file $transfile\n";
  exit 1;
}

my $linecount = 0;
my $checked = 0;
my $lastprot = '';
my $currentprot;

while(my $line = <TRANSFP>)
{
  $linecount++;
  if($line =~ m/^[^>]/)
  {
    my @linearray = split(' ',$line);
    $currentprot = $linearray[1];
    if($lastprot eq '')
    {
      $lastprot = $currentprot;
    } else
    {
      if($lastprot ne $currentprot)
      {
        print STDERR "line $linecount: ", $lastprot, " != ", $currentprot, "\n";
        exit 1;
      }
      $checked++;
      $lastprot = '';
    }
  }
}
print "number of successfully checked alignments: $checked\n";
