#!/usr/bin/env perl

# generate goals and filelists to be included in makefiles
# Stefan Kurtz, Mar 12, 2003.

use strict;
#use warnings;

my $usage = '[-local] [list of keywords liblist liblist4 dbg dbg4 prepro pr splint splintlist prototypelist cleanbuild]';

my $numofargs = scalar @ARGV;

if($numofargs eq 0)
{
  print STDERR "$0 ${usage}\n";
  exit 1;
}

my $withcompiledir;

if($ARGV[0] eq '-local')
{
  $withcompiledir = 0;
  shift @ARGV;
  $numofargs--;
} else
{
  $withcompiledir = 1;
}

my $filename;
my @filenametable = ();

while($filename = <STDIN>)
{
  chomp ${filename};
  if($filename =~ /.*\.c$/)
  {
    $filename =~ s/.c$//;
    push(@filenametable,$filename);
  } else
  {
    printf STDERR "${filename} does not end with .c\n";
    exit 1;
  }
}

my $addcompiledirgoal=0;

for(my $i=0; $i<$numofargs; $i++)
{
  if($ARGV[$i] eq 'cleanbuild')
  {
    printf("cleanbuild:\n\trm -f \${COMPILEDIR}*.[ox]\n\n");
  } elsif($ARGV[$i] eq 'o')
  {
    genericgoal('o');
    $addcompiledirgoal=1;
  } elsif($ARGV[$i] eq 'dbg')
  {
    genericgoal('dbg','dbg.o');
    $addcompiledirgoal=1;
  } elsif($ARGV[$i] eq '4')
  {
    genericgoal('4','4.o');
    $addcompiledirgoal=1;
  } elsif($ARGV[$i] eq 'dbg4')
  {
    genericgoal('dbg4','dbg.4.o');
    $addcompiledirgoal=1;
  } elsif($ARGV[$i] eq 'prepro')
  {
    genericgoal('prepro');
  } elsif($ARGV[$i] eq 'pr')
  {
    genericgoal('pr');
  } elsif($ARGV[$i] eq 'so')
  {
    genericgoal('so');
  } elsif($ARGV[$i] eq 'liblist' || $ARGV[$i] eq 'liblist4')
  {
    my $numoffiles = scalar @filenametable;
    my $libsuffix;

    if($ARGV[$i] eq 'liblist')
    {
      $libsuffix = ''; 
      printf("LIBOBJECTS=\\\n");
    } else
    {
      $libsuffix = '.4'; 
      printf("LIBOBJECTS4=\\\n");
    }
    for(my $i=0; $i< $numoffiles; $i++)
    {
      if($withcompiledir)
      {
        printf("  \${COMPILEDIR}%s%s.o",$filenametable[$i],$libsuffix);
      } else
      {
        printf("  %s%s.o",$filenametable[$i],$libsuffix);
      }
      if($i < $numoffiles-1)
      {
        print "\\\n";
      } else
      {
        print "\n\n";
      }
    }
    if($ARGV[$i] eq 'liblist')
    {
      printf("LIBDEBUGOBJECTS=\\\n");
    } else
    {
      printf("LIBDEBUGOBJECTS4=\\\n");
    }
    for(my $i=0; $i< $numoffiles; $i++)
    {
      if($withcompiledir)
      {
        printf("  \${COMPILEDIR}%s.dbg%s.o",$filenametable[$i],$libsuffix);
      } else
      {
        printf("  %s.dbg%s.o",$filenametable[$i],$libsuffix);
      }
      if($i < $numoffiles-1)
      {
        print "\\\n";
      } else
      {
        print "\n\n";
      }
    }
  } elsif($ARGV[$i] eq 'splintlist')
  {
    my $numoffiles = scalar @filenametable;

    printf("SPLINTALL=\\\n");
    for(my $i=0; $i< $numoffiles; $i++)
    {
      printf("  %s.splint",$filenametable[$i]);
      if($i < $numoffiles-1)
      {
        print "\\\n";
      } else
      {
        print "\n\n";
      }
    }
  } elsif($ARGV[$i] eq 'prototypelist')
  {
    my $numoffiles = scalar @filenametable;

    printf("PROTOTYPES=\\\n");
    for(my $i=0; $i< $numoffiles; $i++)
    {
      printf("  ../include/%s.pr",$filenametable[$i]);
      if($i < $numoffiles-1)
      {
        print "\\\n";
      } else
      {
        print "\n\n";
      }
    }
  } elsif($ARGV[$i] eq 'princ')
  {
    printf("../include/%%.pr:%%.c\n");
    printf("\tskproto \$< > \$@\n\n");
  } elsif($ARGV[$i] eq 'splint')
  {
    genericgoal('splint');
    printf("splintall:\${SPLINTALL}\n\n");
    printf("splintclean:\n\trm -f *.splint\n\n");
  } else
  {
    print STDERR "$0 ${usage}\n";
    exit 1;
  }
}

if($withcompiledir && $addcompiledirgoal)
{
  printf(".PHONY:mkdircompiledir\n");
  printf("mkdircompiledir:\n");
  printf("\tmkdir -p \${COMPILEDIR}\n");
}

sub genericgoal
{
  my $kind;
  my $suffix;
  my $argnum = scalar @_;
  if ($argnum eq 1)
  {
    ($kind) = @_;
    $suffix = $kind;
  } elsif($argnum eq 2)
  {
    ($kind,$suffix) = @_;
  } else
  {
    print STDERR "illegal number of arguments";
    exit 1;
  }
  if($kind eq 'o' || $kind eq 'dbg4' || $kind eq '4' || $kind eq 'dbg')
  {
    if($withcompiledir)
    {
      printf("\${COMPILEDIR}%%.%s:%%.c\n",$suffix);
    } else
    {
      printf("%%.%s:%%.c\n",$suffix);
    }
  } else
  {
    if($kind eq 'princ')
    {
      printf("include/%%.%s:%%.c\n",$suffix);
    } else
    {
      printf("%%.%s:%%.c\n",$suffix);
    }
  }
  if($kind eq 'o')
  {
    printf("\t\${CC} \${CFLAGS} -c \$< -o \$@ -MT \$@ -MMD -MP -MF \$(\@:.o=.d)\n\n");
  } elsif($kind eq 'splint')
  {
    printf("\tsplint \${SPLINTFLAGS} \$<\n");
    printf("\ttouch \$*.splint\n\n");
  } elsif($kind eq 'pr')
  {
    printf("\tskproto \$< > \$@\n\n");
  } elsif($kind eq 'prepro')
  {
    printf("\t\${CC} -E -g3 \${CFLAGS} -DDEBUG -c \$< -o \$@\n");
    printf("\tindent \$@\n\n");
  } elsif($kind eq 'dbg4')
  {
    printf("\t\${CC} \${CFLAGS} -DDEBUG -DSYMBOLBYTES=4 -c \$< -o \$@\n\n");
  } elsif($kind eq '4')
  {
    printf("\t\${CC} \${CFLAGS} -DSYMBOLBYTES=4 -c \$< -o \$@\n\n");
  } elsif($kind eq 'dbg')
  {
    printf("\t\${CC} \${CFLAGS} -DDEBUG -c \$< -o \$@\n\n");
  } elsif($kind eq 'so')
  {
    printf("\t\${CC} \${CFLAGS} \${SHARED} \$< -o \$@\n\n");
  } else
  {
    printf("illegal kind value \"%s\"\n",$kind);
    exit 1;
  }
}
