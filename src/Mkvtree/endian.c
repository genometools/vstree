#include <stdlib.h>
#include <stdio.h>

#define CHECKARGNUM(N,S)\
        if (argc != N)\
        {\
          fprintf(stderr,"Usage: %s %s\n",argv[0],S);\
          exit(EXIT_FAILURE);\
        }

#define FILEOPEN(FP,FILENAME,MODE)\
        if ((FP = fopen(FILENAME,MODE)) == NULL)\
        {\
          fprintf(stderr,"(%s,%d): Cannot open file \"%s\"\n",\
                  __FILE__,__LINE__,FILENAME);\
          exit(EXIT_FAILURE);\
        }

typedef unsigned short Ushort;
typedef unsigned int Uint;

static Uint parseUint(char *s)
{
  int i;
  if(sscanf(s,"%d",&i) != 1 || i < 0)
  {
    fprintf(stderr,"invalid argument \"%s\"\n",s);
    exit(EXIT_FAILURE);
  }
  return (Uint) i;
}

static Ushort convertUshort(Ushort v)
{
  return ((v & ((Ushort) 255)) << 8) | ((v & (((Ushort) 255) << 8)) >> 8);
}

static Uint convertUint(Uint v)
{
  return ((v &        255U)  << 24) | 
         ((v & (255U <<  8)) <<  8) |
         ((v & (255U << 16)) >>  8) |
         ((v & (255U << 24)) >> 24);
}

#define ENDIAN(TYPE)\
        void readint##TYPE(char *program,FILE *fp)\
        {\
          TYPE buf;\
          while(1)\
          {\
            if(fread(&buf,sizeof(TYPE),(size_t) 1,fp) != (size_t) 1)\
            {\
              return;\
            }\
            buf = convert##TYPE(buf);\
            if(fwrite(&buf,sizeof(TYPE),(size_t) 1,stdout) != (size_t) 1)\
            {\
              fprintf(stderr,"%s: cannot write 1 item of type %s",\
                             program,#TYPE);\
              exit(EXIT_FAILURE);\
            }\
          }\
        }

ENDIAN(Ushort)
ENDIAN(Uint)

int main(int argc, char *argv[])
{
  Uint bytes;
  FILE *fp;

  CHECKARGNUM(3,"bytes filename");

  FILEOPEN(fp,argv[2],"r");
  bytes = parseUint(argv[1]);
  switch(bytes)
  {
    case 2: readintUshort(argv[0],fp);
            break;
    case 4: readintUint(argv[0],fp);
            break;
    default: fprintf(stderr,"%s: first argument \"%s\" must be 2 or 4\n",
                    argv[0],argv[1]);
             return EXIT_FAILURE;
  }
  if(fclose(fp) != 0)
  {
    fprintf(stderr,"%s: cannot close file \"%s\"\n",argv[0],argv[1]);
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
