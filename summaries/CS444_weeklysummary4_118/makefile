doc = summary
assignment = weeklysummary4

default: all

all:
	latex --shell-escape $(doc).tex
	dvips -R -Poutline -t letter $(doc).dvi -o $(doc).ps
	ps2pdf $(doc).ps
	open $(doc).pdf

clean:
	rm -f $(doc).aux
	rm -f $(doc).dvi
	rm -f $(doc).log
	rm -f $(doc).ps
	rm -f $(doc).pdf

testlocal:
	python testdoc.py

testremote:
	/usr/local/apps/python/python-3.5.1 testdoc.py

tar:
	tar cvjf CS444_$(assignment)_118.tar.bz2 IEEEtran.cls makefile $(doc).tex
