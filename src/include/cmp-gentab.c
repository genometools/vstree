void COMPAREFUNCTION(char *tag,COMPARETYPE *tab1,COMPARETYPE *tab2,Uint len)
{
  Uint i;

  if(tab1 == NULL && tab2 == NULL)
  {
    printf("# %s is NULL\n",tag);
  } else
  {
    if(tab1 == NULL)
    {
      fprintf(stderr,"%s1 is NULL but %s2 not\n",tag,tag);
      exit(EXIT_FAILURE);
    }
    if(tab2 == NULL)
    {
      fprintf(stderr,"%s2 is NULL but %s1 not\n",tag,tag);
      exit(EXIT_FAILURE);
    }
    for(i=0; i < len; i++)
    {
      if(tab1[i] != tab2[i])
      {
        fprintf(stderr,"%s[%lu]: ",tag,(Showuint) i);
        COMPAREFORMAT(stderr,tab1[i]);
        fprintf(stderr," != ");
        COMPAREFORMAT(stderr,tab2[i]);
        fprintf(stderr,"\n");
        exit(EXIT_FAILURE);
      }
    }
    printf("# %s of length %lu identical\n",tag,(Showuint) len);
  }
}
