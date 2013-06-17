# MS: this Makefile looks a little bit broken for me - no figures, no bib entries
DOC?=patmos_handbook
BIBS=patbib.bib
OPEN=open # program for displaying the pdf

# include file to override e.g. OPEN
# config.mk is ignored by svn
-include config.mk


.PHONY: all clean

all: open

rubber: $(DOC).tex $(BIBS)
	rubber --short --warn all -d $(DOC).tex

$(DOC).pdf: $(DOC).tex $(BIBS)
	pdflatex $(DOC)
	bibtex $(DOC)
	pdflatex $(DOC)
	pdflatex $(DOC)

open: $(DOC).pdf
	$(OPEN) $<


clean:
	rm -fr *.aux *.bbl *.blg *.log
