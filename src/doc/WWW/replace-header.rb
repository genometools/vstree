#!/usr/bin/env ruby

STDIN.each_line do |line|
  if line.match(/<meta name=\"src\" content="vmweb.tex"/)
    print "#{line}"
    puts <<'HEADER'
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
<meta name="description" CONTENT="The Vmatch large scale sequence analysis
software is a versatile software tool for efficiently solving large scale sequence matching tasks."/>
<meta name="keywords" CONTENT="sequence analysis, sequence mapping, BLAST, bioinformatics, computational biology"/>
<meta http-equiv="Content-Style-Type" content="text/css"/>
HEADER
  elsif line.match(/class=\"titleHead|author|date\"/)
    print line.gsub(/class=/,"align=\"center\" class=").gsub(/h2/,"h1")
  elsif line.match(/\/h2/)
    print line.gsub(/\/h2/,"\/h1")
  else
    print line
  end
end
