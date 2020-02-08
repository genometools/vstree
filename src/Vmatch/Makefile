include ../Makedef

#DEFINEVMATCHDB=-DVMATCHDB
#DISTRIBUTEDDFS=-DDISTRIBUTEDDFS
CFLAGS=${DEFINECFLAGS} ${DEFINECPPFLAGS} -I../Vmengine ${DEFINEVMATCHDB} ${DISTRIBUTEDDFS}
SPLINTFLAGS=${DEFINESPLINTFLAGS} -I../Vmengine
#CFLAGS=-DCOUNT
#LIBELECTRICFENCE=-lefence
LDFLAGS=${DEFINELDFLAGS} ${LIBELECTRICFENCE}
LDLIBS=${DEFINELDLIBS}
COMPILEDIR=${COMPILEDIRPREFIX}/Vmatch/
#EXECDIR=${EXECDIRPREFIX}/Vmatch/

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

LIBS=${LIBVMATCH}\
     ${LIBVMENGINE}\
     ${LIBMKVTREE}\
     ${LIBKURTZ}\
     ${LIBKURTZBASIC}

LIBSDBG=${LIBVMATCHDBG}\
        ${LIBVMENGINEDBG}\
        ${LIBMKVTREEDBG}\
        ${LIBKURTZDBG}\
        ${LIBKURTZBASICDBG}

PROTOEXPORT=assigndig.pr\
            echomatch.pr\
            detmatch.pr\
            procargs.pr\
            parsevm.pr\
            vmatch.pr\
            chncallparse.pr\
            chainof.pr

.SUFFIXES: .dbg.o .c .o .pr

LIBVMATCHOBJ=\
          ${COMPILEDIR}vmatch.o\
          ${COMPILEDIR}parsevm.o\
          ${COMPILEDIR}optstring.o\
          ${COMPILEDIR}procargs.o\
          ${COMPILEDIR}runself.o\
          ${COMPILEDIR}runquery.o\
          ${COMPILEDIR}mlendistrib.o\
          ${COMPILEDIR}mapdemand.o\
          ${COMPILEDIR}procmatch.o\
          ${COMPILEDIR}queryindex.o\
          ${COMPILEDIR}verifysub.o\
          ${COMPILEDIR}lrseq.o\
          ${COMPILEDIR}echomatch.o\
          ${COMPILEDIR}substpos.o\
          ${COMPILEDIR}markmat.o\
          ${COMPILEDIR}nomatch.o\
          ${COMPILEDIR}showmasked.o\
          ${COMPILEDIR}vmcluster.o\
          ${COMPILEDIR}assigndig.o\
          ${COMPILEDIR}explain.o\
          ${COMPILEDIR}detmatch.o\
          ${COMPILEDIR}opensel.o\
          ${COMPILEDIR}vmdbfunc.o\
          ${COMPILEDIR}parsedbms.o\
          ${COMPILEDIR}chncallparse.o\
          ${COMPILEDIR}chainvm.o\
          ${COMPILEDIR}chainof.o\
          ${COMPILEDIR}initpost.o\
          ${COMPILEDIR}matchlenparm.o\
          ${COMPILEDIR}xmlfunc.o\
          ${COMPILEDIR}vplugin-open.o\
          ${COMPILEDIR}vmotif-start.o\
          ${COMPILEDIR}cpridx-start.o\
          ${COMPILEDIR}parsepp.o\
          ${COMPILEDIR}parsequery.o\
          ${COMPILEDIR}parsedbcl.o\
          ${COMPILEDIR}keepflags.o\
          ${COMPILEDIR}parsemcl.o\
          ${COMPILEDIR}cluedist.o\
          ${COMPILEDIR}matchclust.o\
          ${COMPILEDIR}clpos.o\
          ${COMPILEDIR}allmclust.o\
          ${COMPILEDIR}threadchain.o\
          ${COMPILEDIR}procfinal.o\
          ${COMPILEDIR}mokay.o

