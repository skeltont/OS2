doc = paper
assignment = CS444_writing1_118.tar.bz2

default: all

all:
	latex --shell-escape $(doc).tex
	bibtex $(doc)
	latex --shell-escape $(doc).tex
	latex --shell-escape $(doc).tex
	dvips -R -Poutline -t letter $(doc).dvi -o $(doc).ps
	ps2pdf $(doc).ps
	open $(doc).pdf

clean:
	rm -f *.aux
	rm -f *.bbl
	rm -f *.blg
	rm -f *.dvi
	rm -f *.log
	rm -f *.pdf
	rm -f *.ps
	rm -f *.toc

bib:
	bibtex $(doc)

tar:
	tar cvjf $(assignment) IEEEtran.cls makefile paper.tex references.bib
