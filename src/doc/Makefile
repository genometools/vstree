VIRTMANTMP=virtman-tmp
MULTIMATTMP=multimat-tmp
CHAIN2DIMTMP=chain2dim-tmp
MATCHCLUSTERTMP=matchcluster-tmp
TALLYMERTMP=tallymer-tmp
UNIQUESUBTMP=uniquesub-tmp

VIRTMANSRC=${VIRTMANTMP}.tex\
           mkvhelpinc.tex\
           vmhelpinc.tex\
           Excludetab.tex\
           Dataflow.png

VIRTMANDIR=virtman-src

AT=${CURDIR}/../testdata/at1MB

all:virtman.pdf\
    multimat.pdf\
    mkvtreeman.pdf\
    chain2dim.pdf\
    matchcluster.pdf\
    tallymer.pdf\
    uniquesub.pdf\

${VIRTMANTMP}.tex:virtman.tex execcommand.pl
	findemptyoutfiles.sh
	execcommand.pl $< > $@

${MULTIMATTMP}.tex:multimat.tex execcommand.pl
	splitmultifasta.pl ATs 70 0 ATsmall
	findemptyoutfiles.sh
	execcommand.pl $< > $@

${CHAIN2DIMTMP}.tex:chain2dim.tex execcommand.pl
	findemptyoutfiles.sh
	execcommand.pl $< > $@

${MATCHCLUSTERTMP}.tex:matchcluster.tex execcommand.pl
	findemptyoutfiles.sh
	execcommand.pl $< > $@

${TALLYMERTMP}.tex:tallymer.tex
	findemptyoutfiles.sh
	head -n 6999 ${AT} > read1.fna
	echo ">" > read2.fna
	tail -n 6946 ${AT} > read2.fna
	execcommand.pl $< > $@

${UNIQUESUBTMP}.tex:uniquesub.tex
	findemptyoutfiles.sh
	head -n 2 ${AT} > indexfile.fna
	echo ">sequence 1" > queryfile.fna
	tail -n 2 ${AT} | head >> queryfile.fna
	echo ">sequence 2" >> queryfile.fna
	tail -n 1 ${AT} | head -n 1 >> queryfile.fna
	execcommand.pl $< > $@

virtman.pdf:${VIRTMANSRC}
	touch ${VIRTMANTMP}.ind
	pdflatex ${VIRTMANTMP}
	pdflatex ${VIRTMANTMP}
	makeindex -q ${VIRTMANTMP}.idx
	bibtex ${VIRTMANTMP}
	pdflatex ${VIRTMANTMP}
	bibtex ${VIRTMANTMP}
	pdflatex ${VIRTMANTMP}
	pdflatex ${VIRTMANTMP}
	mv  ${VIRTMANTMP}.pdf $@

mkvtreeman.pdf:${VIRTMANSRC}
	touch mkvtreeman.ind
	pdflatex mkvtreeman
	pdflatex mkvtreeman
	makeindex -q mkvtreeman.idx
	bibtex mkvtreeman
	pdflatex mkvtreeman
	bibtex mkvtreeman
	pdflatex mkvtreeman
	pdflatex mkvtreeman

virtman.dvi:${VIRTMANSRC}
	touch ${VIRTMANTMP}.ind
	latex ${VIRTMANTMP}
	latex ${VIRTMANTMP}
	makeindex -q ${VIRTMANTMP}.idx
	bibtex ${VIRTMANTMP}
	latex ${VIRTMANTMP}
	bibtex ${VIRTMANTMP}
	latex ${VIRTMANTMP}
	latex ${VIRTMANTMP}
	mv  ${VIRTMANTMP}.dvi $@

virtman/virtman.html:${VIRTMANSRC}
	touch ${VIRTMANTMP}.ind
	latex ${VIRTMANTMP}
	makeindex -q ${VIRTMANTMP}.idx
	bibtex ${VIRTMANTMP}
	latex ${VIRTMANTMP}
	latex2html -split 4\
                   -nonavigation\
                   -noaddress\
                   -html_version 4.0,math\
                   -no_math ${VIRTMANTMP}.tex

multimat.pdf:${MULTIMATTMP}.tex
	pdflatex ${MULTIMATTMP}
	bibtex ${MULTIMATTMP}
	pdflatex ${MULTIMATTMP}
	mv ${MULTIMATTMP}.pdf $@

multimat.dvi:${MULTIMATTMP}.tex
	latex ${MULTIMATTMP}
	bibtex ${MULTIMATTMP}
	latex ${MULTIMATTMP}
	latex ${MULTIMATTMP}
	mv ${MULTIMATTMP}.dvi $@

tallymer.dvi:${TALLYMERTMP}.tex
	latex ${TALLYMERTMP}
	bibtex ${TALLYMERTMP}
	latex ${TALLYMERTMP}
	latex ${TALLYMERTMP}
	mv ${TALLYMERTMP}.dvi $@

