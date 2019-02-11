#!/usr/bin/env perl

# select a list of options from a documentation

use strict;
use warnings;

my $numofargs = scalar @ARGV;

if($numofargs le 1)
{
  print STDERR "Usage: $0 <options to be selected> <filename>\n";
  exit 1;
}

my(%optiontab) = ();

my $filename = $ARGV[$numofargs-1];

for(my $i=0; $i<$numofargs-1; $i++)
{
  my $key = $ARGV[$i];
  $optiontab{$key} = 1;
}

# Get file data

my @manual = get_file_data($filename);

my $currentoptiontext = '';
my $inoption = 0;         # inside an option (which can be multiline)
my $invmatch = 0;

for my $line (@manual) 
{
  if($inoption)
  {
    $currentoptiontext .= $line;
    if($line =~ /^\}/)
    {
      $inoption = 0;
      print $currentoptiontext, "\n";
      $currentoptiontext = '';
    }
  } else
  {
    if($line =~ /^\\Option{([a-z]+)}/) 
    {
      if($invmatch && exists $optiontab{$1})
      {
        $inoption = 1;
        $currentoptiontext .= $line;
      }
    } else
    {
      if($line =~ /\\begin\{AboutVmatch\}/)
      {
        $invmatch = 1;
      } else
      {
        if($line =~ /\\end\{AboutVmatch\}/)
        {
          $invmatch = 1;
        } 
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
        print STDERR "Cannot open file $filename\n";
        exit 1;
    }
    my @filedata = <GET_FILE_DATA>;
    close GET_FILE_DATA;
    return @filedata;
}
