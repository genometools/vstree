#!/bin/sh
printf "types.h\n\
        alphadef.h\n\
        multidef.h\n\
        arraydef.h\n\
        match.h\n\
        maxfiles.h\n\
        evaluedef.h\n\
        absdef.h\n\
        errordef.h\n\
        failures.h\n\
        spacedef.h\n\
        redblackdef.h\n\
        minmax.h\n\
        chardef.h\n\
        scoredef.h\n\
        visible.h\n\
        vmrelease.h\n\
        codondef.h\n\
        xmlindent.h\n\
        select.h\n" | ${AWK} -f ../../bin/Mksrcmake.awk > Makefile.src
