#!/usr/bin/env perl

use strict;
use warnings;

my $numofargs = scalar @ARGV;

if($numofargs ne 1)
{
  print STDERR "Usage: $0 <filename>\n";
  exit 1;
}

my $filename = $ARGV[0];

# Get file data

my @filelines = get_file_data($filename);

my $infirstcomment = 0;  # inside first comment
my $firstcommentskipped = 0;

for my $line (@filelines) 
{
  if($infirstcomment)
  {
    if($line =~ /^\*\//)
    {
      $infirstcomment = 0;
      $firstcommentskipped = 1;
    }
  } else
  {
    if($firstcommentskipped)
    {
      print $line;
    } elsif ($line =~ /^\/\*/) 
    {
      $infirstcomment = 1;
    }
  }
}
       
exit 0;

sub get_file_data
{
    my($filename) = @_;

    unless(open(GET_FILE_DATA, $filename))
    {
        print STDERR "Cannot open file $filename\n";
        exit 1;
    }
    my @filedata = <GET_FILE_DATA>;
    close GET_FILE_DATA;
    return @filedata;
}
