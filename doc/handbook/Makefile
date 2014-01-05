# MS: this Makefile looks a little bit broken for me - no figures, no bib entries
DOC?=patmos_handbook
BIBS=patbib.bib
OPEN=open # program for displaying the pdf


# include file to override e.g. OPEN
# config.mk is ignored by svn
-include config.mk


.PHONY: all clean

all: open

TIKZFIGURES=fig/timing_core.pdf fig/timing_io.pdf fig/timing_burst.pdf \
	fig/intr.pdf fig/exc.pdf

fig/%.pdf: fig/%.tex
	pdflatex --output-directory fig $^

rubber: $(DOC).tex $(BIBS) $(TIKZFIGURES)
	rubber --short --warn all -d $(DOC).tex

$(DOC).pdf: $(DOC).tex $(BIBS) $(TIKZFIGURES)
	pdflatex $(DOC)
	bibtex $(DOC)
	pdflatex $(DOC)
	pdflatex $(DOC)

open: $(DOC).pdf
	$(OPEN) $<


clean:
	rm -fr *.aux *.bbl *.blg *.log *.lof *.lot *.toc *.gz *.pdf
	rm -f fig/*.aux fig/*.log
	rm -f $(TIKZFIGURES)
