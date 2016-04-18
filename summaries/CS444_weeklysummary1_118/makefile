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
