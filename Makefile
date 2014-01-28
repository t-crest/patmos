#
# Main make file for Patmos
#

# COM port for downloader
COM_PORT?=/dev/ttyUSB0

# Application to be stored in boot ROM
#BOOTAPP?=york_loader
BOOTAPP ?= ldst
#BOOTAPP=bootable-bootloader

# Application to be downloaded
APP?=hello

# Altera FPGA configuration cables
#BLASTER_TYPE=ByteBlasterMV
#BLASTER_TYPE=Arrow-USB-Blaster 
BLASTER_TYPE?=USB-Blaster

# Path delimiter for Wdoz and others
ifeq ($(WINDIR),)
	S=:
else
	S=\;
endif

# The Quartus project
#QPROJ=bemicro
QPROJ?=altde2-70

# MS: why do we need all those symbols when
# the various paths are fixed anyway?

# Where to put elf files and binaries
BUILDDIR?=$(CURDIR)/tmp
# Build directories for various tools
SIMBUILDDIR?=$(CURDIR)/simulator/build
CTOOLSBUILDDIR?=$(CURDIR)/tools/c/build
HWBUILDDIR?=$(CURDIR)/hardware/build
# Where to install tools
INSTALLDIR?=$(CURDIR)/bin
HWINSTALLDIR?=$(INSTALLDIR)

all: tools emulator patmos

tools: patsim elf2bin javatools

# Build simulator and assembler
patsim:
	-mkdir -p $(SIMBUILDDIR)
	cd $(SIMBUILDDIR) && cmake ..
	cd $(SIMBUILDDIR) && make
	-mkdir -p $(INSTALLDIR)
	cp $(SIMBUILDDIR)/src/pa* $(INSTALLDIR)

# Build tool to transform elf to binary
elf2bin:
	-mkdir -p $(CTOOLSBUILDDIR)
	cd $(CTOOLSBUILDDIR) && cmake ..
	cd $(CTOOLSBUILDDIR) && make
	-mkdir -p $(INSTALLDIR)
	cp $(CTOOLSBUILDDIR)/src/elf2bin $(INSTALLDIR)

# Build various Java tools
javatools: tools/java/lib/patmos-tools.jar

PATSERDOW_SRC=$(shell find tools/java/src/patserdow/ -name *.java)
PATSERDOW_CLASS=$(patsubst tools/java/src/%.java,tools/java/classes/%.class,$(PATSERDOW_SRC))
JAVAUTIL_SRC=$(shell find tools/java/src/util/ -name *.java)
JAVAUTIL_CLASS=$(patsubst tools/java/src/%.java,tools/java/classes/%.class,$(JAVAUTIL_SRC))

tools/java/lib/patmos-tools.jar: $(PATSERDOW_CLASS) $(JAVAUTIL_CLASS)
	-mkdir -p tools/java/lib
	cd tools/java/classes && jar cf ../lib/patmos-tools.jar $(subst tools/java/classes/,,$^)

tools/java/classes/%.class: tools/java/src/%.java
	-mkdir -p tools/java/classes
	javac -classpath tools/lib/java-binutils-0.1.0.jar:tools/lib/jssc.jar \
		-sourcepath tools/java/src -d tools/java/classes $<

# Build the Chisel emulator
emulator:
	-mkdir -p $(HWBUILDDIR)
	$(MAKE) -C hardware BOOTBUILDROOT=$(CURDIR) BOOTBUILDDIR=$(BUILDDIR) BOOTAPP=$(BOOTAPP) BOOTBIN=$(BUILDDIR)/$(BOOTAPP).bin emulator
	-mkdir -p $(HWINSTALLDIR)
	cp $(HWBUILDDIR)/emulator $(HWINSTALLDIR)

# Assemble a program
asm: asm-$(BOOTAPP)

asm-% $(BUILDDIR)/%.bin $(BUILDDIR)/%.dat: asm/%.s
	-mkdir -p $(dir $(BUILDDIR)/$*)
	$(INSTALLDIR)/paasm $< $(BUILDDIR)/$*.bin
	touch $(BUILDDIR)/$*.dat

