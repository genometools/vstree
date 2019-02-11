#!/usr/bin/env perl

# evaluate include statements by expanding the include file
# Stefan Kurtz, 22. Febuary 2003

use strict;
use warnings;

my $argcount = scalar @ARGV;
if ($argcount ne 1)
{
  print "Usage: $0 <inputfile>\n";
  exit
}

my($inputfile) = $ARGV[0];
unless ( -e $inputfile)
{
  print STDERR "file \"$inputfile\" does not exist\n";
  exit 1;
}

expandfilecontents($inputfile);

exit 0;

sub getfiledata
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

sub expandfilecontents
{
  my($filename) = @_;
  my @filedata = getfiledata($filename);

  foreach my $line (@filedata)
  {
    if($line =~ /^include (.*)$/)
    {
      print "################ start include $1 ###################\n";
      expandfilecontents($1);
      print "################ end include $1 ###################\n";
    } else
    {
      print $line;
    }
  } 
}
