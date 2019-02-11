#!/usr/bin/env ruby

if ARGV.length != 1 then
  STDERR.puts "Usage: #{$0} <directory>"
  exit 1
end

print <<'TEXT'
<html>
 <head>
  <title>Download page for Vmatch and GenAlyzer</title>
 </head>
 <body>
<h1>Download page for Vmatch and GenAlyzer</h1>
<p>

<h2>We currently have the following Vmatch and GenAlyzer distributions available:</h2>
<p>

<div class="list">
<table summary="Directory Listing" cellpadding="0" cellspacing="0">
<thead><tr><th class="n">Program&nbsp;&nbsp;&nbsp;</th>
           <th class="b">Architecture and Operating System&nbsp;&nbsp;&nbsp;</th>
           <th class="m">Last Modified&nbsp;&nbsp;&nbsp;</th>
           <th class="s">Size</th>
</tr></thead>
<tbody>
TEXT

def filesize(filename)
  if File.exist?(filename)
  begin
    f = File.stat(filename)
  rescue => err
    STDERR.puts "Could not open file #{filename}: #{err}"
    exit 1
  end
  else
    STDERR.puts "File #{filename} does not exist!"
    exit 1
  end
  return f.size/1024
end

def showgen(inputfile,arch,fs)
  print "<tr>"
  print "<td class=\"n\"><a href=\"#{inputfile}\">GenAlyzer</a>&nbsp;&nbsp;</td>"
  print "<td class=\"b\">#{arch}</td>"
  print "<td class=\"m\">2004-03-10</td>"
  print "<td class=\"s\">#{fs} KB</td>"
  print "</tr>\n"
end

distpath="/projects/vstree/share/distrib/packages/Distributions/" + ARGV[0]

Dir.entries(distpath).each do |inputfile|
  if not inputfile == "." || inputfile == ".." then
    fs = filesize(distpath + "/" + inputfile)
    machine = inputfile
    if /-32-bit/.match(inputfile) then
      mwsize="32 bit"
      machine=machine.gsub(/-32-bit.*/,"")
    elsif /-64-bit/.match(inputfile)
      mwsize="64 bit"
      machine=machine.gsub(/-64-bit.*/,"")
    else
      STDERR.puts "illegal machine word size for #{machine}"
      exit 1
    end
    program=machine.gsub(/-.*/,"")
    machine=machine.gsub(/^[a-z]*-/,"")
    if program == "vmatch" then
      print "<tr>"
      print "<td class=\"n\"><a href=\"#{inputfile}\">#{program}</a></td>"
      print "<td class=\"b\">#{machine} #{mwsize}</td>"
      print "<td class=\"m\">#{ARGV[0]}</td>"
      print "<td class=\"s\">#{fs} KB</td>"
      print "</tr>\n"
    end
  end
end
showgen("gendist.Mac-OSX.2004-03-08.tar.gz",
        "powerpc-apple-darwin 32 bit",
        "2500")
showgen("gendist.Redhat-linux.2004-03-08.tar.gz", 
        "IA 86 linux-gnu Redhat 32 bit",
        "2400")
showgen("gendist.SUN-sol2.2004-03-08.tar.gz",
        "sparc-sun-solaris 32 bit",
        "2500")
showgen("gendist.SuSe-linux.2004-03-08.tar.gz",
        "i686-pc-linux-gnu 32 bit",
        "2400")

print <<'TEXT'
</tbody>
</table>
<br>
Click on the corresponding link to download the appropriate
distribution.

<p>

Each distribution is a gzipped tar-file.
You can unzip and uncompress it using the following command: 

<p>

gzip -cd tar-file |   tar xvf - 

<p>

For a tar file with a Vmatch distribution this should echo a 
<A HREF="Vmatch-dirlist.txt">directory listing</A>
on your terminal.

<p>

For a tar file with a Genalyzer distribution this should echo a 
<A HREF="Genalyzer-dirlist.txt">directory listing</A>
on your terminal.

<p>

The Vmatch distribution as well as the Genalyzer distribution each come with
a comprehensive manual. For more information on these tools
see also the 
<A HREF="http://www.vmatch.de">Vmatch-web-site</A>
or the 
<A HREF="http://www.genalyzer.de">Genalyzer-web-site</A>.
See also <a href="genalyzer-features.pdf">genalyzer-features.pdf</a>
for an overview of the features of the GenAlyzer Program.

<p>

Note that Genalyzer
requires several non-standard libraries (e.g. gtk, libwww) which may be 
in different locations on your computer or which are not yet installed.
So if your version of Genalyzer does not work immediately, then type 

<p>

ldd genalyzer

<p>

or

<p>

otool -L genalyzer

<p>

for the Mac.

<p>

If one of the reported lines matches the phrase 'not found', then  
a library is missing. You may have to add certain paths to your 
environment variable

<p>

LD_LIBRARY_PATH

<p>

You may also want to contact your local 
system administration and ask for help. If all fails, then
please contact Stefan Kurtz. 

<p>

Stefan Kurtz, last change on 2007-10-18.

</body>
</html>
TEXT
