LATEX=latex
clean:
	rm -f *.inc *.aux *.dvi *.log *.ps *.idx *.blg *.bbl
	rm -f *.ilg *.ind *.toc *.out
	rm -f Ispell.tmp

Cfiles.dvi:Cfiles.ind
	Geninc.sh
	$LATEX Cfiles.tex

Cfiles.ind:Cfiles.idx
	makeindex -q Cfiles.idx -o $@

Ispell:
	cat ../*.c ../../include/*.h | c2lit.x | grep -v '^>' > Ispell.tmp
