#
# Main make file for Patmos
#
##############################################################
# Include user makefile for local configurations
-include config.mk
# COM port for downloader
COM_PORT?=/dev/ttyUSB0

# Application to be stored in boot ROM
BOOTAPP?=basic
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
#BOARD=bemicro
BOARD?=altde2-70
#BOARD?=altde2-115

# MS: why do we need all those symbols when
# the various paths are fixed anyway?

# Where to put elf files and binaries
BUILDDIR?=$(CURDIR)/tmp
# Build directories for various tools
SIMBUILDDIR?=$(CURDIR)/simulator/build
CTOOLSBUILDDIR?=$(CURDIR)/tools/c/build
JAVATOOLSBUILDDIR?=$(CURDIR)/tools/java/build
HWBUILDDIR?=$(CURDIR)/hardware/build
# Where to install tools
INSTALLDIR?=$(CURDIR)/install
HWINSTALLDIR?=$(INSTALLDIR)

all: tools emulator patmos

tools: patsim elf2bin javatools

# Build simulator and assembler
patsim:
	-mkdir -p $(SIMBUILDDIR)
	cd $(SIMBUILDDIR) && cmake ..
	cd $(SIMBUILDDIR) && make
	-mkdir -p $(INSTALLDIR)/bin
	cp $(SIMBUILDDIR)/src/pa* $(INSTALLDIR)/bin

# Build tool to transform elf to binary
elf2bin:
	-mkdir -p $(CTOOLSBUILDDIR)
	cd $(CTOOLSBUILDDIR) && cmake ..
	cd $(CTOOLSBUILDDIR) && make
	-mkdir -p $(INSTALLDIR)/bin
	cp $(CTOOLSBUILDDIR)/src/elf2bin $(INSTALLDIR)/bin

# Build various Java tools
javatools: $(JAVATOOLSBUILDDIR)/lib/patmos-tools.jar \
		tools/lib/java-binutils-0.1.0.jar tools/lib/jssc.jar \
		tools/scripts/patserdow
	-mkdir -p $(INSTALLDIR)/lib/java
	cp $(JAVATOOLSBUILDDIR)/lib/patmos-tools.jar $(INSTALLDIR)/lib/java
	cp tools/lib/java-binutils-0.1.0.jar $(INSTALLDIR)/lib/java
	cp tools/lib/jssc.jar $(INSTALLDIR)/lib/java
	-mkdir -p $(INSTALLDIR)/bin
	cp tools/scripts/patserdow $(INSTALLDIR)/bin

PATSERDOW_SRC=$(shell find tools/java/src/patserdow/ -name *.java)
PATSERDOW_CLASS=$(patsubst tools/java/src/%.java,$(JAVATOOLSBUILDDIR)/classes/%.class,$(PATSERDOW_SRC))
JAVAUTIL_SRC=$(shell find tools/java/src/util/ -name *.java)
JAVAUTIL_CLASS=$(patsubst tools/java/src/%.java,$(JAVATOOLSBUILDDIR)/classes/%.class,$(JAVAUTIL_SRC))

$(JAVATOOLSBUILDDIR)/lib/patmos-tools.jar: $(PATSERDOW_CLASS) $(JAVAUTIL_CLASS)
	-mkdir -p $(JAVATOOLSBUILDDIR)/lib
	cd $(JAVATOOLSBUILDDIR)/classes && jar cf ../lib/patmos-tools.jar $(subst $(JAVATOOLSBUILDDIR)/classes/,,$^)

$(JAVATOOLSBUILDDIR)/classes/%.class: tools/java/src/%.java
	-mkdir -p $(JAVATOOLSBUILDDIR)/classes
	javac -classpath tools/lib/java-binutils-0.1.0.jar:tools/lib/jssc.jar \
		-sourcepath tools/java/src -d $(JAVATOOLSBUILDDIR)/classes $<

# Build the Chisel emulator
emulator:
	-mkdir -p $(HWBUILDDIR)
	$(MAKE) -C hardware BOOTBUILDROOT=$(CURDIR) BOOTBUILDDIR=$(BUILDDIR) BOOTAPP=$(BOOTAPP) BOOTBIN=$(BUILDDIR)/$(BOOTAPP).bin emulator
	-mkdir -p $(HWINSTALLDIR)/bin
	cp $(HWBUILDDIR)/emulator $(HWINSTALLDIR)/bin

# Assemble a program
asm: asm-$(BOOTAPP)

asm-% $(BUILDDIR)/%.bin $(BUILDDIR)/%.dat: asm/%.s
	-mkdir -p $(dir $(BUILDDIR)/$*)
	$(INSTALLDIR)/bin/paasm $< $(BUILDDIR)/$*.bin
	touch $(BUILDDIR)/$*.dat

# Compile a program with flags for booting
bootcomp: bin-$(BOOTAPP)

# Convert elf file to binary
bin-% $(BUILDDIR)/%.bin $(BUILDDIR)/%.dat: $(BUILDDIR)/%.elf
	$(INSTALLDIR)/bin/elf2bin $< $(BUILDDIR)/$*.bin $(BUILDDIR)/$*.dat

# Convert elf file to flat memory image
img: img-$(APP)
img-% $(BUILDDIR)/%.img: $(BUILDDIR)/%.elf
	$(INSTALLDIR)/bin/elf2bin -f $< $(BUILDDIR)/$*.img

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
	$(MAKE) -C hardware verilog BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)

synth: csynth

csynth:
	$(MAKE) -C hardware qsyn BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)

config_byteblaster:
	quartus_pgm -c $(BLASTER_TYPE) -m JTAG hardware/quartus/$(BOARD)/patmos.cdf

download: $(BUILDDIR)/$(APP).elf
	$(INSTALLDIR)/bin/patserdow -v $(COM_PORT) $<

fpgaexec: $(BUILDDIR)/$(APP).elf
	$(INSTALLDIR)/bin/patserdow $(COM_PORT) $<

# TODO: no Xilinx Makefiles available yet
config_xilinx:
	echo "No Xilinx Makefile"
#	$(MAKE) -C xilinx/$(XPROJ) config

# cleanup
CLEANEXTENSIONS=rbf rpt sof pin summary ttf qdf dat wlf done qws

mostlyclean:
	-rm -rf $(SIMBUILDDIR) $(CTOOLSBUILDDIR) $(BUILDDIR)
	-rm -rf $(JAVATOOLSBUILDDIR)/classes

clean: mostlyclean
	-rm -rf $(INSTALLDIR)/bin
	-rm -rf $(INSTALLDIR)/lib
	-rm -rf $(JAVATOOLSBUILDDIR)/lib
	for ext in $(CLEANEXTENSIONS); do \
		find `ls` -name \*.$$ext -print -exec rm -r -f {} \; ; \
	done
	-find `ls` -name patmos.pof -print -exec rm -r -f {} \;
	-find `ls` -name db -print -exec rm -r -f {} \;
	-find `ls` -name incremental_db -print -exec rm -r -f {} \;
	-find `ls` -name patmos_description.txt -print -exec rm -r -f {} \;

# Dummy target to force the execution of recipies for things that are not really phony
.FORCE:
