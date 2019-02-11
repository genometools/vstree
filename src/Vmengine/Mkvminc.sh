#!/bin/sh

sizes="4 12 21 31 42 53 66"
outfile=Vmatdef1.mf
rm -f ${outfile}
touch ${outfile}

printf "VMAT=" >> ${outfile}
for size in ${sizes}
do
  printf "\${COMPILEDIR}vmatfind${size}.o \${COMPILEDIR}vmatcount${size}.o " >> ${outfile}
done
printf "\n\n" >> ${outfile}

printf "VMATDBG=" >> ${outfile}
for size in ${sizes}
do
  printf "\${COMPILEDIR}vmatfind${size}.dbg.o \${COMPILEDIR}vmatcount${size}.dbg.o " >> ${outfile}
done
printf "\n\n" >> ${outfile}

VMATFINDHEADERS="alphasize.h ../include/vdfstrav.c"

outfile=Vmatdef2.mf
rm -f ${outfile}
touch ${outfile}
for size in ${sizes}
do
  printf "\${COMPILEDIR}vmatfind${size}.o:vmatfind.c\n" >> ${outfile}
  printf "\t\$(CC) \$(CFLAGS) -DALPHABETSIZE=${size} -c vmatfind.c -o \$@\n\n" >> ${outfile}

  printf "\${COMPILEDIR}vmatcount${size}.o:vmatcount.c\n" >> ${outfile}
  printf "\t\$(CC) \$(CFLAGS) -DALPHABETSIZE=${size} -c vmatcount.c -o \$@\n\n" >> ${outfile}

  printf "\${COMPILEDIR}vmatfind${size}.dbg.o:vmatfind.c\n" >> ${outfile}
  printf "\t\$(CC) \$(CFLAGS) -DDEBUG -DALPHABETSIZE=${size} -c vmatfind.c -o \$@\n\n" >> ${outfile}

  printf "\${COMPILEDIR}vmatcount${size}.dbg.o:vmatcount.c\n" >> ${outfile}
  printf "\t\$(CC) \$(CFLAGS) -DDEBUG -DALPHABETSIZE=${size} -c vmatcount.c -o \$@\n\n" >> ${outfile}

  printf "\${COMPILEDIR}vmatfind${size}.o:${VMATFINDHEADERS}\n\n" >> ${outfile}

  printf "\${COMPILEDIR}vmatfind${size}.dbg.o:${VMATFINDHEADERS}\n\n" >> ${outfile}

  printf "\${COMPILEDIR}vmatcount${size}.o:${VMATFINDHEADERS}\n\n" >> ${outfile}

  printf "\${COMPILEDIR}vmatcount${size}.dbg.o:${VMATFINDHEADERS}\n\n" >> ${outfile}
done

outfile=vmatgen.c
rm -f ${outfile}
touch ${outfile}

cat ../Copyright >> ${outfile}
printf "/* This file is generated. Do not edit. */\n" >> ${outfile}
for size in ${sizes}
do
  printf " Sint vmatmaxout${size}(VMATMAXOUTTYPE);    // ALPHABETSIZE\n" >> ${outfile}
  printf " Sint vmatmaxcount${size}(VMATMAXCOUNTTYPE);  // ALPHABETSIZE\n" >> ${outfile}
done

printf "\n" >> ${outfile}
printf "Sint vmatmaxoutgeneric(Virtualtree *virtualtree,\n" >> ${outfile}
printf "                       Uint numberofprocessors,\n" >> ${outfile}
printf "                       Uint searchlength,\n" >> ${outfile}
printf "                       /*@unused@*/ void *repeatgapspec,\n" >> ${outfile}
printf "                       void *outinfo,\n" >> ${outfile}
printf "                       Outputfunction output)\n" >> ${outfile}
printf "{\n" >> ${outfile}

for size in ${sizes}
do
  printf "  MAXOUTCASE(${size});    // ALPHABETSIZE\n" >> ${outfile}
done

printf "  return 0;\n" >> ${outfile}
printf "}\n" >> ${outfile}

printf "\n" >> ${outfile}
printf "Sint vmatmaxcountgeneric(Virtualtree *virtualtree,\n" >> ${outfile}
printf "                         Uint numberofprocessors,\n" >> ${outfile}
printf "                         Uint searchlength,\n" >> ${outfile}
printf "                         ArrayUint *counttab)\n" >> ${outfile}
printf "{\n" >> ${outfile}

for size in ${sizes}
do
  printf "  MAXCOUNTCASE(${size});    // ALPHABETSIZE\n" >> ${outfile}
done

printf "  return 0;\n" >> ${outfile}
printf "}\n" >> ${outfile}

outfile=alphasize.h
rm -f ${outfile}
touch ${outfile}

cat ../Copyright >> ${outfile}
printf "/* This file is generated. Do not edit. */\n" >> ${outfile}
printf "#ifndef ALPHASIZE_H\n" >> ${outfile}
printf "#define ALPHASIZE_H\n" >> ${outfile}

for size in ${sizes}
do
  printf "#if (ALPHABETSIZE==${size})\n" >> ${outfile}
  printf "#define VMATMAXOUT vmatmaxout${size}\n" >> ${outfile}
  printf "#define VMATMAXCOUNT vmatmaxcount${size}\n" >> ${outfile}
  printf "#endif\n" >> ${outfile}
done
  
printf "#endif\n" >> ${outfile}
