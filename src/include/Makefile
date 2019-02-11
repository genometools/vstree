.SUFFIXES:.inc .h

INCFILES=absdef.inc\
         alignment.inc\
         alphadef.inc\
         args.inc\
         arraydef.inc\
         calloc.inc\
         chardef.inc\
         chartype.inc\
         clock.inc\
         clusterdef.inc\
         column.inc\
         compl.inc\
         count.inc\
         debugdef.inc\
         dpbitvec.inc\
         edge.inc\
         errordef.inc\
         experim.inc\
         fdrewr.inc\
         fopen.inc\
         inline.inc\
         intbits.inc\
         iter.inc\
         iubdef.inc\
         logbase.inc\
         longbits.inc\
         longpair.inc\
         match.inc\
         maxfiles.inc\
         megabytes.inc\
         minmax.inc\
         multidef.inc\
         optdesc.inc\
         output.inc\
         overflow.inc\
         processap.inc\
         protodef.inc\
         qsortdef.inc\
         rPair.inc\
         result.inc\
         rsubword.inc\
         scoredef.inc\
         spacedef.inc\
         trie.inc\
         types.inc\
         virtualdef.inc\
         visible.inc\
         xdropdef.inc

TMPHEADERS=libautomata.h\
           matchreport.h\
           skiptree.h\
           textutils.h\
           onlinesimplesearch.h\
           onlinesimplesearch.templ.c\
           onlinesimplesearchundef.h\
           vtreematchdepthfirst.h\
           vtreematchdepthfirst.templ.c\
           vtreematchdepthfirstundef.h\
           agrepexport.h\
           agrepdef.h

all:
	@echo "no default goal"

Cfiles.dvi:$(INCFILES) Cfiles.tex
	latex Cfiles.tex

.PHONY:clean
clean:
	rm -f *.inc Cfiles.dvi Cfiles.aux Cfiles.log
	rm -f ${TMPHEADERS}

%.inc:%.h
	c2lit.x $^ | lit2alt.x -C > $@

