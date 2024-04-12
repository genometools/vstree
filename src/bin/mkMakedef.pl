#!/usr/bin/env perl

# generate DEFINECFLAGS and DEFINELDFLAGS, if necessary. 
# Stefan Kurtz, Feb 21, 2003.

#ich habe das Problem mit der Sunfire gel"o"st.
#Falls Du einmal shared-objects unter der Sunfire laden m"ochtest compiliere sie
#zus"atzlich mit 
#-static-libgcc -fPIC
#ansonsten haben die shared objects eine
#Abh"angigkeit zu einer Library die nicht existiert.

#blade-0 [178] diff Makedef-linux-xlc Makedef-linux-gcc
#5c5
#< CC?=xlc
#---
#> CC?=gcc
#21c21
#< DEFINECFLAGS=-DWITHREGEXP -g -O3
#---
#> DEFINECFLAGS=-DWITHREGEXP -g -O3 -Wundef -Wshadow -Wstrict-prototypes -Wcast-align -Wsign-compare -Wnested-externs -Wall -Winline -Werror
#42c42
#< SHARED=-shared -fPIC
#---
#> SHARED=-shared

# use compiler flag: xlc -qstrict

use strict;
use warnings;

my $additionalWflags = " -Wcast-qual -Wpointer-arith";

my $usage = '<sol2|linux|tru64|irix|hpux|osx|freebsd> (gcc|g++,cc|icc) [64|profiling]';

my $numofargs = scalar @ARGV;

if(($numofargs ne 2) && ($numofargs ne 3))
{
  print STDERR "$0 $usage\n";
  exit 1;
}

my $opsys = $ARGV[0];
my $compiler = $ARGV[1];

my $sixtyfour = 0;
my $profiling = 0;
my $pathend = "32bit";

if($numofargs eq 3)
{
  if($ARGV[2] eq '64')
  {
    $sixtyfour = 1;
    $pathend = "64bit";
  } elsif($ARGV[2] eq 'profiling')
  {
    if($compiler ne 'gcc')
    {
      print STDERR "$0: profiling only available for gcc\n";
      exit 1;
    }
    $profiling = 1;
  } else
  {
    print STDERR "$0 $usage\n";
    exit 1;
  }
}

my $cflags = '';
my $cc64 = '-xarch=v9';
my $gcc64;
if($opsys eq 'irix')
{
  $gcc64 = '-mabi=64';
} else
{
  $gcc64 = '-m64';
}

my $sparcpath;
if ($sixtyfour)
{
  $sparcpath = "/vol/local/lib/sparcv9";
} else
{
  $sparcpath = "/vol/local/lib";
}
my $sol2path = "-L$sparcpath -R $sparcpath";
my $extrainclude = "/vol/local/include";

# possible add -Wfloat-equal if new gcc becomes available
# possible add -Wmissing-prototypes if new gcc becomes available
# possible add -Waggregate-return 

my $gccstdopt = '-O3 -Wundef -Wshadow -Wstrict-prototypes -Wcast-align -Wsign-compare -Wnested-externs -Wall'.$additionalWflags;# -DNOSPACEBOOKKEEPING'

my %compileroptions = 
(
  'sol2 gcc' =>     "-Wstrict-prototypes $gccstdopt -Winline -Werror",
  'sol2 gcc 64' =>  "-Wstrict-prototypes $gccstdopt -Winline -Werror $gcc64",
  'sol2 cc' =>      '-xCC -xO2 -v',
  'sol2 cc 64' =>   "-xCC -xO2 -v $cc64",
  'linux gcc' =>    "-Wstrict-prototypes $gccstdopt -m32 -Winline -Werror",
  'linux icc' =>    '-O3 -Wall',
  'linux gcc 64' => "$gccstdopt -Winline -Werror $gcc64",
  'osx gcc' =>      "$gccstdopt -Winline -DNOINITSTATE -DNOMADVISE -Werror -m32",
  'osx gcc 64' =>   "$gccstdopt -Winline -DNOINITSTATE -DNOMADVISE -Werror $gcc64" ,
  'tru64 gcc' =>    "$gccstdopt -Winline -Werror",
  'tru64 cc' =>     '-xCC -D__alpha -fast',
  'tru64 gcc 64' => "$gccstdopt -Winline -Werror",
  'irix gcc' =>     "$gccstdopt -Winline -Werror -DNOMADVISE",
  'irix gcc 64' =>  "$gccstdopt -Winline -Werror -DNOMADVISE $gcc64",
  'hpux cc' =>      '-O -DHPUX +DAportable +DS1.1',
  'freebsd gcc' =>  "$gccstdopt -Werror"
);

my $key;

if($compiler eq 'g++')
{
  $key = "$opsys gcc";
} else
{
  $key = "$opsys $compiler";
}

if($sixtyfour)
{
  $key = "$key 64";
}

my $ldflags = '';

if($profiling)
{
  $ldflags = '-pg';
}

print <<HEREDOC;
# this file is generated by
# mkMakedef.pl @ARGV
# do not edit

CC?=$compiler

# the base directory: 

VSTREEBASEDIR=\${DIRVSTREE}

# we redefine it and later remove the next line in other directories

VSTREEBASEDIR=..

# the include directory path

HEREDOC

if($opsys eq 'osx')
{
  print "INCLUDEOPT=-I\${VSTREEBASEDIR}/include -I/sw/include\n";
} else
{
  if(($opsys eq 'sol2' || $opsys eq 'tru64'))
  {
    print "INCLUDEOPT=-I\${VSTREEBASEDIR}/include -I${extrainclude}\n";
  } else
  {
    print "INCLUDEOPT=-I\${VSTREEBASEDIR}/include\n";
  }
}

print "\n";

my $go64 = '';

if(exists $compileroptions{$key})
{
  print "# the following should be used to define the CFLAGS\n\n";
  print "DEFINECFLAGS=";
  # print "-DWITHREGEXP ";
  print "-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g $compileroptions{$key}";
  if($profiling)
  {
    print " -pg\n\n";
    print "DEFINELDFLAGS=-pg\n\n";
    print "\n";
  } else
  {
    print "\n\n";
    if($opsys eq 'sol2')
    {
      print "# the following should be used to define the LDFLAGS\n\n";
      if($sixtyfour)
      {
        if($compiler eq 'cc')
        {
          $go64=$cc64;
        } else
        {
          $go64=$gcc64;
        }
      } 
      print "DEFINELDFLAGS=$go64 $sol2path\n\n";
    } elsif($opsys eq 'tru64')
    {
      print "# the following should be used to define the LDFLAGS\n\n";
      print "DEFINELDFLAGS=-L/vol/local/lib\n\n";
    } elsif($sixtyfour)
    {
      print "# the following should be used to define the LDFLAGS\n\n";
      print "DEFINELDFLAGS=$gcc64\n\n";
    } else
    {
      print "# the following should be used to define the LDFLAGS\n\n";
      print "DEFINELDFLAGS=-m32\n\n";
    }
  }
} else
{
  print STDERR "$0: $compiler not available for $opsys\n";
  exit 1;
}

print "# this is used to define the flags for the preprocessor\n\n";
print "DEFINECPPFLAGS=\${INCLUDEOPT}\n\n";

if(($opsys ne 'tru64') || $sixtyfour)
{
  print "# this is used to trigger the use of pointers in Mkvtree\n\n";
  print "DEFINESUFFIXPTR=-DSUFFIXPTR\n";
  print "\n";
}

my $ldlibsvar = "-lm";

if($opsys ne 'tru64')
{
  if($opsys eq 'hpux')
  {
    $ldlibsvar .= " -ldld";
  } elsif($opsys ne 'osx' && $opsys ne 'irix' && $opsys ne 'freebsd')
  {
    $ldlibsvar .= " -ldl";
  }
  print "\n";
  if($opsys ne 'osx' && $opsys ne 'irix' && $opsys ne 'freebsd')
  {
    print "# use the sysconf command\n\n";
    print "WITHSYSCONF=-DWITHSYSCONF\n";
  }
  print "\n";
}

print "# the following defines the libraries usually used\n\n";

print "DEFINELDLIBS=$ldlibsvar \# -static\n\n";

print "# the following is defined to properly compile shared libraries\n\n";

if($opsys eq 'sol2')
{
  if($sixtyfour)
  {
    print "SHARED=-G $go64\n";
  } else
  {
    print "SHARED=-G\n";
  }
} elsif($opsys eq 'linux')
{
  if($sixtyfour)
  {
    print "SHARED=-shared -fPIC $gcc64\n";
  } else
  {
    print "SHARED=-shared -m32\n";
  }
} elsif($opsys eq 'true64')
{
  print "SHARED=-shared\n";
} elsif($opsys eq 'osx')
{
  print "SHARED=-bundle\n";
} elsif($opsys eq 'irix')
{
  if($sixtyfour)
  {
    print "SHARED=-shared $gcc64\n";
  } else
  {
    print "SHARED=-shared\n";
  }
} 

if($opsys eq 'hpux')
{
  print "SHAREDSUFFIX=sl\n"
} else
{
  print "SHAREDSUFFIX=so\n"
}

print "# the following defines the flags for splint\n";

print "DEFINESPLINTFLAGS=\${DEFINECPPFLAGS} -DDEBUG -f ../Splintoptions\n";

print "# define BIT variable\n";
if($sixtyfour)
{
  print "BIT=64bit\n"
} else
{
  print "BIT=32bit\n"
}

print <<HEREDOC;

# the loader is the same as the compiler

LD=\${CC}

# the libraries implemented in the vstree package

PATHEND=$pathend
LIBBASEDIR=lib/\${CONFIGGUESS}/\${PATHEND}
COMPILEDIRPREFIX=../\${LIBBASEDIR}
EXECDIRPREFIX=../\${LIBBASEDIR}
LIBDIR=\${VSTREEBASEDIR}/\${LIBBASEDIR}
LIBKURTZBASIC=\${LIBDIR}/libkurtz-basic.a
LIBKURTZBASICDBG=\${LIBDIR}/libkurtz-basic.dbg.a
LIBKURTZ=\${LIBDIR}/libkurtz.a
LIBKURTZDBG=\${LIBDIR}/libkurtz.dbg.a
LIBKURTZEXTRA=\${LIBDIR}/libkurtzextra.a
LIBKURTZEXTRADBG=\${LIBDIR}/libkurtzextra.dbg.a
LIBMKVTREE=\${LIBDIR}/libmkvtree.a
LIBMKVTREEDBG=\${LIBDIR}/libmkvtree.dbg.a
LIBVMENGINE=\${LIBDIR}/libvmengine.a
LIBVMENGINEDBG=\${LIBDIR}/libvmengine.dbg.a
LIBVMATCH=\${LIBDIR}/libvmatch.a
LIBVMATCHDBG=\${LIBDIR}/libvmatch.dbg.a
LIBCHAIN=\${LIBDIR}/libchain.a
LIBCHAINDBG=\${LIBDIR}/libchain.dbg.a
LIBAUTOMATA=\${LIBDIR}/libautomata.a
LIBAUTOMATADBG=\${LIBDIR}/libautomata.dbg.a
LIBMULTIMAT=\${LIBDIR}/libmultimat.a
LIBMULTIMATDBG=\${LIBDIR}/libmultimat.dbg.a
HEREDOC

exit 0;
