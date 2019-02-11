/FILE/  {filename=$2; print $0;}
/SPACE/ {printf("SPACE %s %s %.2f\n",prog,filename,$2+$3);}
/TIME/  {printf("TIME %s %s %.2f\n",prog,$2,$3);}
