include ../Makedef

.SUFFIXES: .dbg.o .c .o .inc

CFLAGS=${DEFINECFLAGS} ${DEFINECPPFLAGS}
SPLINTFLAGS=${WITHSYSCONF} ${DEFINESPLINTFLAGS} ${DEFINECPPFLAGS}
COMPILEDIR=${COMPILEDIRPREFIX}/kurtz/

ifeq ($(licensemanager),no)
  CFLAGS += -DNOLICENSEMANAGER
endif

-include Filelists.mf

all:mkdircompiledir prototypes $(LIBKURTZ) $(LIBKURTZDBG)

$(LIBKURTZ):$(LIBOBJECTS)
	$(AR) ruv $@ $(LIBOBJECTS)
	$(RANLIB) $@

-include Filegoals.mf

$(LIBKURTZDBG):$(LIBDEBUGOBJECTS)
	$(AR) ruv $@ $(LIBDEBUGOBJECTS)
	$(RANLIB) $@

release:
	vmrelease.sh VMATCH ${CFLAGS} > ../include/vmrelease.h

.PHONY:clean
clean:splintclean
	rm -f *.[oa] *.dbg.o *.inc *.aux *.dvi *.log

cflagsstring:
	@echo "${CFLAGS}"

libtest:$(LIBKURTZ) $(LIBKURTZDBG)
	${MAKE} -C libtest

.PHONY:prototypes
prototypes:${PROTOTYPES}

-include $(wildcard ${COMPILEDIR}/*.d)
