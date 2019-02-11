#!/usr/bin/env ruby

puts STDIN.read.gsub!(/<p class=\"noindent\" > <!--delete paragraph-->(.*)\n?<\/p\>/,"\\1\n")
