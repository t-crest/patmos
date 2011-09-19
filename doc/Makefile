DOC=patmos_tr

all:
	pdflatex $(DOC)
	bibtex $(DOC)
	open $(DOC).pdf
bib:
	cat ../../../bib/*.bib > msbib.bib
