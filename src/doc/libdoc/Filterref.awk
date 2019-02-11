BEGIN{}
/\\textsf/ { print "F " $3 " " $4}
/\\emph/   { print "T " $3 " " $4}
/\\texttt/ { print "D " $3 " " $4}
END{}
