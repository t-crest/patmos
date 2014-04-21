BUILDDIR?=$(CURDIR)/build

all: $(BUILDDIR)/patserdow $(BUILDDIR)/patex

$(BUILDDIR)/patserdow: patserdow
	cp $< $@

$(BUILDDIR)/patex: patex.in
	sed "s#@PATMOS_HOME@#\"$(PATMOS_HOME)\"#; s#@COM_PORT@#\"$(COM_PORT)\"#" $< > $@
	chmod a+x $@
