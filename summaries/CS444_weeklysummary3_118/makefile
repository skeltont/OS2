default: all

all:
	latex --shell-escape summary.tex
	dvips -R -Poutline -t letter summary.dvi -o summary.ps
	ps2pdf summary.ps
	rm -f summary.aux
	rm -f summary.dvi
	rm -f summary.log
	rm -f summary.ps
	# open summary.pdf

clean:
	rm -f summary.aux
	rm -f summary.dvi
	rm -f summary.log
	rm -f summary.ps
	rm -f summary.pdf

testlocal:
	python testdoc.py

testremote:
	/usr/local/apps/python/python-3.5.1 testdoc.py

tar:
	tar cvjf CS444_weeklysummary3_118.tar.bz2 IEEEtran.cls makefile summary.tex
