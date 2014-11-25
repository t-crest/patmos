all: doxygen libmp.pdf

doxygen:
	doxygen libmp.doxy

latexdoc:
	make -C doc/latex

libmp.pdf: latexdoc
	cp doc/latex/refman.pdf libmp.pdf
