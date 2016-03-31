default: all

all:
	latex --shell-escape report.tex
	dvips -R -Poutline -t letter report.dvi -o report.ps
	ps2pdf report.ps
	rm report.aux
	rm report.dvi
	rm report.log
	rm report.ps
	open report.pdf
