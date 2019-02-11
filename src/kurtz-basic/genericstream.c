#include "types.h"
#include "genstream.h"
#include "checkgzip.pr"

void opengenericstream(Genericstream *inputstream,const char *inputfile)
{
  if(checkgzipsuffix(inputfile))
  {
    inputstream->stream.gzippedstream = gzopen(inputfile,"rb");
    if(inputstream->stream.gzippedstream == NULL)
    {
      fprintf(stderr,"cannot open file \"%s\"\n",inputfile);
      exit(EXIT_FAILURE);
    }
    inputstream->isgzippedstream = True;
  } else
  {
    inputstream->stream.fopenstream = fopen(inputfile,"rb");
    if(inputstream->stream.fopenstream == NULL)
    {
      fprintf(stderr,"cannot open file \"%s\"\n",inputfile);
      exit(EXIT_FAILURE);
    }
    inputstream->isgzippedstream = False;
  }
}

void closegenericstream(Genericstream *inputstream,const char *inputfile)
{
  Fclosereturntype retval;

  if(inputstream->isgzippedstream)
  {
    retval = gzclose(inputstream->stream.gzippedstream);
  } else
  {
    retval = fclose(inputstream->stream.fopenstream);
  }
  if(retval != 0)
  {
    fprintf(stderr,"cannot close file \"%s\"\n",inputfile);
    exit(EXIT_FAILURE);
  }
}

void rewindgenericstream(Genericstream *inputstream)
{
  if(inputstream->isgzippedstream)
  {
    (void) gzrewind(inputstream->stream.gzippedstream);
  } else
  {
    rewind(inputstream->stream.fopenstream);
  }
}
