doc = report
assignment = project2

default: all

all:
	latex --shell-escape $(doc).tex
	latex --shell-escape $(doc).tex
	dvips -R -Poutline -t letter $(doc).dvi -o $(doc).ps
	ps2pdf $(doc).ps
	#open $(doc).pdf

clean:
	rm -f *.aux
	rm -f *.bbl
	rm -f *.blg
	rm -f *.dvi
	rm -f *.log
	rm -f *.pdf
	rm -f *.ps
	rm -f *.toc

testlocal:
	python testdoc.py

testremote:
	/usr/local/apps/python/python-3.5.1 testdoc.py

tar:
	tar cvjf CS444_$(assignment)_118.tar.bz2 IEEEtran.cls makefile $(doc).tex worklog.tex changelog.tex adds_sstf_118.patch
