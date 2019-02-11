#!/usr/bin/env ruby

def remove_html_tags(s)
  re = /<("[^"]*"|'[^']*'|[^'">])*>/
  return s.gsub(re, "")
end

ARGV.each do |filename|
  file = File.open(filename, "rb")
  contents = file.read
  print remove_html_tags(contents)
end
