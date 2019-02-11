/^# dbfile=/ {printf("FILE %s\n",$2);}
/^# overall space peak:/ {printf("SPACE %s %s\n",$5,$9);}
/TIME/ {print $0;}
