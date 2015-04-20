all: handbook noc source

handbook:
	make -C handbook all

noc: source
	make -C noc all

source: doxygen c_doc.pdf

doxygen:
	doxygen c_doc.doxy

latexdoc:
	make -C c_doc/latex

c_doc.pdf: latexdoc
	cp c_doc/latex/refman.pdf c_doc.pdf

clean:
	-rm -r c_doc
	-rm c_doc.pdf
	make -C handbook clean
	make -C noc clean

.PHONY: all handbook noc source doxygen latexdoc clean cleanall
