#!/usr/bin/env ruby

if ARGV.length != 1 then
  STDERR.puts "Usage: #{$0} <directory>"
  exit 1
end

print <<'TEXT'
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
 <head>
 <meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
 <title>Download page for Vmatch</title>
 <link rel="stylesheet" type="text/css" href="style.css">
</head>

<body>
<h1>Download page for Vmatch</h1>
<p>


<h2>We currently have the following distributions of Vmatch available:</h2>
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

This should echo a 
<a href="Vmatch-dirlist.txt">directory listing</a>
on your terminal.

<p>

The Vmatch distribution comes with
a comprehensive manual. This and other information
can also be found at the
<a href="http://www.vmatch.de">Vmatch-web-site</a>.

<p>

Stefan Kurtz, last change on 2007-10-19.

</body>
</html>
TEXT
