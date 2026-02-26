all: paper.pdf

paper.dvi: paper.tex references.bib
	latex paper.tex
	bibtex paper
	latex paper.tex
	latex paper.tex

paper.ps: paper.dvi
	dvips -o paper.ps paper.dvi

paper.pdf: paper.ps
	ps2pdf paper.ps paper.pdf

clean:
	rm -f *.dvi *.aux *.log *.ps *.pdf *.blg *.bbl
