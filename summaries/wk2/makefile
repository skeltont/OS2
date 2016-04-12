default: all

all:
	latex --shell-escape summary.tex
	dvips -R -Poutline -t letter summary.dvi -o summary.ps
	ps2pdf summary.ps
	rm summary.aux
	rm summary.dvi
	rm summary.log
	rm summary.ps
	open summary.pdf

clean:
	rm summary.aux
	rm summary.dvi
	rm summary.log
	rm summary.ps
