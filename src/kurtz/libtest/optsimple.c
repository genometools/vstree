#include <string.h>
#include "optdesc.h"
#include "debugdef.h"
#include "spacedef.h"
#include "fhandledef.h"
#include "simpleopt.h"

#include "procopt.pr"
#include "applall.pr"
#include "filehandle.pr"
#include "dstrdup.pr"

static void showsimpleoptions(SimpleOptionvalues *optionvalues)
{
  switch(optionvalues->optnum)
  {
    case OPTTWOSTRINGS:
      printf("# two strings \"%s\" \"%s\"\n",optionvalues->string1,
                                             optionvalues->string2);
      break;
    case OPTTWOFILES:
      printf("# two files \"%s\" \"%s\"\n",optionvalues->file1,
                                           optionvalues->file2);
      break;
    case OPTTEXT:
      printf("# text \"%s\"\n",optionvalues->text);
      break;
    case OPTALPHALEN:
      printf("# alphalen \"%s\" %lu\n",
             optionvalues->charlist,
             (Showuint) optionvalues->lengthofstrings);
      break;
    default:
      fprintf(stderr,"optnum %lu not  recognized\n",
              (Showuint) optionvalues->optnum);
      exit(EXIT_FAILURE);
  }
}

void freesimpleoptions(SimpleOptionvalues *optionvalues)
{
  switch(optionvalues->optnum)
  {
    case OPTTWOSTRINGS:
      FREESPACE(optionvalues->string1);
      FREESPACE(optionvalues->string2);
      break;
    case OPTTWOFILES:
      FREESPACE(optionvalues->file1);
      FREESPACE(optionvalues->file2);
      break;
    case OPTTEXT:
      FREESPACE(optionvalues->text);
      break;
    case OPTALPHALEN:
      FREESPACE(optionvalues->charlist);
      break;
    default:
      fprintf(stderr,"optnum %lu not  recognized\n",
              (Showuint) optionvalues->optnum);
      exit(EXIT_FAILURE);
  }
}

Sint parsesimpleoptions(SimpleOptionvalues *optionvalues,
                        Argctype argc,
                        const char **argv)
{
  OptionDescription options[NUMOFOPTIONS];
  Uint argnum;
  Optionnumbertype optval;

  initoptions(&options[0],(Uint) NUMOFOPTIONS);
  ADDOPTION(OPTTWOSTRINGS,"-ss","use two strings");
  ADDOPTION(OPTTWOFILES,"-ff","use two files");
  ADDOPTION(OPTTEXT,"-t","use a text");
  ADDOPTION(OPTALPHALEN,"-a","use character list and length");
  ADDOPTION(OPTHELP,"-help","this option");
  if(argc == 1)
  {
    ERROR2("Usage: %s options\n"\
           "%s -help shows possible options",\
           argv[0],argv[0]);
    return (Sint) -1;
  }
  optionvalues->string1 = NULL;
  optionvalues->string2 = NULL;
  optionvalues->file1 = NULL;
  optionvalues->file2 = NULL;
  optionvalues->text = NULL;
  optionvalues->charlist = NULL;
  optionvalues->lengthofstrings = 0;
  for(argnum = UintConst(1);
      argnum < (Uint) argc && ISOPTION(argv[argnum]);
      argnum++)
  {
    optval = procoption(options,(Uint) NUMOFOPTIONS,argv[argnum]);
    if(optval < 0)
    {
      return (Sint) -4;
    }
    switch(optval)
    {
      case OPTHELP:
        showoptions(stdout,
                    argv[0],
                    options,
                    (Uint) NUMOFOPTIONS);
        return (Sint) 1;
      case OPTTWOFILES:
      case OPTTWOSTRINGS:
        if(MOREARGOPTSWITHLAST(argnum))
        {
          argnum++;
          if(optval == (Optionnumbertype) OPTTWOFILES)
          {
            ASSIGNDYNAMICSTRDUP(optionvalues->file1,argv[argnum]);
          } else
          {
            ASSIGNDYNAMICSTRDUP(optionvalues->string1,argv[argnum]);
          }
          if(MOREARGOPTSWITHLAST(argnum))
          {
            argnum++;
            if(optval == (Optionnumbertype) OPTTWOFILES)
            {
              ASSIGNDYNAMICSTRDUP(optionvalues->file2,argv[argnum]);
            } else
            {
              ASSIGNDYNAMICSTRDUP(optionvalues->string2,argv[argnum]);
            }
          } else
          {
            ERROR1("missing string argument to option \"%s\"",
                   options[optval].optname);
            return (Sint) -1;
          }
        } else
        {
          ERROR1("missing string argument to option \"%s\"",
                 options[optval].optname);
          return (Sint) -1;
        }
        if(optval == (Optionnumbertype) OPTTWOFILES)
        {
          optionvalues->optnum = OPTTWOFILES;
        } else
        {
          optionvalues->optnum = OPTTWOSTRINGS;
        }
        break;
      case OPTTEXT:
        if(MOREARGOPTSWITHLAST(argnum))
        {
          argnum++;
          ASSIGNDYNAMICSTRDUP(optionvalues->text,argv[argnum]);
        } else
        {
          ERROR1("missing string argument to option \"%s\"",
                 options[OPTTEXT].optname);
          return (Sint) -1;
        }
        optionvalues->optnum = OPTTEXT;
        break;
      case OPTALPHALEN:
        if(MOREARGOPTSWITHLAST(argnum))
        {
          argnum++;
          ASSIGNDYNAMICSTRDUP(optionvalues->charlist,argv[argnum]);
          if(MOREARGOPTSWITHLAST(argnum))
          {
            Scaninteger readint;
            argnum++;
            if(sscanf(argv[argnum],"%ld",&readint) != 1)
            {
              ERROR1("missing integer argument to option \"%s\"",
                     options[optval].optname);
              return (Sint) -1;
            } else
            {
              if(readint <= (Scaninteger) 1)
              {
                ERROR1("integer argument to option \"%s\" must be > 1",
                       options[optval].optname);
                return (Sint) -2;
              }
            }
            optionvalues->lengthofstrings = (Uint) readint;
          } else
          {
            ERROR1("missing string argument to option \"%s\"",
                   options[optval].optname);
            return (Sint) -1;
          }
        } else
        {
          ERROR1("missing string argument to option \"%s\"",
                 options[optval].optname);
          return (Sint) -1;
        }
        optionvalues->optnum = OPTALPHALEN;
        break;
    }
  }
  showsimpleoptions(optionvalues);
  return 0;
}

