include ../../Makedef
LD=${CC}
SUBINCLUDEOPT=${subst -I../,-I../../,${INCLUDEOPT}}
CFLAGS=${DEFINECFLAGS} -g ${SUBINCLUDEOPT} -DDEBUG 
CFLAGSWITHOUTDEBUG=${DEFINECFLAGS} -g ${SUBINCLUDEOPT}
LIBS=../${LIBKURTZDBG} ../${LIBKURTZBASICDBG} ${VSTREEBASEDIR}/../../../genometools/lib/libgenometools.a
LIBSDBG=../${LIBKURTZDBG} ../${LIBKURTZBASICDBG} ${VSTREEBASEDIR}/../../../genometools/lib/libgenometools.a
LDFLAGS=${DEFINELDFLAGS} 
LDLIBS=${DEFINELDLIBS}
#-lefence

SPLINTALL:=$(patsubst %.c,%.splint,$(wildcard *.c))

SPLINTFLAGS=-I../../include -DDEBUG -f ../../Splintoptions -casebreak

all:checkEvalue.x\
    readdb.x\
    strmfna.x\
    addmulti.x\
    rmulfast.x\
    checkgalign.x\
    checkxalign.x\
    checkcluster.x\
    rblastm.x\
    codon.x\
    rsymmap.x\
    verifycld.x\
    mygzip.x\
    runredblack.x\
    rundict.x\
    suffixprefix.x\
    spacemanage.x\
    regioncheck.x\
    checklongdist.x\
    checkqsort.x\
    vmatch-mini.x\
    strm-selfcmp.x\
    fillscores.x\
    chainqhits.x\
    checkredrange.x\
    queuetest.x

checkEvalue.x:checkEvalue.o ${LIBSDBG}
	${LD} ${LDFLAGS} checkEvalue.o ${LIBSDBG} ${LDLIBS} -o $@

readdb.x:readdb.o ${LIBS}
	${LD} ${LDFLAGS} readdb.o ${LIBS} ${LDLIBS} -o $@

strmfna.x:strmfna.o ${LIBS}
	${LD} ${LDFLAGS} strmfna.o ${LIBS} ${LDLIBS} -o $@

readdb.o:readdb.c
	${CC} -c ${CFLAGS} ${CPPFLAGS} readdb.c

suffixprefix.x:suffixprefix.o
	${LD} ${LDFLAGS} suffixprefix.o ${LIBSDBG} ${LDLIBS} -o $@

checkgalign.x:checkgalign.o optsimple.o ${LIBSDBG}
	${LD} ${LDFLAGS} checkgalign.o optsimple.o\
              ${LIBSDBG} ${LDLIBS} -o $@

codon.x:codon.mn.o ${LIBSDBG}
	${LD} ${LDFLAGS} codon.mn.o ${LIBSDBG} ${LDLIBS} -o $@

checkuedist.x:checkuedist.o ${LIBSDBG}
	${LD} ${LDFLAGS} checkuedist.o ${LIBSDBG} ${LDLIBS} -o $@

regioncheck.x:regioncheck.o ${LIBSDBG}
	${LD} ${LDFLAGS} regioncheck.o ${LIBSDBG} ${LDLIBS} -o $@

checkxalign.x:checkxalign.o optsimple.o ${LIBSDBG}
	${LD} ${LDFLAGS} checkxalign.o optsimple.o ${LIBSDBG} ${LDLIBS} -o $@

checkcluster.x:checkcluster.o ${LIBSDBG}
	${LD} ${LDFLAGS} checkcluster.o ${LIBSDBG} ${LDLIBS} -o $@

rsymmap.x:rsymmap.o ${LIBSDBG}
	${LD} ${LDFLAGS} rsymmap.o ${LIBSDBG} ${LDLIBS} -o $@

rmulfast.x:rmulfast.mn.o ${LIBSDBG}
	${LD} ${LDFLAGS} rmulfast.mn.o ${LIBSDBG} ${LDLIBS} -o $@

rblastm.x:rblastm.mn.o ${LIBSDBG}
	${LD} ${LDFLAGS} rblastm.mn.o ${LIBSDBG} ${LDLIBS} -o $@

