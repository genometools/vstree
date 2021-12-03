SYSTEM:=$(shell uname -s)
MACHINE:=$(shell uname -m)
ifeq ($(64bit),yes)
  BIT=64bit
else
  BIT=32bit
endif
VERSION:="`cat $(CURDIR)/VERSION`"
SYSTEMNAME:="$(SYSTEM)_$(MACHINE)"
VMATCHDISTBASENAME:="vmatch-$(VERSION)-$(SYSTEMNAME)-$(BIT)"
DISTDIR:="$(CURDIR)/dist/$(SYSTEMNAME)"
VMATCHDISTDIR:="$(DISTDIR)/$(VMATCHDISTBASENAME)"

SELECTDIR=SELECT
SMAPDIR=TRANS
INSTALLDIR=${DIRVSTREE}
INSTALLINCLUDE=${INSTALLDIR}/include
INSTALLSHARE=${INSTALLDIR}/share
INSTALLDOC=${INSTALLSHARE}/doc
INSTALLBIN=${WORK}/bin-ops/${CONFIGGUESS}
INSTALLDVLBIN=${INSTALLSHARE}/dvlbin
# the commands to use to install binaries, directories and data
# files respectively (data files meaning anything, which is not a binary
# file or directory)
# INSTALL_CMD_BIN=install -m 755
INSTALL_CMD_BIN=cp
INSTALL_CMD_DIR=install -d
# INSTALL_CMD_DATA=install -m 644
INSTALL_CMD_DATA=cp

INSTALLLIST=Mkvtree/mkvtree.x\
	    Mkvtree/vseqinfo.x\
	    Mkvtree/vseqselect.x\
	    Mkvtree/vsubseqselect.x\
	    Mkvtree/vendian.x\
	    Mkvtree/vstree2tex.x\
	    Mkvtree/mkdna6idx.x\
	    Mkvtree/mkrcidx.x\
	    Mkvtree/mkcfr.x\
	    Mkvtree/mkcld.x\
	    Mkvtree/mkiso.x\
	    Mkvtree/mklsf.x\
	    Mkvtree/mksti.x\
	    Vmatch/vmatch.x\
	    Vmatch/vmatchselect.x\
	    Vmatch/matchcluster.x\
	    Vmatch/chain2dim.x

all:
	${MAKE} -C kurtz-basic
	${MAKE} -C kurtz
	${MAKE} -C Mkvtree
	${MAKE} -C Vmengine
	${MAKE} -C Vmatch

release:
	git tag "v`cat VERSION`"
	git push --tags origin master

libs:
	${MAKE} -C kurtz-basic
	${MAKE} -C kurtz
	${MAKE} -C Vmengine
	${MAKE} -C kurtz/libtest

vmatch:libs
	${MAKE} -C kurtz/libtest
	${MAKE} -C Mkvtree
	${MAKE} -C Vmatch

cleanup:
	find . -name '*.[oa]' | xargs rm -f

splintall:
	${MAKE} -C kurtz-basic splintall
	${MAKE} -C kurtz splintall
	${MAKE} -C Mkvtree splintall
	${MAKE} -C Vmengine splintall
	${MAKE} -C Vmatch splintall

splintclean:
	${MAKE} -C kurtz-basic splintclean
	${MAKE} -C kurtz splintclean
	${MAKE} -C Mkvtree splintclean
	${MAKE} -C Vmengine splintclean
	${MAKE} -C Vmatch splintclean

clean:
	${MAKE} -C Mkvtree clean cleanpp
	${MAKE} -C Vmengine clean
	${MAKE} -C Vmatch clean cleanpp
	${MAKE} -C kurtz-basic clean
	${MAKE} -C kurtz clean
	${MAKE} -C kurtz/libtest clean

cleanbuild:
	${MAKE} -C Mkvtree cleanbuild clean
	${MAKE} -C Vmatch cleanbuild clean
	${MAKE} -C kurtz cleanbuild clean
	${MAKE} -C kurtz-basic cleanbuild clean
	${MAKE} -C Vmengine cleanbuild clean

kurtz-basic.tar.gz:
	tar -cvf kurtz-basic.tar Makedef Makefile include lib kurtz-basic
	gzip -9 kurtz-basic.tar

cleanpp:
	${MAKE} -C Mkvtree cleanpp
	${MAKE} -C Vmatch cleanpp

