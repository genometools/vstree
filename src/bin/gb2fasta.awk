BEGIN{
  firstline=1;
  sequence=0;
}

/^LOCUS/      {     
                if(firstline != 1)
                {
                  printf("file does not begin with LOCUS\n");
                  exit(1);
                }
                printf(">%s ",$2);
                firstline=0;
              }

/^DEFINITION/ { 
                for(i=2; i<=NF-1; i++)
                {
                  printf("%s ",$i);
                }
                printf("%s\n",$i);
                firstline=0;
              }

/^ORIGIN.*/   { 
                sequence = 1;
              }

/^[ ]*[0-9][0-9]*.*/ {  if(sequence == 1)
                        {
                          for(i=2; i<=NF; i++)
                          {
                            printf("%s",$i);
                          }
                          printf("\n");
                        }
                      }