verifycld.x:verifycld.o ${LIBSDBG}
	${LD} ${LDFLAGS} verifycld.o ${LIBSDBG} ${LDLIBS} -o $@

basename.x:basename.o ${LIBSDBG}
	${LD} ${LDFLAGS} basename.o ${LIBSDBG} ${LIBGEN} ${LDLIBS} -o $@

bitvector.x:bitvector.o ${LIBSDBG}
	${LD} ${LDFLAGS} bitvector.o ${LIBSDBG} ${LIBGEN} ${LDLIBS} -o $@

mygzip.x:mygzip.o ${LIBSDBG}
	${LD} ${LDFLAGS} mygzip.o ${LIBSDBG} ${LDLIBS} -o $@

getEvalue.x:${LIBS} getEvalue.o
	${LD} ${LDFLAGS} getEvalue.o ${LIBS} ${LDLIBS} -o $@

runredblack.x:runredblack.o ${LIBSDBG}
	${LD} ${LDFLAGS} runredblack.o ${LIBSDBG} ${LDLIBS} -o $@

rundict.x:rundict.o ${LIBSDBG}
	${LD} ${LDFLAGS} rundict.o ${LIBSDBG} ${LDLIBS} -o $@

fillscores.x:fillscores.o ${LIBSDBG}
	${LD} ${LDFLAGS} fillscores.o ${LIBSDBG} ${LDLIBS} -o $@

vmatch-mini.x:vmatch-mini.o ../${LIBVMENGINEDBG} ${LIBSDBG}
	${LD} ${LDFLAGS} vmatch-mini.o ../${LIBVMENGINEDBG} ${LIBSDBG} ${LDLIBS} -o $@

strm-selfcmp.x:strm-selfcmp.o ../${LIBVMENGINEDBG} ${LIBSDBG}
	${LD} ${LDFLAGS} strm-selfcmp.o ../${LIBVMENGINEDBG} ${LIBSDBG} ${LDLIBS} -o $@

queuetest.x:queuetest.o ${LIBSDBG}
	${LD} ${LDFLAGS} queuetest.o ${LIBSDBG} ${LDLIBS} -o $@

chainqhits.x:chainqhits.o ${LIBSDBG}
	${LD} ${LDFLAGS} chainqhits.o ${LIBSDBG} ${LDLIBS} -o $@

# We do not need the dynamic space allocation if MULTISEQSTATIC is set
# to check this I have include only those object files from the lib
# which are actually needed. If MULTISEQSTATIC is not set, then add
# ../space.dbg.o

addmulti.x:addmulti.o ${LIBSDBG}
	${LD} ${LDFLAGS} addmulti.o ${LIBSDBG} ${LDLIBS} -o $@

spacemanage.x:spacemanage.c ${LIBS}
	${CC} -c ${CFLAGSWITHOUTDEBUG} spacemanage.c
	${LD} ${LDFLAGS} spacemanage.o ${LIBS} ${LDLIBS} -o $@

checklongdist.x:checklongdist.c ${LIBS}
	${CC} -c ${CFLAGSWITHOUTDEBUG} checklongdist.c
	${LD} ${LDFLAGS} checklongdist.o ${LIBS} ${LDLIBS} -o $@

checkqsort.x:checkqsort.c ${LIBS}
	${CC} -c ${CFLAGSWITHOUTDEBUG} checkqsort.c
	${LD} ${LDFLAGS} checkqsort.o ${LIBS} ${LDLIBS} -o $@

checkredrange.x:checkredrange.c ${LIBS}
	${CC} -c ${CFLAGSWITHOUTDEBUG} checkredrange.c
	${LD} ${LDFLAGS} checkredrange.o ${LIBS} ${LDLIBS} -o $@

-include Filegoals.mf

.PHONY:clean
clean:splintclean
	rm -f *.[ox] shit* at1MB tmp[123]
	cleanpp.sh

cflagsstring:
	@echo "${CFLAGS} ${CPPFLAGS}"

-include $(wildcard *.d)
