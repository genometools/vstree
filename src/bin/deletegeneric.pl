#!/usr/bin/env perl

# extract lines between some BEGIN{tag} and END{tag}
# both occurring with exactly three preceeding comment characters
# at the beginning of a line.

use strict;
use warnings;

my $numofargs = scalar @ARGV;

if($numofargs lt 2)
{
  print STDERR "Usage: $0 <commentchar> <tags to be selected>\n";
  exit 1;
}

my(%tagtab) = ();

my $commentchar = $ARGV[0];

if((length $commentchar) ne 1)
{
  print STDERR "$0: only one commentchar allowed\n";
  exit 1
}

my $commentblock = "${commentchar}${commentchar}${commentchar}";

print STDERR "commentblock=\"$commentblock\"\n";

for(my $i=1; $i<$numofargs; $i++)
{
  my $key = $ARGV[$i];
  if($key =~ /^[a-z][a-z]*$/)
  {
    print STDERR "store tag $key\n";
  } else
  {
    print STDERR "illegal tag $key: must consist of lower case characters\n";
    exit 1;
  }
  $tagtab{$key} = 1;
}

# Get file data

my $currenttag = '';
my $intag = 0;  # inside an cccBEGIN{tag} cccEND{tag}, where c is a 
                # comment character (which can be multiline)
my $linenum = 0;

while(my $line = <STDIN>)
{
  $linenum++;
  if($intag)
  {
    if($line =~ /^${commentblock}END\{([a-z]+)/)
    {
      if($1 ne $currenttag)
      {
        print STDERR "$0: BEGIN{$currenttag} ends with END{$1}\n";
        exit 1;
      }
      print STDERR "matched END with \"$currenttag\" in line $linenum\n";
      $intag = 0;
      $currenttag = '';
    }
  } else
  {
    if($line =~ /^${commentblock}BEGIN\{([a-z]+)\}/)
    {
      print STDERR "matched BEGIN with \"$1\" in line $linenum\n";
      if(exists $tagtab{$1})
      {
        $intag = 1;
        $currenttag = $1;
      }
    } else
    {
      print $line;
    }
  }
}

exit 0;
