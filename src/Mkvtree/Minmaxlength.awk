BEGIN{minlength=-1;
      maxlength=-1;
      countseq=0;
}

/.*/ {
       value=$2;
       if(maxlength == -1 || value > maxlength)
       {
         maxlength = value;
       }
       if(minlength == -1 || value < minlength)
       {
         minlength = value;
       }
       sumlength+=value;
       countseq++;
     }

END{
  printf("minlength=%d maxlength=%u averagelength=%d\n",
          minlength,maxlength,sumlength/countseq);
}