install:
	bininstall.sh ${INSTALLLIST}
	${INSTALL_CMD_BIN} bin/cleanmkv.sh ${INSTALLBIN}/.
	${INSTALL_CMD_BIN} bin/cleanpp.sh ${INSTALLBIN}/.
	${INSTALL_CMD_BIN} bin/vmigrate.sh ${INSTALLBIN}/.
	${INSTALL_CMD_BIN} Emulate/repfind.pl ${INSTALLBIN}/.
	test -d ${INSTALLBIN}/SELECT || ${INSTALL_CMD_DIR} ${INSTALLBIN}/SELECT
	${INSTALL_CMD_DATA} Vmatch/SELECT/*.[ch] ${INSTALLBIN}/SELECT
	${INSTALL_CMD_DATA} Vmatch/SELECT/makefile ${INSTALLBIN}/SELECT
	${INSTALL_CMD_DATA} Vmatch/SELECT/Shareddef ${INSTALLBIN}/SELECT
	chmod ug+rx ${INSTALLBIN}/*.sh

srcinstall:
	mkdir -p ${INSTALLINCLUDE}
	mkdir -p ${INSTALLDOC}
	mkdir -p ${INSTALLDVLBIN}
	test -d ${INSTALLINCLUDE} || ${INSTALL_CMD_DIR} ${INSTALLINCLUDE}
	${INSTALL_CMD_DATA} include/*.[hc] include/*.pr ${INSTALLINCLUDE}
	${INSTALL_CMD_DATA} Vmatch/*.h ${INSTALLINCLUDE}
	${INSTALL_CMD_DATA} Vmengine/*.h ${INSTALLINCLUDE}
	libinstall.sh `grep '^PATHEND=' Makedef | sed 's/^PATHEND=//'`
	test -d ${INSTALLDOC} || ${INSTALL_CMD_DIR} ${INSTALLDOC}
	-${INSTALL_CMD_DATA} doc/virtman.pdf ${INSTALLDOC}
	-${INSTALL_CMD_DATA} doc/mkvtreeman.pdf ${INSTALLDOC}
	-${INSTALL_CMD_DATA} doc/multimat.pdf ${INSTALLDOC}
	-${INSTALL_CMD_DATA} doc/chain2dim.pdf ${INSTALLDOC}
	-${INSTALL_CMD_DATA} doc/matchcluster.pdf ${INSTALLDOC}
	-${INSTALL_CMD_DATA} doc/tallymer.pdf ${INSTALLDOC}
	-chmod ug+r ${INSTALLDOC}/*
	-test -d ${INSTALLDVLBIN} || ${INSTALL_CMD_DIR} ${INSTALLDVLBIN}
	-${INSTALL_CMD_DATA} bin/*.sh ${INSTALLDVLBIN}
	-chmod -f ug+r ${INSTALLDVLBIN}/*

check:
	cd Mkvtree; make check; cd ..
	cd Vmatch; make check; cd ..
	
.PHONY:SELECT
SELECT:
	${MAKE} -C Vmatch SELECT

STRIP:=strip

dist: SELECT
	@echo "[build distribution]"
	@rm -rf $(VMATCHDISTDIR)
	@mkdir -p $(VMATCHDISTDIR)
	@cp bin/vmigrate.sh $(VMATCHDISTDIR)
	@cp Emulate/repfind.pl $(VMATCHDISTDIR)
	@cp Mkvtree/upgradeprj.pl $(VMATCHDISTDIR)
ifeq ($(SYSTEM),Windows)
	@cp Mkvtree/mkvtree.x $(VMATCHDISTDIR)/mkvtree.exe
	@cp Mkvtree/vseqinfo.x $(VMATCHDISTDIR)/vseqinfo.exe
	@cp Mkvtree/vseqselect.x $(VMATCHDISTDIR)/vseqselect.exe
	@cp Mkvtree/vsubseqselect.x $(VMATCHDISTDIR)/vsubseqselect.exe
	@cp Mkvtree/vendian.x $(VMATCHDISTDIR)/vendian.exe
	@cp Mkvtree/vstree2tex.x $(VMATCHDISTDIR)/vstree2tex.exe
	@cp Mkvtree/mkdna6idx.x $(VMATCHDISTDIR)/mkdna6idx.exe
	@cp Vmatch/vmatch.x $(VMATCHDISTDIR)/vmatch.exe
	@cp Vmatch/vmatchselect.x $(VMATCHDISTDIR)/vmatchselect.exe
	@cp Vmatch/matchcluster.x $(VMATCHDISTDIR)/matchcluster.exe
	@cp Vmatch/chain2dim.x $(VMATCHDISTDIR)/chain2dim.exe
else
	@cp Mkvtree/mkvtree.x $(VMATCHDISTDIR)/mkvtree
	@cp Mkvtree/vseqinfo.x $(VMATCHDISTDIR)/vseqinfo
	@cp Mkvtree/vseqselect.x $(VMATCHDISTDIR)/vseqselect
	@cp Mkvtree/vsubseqselect.x $(VMATCHDISTDIR)/vsubseqselect
	@cp Mkvtree/vendian.x $(VMATCHDISTDIR)/vendian
	@cp Mkvtree/vstree2tex.x $(VMATCHDISTDIR)/vstree2tex
	@cp Mkvtree/mkdna6idx.x $(VMATCHDISTDIR)/mkdna6idx
	@cp Vmatch/vmatch.x $(VMATCHDISTDIR)/vmatch
	@cp Vmatch/vmatchselect.x $(VMATCHDISTDIR)/vmatchselect
	@cp Vmatch/matchcluster.x $(VMATCHDISTDIR)/matchcluster
	@cp Vmatch/chain2dim.x $(VMATCHDISTDIR)/chain2dim
endif
	@mkdir $(VMATCHDISTDIR)/${SMAPDIR}
	@cp Mkvtree/TRANS/Trans* $(VMATCHDISTDIR)/${SMAPDIR}
	@cp bin/cleanpp.sh $(VMATCHDISTDIR)
	@cp bin/Vmatchtrans.pl $(VMATCHDISTDIR)
	@cp doc/virtman.pdf $(VMATCHDISTDIR)
	@cp doc/chain2dim.pdf $(VMATCHDISTDIR)
	@cp doc/matchcluster.pdf $(VMATCHDISTDIR)
	@cp ../LICENSE $(VMATCHDISTDIR)
	@cp README.distrib $(VMATCHDISTDIR)
	@cp CHANGELOG $(VMATCHDISTDIR)
	@mkdir $(VMATCHDISTDIR)/${SELECTDIR}
	@checkforSELECTheaders.sh
	@cp Vmatch/SELECT/makefile $(VMATCHDISTDIR)/${SELECTDIR}
	@cp Vmatch/SELECT/*.[ch] $(VMATCHDISTDIR)/${SELECTDIR}
	@cp Vmatch/SELECT/Shareddef $(VMATCHDISTDIR)/${SELECTDIR}
ifeq ($(SYSTEM),Windows)
	@$(STRIP) $(VMATCHDISTDIR)/chain2dim.exe
	@$(STRIP) $(VMATCHDISTDIR)/matchcluster.exe
	@$(STRIP) $(VMATCHDISTDIR)/mkdna6idx.exe
	@$(STRIP) $(VMATCHDISTDIR)/mkvtree.exe
	@$(STRIP) $(VMATCHDISTDIR)/vendian.exe
	@$(STRIP) $(VMATCHDISTDIR)/vmatch.exe
	@$(STRIP) $(VMATCHDISTDIR)/vmatchselect.exe
	@$(STRIP) $(VMATCHDISTDIR)/vseqinfo.exe
	@$(STRIP) $(VMATCHDISTDIR)/vseqselect.exe
	@$(STRIP) $(VMATCHDISTDIR)/vstree2tex.exe
	@$(STRIP) $(VMATCHDISTDIR)/vsubseqselect.exe
else
	@$(STRIP) $(VMATCHDISTDIR)/chain2dim
	@$(STRIP) $(VMATCHDISTDIR)/matchcluster
	@$(STRIP) $(VMATCHDISTDIR)/mkdna6idx
	@$(STRIP) $(VMATCHDISTDIR)/mkvtree
	@$(STRIP) $(VMATCHDISTDIR)/vendian
	@$(STRIP) $(VMATCHDISTDIR)/vmatch
	@$(STRIP) $(VMATCHDISTDIR)/vmatchselect
	@$(STRIP) $(VMATCHDISTDIR)/vseqinfo
	@$(STRIP) $(VMATCHDISTDIR)/vseqselect
	@$(STRIP) $(VMATCHDISTDIR)/vstree2tex
	@$(STRIP) $(VMATCHDISTDIR)/vsubseqselect
endif
ifeq ($(SYSTEM),Windows)
	@cd $(DISTDIR) && 7z a -tzip $(VMATCHDISTBASENAME).zip $(VMATCHDISTBASENAME)
	@echo "$(DISTDIR)/$(VMATCHDISTBASENAME).zip"
else
	@cd $(DISTDIR) && tar --numeric-owner -cf $(VMATCHDISTBASENAME).tar $(VMATCHDISTBASENAME)
	@cd $(DISTDIR) && gzip -f -9 $(VMATCHDISTBASENAME).tar
	@echo "$(DISTDIR)/$(VMATCHDISTBASENAME).tar.gz"
endif
