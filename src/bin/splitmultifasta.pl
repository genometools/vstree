#!/usr/bin/perl
use strict;
use warnings;

# Perl script to split a fasta file into different files, with exactly one
# sequence per file. 
# Stefan Kurtz, June, 2, 2005.

# First parameter is prefix of files to generate. 
# Second parameter is width of output line containg the sequences
# third parameter is number of files to generate
# fourth parameter is input file.

# let infile be the name of the input file in fasta format. Suppose 
# that it contain 1952 # sequences, each formatted within 70 characters per
# line. Then

# splitmultifasta.pl tmp 70 1 infile

# generates 1952 files named tmp-0000, tmp-0001, ..., tmp-1951.
# To check that the split was correct, execute the following commands:
# cat tmp-* > ALL
# diff ALL infile
# when nothing is reported by diff, then everything is fine. Otherwise
# check if there are other (not generated files) that begin with
# tmp-

if (scalar @ARGV  ne 4)
{
  print STDERR "Usage: $0 <splitprefix> <width> <num of files> <multiple fasta file>\n";
  exit 1;
}

my $splitprefix = $ARGV[0];
my $width = $ARGV[1];
my $numoffiles = $ARGV[2];
my $inputfile = $ARGV[3];

my $numofsequences = countnumofsequences($inputfile);

splitfiles($inputfile,$splitprefix,$numoffiles,$numofsequences);

sub countnumofsequences
{
  my($inputfile) = @_;
  unless ( -e $inputfile)
  {
    print STDERR "file \"$inputfile\" does not exist\n";
    exit 1;
  }
  unless(open(FILEDATA, $inputfile))
  {
    print STDERR "Cannot open file $inputfile\n";
    exit 1;
  }
  my $seqnum = 0;
  while(my $line = <FILEDATA>)
  {
      if ($line =~ /^>/)     # header line
      {
        $seqnum++;
      }
  }
  close FILEDATA;
  return $seqnum;
}


sub log10 
{
  my($n) = @_;
  return log($n)/log(10);
}

sub splitfiles
{
  my($inputfile,$splitprefix,$numoffiles,$numofsequences) = @_;
  my $line;
  # printf("numoffiles=%d,numofsequences=%d,sequencesperfile=%d\n",
  #         $numoffiles,$numofsequences,$sequencesperfile);

  # Declare and initialize variable to store final sequence
  my $totalseqcount = 0;
  my $seqcount = 0;
  my $filenum = 0;
  my $fh;
  my $outfilename;

  unless ( -e $inputfile)
  {
    print STDERR "file \"$inputfile\" does not exist\n";
    exit 1;
  }
  unless(open(FILEDATA, $inputfile))
  {
    print STDERR "Cannot open file $inputfile\n";
    exit 1;
  }
  my $fhisopen = 0;
  my $maxseqnum;
  if($numoffiles == 0)
  {
    $maxseqnum = 1;
  } else
  {
    $maxseqnum = $numofsequences/$numoffiles + $numofsequences % $numoffiles;
  }
  my $numwidth;
  if($numoffiles == 0)
  {
    $numwidth = 1+log10($numofsequences-1);
  } else
  {
    $numwidth = 1+log10($numoffiles-1);
  }
  while(my $line = <FILEDATA>)
  {
    if ($line =~ /^\s*$/)     # discard blank line
    {
      next;   # start the next iteration of the loop
    } elsif($line =~ /^\s*#/) # discard comment line
    {
      next;
    } elsif($line =~ /^>/)
    {
      # printf("seqcount=%d,totalseqcount=%d\n",$seqcount,$totalseqcount);
      # printf("maxseqnum=%d\n",$maxseqnum);
      if($seqcount >= $maxseqnum)
      {
        # printf("close %s\n",$outfilename);
        close $fh;
        $fhisopen = 0;
        $seqcount = 0;
      }
      if($seqcount == 0)
      {
        $outfilename = sprintf("%s-%0*d",$splitprefix,
                                $numwidth,
                                $filenum);
        # printf("outfilename=%s\n",$outfilename);
        unless(open($fh, ">$outfilename"))
        {
          print STDERR "Cannot open file $outfilename\n";
          exit 1;
        }
        $fhisopen = 1;
        $filenum++;
        if($numoffiles == 0)
        {
          $maxseqnum = 1;
        } else
        {
          $maxseqnum = $numofsequences/$numoffiles;
        }
      }
      $totalseqcount++;
      $seqcount++;
    }
    print $fh $line;
  }
  if($fhisopen)
  {
    close $fh;
  }
  close FILEDATA;
}
