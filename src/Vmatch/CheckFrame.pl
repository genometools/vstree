#!/usr/bin/env perl

use strict;
use warnings;

my $argcount = scalar @ARGV;
my $dnamatchfile;
my $codonmatchfile;

if ($argcount eq 2)
{
  $dnamatchfile = $ARGV[0];
  $codonmatchfile = $ARGV[1];
} else
{
  print "Usage: $0 <dnamatchfile> <codonmatchfile>\n";
  exit 1;
}

unless ( -e $dnamatchfile)
{
  print STDERR "file \"$dnamatchfile\" does not exist\n";
  exit 1;
}

unless ( -e $codonmatchfile)
{
  print STDERR "file \"$codonmatchfile\" does not exist\n";
  exit 1;
}

unless(open(DNAMATCHFP, $dnamatchfile))
{
  print STDERR "Cannot open file $dnamatchfile\n";
  exit 1;
}

unless(open(CODONMATCHFP, $codonmatchfile))
{
  print STDERR "Cannot open file $codonmatchfile\n";
  exit 1;
}

my @dnamatcharray = <DNAMATCHFP>;
my @codonmatcharray = <CODONMATCHFP>;

foreach my $dnaline (@dnamatcharray)
{
  if(not (finddnamatch($dnaline,\@codonmatcharray)))
  {
    exit 1;
  }
}

sub finddnamatch
{
  my ($dnaline,$dnamatcharrayref) = @_;

  my @codonmatcharray = @$dnamatcharrayref;

  foreach my $codonline (@codonmatcharray)
  {
    if(cmplines($dnaline,$codonline))
    {
      return 1;
    }
  }
  print STDERR "cannot find $dnaline";
  return 0;
}

sub contained
{
  my($start1,$len1,$start2,$len2) = @_;

  # printf("%d %d/%d %d\n",$start1,$len1,$start2,$len2);
  if($start1-2 le $start2 &&
     $start2 + $len2 - 1 le $start1 + $len1 - 1 + 2)
  {
    return 1;
  }
  return 0;
}

sub cmplines
{
  my ($dnaline,$codonline) = @_;

  my @dnalinearray = split(' ',$dnaline);
  my @codonlinearray = split(' ',$codonline);

  #my $modval = $dnalinearray[0] % 3;
  #$dnalinearray[0] -= $modval;
  #$modval = $dnalinearray[3] % 3;
  #$dnalinearray[3] -= $modval;
  if((contained($codonlinearray[1],$codonlinearray[0],
               $dnalinearray[1],$dnalinearray[0]) &&
     contained($codonlinearray[4],$codonlinearray[3],
               $dnalinearray[4],$dnalinearray[3])) ||
     (contained($codonlinearray[1],$codonlinearray[1],
               $dnalinearray[4],$dnalinearray[3]) &&
     contained($codonlinearray[4],$codonlinearray[0],
               $dnalinearray[1],$dnalinearray[0])))
  {
    # print $dnaline, " = ", $codonline, "\n";
    return 1;
  }
  return 0;
}
