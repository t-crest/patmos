all: doxygen libnoc.pdf

doxygen:
	doxygen libnoc.doxy

latexdoc:
	make -C doc/latex

libnoc.pdf: latexdoc
	cp doc/latex/refman.pdf libnoc.pdf
