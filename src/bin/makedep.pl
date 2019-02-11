#!/usr/bin/env perl

# generate dependencies to be included in makefiles
# Stefan Kurtz, Feb 13, 2005.

use strict;

my $usage = '<outputfile> <list of cfiles>';

my $numofargs = scalar @ARGV;

if($numofargs lt 1)
{
  print STDERR "$0 ${usage}\n";
  exit 1;
}

my $outfile = $ARGV[0];

shift @ARGV;

my $argstring = "which gcc 1> /dev/null";

my($retcode) = system($argstring);
$retcode = $? >> 8;
if($retcode ne 0)
{
  print STDERR "failure: $argstring\n";
  exit 1;
}

my $cflagsstring = `make cflagsstring`;

chomp($cflagsstring);

# print "$cflagsstring\n";

my $tmpfile = 'tmp0.mf';

my $gcccall = 'gcc -M -MG -MM -DDEBUG -DCHECK ' .
               $cflagsstring . " " .
               join(' ',@ARGV) . " > ${tmpfile}";

#print "$gcccall\n";
               
$retcode = system($gcccall);
$retcode = $? >> 8;
if($retcode ne 0)
{
  print STDERR "failure: $gcccall\n";
  exit 1;
}

unless(open(TMPFILEPTR,$tmpfile))
{
  print STDERR "$0: cannot open $tmpfile\n";
  exit 1
}

my @dependencies = <TMPFILEPTR>;

close TMPFILEPTR;

unless(open(OUTFILEPTR,">$outfile"))
{
  print STDERR "$0: cannot open $outfile\n";
  exit 1
}

changedependencies(0,\@dependencies);
changedependencies(1,\@dependencies);
unlink $tmpfile;

sub changedependencies
{
  my ($dbgmode,$dependenciesptr) = @_;
  my @dependencies = @$dependenciesptr;

  foreach my $line (@dependencies)
  {
    my @linelist = split(' ',$line);
    foreach my $item (@linelist)
    {
      if($item =~ m/(\S+)\/([_A-Za-z0-9\.\-]+\.[ch])/)
      {
        my $filepath = $1;
        my $filename = $2;
        if($filepath =~ /vstree\/include\.extra$/)
        {
          print OUTFILEPTR "\${VSTREEBASEDIR}/include.extra/$filename ";
        } elsif($filepath =~ /vstree\/include$/)
        {
          print OUTFILEPTR "\${VSTREEBASEDIR}/include/$filename ";
        } elsif($filepath =~ /Vmengine$/)
        {
          print OUTFILEPTR "../Vmengine/$filename ";
        } elsif($filepath =~ /Multimat$/)
        {
          print OUTFILEPTR "../Multimat/$filename ";
        } else
        {
          print OUTFILEPTR "$filepath/$filename ";
        }
      } elsif($item =~ m/^([_A-Za-z0-9\.\-]+)\.o:/)
      {
        if($dbgmode)
        {
          print OUTFILEPTR "\n\${COMPILEDIR}$1.dbg.o: ";
        } else
        {
          print OUTFILEPTR "\n\${COMPILEDIR}$1.o: ";
        }
      } elsif($item eq "\\")
      {
        print OUTFILEPTR "$item";
      } else
      {
        print OUTFILEPTR "$item ";
      }
    }
    print OUTFILEPTR "\n";
  }
}
