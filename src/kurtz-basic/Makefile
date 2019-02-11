include ../Makedef

.SUFFIXES: .dbg.o .c .o .inc

CFLAGS=${WITHSYSCONF} ${DEFINECFLAGS} ${DEFINECPPFLAGS}
SPLINTFLAGS=${WITHSYSCONF} ${DEFINESPLINTFLAGS} ${DEFINECPPFLAGS}
COMPILEDIR=${COMPILEDIRPREFIX}/kurtz-basic/

ifneq ($(licensemanager),no)
  CFLAGS += -I../../../licensemanager/src \
            -I../../../genometools/src
else
  CFLAGS += -DNOLICENSEMANAGER
endif

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
	vmrelease.sh VMATCH ${CFLAGS} > ../include/vmrelease.h

chain2dim2.o:chain2dim.c
	${CC} ${CFLAGS} -DREVERSEDIM -c $< -o $@

.PHONY:clean
clean:splintclean
	rm -f *.[oa] *.dbg.o *.inc *.aux *.dvi *.log

cflagsstring:
	@echo "${CFLAGS}"

.PHONY:prototypes
prototypes:${PROTOTYPES}

-include $(wildcard ${COMPILEDIR}/*.d)
