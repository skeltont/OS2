default: all

all:
	latex --shell-escape report.tex
	dvips -R -Poutline -t letter report.dvi -o report.ps
	ps2pdf report.ps
	rm -f report.aux
	rm -f report.dvi
	rm -f report.log
	rm -f report.ps
	open report.pdf

clean:
	rm -f report.aux
	rm -f report.dvi
	rm -f report.log
	rm -f report.ps
