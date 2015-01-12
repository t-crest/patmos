all: handbook noc source

handbook:
	make -C handbook all
noc:
	make -C noc all
source: doxygen c_doc.pdf

doxygen:
	doxygen c_doc.doxy

latexdoc:
	make -C c_doc/latex

c_doc.pdf: latexdoc
	cp c_doc/latex/refman.pdf c_doc.pdf


.PHONY: handbook noc libcorethread libnoc libmp