static Sint runcheckfunctionontwofiles(Checkalignfuntype checkfunction,
                                       char *file1,char *file2)
{
  Uchar *u, *v;
  Uint ulen, vlen;
  
  u = CREATEMEMORYMAP(file1,False,&ulen);
  if(u == NULL)
  {
    return (Sint) -1;
  }
  v = CREATEMEMORYMAP(file2,False,&vlen);
  if(v == NULL)
  {
    return (Sint) -2;
  }
  if(checkfunction(True,u,ulen,v,vlen) != 0)
  {
    return (Sint) -3;
  }
  if(checkfunction(False,u,ulen,v,vlen) != 0)
  {
    return (Sint) -4;
  }
  if(DELETEMEMORYMAP(u) != 0)
  {
    return (Sint) -5;
  }
  if(DELETEMEMORYMAP(v) != 0)
  {
    return (Sint) -6;
  }
  return 0;
}

static Sint runcheckfunctionontext(Checkalignfuntype checkfunction,char *text)
{
  Uint i, len;
  
  len = (Uint) strlen(text);
  for(i=UintConst(1); i<=len/2; i++)
  {
    if(checkfunction(True,(Uchar *) text,i,(Uchar *) (text+i),len-i) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint applycheckfunctiontotext(Uchar *text,Uint textlen,void *info)
{
  Uint i;
  Checkalignfuntype checkfunction = (Checkalignfuntype) info;

  printf("%s\n",(char *) text);
  for(i=0; i<=textlen/2; i++)
  {
    if(checkfunction(True,text,i,text+i,textlen-i) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint runcheckfunctiononalphalen(Checkalignfuntype checkfunction,
                                       char *charlist,
                                       Uint lengthofstrings)
{
  if(applyall(charlist,
              lengthofstrings,
              (void *) checkfunction,applycheckfunctiontotext) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

Sint applycheckfunctiontosimpleoptions(Checkalignfuntype checkfunction,
                                       Argctype argc,const char *argv[])
{
  SimpleOptionvalues optionvalues;
  Sint retval;
 
  retval = parsesimpleoptions(&optionvalues,argc,argv);
  if(retval < (Sint) 0)
  {
    return (Sint) -1;
  }
  if(retval == (Sint) 1)
  {
    return 0;
  }
  switch(optionvalues.optnum)
  {
    case OPTTWOSTRINGS:
      if(checkfunction(True,
                       (Uchar *) optionvalues.string1,
                       (Uint) strlen(optionvalues.string1),
                       (Uchar *) optionvalues.string2,
                       (Uint) strlen(optionvalues.string2)) != 0)
      {
        return (Sint) -2;
      }
      if(checkfunction(False,
                       (Uchar *) optionvalues.string1,
                       (Uint) strlen(optionvalues.string1),
                       (Uchar *) optionvalues.string2,
                       (Uint) strlen(optionvalues.string2)) != 0)
      {
        return (Sint) -3;
      }
      break;
    case OPTTWOFILES:
      if(runcheckfunctionontwofiles(checkfunction,
                                    optionvalues.file1,
                                    optionvalues.file2) != 0)
      {
        return (Sint) -4;
      }
      break;
    case OPTTEXT:
      if(runcheckfunctionontext(checkfunction,optionvalues.text) != 0)
      {
        return (Sint) -5;
      }
      break;
    case OPTALPHALEN:
      if(runcheckfunctiononalphalen(checkfunction,
                                    optionvalues.charlist,
                                    optionvalues.lengthofstrings) != 0)
      {
        return (Sint) -6;
      }
      break;
    default:
      fprintf(stderr,"optnum %lu not  recognized\n",
              (Showuint) optionvalues.optnum);
      exit(EXIT_FAILURE);
  }
  freesimpleoptions(&optionvalues);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return 0;
}
