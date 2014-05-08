BUILDDIR?=$(CURDIR)/build

all: $(BUILDDIR)/config_altera $(BUILDDIR)/config_xilinx \
	$(BUILDDIR)/patserdow $(BUILDDIR)/patex

$(BUILDDIR)/%: %
	cp $< $@

$(BUILDDIR)/patex: patex.in
	sed "s#@PATMOS_HOME@#\"$(PATMOS_HOME)\"#; s#@COM_PORT@#\"$(COM_PORT)\"#" $< > $@
	chmod a+x $@

