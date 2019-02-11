#!/usr/bin/env perl

use strict;
use warnings;
use Vmcallstab;

my $countsystemcall = 0;

my $firstarg = '(VM|OL)';

if(scalar @ARGV ne 5)
{
  print STDERR "Usage: $0 ${firstarg} <minlength> <mindist> <query> <database>\n";
  exit 1;
}

my $dovmatch;

if($ARGV[0] =~ /^VM$/)
{
  $dovmatch = 1;
} elsif($ARGV[0] =~ /^OL$/)
{
  $dovmatch = 0;
} else
{
  print STDERR "$0: first argument must be ${firstarg}\n";
  exit 1;
}

my $minlength = $ARGV[1];
my $mindist = $ARGV[2];
my $query = $ARGV[3];
my $database = $ARGV[4];

my $alphaspec;

if($query eq "")
{
  makesystemcall("../Mkvtree/mkdna6idx.x -db ${database} -v");
  $database = $database . ".6fr";
} else
{
  if($database =~ /dna/)
  {
    $alphaspec = "-dna";
  } else
  {
    $alphaspec = "-protein";
  }
  makesystemcall("mkvtree.sh -db ${database} ${alphaspec} -pl -allout -v");
}

if($query ne "")
{
  if($query =~ /dna/)
  {
    $alphaspec = "-dna";
  } else
  {
    $alphaspec = "-protein";
  }
  makesystemcall("mkvtree.sh -db ${query} ${alphaspec} -pl -tis -ois -v");
}

my $arglisttableref = makevmatchargumenttable($minlength,
                                              $mindist,
                                              $query,
                                              $database);

foreach my $arglist (@$arglisttableref)
{
  runprogram($dovmatch,$query,$database,$arglist);
}

print "number of calls was $countsystemcall\n";

sub makesystemcall
{
  my ($argstring) = @_;
  my @args = split(' ',$argstring);
  if(system(@args) == 0)
  {
    print "success $argstring\n";
    $countsystemcall++;
  } else
  {
    print STDERR "system @args failed: $?\n";
    exit 1;
  }
}

sub runprogram 
{
  my ($dovmatch,$query,$database,$argstring) = @_;
  my $program;

  if($argstring =~ /-p / && ($query =~ /prot/ || $database =~ /prot/))
  {
    return;
  }
  if($dovmatch)
  {
    $program = "./vmatch.x";
  } else
  {
    if($argstring =~ ' -online ')
    {
      return;
    }
    if(not ($argstring =~ ' -q '))
    {
      return;
    }
    if($argstring =~ ' -mum ')
    {
      return;
    }
    if($argstring =~ ' -pp chain')
    {
      return;
    }
    $program = "./Checkonline.sh";
  }
  if($query =~ /dna/ && 
     $database =~ /prot/ &&
     $argstring =~ ' -q ')
  {
    $argstring = "-dnavsprot 1 " . $argstring;
  }
  if($database =~ /\.6fr$/)
  {
    if($argstring =~ ' -q ' || $argstring =~ '-p ')
    {
      return;
    }
  }
  makesystemcall($program . " " . $argstring);
}
