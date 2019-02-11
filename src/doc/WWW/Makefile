vmwebfiles=Dataflowfig.pdf\
           AboKurOhl2004.pdf\
           AboKurOhl2002.pdf\
           BreKurWal2002.pdf\
           ChoSchleKurGie2004.pdf\
           FitGarKucKurMyeOttSleVitZemMcc2002.pdf\
           KurChoOhlSchleStoGie2001.pdf\
           HoehKurOhl2002.pdf\
           GraeStrKurSte2001.pdf\
           BecStroHomGieKur2004.pdf\
           KrueSczKurGie2004.pdf\
           virtman.pdf

PAPERS=${WORK}/archive-etc/own-papers

SERVER=vmatchserver
WWWBASEDIR=/var/www/html

all:vmweb.pdf vmweb.tgz

vmweb.pdf:vmweb.tex introduction.inc
	latexmk vmweb

introduction.inc:../virtman.tex introexclude
	extractpart.pl introduction ../virtman.tex |\
               grep -v -f introexclude |\
               sed -e 's/\\subsection\*{The parts/\\section*{The parts/' | \
               perl -pe 's/\\cite{ABO:KUR:OHL:2004}\.\%\%second/(Abouelhoda, Kurtz, Ohlebusch 2004)./' | \
               perl -pe 's/\\cite{(.*)}[\.,]?/\n\\bibentry{\1}\n/' > $@

Dataflow.inc:../Dataflow.pic
	pic -t ../Dataflow.pic > $@

index.html:vmweb.tex vmweb.pdf introduction.inc replace-header.rb replace-par.rb
	htlatex vmweb.tex "xhtml, charset=utf-8" " -cunihtf -utf8"
	cat vmweb.html | replace-header.rb | \
                         replace-par.rb | \
                         sed -e 's/CONTENT=/content=/' \
                             -e 's/ALT=/alt=/' > $@

validate:index.html xhtml-lat1.ent xhtml-symbol.ent xhtml-special.ent xhtml1-transitional.dtd
	cat index.html | sed -e 's/\"http:\/\/www.w3.org\/TR\/xhtml1\/DTD\//\"/' > .tmp.html
	xmllint --valid --noout .tmp.html
	rm -f .tmp.html

# also run validation on https://validator.w3.org/

Dataflowfig.dvi:Dataflowfig.tex Dataflow.inc
	latex Dataflowfig.tex

Dataflowfig.pdf:Dataflowfig.dvi
	dvipdf $<

vmweb.tgz:matchgraph.gif ${vmwebfiles} index.html vmweb.pdf
	tar -cvzf $@ matchgraph.gif ${vmwebfiles} index.html vmweb.pdf

virtman.pdf:
	cp ../virtman.pdf .

AboKurOhl2004.pdf:
	cp ${PAPERS}/$@ .

AboKurOhl2002.pdf:
	cp ${PAPERS}/$@ .

BreKurWal2002.pdf:
	cp ${PAPERS}/$@ .

ChoSchleKurGie2004.pdf:
	cp ${PAPERS}/$@ .

FitGarKucKurMyeOttSleVitZemMcc2002.pdf:
	cp ${PAPERS}/$@ .

KurChoOhlSchleStoGie2001.pdf:
	cp ${PAPERS}/$@ .

HoehKurOhl2002.pdf:
	cp ${PAPERS}/$@ .

GraeStrKurSte2001.pdf:
	cp ${PAPERS}/$@ .

KrueSczKurGie2004.pdf:
	cp ${PAPERS}/$@ .

BecStroHomGieKur2004.pdf:
	cp ${PAPERS}/$@ .

installwww:
	rsync -rv index.html vmweb.css download.html virtman.pdf vmweb.pdf matchgraph.gif distributions $(SERVER):$(WWWBASEDIR)

clean:
	rm -f *.toc *.ilg *.out *.idx *.ind *.dvi *.ps *.log *.aux
	rm -f *.bbl *.blg
	rm -f vmweb.4ct vmweb.xref vmweb.tmp vmweb.lg vmweb.idv vmweb.html vmweb.4tc vmweb.fls vmweb.fdb_latexmk
	rm -f comment.cut introduction.inc
	rm -f Dataflow.inc 
	rm -f ${vmwebfiles}
