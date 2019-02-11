function getiter(iter,sizeoffile)
{
  if(iter > 0)
  {
    return iter;
  } 
  if(sizeoffile < 10000)
  {
    return 1000;
  } else
  {
    if(sizeoffile < 100000)
    {
      return 100;
    } else
    {
      return 10;
    }
  }
}

BEGIN{nof=1; 
      nop=1; 
      showspace=0;
      compresstimefound=0;
      compresssizefound=0;
      spacefound=0;
      decompresstimefound=0;
      constimefound=0;
      searchtimefound=0;
      brjoptionsfound=0;
      lazyinfofound=0;
      intspernode["mcc5"]=5;
      intspernode["mcc4a"]=4;
      intspernode["mcc4b"]=4;
      intspernode["mcc3"]=3;
      showalphasize=0;
      usegreybox=0;
      showcsize=0;
      showthroughput=0;
      showtime=1;
      showbestspace=0;
      leastlen=0;
      rotate=0;
      withheader=1;
      precision=3;
      }

/^FILE/ {
         if($3 >= leastlen)
         {
           file2num[$2]=nof;
           filesize[nof]=$3;
           alphasize[nof]=$4;
           sumfilesize+=$3;
           num2file[nof++]=$2;
         }
        }
/^PROG/ {prog2num[$2]=nop;
         num2prog[nop++]=$2}
/^BRJOPTION/     {brjoptions=$2 $3; brjoptionsfound=1;}
/^LAZYINFO/       {tpc=$2; maxpat=$3; lazyinfofound=1;}
/^COMPRESSTIME/   {compresstime[prog2num[$2],file2num[$3]]+=$4;
                   compresstimefound=1;}
/^COMPRESSSIZE/   {compresssize[prog2num[$2],file2num[$3]]=$4; 
                   compresssizefound=1;}
/^TIME/          {time[prog2num[$2],file2num[$3]]+=$4; 
                   timefound=1;}
/^NODES/          {branchnodes[file2num[$2]]=$3; 
                   branchnodesum+=$3;
                   allocnodes[file2num[$2]]=$4; 
                   allocnodesum+=$4;
                   nodesfound=1;}
/^SPACE/          {spaceval = $4;
                   space[prog2num[$2],file2num[$3]]+=spaceval; 
                   spacefound=1;}
/^DECOMPRESSTIME/ {decompresstime[prog2num[$2],file2num[$3]]+=$4;
                   decompresstimefound=1;}
/^CONSTIME/       {constime[prog2num[$2],file2num[$3]]+=$4;
                   constimefound=1;}
/^SEARCHTIME/     {searchtime[prog2num[$2],file2num[$3]]+=$4;
                   searchtimefound=1;}
