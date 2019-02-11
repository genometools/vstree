BEGIN{
  numoffiles=0;
}
/.*/ {filenamelist[numoffiles++] = $1;}

END{
  printf("INCLUDEDIR=../../include\n\n");
  printf("HEADERFILES=\\\n");
  for(i=0; i<numoffiles; i++)
  {
    printf("    %s",filenamelist[i]);
    if(i<numoffiles-1)
    {
      printf("\\");
    }
    printf("\n");
  }
  printf("\nall:${HEADERFILES} Shareddef xmlfunc.c\n\n");
  printf(".PHONY:Shareddef\n");
  printf("Shareddef:../../Makedef\n");
  printf("\tgrep '^SHARED' ../../Makedef > $@\n\n");
  printf(".PHONY:xmlfunc.c\n");
  printf("xmlfunc.c:../xmlfunc.c\n");
  printf("\tcat $< | deletegeneric.pl '/' xmlshowdesc | egrep -v 'multiseq-adv.pr|genfile.h' > $@\n\n");
  for(i=0; i<numoffiles; i++)
  {
    printf("%s:${INCLUDEDIR}/%s\n",filenamelist[i],filenamelist[i]);
    printf("\tcp ${INCLUDEDIR}/%s .\n\n",filenamelist[i]);
  }
}