tallymer.pdf:${TALLYMERTMP}.tex
	pdflatex ${TALLYMERTMP}
	pdflatex ${TALLYMERTMP}
	mv ${TALLYMERTMP}.pdf $@

uniquesub.dvi:${UNIQUESUBTMP}.tex
	latex ${UNIQUESUBTMP}
	latex ${UNIQUESUBTMP}
	mv ${UNIQUESUBTMP}.dvi $@

uniquesub.pdf:${UNIQUESUBTMP}.tex
	pdflatex ${UNIQUESUBTMP}
	pdflatex ${UNIQUESUBTMP}
	mv ${UNIQUESUBTMP}.pdf $@

ecolicmp.of:ecolicmp.vm
	Vmatchtrans.pl open $< > $@

chain2dim.pdf:${CHAIN2DIMTMP}.tex ecolicmp.of
	pdflatex ${CHAIN2DIMTMP}
	pdflatex ${CHAIN2DIMTMP}
	mv ${CHAIN2DIMTMP}.pdf $@

chain2dim.dvi:${CHAIN2DIMTMP}.tex ecolicmp.of
	latex ${CHAIN2DIMTMP}
	latex ${CHAIN2DIMTMP}
	mv ${CHAIN2DIMTMP}.dvi $@

matchcluster.pdf:${MATCHCLUSTERTMP}.tex
	pdflatex ${MATCHCLUSTERTMP}.tex
	bibtex ${MATCHCLUSTERTMP}
	pdflatex ${MATCHCLUSTERTMP}.tex
	bibtex ${MATCHCLUSTERTMP}
	pdflatex ${MATCHCLUSTERTMP}.tex
	pdflatex ${MATCHCLUSTERTMP}.tex
	cp ${MATCHCLUSTERTMP}.pdf $@

matchcluster.dvi:${MATCHCLUSTERTMP}.tex
	latex ${MATCHCLUSTERTMP}.tex
	bibtex ${MATCHCLUSTERTMP}
	latex ${MATCHCLUSTERTMP}.tex
	bibtex ${MATCHCLUSTERTMP}
	latex ${MATCHCLUSTERTMP}.tex
	latex ${MATCHCLUSTERTMP}.tex
	cp ${MATCHCLUSTERTMP}.dvi $@

Dataflow.inc:Dataflow.pic
	pic -t Dataflow.pic > $@

Dataflow.png:Dataflow.tex Dataflow.inc
	latex Dataflow
	latex Dataflow
	dvips -P www -o Dataflow.ps Dataflow.dvi
	convert Dataflow.ps $@

skaff.sty:
	cp ${HOME}/texinputs/$@ .

optionman.sty:
	cp ${HOME}/texinputs/$@ .

vmotif.opt:${VIRTMANTMP}.tex
	echo "% This file is generated. Do not edit\n" >  $@
	parseOptions.pl dbnomatch dbmaskmatch online q d p h e\
                        identity selfun s showdesc absolute nodist\
                        noscore noidentity iub help $< >> $@

${VIRTMANTMP}.bib:${VIRTMANTMP}.tex
	aux2bib.pl ${VIRTMANTMP}.aux
	cat $(HOME)/bibinputs/defines.bib references.bib > $@
	rm -f references.bib

virtman-src:${VIRTMANSRC} ${VIRTMANTMP}.bib
	mkdir -p ${VIRTMANDIR} ${VIRTMANDIR}/output
	cp ${VIRTMANSRC} *.sty ${VIRTMANTMP}.bib ${VIRTMANDIR}
	sed -e 's/defines,kurtz,strings/virtman/'\
               ${VIRTMANTMP}.tex > ${VIRTMANDIR}/${VIRTMANTMP}.tex
	cp Mkf-virtman ${VIRTMANDIR}/Makefile
	cp output/*.out ${VIRTMANDIR}/output
	tar -cvzf virtman-src.tgz ${VIRTMANDIR}

.PHONY:clean
clean:
	rm -f *.toc *.ilg *.out *.idx *.ind *.dvi *.ps *.log *.aux
	rm -f *.bbl *.blg
	rm -f vmotif.opt comment.cut ecolicmp.of
	rm -rf virtman-src virtman-src.tgz ${VIRTMANTMP}.bib
	rm -rf CLatEST.*.match CLatEST.*.fna ${VIRTMANTMP}.* ${MULTIMATTMP}.*
	rm -f read1.fna read2.fna ${TALLYMERTMP}.*
	rm -f indexfile.fna queryfile.fna ${UNIQUESUBTMP}.*
	rm -f clout.*.match
	rm -f ATs-[01][0-9]
	rm -rf ${CHAIN2DIMTMP}.* ${MATCHCLUSTERTMP}.*
	rm -f *+match *-match U89959.fna
	cleanpp.sh

#Excludetab.tex:
#../Vmatch/vmatch.x -l 3 Tmp > $@