# Compile a program with flags for booting
bootcomp: bin-$(BOOTAPP)

# Convert elf file to binary
bin-% $(BUILDDIR)/%.bin $(BUILDDIR)/%.dat: $(BUILDDIR)/%.elf
	$(INSTALLDIR)/elf2bin $< $(BUILDDIR)/$*.bin $(BUILDDIR)/$*.dat

# Convert elf file to flat memory image
img: img-$(APP)
img-% $(BUILDDIR)/%.img: $(BUILDDIR)/%.elf
	$(INSTALLDIR)/elf2bin -f $< $(BUILDDIR)/$*.img

# Convert binary memory image to decimal representation
imgdat: imgdat-$(APP)
imgdat-% $(BUILDDIR)/%.img.dat: $(BUILDDIR)/%.img
	hexdump -v -e '"%d,"' -e '" // %08x\n"' $< > $(BUILDDIR)/$*.img.dat

# Compile a program to an elf file
comp: comp-$(APP)

comp-% $(BUILDDIR)/%.elf: .FORCE
	-mkdir -p $(dir $@)
	$(MAKE) -C c BUILDDIR=$(BUILDDIR) APP=$* compile

.PRECIOUS: $(BUILDDIR)/%.elf

# High-level pasim simulation
swsim: $(BUILDDIR)/$(BOOTAPP).bin
	bin/pasim --debug --debug-fmt=short $(BUILDDIR)/$(BOOTAPP).bin

# C simulation of the Chisel version of Patmos
hwsim:
	$(MAKE) -C hardware test BOOTBUILDROOT=$(CURDIR) BOOTAPP=$(BOOTAPP)

# Testing
test:
	testsuite/run.sh

# Compile Patmos and download
patmos: gen synth config

# configure the FPGA
config:
ifeq ($(XFPGA),true)
	make config_xilinx
else
	make config_byteblaster
endif

gen:
	$(MAKE) -C hardware verilog BOOTAPP=$(BOOTAPP) QPROJ=$(QPROJ)

synth: csynth

csynth:
	$(MAKE) -C hardware qsyn BOOTAPP=$(BOOTAPP) QPROJ=$(QPROJ)

config_byteblaster:
	quartus_pgm -c $(BLASTER_TYPE) -m JTAG hardware/quartus/$(QPROJ)/patmos.cdf

download: $(BUILDDIR)/$(APP).elf
	java -Dverbose=true -cp tools/lib/*:tools/java/lib/* patserdow.Main $(COM_PORT) $<

fpgaexec: $(BUILDDIR)/$(APP).elf
	java -Dverbose=false -cp tools/lib/*:tools/java/lib/* patserdow.Main $(COM_PORT) $<

# TODO: no Xilinx Makefiles available yet
config_xilinx:
	echo "No Xilinx Makefile"
#	$(MAKE) -C xilinx/$(XPROJ) config

# cleanup
CLEANEXTENSIONS=rbf rpt sof pin summary ttf qdf dat wlf done qws

mostlyclean:
	-rm -rf $(SIMBUILDDIR) $(CTOOLSBUILDDIR) $(BUILDDIR)
	-rm -rf tools/java/classes

clean: mostlyclean
	-rm -rf $(INSTALLDIR)
	-rm -rf tools/java/lib
	for ext in $(CLEANEXTENSIONS); do \
		find `ls` -name \*.$$ext -print -exec rm -r -f {} \; ; \
	done
	-find `ls` -name patmos.pof -print -exec rm -r -f {} \;
	-find `ls` -name db -print -exec rm -r -f {} \;
	-find `ls` -name incremental_db -print -exec rm -r -f {} \;
	-find `ls` -name patmos_description.txt -print -exec rm -r -f {} \;

# Dummy target to force the execution of recipies for things that are not really phony
.FORCE:
