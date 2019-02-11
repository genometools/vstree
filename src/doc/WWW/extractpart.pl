#!/usr/bin/env perl

# select a list of options from a documentation

use strict;
use warnings;

my $numofargs = scalar @ARGV;

if($numofargs le 1)
{
  print STDERR "Usage: $0 <tags to be selected> <filename>\n";
  exit 1;
}

my(%tagtab) = ();

my $filename = $ARGV[$numofargs-1];

for(my $i=0; $i<$numofargs-1; $i++)
{
  my $key = $ARGV[$i];
  $tagtab{$key} = 1;
}

# Get file data

my @filecontents = get_file_data($filename);

my $currenttagtext = '';
my $currenttag = '';
my $intag = 0;         # inside an option (which can be multiline)

for my $line (@filecontents) 
{
  if($intag)
  {
    if($line =~ /^\%\%\%END{([a-z]+)/)
    {
      if($1 ne $currenttag)
      {
        print STDERR "$0: BEGIN{$currenttag} ends with END{$1}\n";
        exit 1;
      }
      print STDERR "\%\%\%matched END with \"$currenttag\"\n";
      $intag = 0;
      print $currenttagtext, "\n";
      $currenttagtext = '';
      $currenttag = '';
    } else
    {
      $currenttagtext .= $line;
    }
  } else
  {
    if($line =~ /^\%\%\%BEGIN{([a-z]+)}/)
    {
      print STDERR "\%\%\%matched BEGIN with \"$1\"\n";
      if(exists $tagtab{$1})
      {
        $intag = 1;
        $currenttag = $1;
      }
    } 
  }
}

exit 0;

# check if the number of arguments is as expected
# get last argument

sub get_file_data
{
    my($filename) = @_;

    unless(open(GET_FILE_DATA, $filename))
    {
      print STDERR "$0: Cannot open file $filename\n";
      exit 1;
    }
    my @filedata = <GET_FILE_DATA>;
    close GET_FILE_DATA;
    return @filedata;
}
