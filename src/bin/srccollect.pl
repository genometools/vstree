#!/usr/bin/env perl

use strict;
use warnings;

my $numofargs = scalar @ARGV;

if ($numofargs == 0)
{
  usage();
  exit 1;
}

my @stack;
my %donehash = ();
my @cfilelist = ();
my @hfilelist = ();
my $startpath = '/projects/vstree/src/vstree/src';
my @dirlist = (".",
               "$startpath/include",
               "$startpath/kurtz-basic",
               "$startpath/kurtz",
               "$startpath/Vmengine");
my $dir;

print "#!/bin/sh\n";
print "set -x\n";

my $keyfile = $ARGV[0];
if($keyfile =~ m/(.*)\.c$/)
{
  $keyfile = $1;
} else
{
  usage();
  exit(1);
}

for my $cfile (@ARGV)
{
  if(not ($cfile =~ m/.*\.c$/))
  {
    usage();
    exit 1;
  }
  push(@stack,$cfile);
  push(@cfilelist,$cfile);
}

while(@stack)
{
  my $filename = pop(@stack);
  printf("# open %s\n",$filename);
  unless(open(FHANDLE,$filename))
  {
    print STDERR "Cannot open $filename\n";
    exit 1;
  }
  for my $line (<FHANDLE>)
  {
    if($line =~ m/^#include \"([a-zA-Z\.\-0-9]*)\"/)
    {
      my $includefile = $1;
      print "# found include $includefile\n";
      if(not ($includefile =~ m/^libgtcore/) && 
         not (exists $donehash{$includefile}))
      {
        $donehash{$includefile} = 1;
        $dir = findfiledir($includefile);
        if($dir =~ m/^\.$/)
        {
          push(@hfilelist,"$includefile");
        } else
        {
          push(@hfilelist,"$dir/$includefile");
        }
        if($includefile =~ m/^(.*)\.pr$/)
        {
          my $prefix = $1;
          $dir = findfiledir("$prefix.c");
          my $cfile;
          if($dir =~ m/^\.$/)
          {
            $cfile = "$prefix.c";
          } else
          {
            $cfile = "$dir/$prefix.c";
          }
          push(@cfilelist,$cfile);
          push(@stack,$cfile);
        } else
        {
          my $hfile = "$dir/$includefile";
          push(@stack,$hfile);
        }
      }
    }
  }
  close FHANDLE;
}

print "DISTDIR=${keyfile}.dir\n";
print "mkdir -p \${DISTDIR}\n";

for my $filename (@cfilelist)
{
  print "cp $filename \${DISTDIR}/.\n";
}

for my $filename (@hfilelist)
{
  print "cp $filename \${DISTDIR}/.\n";
}

sub usage
{
  print STDERR "Usage: $0 <file1.c> <file2.c> ...\n";
}

sub findfiledir
{
  my($filename) = @_;
  my($directory) = @_;
  my @files = ();

  for my $directory (@dirlist)
  {
    unless(opendir(DIRECTORY, $directory)) # Open the directory
    {
      print STDERR "Cannot open directory $directory!\n";
      exit 1;
    }
    # Read the directory, ignoring special entries "." and ".."
    for my $dirfile (grep (!/^\.\.?$/, readdir(DIRECTORY)))
    {
      # Notice that we need to prepend the directory name!
      if (-f "$directory/$dirfile") # directory entry is a regular file
      {
        if($dirfile eq $filename)
        {
          closedir(DIRECTORY);
          return $directory
        }
      }
    }
    closedir(DIRECTORY);
  }
  print STDERR "Cannot find file $filename\n";
  exit 1;
}

print "Makefile-dist.sh $keyfile > \${DISTDIR}/Makefile\n";

exit 0;
