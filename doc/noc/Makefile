GUIDE=programming_guide
EXERCISE=programming_exercise

.PHONY: all clean guide exercise

all: guide exercise

guide: $(GUIDE).pdf 

exercise: ../c_doc/latex/doxygen.sty $(EXERCISE).pdf

../c_doc/latex/doxygen.sty:
	@echo --------------------------------------------------------------------------
	@echo -- Please build the programming exercise from 'patmos/doc' with 'make noc'
	@echo --------------------------------------------------------------------------
	@exit 1

%.pdf: %.tex 
	pdflatex $(*F)
	bibtex   $(*F)
	pdflatex $(*F)
	pdflatex $(*F)

clean:
	-rm -fr *.aux *.bbl *.blg *.log *.lof *.lot *.toc *.gz *.pdf
	-rm -f fig/*.aux fig/*.log
