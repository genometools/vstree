/reading databasefile/ {printf("FILE %s ",$3);}
/total length of sequences:/ {printf("%d\n",$5);}
/overall space peak:/ {printf("SPACE %s %s\n",$4,$8);}
/TIME/ {print $0;}
