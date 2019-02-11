#!/usr/bin/env ruby

def checkpalindromic(argv,numofargs,referenceindex)
  (referenceindex+1).upto(numofargs-1) do |i|
    if /^-/.match(argv[i])
      return true
    end
  end
  return false
end

usage = "Usage: $0 (-allfiles|-reference|-queries|-pqueries|-queriesdropsign|" +
        "-palindromic|-nonfileparms) args\n"

numofargs = ARGV.length

if numofargs <= 2
  STDERR.puts usage
  exit 1
end

option = ARGV[0]
referenceindex = 0

(numofargs-1).downto(1) do |i| 
  if /^[0-9]+$/.match(ARGV[i])
    referenceindex = i+1
    break
  end
end

if option == '-allfiles'
  referenceindex.upto(numofargs-1) do |i|
    print ARGV[i]
    if i < numofargs-1
      print " "
    end
  end
elsif option == '-reference'
  printf("%s",ARGV[referenceindex])
elsif option == '-queries' || option == '-pqueries'
  (referenceindex+1).upto(numofargs-1) do |i|
    if option == '-pqueries'
      print "-#{ARGV[i]}"
    else
      print ARGV[i]
    end
    if i < numofargs-1
      print " "
    end
  end
elsif option == '-queriesdropsign'
  (referenceindex+1).upto(numofargs-1) do |i|
    query = ARGV[i]
    if /^[-+]/.match(query)
      query = query.gsub(/^[-+]/,"")
    end
    print query
    if i < numofargs-1
      print " "
    end
  end
elsif option == '-palindromic'
  if checkpalindromic(ARGV,numofargs,referenceindex)
    print "p\n"
  else
    print "d\n"
  end
elsif option == '-nonfileparms'
  1.upto(referenceindex-1) do |i|
    if ARGV[i] != 'd' && ARGV[i] != 'p'
      printf ARGV[i]
      if i < referenceindex-1
        print " "
      end
    end
  end
else
  STDERR.puts usage
  exit 1
end
print "\n"
