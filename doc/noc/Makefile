GUIDE=programming_guide
EXERCISE=programming_exercise
BIBS=../handbook/patbib.bib

.PHONY: all clean guide exercise

all: guide exercise

guide: $(GUIDE).pdf 

exercise: $(EXERCISE).pdf

%.pdf: %.tex 
	pdflatex $(*F)
	bibtex $(BIBS)
	pdflatex $(*F)
	pdflatex $(*F)

clean:
	-rm -fr *.aux *.bbl *.blg *.log *.lof *.lot *.toc *.gz *.pdf
	-rm -f fig/*.aux fig/*.log
