include ../Makedef

MAINFLAGS=-DWITHLCP # ${DEFINESUFFIXPTR}
CFLAGS=${DEFINECFLAGS} ${DEFINECPPFLAGS} ${MAINFLAGS}
SPLINTFLAGS=${MAINFLAGS} ${DEFINESPLINTFLAGS}
VPATH=${COMPILEDIRPREFIX}
LDFLAGS=-L${COMPILEDIRPREFIX} ${DEFINELDFLAGS}
LDLIBS=${DEFINELDLIBS}
COMPILEDIR=${COMPILEDIRPREFIX}/Mkvtree/
#EXECDIR=${EXECDIRPREFIX}/Mkvtree/

ifneq ($(licensemanager),no)
  CFLAGS += -I../../../licensemanager/src \
            -I../../../genometools/src
  LDLIBS = ${CURDIR}/../../../licensemanager/lib/$(BIT)/liblicensemanager.a \
           ${CURDIR}/../../../genometools/lib/libgenometools.a \
           ${DEFINELDLIBS}
else
  CFLAGS += -DNOLICENSEMANAGER
  LDLIBS = ${CURDIR}/../../../genometools/lib/libgenometools.a \
           ${DEFINELDLIBS}
endif

MKVTREEOBJ=${COMPILEDIR}mkvtree.o\
           ${COMPILEDIR}mkvprocess.o\
           ${COMPILEDIR}bese.o\
           ${COMPILEDIR}besespecial.o\
           ${COMPILEDIR}ppsort.o\
           ${COMPILEDIR}mkvinput.o\
           ${COMPILEDIR}cprsuf.o\
           ${COMPILEDIR}remainsort.o

MKVTREEDBGOBJ=${COMPILEDIR}mkvtree.dbg.o\
              ${COMPILEDIR}mkvprocess.dbg.o\
              ${COMPILEDIR}bese.dbg.o\
              ${COMPILEDIR}besespecial.dbg.o\
              ${COMPILEDIR}ppsort.dbg.o\
              ${COMPILEDIR}mkvinput.dbg.o\
              ${COMPILEDIR}cprsuf.dbg.o\
              ${COMPILEDIR}remainsort.dbg.o

# add mkcfr.splint

EXPORTPROGS=${EXECDIR}mkvtree.x\
            ${EXECDIR}mkrcidx.x\
            ${EXECDIR}mkdna6idx.x\
            ${EXECDIR}mkcfr.x\
            ${EXECDIR}mklsf.x\
            ${EXECDIR}mkcld.x\
            ${EXECDIR}mksti.x\
            ${EXECDIR}mkiso.x\
            ${EXECDIR}vseqinfo.x\
            ${EXECDIR}vsubseqselect.x\
            ${EXECDIR}vendian.x\
            ${EXECDIR}vstree2tex.x\
            ${EXECDIR}vseqselect.x

LIBS=${LIBMKVTREE}\
     ${LIBKURTZ}\
     ${LIBKURTZBASIC}

LIBSDBG=${LIBMKVTREEDBG}\
        ${LIBKURTZDBG}\
        ${LIBKURTZBASICDBG}

.SUFFIXES: .dbg.o .c .o .pr

all:mkdircompiledir\
    ${EXPORTPROGS}\
    ${EXECDIR}allwhatsthere.x\
    ${EXECDIR}mapcfrcrf.x\
    ${EXECDIR}mkvram.x\
    ${EXECDIR}mkvcmp.dbg.x\
    ${EXECDIR}allwhatsthere.dbg.x\
    ${EXECDIR}mkrcidx.dbg.x\
    ${EXECDIR}mkdna6idx.dbg.x\
    ${EXECDIR}mkvtree.dbg.x

check:
	env -i MKVTREESMAPDIR=./TRANS ./Checkall.sh
	env -i ../bin/cleanpp.sh

bin:${EXPORTPROGS}
	cp ${EXPORTPROGS} ${BINDIRROOT}/`binarypath.sh mkvtree.x`

libs:${LIBMKVTREE} ${LIBMKVTREEDBG}

# the following programs are part of the mkvtree/vmatch distribution

${EXECDIR}mkvtree.x:${COMPILEDIR}mkvfile.o ${LIBS}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkvfile.o ${LIBS} ${LDLIBS} -o $@

${EXECDIR}mkvtree.dbg.x:${COMPILEDIR}mkvfile.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkvfile.dbg.o ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}vendian.x:${COMPILEDIR}endian.o
	${LD} ${LDFLAGS} ${COMPILEDIR}endian.o -o $@

