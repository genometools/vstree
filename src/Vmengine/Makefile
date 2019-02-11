include ../Makedef

.SUFFIXES: .dbg.o .c .o .inc

CFLAGS=${DEFINECFLAGS} ${DEFINECPPFLAGS}
SPLINTFLAGS=${WITHSYSCONF} ${DEFINESPLINTFLAGS} ${DEFINECPPFLAGS}
COMPILEDIR=${COMPILEDIRPREFIX}/Vmengine/
VMENGINEKERN=fcomplete.c\
             fmumself.c\
             fquery.c\
             fself.c\
             fsuper.c\
             ftandem.c

-include Vmatdef1.mf

-include Filelists.mf

LIBVMENGINEOBJ=${LIBOBJECTS}\
               ${VMAT}\
               ${COMPILEDIR}vmatfind-dyn.o\
               ${COMPILEDIR}vmatfind-strm.o

LIBDEBUGVMENGINEOBJ=${LIBDEBUGOBJECTS}\
                    ${VMATDBG}\
                    ${COMPILEDIR}vmatfind-dyn.dbg.o\
                    ${COMPILEDIR}vmatfind-strm.dbg.o

all:mkdircompiledir ${LIBVMENGINE} ${LIBVMENGINEDBG}

${LIBVMENGINE}:${LIBVMENGINEOBJ}
	$(AR) ruv $@ ${LIBVMENGINEOBJ}
	$(RANLIB) $@

-include Filegoals.mf

${LIBVMENGINEDBG}:${LIBDEBUGVMENGINEOBJ}
	$(AR) ruv $@ ${LIBDEBUGVMENGINEOBJ}
	$(RANLIB) $@

.PHONY:clean
clean:splintclean
	rm -f *.[oa] *.dbg.o *.inc *.aux *.dvi *.log

cflagsstring:
	@echo "${CFLAGS}"

libtest:${LIBVMENGINE} ${LIBVMENGINEDBG}
	${MAKE} -C libtest

vmengineexport.h:${VMENGINEKERN}
	cat ${VMENGINEKERN} | skproto > $@

${COMPILEDIR}vmatfind-dyn.o:vmatfind.c ../include/virtualdef.h
	${CC} ${CFLAGS} -DDYNAMICALPHABET -c vmatfind.c -o $@

${COMPILEDIR}vmatfind-strm.o:vmatfind.c ../include/esastream.h
	${CC} ${CFLAGS} -DDYNAMICALPHABET -DESASTREAMACCESS -c vmatfind.c -o $@

${COMPILEDIR}vmatfind-dyn.dbg.o:vmatfind.c ../include/virtualdef.h
	${CC} ${CFLAGS} -DDEBUG -DDYNAMICALPHABET -c vmatfind.c -o $@

${COMPILEDIR}vmatfind-strm.dbg.o:vmatfind.c ../include/esastream.h
	${CC} ${CFLAGS} -DDEBUG -DDYNAMICALPHABET -DESASTREAMACCESS -c vmatfind.c -o $@

vmatfind4.splint:vmatfind.c
	splint ${SPLINTFLAGS} -DDEBUG -DALPHABETSIZE=4 vmatfind.c
	touch vmatfind.c

vmatcount4.splint:vmatcount.c
	splint ${SPLINTFLAGS} -DDEBUG -DALPHABETSIZE=4 vmatcount.c
	touch vmatcount.c

-include $(wildcard ${COMPILEDIR}/*.d)

-include Vmatdef2.mf
