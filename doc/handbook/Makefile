# MS: this Makefile looks a little bit broken for me - no figures, no bib entries
DOC?=patmos_handbook
BIBS=patbib.bib
OPEN=open # program for displaying the pdf

PDFLATEX = pdflatex -interaction=nonstopmode -halt-on-error

TEX=$(wildcard *.tex)

# include file to override e.g. OPEN
# config.mk is in .gitignore
-include config.mk


.PHONY: all clean

all: $(DOC).pdf

TIKZFIGURES=fig/timing_core.pdf fig/timing_io.pdf fig/timing_burst.pdf

fig/%.pdf: fig/%.tex
	$(PDFLATEX) --output-directory fig $^

rubber: $(TEX) $(BIBS) $(TIKZFIGURES)
	rubber --short --warn all -d $(DOC).tex

$(DOC).pdf: $(TEX) $(BIBS) $(TIKZFIGURES)
	$(PDFLATEX) $(DOC)
	bibtex $(DOC)
	$(PDFLATEX) $(DOC)
	$(PDFLATEX) $(DOC)

open: $(DOC).pdf
	$(OPEN) $<


clean:
	rm -fr *.aux *.bbl *.blg *.log *.lof *.lot *.toc *.gz *.pdf *.lol
	rm -f fig/*.aux fig/*.log
	rm -f $(TIKZFIGURES)
