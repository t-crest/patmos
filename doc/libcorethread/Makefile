all: doxygen libcorethread.pdf

doxygen:
	doxygen libcorethread.doxy

latexdoc:
	make -C doc/latex

libcorethread.pdf: latexdoc
	cp doc/latex/refman.pdf libcorethread.pdf
