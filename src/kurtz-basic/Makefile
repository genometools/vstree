include ../Makedef

.SUFFIXES: .dbg.o .c .o .inc

CFLAGS=${WITHSYSCONF} ${DEFINECFLAGS} ${DEFINECPPFLAGS}
SPLINTFLAGS=${WITHSYSCONF} ${DEFINESPLINTFLAGS} ${DEFINECPPFLAGS}
COMPILEDIR=${COMPILEDIRPREFIX}/kurtz-basic/

-include Filelists.mf

all:release mkdircompiledir prototypes $(LIBKURTZBASIC) $(LIBKURTZBASICDBG)

$(LIBKURTZBASIC):$(LIBOBJECTS)
	$(AR) ruv $@ $(LIBOBJECTS)
	$(RANLIB) $@

-include Filegoals.mf

$(LIBKURTZBASICDBG):$(LIBDEBUGOBJECTS)
	$(AR) ruv $@ $(LIBDEBUGOBJECTS)
	$(RANLIB) $@

release:
	../bin/vmrelease.sh VMATCH ${CFLAGS} ${CPPFLAGS} > ../include/vmrelease.h

chain2dim2.o:chain2dim.c
	${CC} ${CFLAGS} ${CPPFLAGS} -DREVERSEDIM -c $< -o $@

.PHONY:clean
clean:splintclean
	rm -f *.[oa] *.dbg.o *.inc *.aux *.dvi *.log

cflagsstring:
	@echo "${CFLAGS} ${CPPFLAGS}"

.PHONY:prototypes
prototypes:${PROTOTYPES}

-include $(wildcard ${COMPILEDIR}/*.d)
