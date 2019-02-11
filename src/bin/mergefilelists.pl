#!/usr/bin/env perl

use strict;
use warnings;

my $numofargs = scalar @ARGV;

if($numofargs ne 2)
{
  print STDERR "Usage: $0 cvsfiles currentfiles\n";
  exit 1;
}

my @cvsfiles = getfiledata($ARGV[0]);
my @currentfiles = getfiledata($ARGV[1]);
my $cvsptr = 0;
my $currentptr = 0;
my @deletefiles = ();
my @addfiles = ();

while($cvsptr < scalar @cvsfiles && $currentptr < scalar @currentfiles)
{
  my $cvsfile = $cvsfiles[$cvsptr];
  my $currentfile = $currentfiles[$currentptr];
  chomp $cvsfile;
  chomp $currentfile;
  if($cvsfile eq $currentfile)
  {
    $cvsptr++;
    $currentptr++;
  } else
  {
    if($cvsfile lt $currentfile)
    {
      push(@deletefiles,$cvsfile);
      $cvsptr++;
    } else
    {
      push(@addfiles,$currentfile);
      $currentptr++;
    }
  }
}

while($cvsptr < scalar @cvsfiles)
{
  my $cvsfile = $cvsfiles[$cvsptr];
  chomp $cvsfile;
  push(@deletefiles,$cvsfile);
  $cvsptr++;
}

while($currentptr < scalar @currentfiles)
{
  my $currentfile = $currentfiles[$currentptr];
  chomp $currentfile;
  push(@addfiles,$currentfile);
  $currentptr++;
}

if(@deletefiles)
{
  printf("cvs -d \${CVSROOT} delete ");
  print "@deletefiles\n";
} 

if(@addfiles)
{
  printf("cvs -d \${CVSROOT} add ");
  print "@addfiles\n";
}

sub getfiledata
{
  my($filename) = @_;

  unless(open(GET_FILE_DATA, $filename))
  {
    print STDERR "Cannot open file $filename\n";
    exit 1;
  }
  my @filedata = <GET_FILE_DATA>;
  return sort @filedata;
}