${EXECDIR}vseqinfo.x:${COMPILEDIR}vseqinfo.o ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}vseqinfo.o ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}vseqselect.x:${COMPILEDIR}vseqselect.o ${LIBKURTZ} ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}vseqselect.o ${LIBKURTZ} ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}vsubseqselect.x:${COMPILEDIR}vsubseqselect.o \
                          ${LIBKURTZ}\
                          ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}vsubseqselect.o\
                         ${LIBKURTZ} ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}vstree2tex.x:${COMPILEDIR}vstree2tex.o ${LIBKURTZ} ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}vstree2tex.o ${LIBKURTZ} ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}allwhatsthere.x:${COMPILEDIR}allwhatsthere.o\
                          ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}allwhatsthere.o\
              ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}allwhatsthere.dbg.x:${COMPILEDIR}allwhatsthere.dbg.o ${LIBKURTZBASICDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}allwhatsthere.dbg.o\
              ${LIBKURTZBASICDBG} ${LDLIBS} -o $@

# the following programs are used to generate some extra table

${EXECDIR}mkvcmp.dbg.x:${COMPILEDIR}mkvcmp.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkvcmp.dbg.o ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}mkcfr.x:${COMPILEDIR}mkcfr.o\
                  ${LIBKURTZ}\
                  ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkcfr.o\
              ${LIBKURTZ} ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}mksti.x:${COMPILEDIR}mksti.o ${LIBKURTZ} ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}mksti.o\
              ${LIBKURTZ} ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}mapcfrcrf.x:${COMPILEDIR}mapcfrcrf.o ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}mapcfrcrf.o ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}mkcld.x:${COMPILEDIR}mkcld.mn.o ${LIBKURTZ} ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkcld.mn.o\
              ${LIBKURTZ} ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}mklsf.x:${COMPILEDIR}mklsf.o ${LIBKURTZ} ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}mklsf.o ${LIBKURTZ}\
              ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}mkiso.x:${COMPILEDIR}mkiso.o ${LIBKURTZ} ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkiso.o\
              ${LIBKURTZ} ${LIBKURTZBASIC} ${LDLIBS} -o $@

${EXECDIR}mkrcidx.x:${COMPILEDIR}mkrcidx.o ${LIBS}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkrcidx.o ${LIBS} ${LDLIBS} -o $@

${EXECDIR}mkdna6idx.x:${COMPILEDIR}mkdna6idx.o ${LIBS}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkdna6idx.o ${LIBS} ${LDLIBS} -o $@

# the following programs are mainly for testing purpose

${EXECDIR}mkvram.x:${COMPILEDIR}mkvram.o ${LIBS}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkvram.o ${LIBS} ${LDLIBS} -o $@

${EXECDIR}gccont.x:${COMPILEDIR}gccont.o ${LIBKURTZBASIC}
	${LD} ${LDFLAGS} ${COMPILEDIR}gccont.o ${LIBKURTZBASIC} -o $@

${EXECDIR}mklsf.dbg.x:${COMPILEDIR}mklsf.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}mklsf.dbg.o ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}vseqselect.dbg.x:${COMPILEDIR}vseqselect.dbg.o ${LIBKURTZBASICDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}vseqselect.dbg.o\
              ${LIBKURTZBASICDBG} ${LDLIBS} -o $@

${EXECDIR}makecld.dbg.x:${COMPILEDIR}makecld.dbg.o ${LIBKURTZBASICDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}makecld.dbg.o\
              ${LIBKURTZBASICDBG} ${LDLIBS} -o $@

${EXECDIR}mkrcidx.dbg.x:${COMPILEDIR}mkrcidx.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkrcidx.dbg.o ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}mkdna6idx.dbg.x:${COMPILEDIR}mkdna6idx.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} ${COMPILEDIR}mkdna6idx.dbg.o ${LIBSDBG} ${LDLIBS} -o $@

${COMPILEDIR}besespecial.o:besespecial.c
	${CC} ${CFLAGS} ${CPPFLAGS} -DSPECIAL -c besespecial.c -o $@

${COMPILEDIR}besespecial.dbg.o:besespecial.c
	${CC} ${CFLAGS} ${CPPFLAGS} -DSPECIAL -DDEBUG -c besespecial.c -o $@

besespecial.splint:besespecial.c
	splint ${SPLINTFLAGS} -DSPECIAL -DDEBUG besespecial.c
	touch $@

besespecial.c:bese.c
	cp bese.c $@

endian.c:endian.copy
	cp endian.copy $@

Calls.sh:Argtest.lhs
	runhugs -h2M $<
	chmod u+x $@

Mkvcallstab.pm:Argtest.lhs
	runhugs -h2M $<

${LIBMKVTREE}:${MKVTREEOBJ}
	$(AR) ruv $@ ${MKVTREEOBJ}
	$(RANLIB) $@

${LIBMKVTREEDBG}:${MKVTREEDBGOBJ}
	$(AR) ruv $@ ${MKVTREEDBGOBJ}
	$(RANLIB) $@

-include Filegoals.mf

.PHONY:clean
clean:splintclean
	rm -f *.[ox] besespecial.c

.PHONY:cleanpp
cleanpp:
	cleanpp.sh

cflagsstring:
	@echo "${CFLAGS} ${CPPFLAGS}"

-include $(wildcard ${COMPILEDIR}/*.d)
