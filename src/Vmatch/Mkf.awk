BEGIN{i=0;}

/.*/ { printf(">REGEXP QUERY%d\n%s\n",i++,$0); }
