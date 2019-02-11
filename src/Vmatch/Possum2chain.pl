#!/usr/bin/env perl

# Stefan Kurtz, September 2004.
# parse a file in POSSUM-format
# group the matches by groupid and sequence id
# output all matches with the same groupd and sequence id into a file
# process this file with chain2dim

use strict;
use warnings;

my $argcount = scalar @ARGV;
my $inputfile;

if ($argcount eq 1)
{
  $inputfile = $ARGV[0];
} else
{
  print "Usage: $0 <inputfile>\n";
  exit 1;
}

unless ( -e $inputfile)
{
  print STDERR "file \"$inputfile\" does not exist\n";
  exit 1;
}

unless(open(GET_FILE_DATA, $inputfile))
{
  print STDERR "Cannot open file $inputfile\n";
  exit 1;
}

my $linenum = 0;
my $mustcolumns = 18;
my %collectgroupids = ();
my %collectseqids = ();

foreach my $line (<GET_FILE_DATA>)
{
  my @linearray = split('\t',$line);
  $linenum++;
  my $numofcolumns = scalar @linearray;
  if($numofcolumns ne $mustcolumns)
  {
    print STDERR "file $inputfile, line $linenum contains ";
    print STDERR "$numofcolumns columns; $mustcolumns are required\n";
    exit 1
  }
  my $groupid = $linearray[3];
  if(not (exists $collectgroupids{$groupid}))
  {
    $collectgroupids{$groupid} = 1;
  }
  my $seqid = $linearray[15];
  if(not (exists $collectseqids{$seqid}))
  {
    $collectseqids{$seqid} = 1;
  }
}

my @storevaluecount = ();

foreach my $groupid (keys %collectgroupids)
{
  foreach my $seqid (keys %collectseqids)
  {
    $storevaluecount[$groupid][$seqid] = 0;
  }
}

seek(GET_FILE_DATA, 0, 0);

my @storevalues = ();

foreach my $line (<GET_FILE_DATA>)
{
  my @linearray = split('\t',$line);
  my $groupid = $linearray[3];
  my $seqid = $linearray[15];
  my $groupposition = $linearray[4];
  my $startmatch = $linearray[5];
  my $lenmatch = $linearray[6];
  my $matchscore = $linearray[9];
  my $hitcount = $storevaluecount[$groupid][$seqid];
  $storevaluecount[$groupid][$seqid]++;
  $storevalues[$groupid][$seqid][$hitcount] 
   = "$groupposition $startmatch $lenmatch $matchscore";
}

close GET_FILE_DATA;

foreach my $groupid (keys %collectgroupids)
{
  foreach my $seqid (keys %collectseqids)
  {
    my $hitcount = $storevaluecount[$groupid][$seqid];
    if($hitcount gt 1)
    {
      my $outfile = "/tmp/PSSMchain-$hitcount-$groupid-$seqid";
      unless(open(OUTFILE,">", $outfile))
      {
        print STDERR "Cannot open file $outfile\n";
        exit 1;
      }
      print "# output $hitcount matches for group ";
      print "$groupid and sequence $seqid into file $outfile\n";
      for(my $i = 0; $i < $hitcount; $i++)
      {
        my @values = split(' ',$storevalues[$groupid][$seqid][$i]);
        my $groupposition = $values[0];
        my $startmatch = $values[1];
        my $lenmatch = $values[2];
        my $matchscore = $values[3];
        printf OUTFILE
               "%d %d %d %d %.0f\n",
               $startmatch,
               $startmatch+$lenmatch-1,
               $groupposition,
               $groupposition,
               $matchscore;
      }
      close OUTFILE;
      callchain2dim($outfile);
    }
  }
}

sub callchain2dim
{
  my($outfile) = @_;
  my @runchain = ("chain2dim.x", "-local", "10b", $outfile);
  if(system(@runchain) ne 0)
  {
    printf("system \"@runchain\" failed\n");
    exit 1;
  }
}