LIBVMATCHDBGOBJ=\
          ${COMPILEDIR}vmatch.dbg.o\
          ${COMPILEDIR}parsevm.dbg.o\
          ${COMPILEDIR}optstring.dbg.o\
          ${COMPILEDIR}procargs.dbg.o\
          ${COMPILEDIR}runself.dbg.o\
          ${COMPILEDIR}runquery.dbg.o\
          ${COMPILEDIR}mlendistrib.dbg.o\
          ${COMPILEDIR}mapdemand.dbg.o\
          ${COMPILEDIR}procmatch.dbg.o\
          ${COMPILEDIR}queryindex.dbg.o\
          ${COMPILEDIR}verifysub.dbg.o\
          ${COMPILEDIR}lrseq.dbg.o\
          ${COMPILEDIR}echomatch.dbg.o\
          ${COMPILEDIR}substpos.dbg.o\
          ${COMPILEDIR}markmat.dbg.o\
          ${COMPILEDIR}nomatch.dbg.o\
          ${COMPILEDIR}showmasked.dbg.o\
          ${COMPILEDIR}vmcluster.dbg.o\
          ${COMPILEDIR}assigndig.dbg.o\
          ${COMPILEDIR}explain.dbg.o\
          ${COMPILEDIR}detmatch.dbg.o\
          ${COMPILEDIR}opensel.dbg.o\
          ${COMPILEDIR}vmdbfunc.dbg.o\
          ${COMPILEDIR}parsedbms.dbg.o\
          ${COMPILEDIR}chncallparse.dbg.o\
          ${COMPILEDIR}chainvm.dbg.o\
          ${COMPILEDIR}chainof.dbg.o\
          ${COMPILEDIR}initpost.dbg.o\
          ${COMPILEDIR}matchlenparm.dbg.o\
          ${COMPILEDIR}xmlfunc.dbg.o\
          ${COMPILEDIR}vplugin-open.dbg.o\
          ${COMPILEDIR}vmotif-start.dbg.o\
          ${COMPILEDIR}cpridx-start.dbg.o\
          ${COMPILEDIR}parsepp.dbg.o\
          ${COMPILEDIR}parsequery.dbg.o\
          ${COMPILEDIR}parsedbcl.dbg.o\
          ${COMPILEDIR}keepflags.dbg.o\
          ${COMPILEDIR}parsemcl.dbg.o\
          ${COMPILEDIR}cluedist.dbg.o\
          ${COMPILEDIR}matchclust.dbg.o\
          ${COMPILEDIR}clpos.dbg.o\
          ${COMPILEDIR}allmclust.dbg.o\
          ${COMPILEDIR}threadchain.dbg.o\
          ${COMPILEDIR}procfinal.dbg.o\
          ${COMPILEDIR}mokay.dbg.o

EXPORTPROGS=chain2dim.x\
            matchcluster.x\
            vmatch.x\
            vmatchselect.x

all:mkdircompiledir\
    libs\
    ${EXECDIR}vmatch.x\
    ${EXECDIR}vmatch.dbg.x\
    ${EXECDIR}readm.x\
    ${EXECDIR}readm.dbg.x\
    ${EXECDIR}vmatchselect.x\
    ${EXECDIR}vmatchselect.dbg.x\
    ${EXECDIR}chain2dim.x\
    ${EXECDIR}chain2dim.dbg.x\
    ${EXECDIR}matchcluster.x\
    ${EXECDIR}matchcluster.dbg.x\
    apmcontain.x\
    virtualexp.h

check:
	make SELECT
	env -i MKVTREESMAPDIR=../Mkvtree/TRANS \
        CONFIGGUESS=${CONFIGGUESS} \
	PATH="/usr/bin:/bin:../bin:../bin/${CONFIGGUESS}" \
	LD_LIBRARY_PATH="./SELECT" \
	./Checkall.sh
	env -i ../bin/cleanpp.sh

libs:${LIBVMATCH} ${LIBVMATCHDBG}

bin:${EXPORTPROGS}
	cp ${EXPORTPROGS} ${BINDIRROOT}/`binarypath.sh vmatch.x`

${EXECDIR}vmatch.x:${COMPILEDIR}vmatch.mn.o ${LIBS}
	${LD} ${LDFLAGS} $< ${LIBS} ${LDLIBS} -o $@

${EXECDIR}vmatch.dbg.x:${COMPILEDIR}vmatch.mn.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} $< ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}vmatchselect.x:${COMPILEDIR}vmatsel.mn.o ${LIBS}
	${LD} ${LDFLAGS} $< ${LIBS} ${LDLIBS} -o $@

${EXECDIR}vmatchselect.dbg.x:${COMPILEDIR}vmatsel.mn.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} $< ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}readm.x:${COMPILEDIR}readm.mn.o ${LIBS}
	${LD} ${LDFLAGS} $< ${LIBS} ${LDLIBS} -o $@