/^DATE/ {datestring=$2;}
/^COMPILER/ {compiler=$2;}
/^ITERATIONS/ {iterations=$2;}
/^MACHINE/ {machine=$2;}
END{
  nop--;
  nof--;
  if(withheader)
  {
    print("\\documentclass[10pt]{article}");
    print("\\usepackage{dina4}");
    if(rotate)
    {
      print("\\usepackage{rotating}");
    }
    if(usegreybox)
    {
      print("\\usepackage{color,colortbl}");
    }
    print("\\newcommand{\\Csrate}{$\\frac{\\mathit{csize}\\cdot 8}{\\mathit{size}}$}");
    print("\\newcommand{\\MCone}[1]{\\multicolumn{1}{|c|}{#1}}");
    print("\\newcommand{\\Showtag}[1]{\\MCone{$\\mathit{#1}$}}");
    print("\\newcommand{\\Ctrate}{\\MCone{$\\frac{\\mathit{size}}{\\mathit{ctime}}$}}");
    print("\\newcommand{\\Dtrate}{\\MCone{$\\frac{\\mathit{size}}{\\mathit{dtime}}$}}");
    if(usegreybox)
    {
      print("\\newcolumntype{G}{>{\\columncolor[gray]{.9}}r}");
      print("\\newcommand{\\Grey}[1]{\\multicolumn{1}{|G|}{#1}}");
    } else
    {
      print("\\newcommand{\\Grey}[1]{\\underline{#1}}");
    }
    print("\\begin{document}");
  }
  colsperprog=0;
  if(compresssizefound == 1 && showcsize == 1)
  {
    colsperprog += 2;
  } 
  if(compresstimefound == 1)
  {
    colsperprog += 1;
  }
  if(showspace == 1 && spacefound == 1)
  {
    colsperprog += 1;
  }
  if(nodesfound == 1)
  {
    colsperprog += 1;
  }
  if(timefound == 1)
  {
    if(showtime == 1)
    {
      colsperprog += 1;
    }
    if(showthroughput == 1)
    {
      colsperprog += 1;
    } 
  }
  if(decompresstimefound == 1)
  {
    colsperprog += 1;
  }
  if(constimefound == 1)
  {
    colsperprog += 1;
  }
  if(searchtimefound == 1)
  {
    colsperprog += 1;
  }
  progcolumns = nop * colsperprog;
  if(nodesfound == 1)
  {
    nodecolumns = 5;
  } else
  {
    nodecolumns = 1;
  }
  if(showalphasize == 1)
  {
    nodecolumns++;
  }
  if(rotate)
  {
    print("\\begin{sideways}");
  }
  printf("\\begin{tabular}{|l|*{%d}{r|}|*{%d}{r|}}\n",nodecolumns,progcolumns);
  totalcolumns = 1 + progcolumns + nodecolumns;
  print("\\hline");
  printf("\\multicolumn{%d}{|c|}{\\mbox{Results on %s (Date: %s) %s",
         totalcolumns,machine,datestring,compiler);
  if(lazyinfofound)
  {
    printf("trialperc=%s, maxpat=%s}}",tpc,maxpat);
  } else
  {
    printf("}}");
  }
  print("\\\\\\hline");
  for(iter=0; iter < nodecolumns; iter++)
  {
    printf("&");
  }
  for(k=1; k<=nop; k++)
  {
    if(brjoptionsfound)
    {
      printf("&\\multicolumn{%d}{c|}{%s~\\texttt{%s}}",
              colsperprog,num2prog[k],brjoptions);
    } else
    {
      printf("&\\multicolumn{%d}{c|}{%s}",colsperprog,num2prog[k]);
    }
    if(k==nop)
    {
      printf("\\\\\\hline");
    }
  }
  if(showalphasize == 1)
  {
    print("\\Showtag{file} &\\Showtag{size} &\\multicolumn{1}{c||}{$k$}");
  } else
  {
    print("\\Showtag{file} &\\multicolumn{1}{c||}{\\emph{size}}");
  }
  if(nodesfound == 1)
  {
    printf(" &\\MCone{$b$} &\\MCone{$b/n$}");
    printf(" &\\MCone{$\\Roundup{b}$} &\\multicolumn{1}{c||}{$\\Roundup{b}/n$}\n");
  }
  for(k=1; k<=nop; k++)
  {
    if(compresssizefound == 1 && showcsize == 1)
    {
      printf(" &\\Showtag{csize} &\\Csrate");
    }
    if(compresstimefound == 1)
    {
      printf(" &\\Showtag{ctime}");
    }
    if(timefound == 1)
    {
      if(showtime == 1)
      {
        printf(" &\\Showtag{time (sec)}");
      }
      if(showthroughput == 1)
      {
        printf(" &\\Showtag{tput}");
      }
    }
    if(decompresstimefound == 1)
    {
      printf(" &\\Showtag{dtime}");
    }
    if(showspace == 1 && spacefound == 1)
    {
      printf(" &\\Showtag{space (MB)}");
    }
    if(nodesfound == 1)
    {
      printf(" &\\Showtag{space}");
    }
    if(constimefound == 1)
    {
      printf(" &\\Showtag{constime}");
    }
    if(searchtimefound == 1)
    {
      printf(" &\\Showtag{searchtime}");
    }
  }
  print("\\\\\\hline\\hline");
  for(i=1; i<=nof; i++)
  {
    if(showalphasize == 1)
    {
      printf("%s &%d &%d",num2file[i],filesize[i],alphasize[i]);
    } else
    {
      printf("%s &%d",num2file[i],filesize[i]);
    }
    if(nodesfound == 1)
    {
      printf(" &%d &%.2f &%d &%.2f",branchnodes[i],branchnodes[i]/filesize[i],
                                allocnodes[i],allocnodes[i]/filesize[i]);
      branchnoderate+=branchnodes[i]/filesize[i];
      allocnoderate+=allocnodes[i]/filesize[i];
    }
    if(showspace == 1 && showbestspace == 1 && spacefound == 1)
    {
      bestspace = 100000.0;
      for(k=1; k<=nop; k++)
      {
        if(space[k,i] > 0.0 && bestspace > space[k,i])
        {
          bestspace = space[k,i];
        }
      }
    }
    if(showthroughput == 1)
    {
      bestthrough = 100000.0;
      for(k=1; k<=nop; k++)
      {
        timing = time[k,i]/getiter(iterations,filesize[i]);
        throughput = (1000000*timing)/filesize[i];
        if(bestthrough > throughput)
        {
          bestthrough = throughput;
        }
      }
    } else
    {
      bestthrough = 100000.0;
      for(k=1; k<=nop; k++)
      {
        timing = time[k,i]/getiter(iterations,filesize[i]);
        if(bestthrough > timing)
        {
          bestthrough = timing;
        }
      }
    }
    for(k=1; k<=nop; k++)
    {
      if(compresssizefound == 1 && showcsize == 1)
      {
        compressrate=(compresssize[k,i]*8)/filesize[i];
        compressratesum[k]+=compressrate;
        compresssizesum[k]+=compresssize[k,i];
        printf(" &%d &%.*f",compresssize[k,i],precision,compressrate);
      }
      if(compresstimefound == 1)
      {
        avgcompresstime=compresstime[k,i]/iterations;
        avgcompresstimesum[k] += avgcompresstime;
        ctimerate=filesize[i]/avgcompresstime;
        ctimeratesum[k]+=ctimerate;
        printf(" &%.2f",avgcompresstime);
      }
      if(timefound == 1)
      {
        timing = time[k,i]/getiter(iterations,filesize[i]);
        timesum[k] += timing;
        if(showtime == 1)
        {
          if(timing < 0.01)
          {
            thprec = 3;
          } else
          {
            thprec = 2;
          }
          printf(" &%.*f",thprec,timing);
        }
        if(showthroughput == 1)
        {
          throughput = (1000000*timing)/filesize[i];
          throughputsum[k] += throughput;
          if(throughput == bestthrough)
          {
            printf(" &\\Grey{%.2f}",throughput);
          } else
          {
            printf(" &%.2f",throughput);
	  }
	}
      }
      if(decompresstimefound == 1)
      {
        avgdecompresstime=decompresstime[k,i]/iterations;
        avgdecompresstimesum[k] += avgdecompresstime;
        dtimerate=filesize[i]/avgdecompresstime;
        dtimeratesum[k]+=dtimerate;
        printf(" &%.2f",avgdecompresstime);
      }
      if(nodesfound == 1)
      {
        streeints = intspernode[num2prog[k]]*allocnodes[i] + filesize[i] + 256;
        usedbytes[k,i] = filesize[i] + 4 * streeints;
        usedbytessum[k] += usedbytes[k,i];
        printf(" &%.2f",usedbytes[k,i]/1000000);
      }
      if(showspace == 1 && spacefound == 1)
      {
        spacesum[k] += space[k,i];
        printf(" &%.2f",space[k,i]);
      }
      if(constimefound == 1)
      {
        avgconstime=constime[k,i]/iterations;
        avgconstimesum[k] += avgconstime;
        printf(" &%.2f",avgconstime);
      }
      if(searchtimefound == 1)
      {
        avgsearchtime=(searchtime[k,i]-constime[k,i])/iterations;
        avgsearchtimesum[k] += avgsearchtime;
        printf(" &%.2f",avgsearchtime);
      }
    }
    print("\\\\\\hline");
  }
  print("\\hline");
  if(nodesfound == 1)
  {
    printf(" &%d & &%d &%.2f &%d &%.2f",sumfilesize,branchnodesum,
                                  branchnoderate/nof,
                                  allocnodesum,allocnoderate/nof);
  } else
  {
    printf(" &%d",sumfilesize)
  }
  if(showalphasize == 1)
  {
    printf(" &");
  }
  for(k=1; k<=nop; k++)
  {
    if(compresssizefound == 1 && showcsize == 1)
    {
      printf(" &%d &%.3f",compresssizesum[k],compressratesum[k]/nof);
    }
    if(compresstimefound == 1)
    {
      printf(" &%.2f",avgcompresstimesum[k]);
    }
    if(timefound == 1)
    {
      if(showtime == 1)
      {
        printf(" &%.2f",timesum[k]);
      }
      if(showthroughput == 1)
      {
        printf(" &%.2f",throughputsum[k]/nof);
      }
    }
    if(decompresstimefound == 1)
    {
      printf(" &%.2f",avgdecompresstimesum[k]);
    }
    if(showspace == 1 && spacefound == 1)
    {
      printf(" &%.2f",spacesum[k]);
    }
    if(nodesfound == 1)
    {
      printf(" &%.2f",usedbytessum[k]/1000000);
    }
    if(constimefound == 1)
    {
      printf(" &%.2f",avgconstimesum[k]/nof);
    }
    if(searchtimefound == 1)
    {
      printf(" &%.2f",avgsearchtimesum[k]/nof);
    }
  }
  print("\\\\\\hline");
  print("\\end{tabular}");
  if(withheader)
  {
    if(rotate)
    {
      print("\\end{sideways}");
    }
    print("\\end{document}");
  }
}