${EXECDIR}readm.dbg.x:${COMPILEDIR}readm.mn.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} $< ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}chain2dim.x:${COMPILEDIR}chain2dim.mn.o ${LIBS}
	${LD} ${LDFLAGS} $< ${LIBS} ${LDLIBS} -o $@

${EXECDIR}chain2dim.dbg.x:${COMPILEDIR}chain2dim.mn.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} $< ${LIBSDBG} ${LDLIBS} -o $@

${EXECDIR}matchcluster.x:${COMPILEDIR}matchcl.mn.o ${LIBS}
	${LD} ${LDFLAGS} $< ${LIBS} ${LDLIBS} -o $@

${EXECDIR}matchcluster.dbg.x:${COMPILEDIR}matchcl.mn.dbg.o ${LIBSDBG}
	${LD} ${LDFLAGS} $< ${LIBSDBG} ${LDLIBS} -o $@

profiler.x:${COMPILEDIR}profiler.o ${LIBKURTZ}
	${LD} ${LDFLAGS} $< ${LIBKURTZ} ${LDLIBS} -o $@

profiler.dbg.x:${COMPILEDIR}profiler.dbg.o ${LIBKURTZDBG}
	${LD} ${LDFLAGS} $< ${LIBKURTZDBG} ${LDLIBS} -o $@

apmcontain.x:${COMPILEDIR}apmcontain.o ${LIBS}
	${LD} ${LDFLAGS} $< ${LIBS} ${LDLIBS} -o $@

${LIBVMATCH}:${LIBVMATCHOBJ}
	$(AR) ruv $@ ${LIBVMATCHOBJ}
	$(RANLIB) $@

${LIBVMATCHDBG}:${LIBVMATCHDBGOBJ}
	$(AR) ruv $@ ${LIBVMATCHDBGOBJ}
	$(RANLIB) $@

vmotif-demo.o:vmotif-demo.c
	${CC} ${CFLAGS} ${CPPFLAGS} -fPIC -c $< -o $@

cpridxps-demo.o:cpridxps-demo.c
	${CC} ${CFLAGS} ${CPPFLAGS} -fPIC -c $< -o $@

vmotif-demo.so:vmotif-demo.o ${LIBKURTZ}
	${CC} ${SHARED} $< ${LIBKURTZ} -o $@

cpridxps-demo.so:cpridxps-demo.o ${LIBKURTZ}
	${CC} ${SHARED} $< ${LIBKURTZ} -o $@


.PHONY:SELECT
SELECT:../Makedef
	${MAKE} -C SELECT -f Makefile.src all
	${MAKE} -C SELECT

Vmcallstab.pm:Argtest.lhs
	runhugs -h2M $<
	chmod u+x $@

.PHONY:clean
clean:splintclean
	rm -f *.[oxa] *.so shit* R100000ACGT R100000acgt tmp[12]
	rm -f query.tmp complete.tmp tmp.match match.tmp test.match .shit
	rm -f tmp tmp2.sort tmp0 tmp0.match .tmp Tmp.index Tmp3 Tmp4 
	rm -f Pattern.tmp
	rm -f .tmp1 .tmp2
	rm -f clout.[0-9]*.[0-9]*.match PAIRS.* MATCH.* SUPERMATCH.*
	rm -rf SELECT/*.so.dSYM/
	${MAKE} -C SELECT clean

.PHONY:cleanpp
cleanpp:
	cleanpp.sh
	rm -f shit.[0-9]*.fna shit.[0-9]*.match shit
	rm -f shit1 shit2 shit2.* shit3
	rm -f tmp.match shitlebrit tmp[12] *.tmp .tmp
	rm -f Tmp.*

virtualexp.h:${PROTOEXPORT}
	@cat ../Copyright > $@
	@echo "/* This file is generated. Do not edit */" >> $@
	@echo "/* It contains the prototypes for some important exported functions. */" >> $@
	@echo "#ifndef VIRTUALEXP_H" >> $@
	@echo "#define VIRTUALEXP_H" >> $@
	@echo "#include \"matchinfo.h\"" >> $@
	@echo "#include \"matchtask.h\"" >> $@
	@echo "#include \"chaindef.h\"" >> $@
	@echo "#include \"xdropdef.h\"" >> $@
	@cat ${PROTOEXPORT} >> $@
	@echo "#endif" >> $@

.PHONY:mkincludes
mkincludes:
	Mkincludes.sh

-include Filegoals.mf

cflagsstring:
	@echo "${CFLAGS} ${CPPFLAGS} -DALPHABETSIZE=4"

-include $(wildcard ${COMPILEDIR}/*.d)
