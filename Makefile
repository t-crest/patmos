#
# Main make file for Patmos
#

#
#	Set USB to true for an FTDI chip based board (dspio, usbmin, lego)
#
USB=false
# COM port for downloader
COM_PORT=/dev/ttyUSB0

# Assembler files
APP=ALU
# Altera FPGA configuration cable
#BLASTER_TYPE=ByteBlasterMV
BLASTER_TYPE=USB-Blaster

# Is there an intention to continue to support the Cycore board?
# Maybe the BeMicro is a better substitution
ifeq ($(WINDIR),)
	USBRUNNER=./USBRunner
	S=:
else
	USBRUNNER=USBRunner.exe
	S=\;
endif

# The Quartus project
QPROJ=dspio
QPROJ=altde2-70

# Where to put elf files and binaries
BUILDDIR=tmp
# Where to install tools
INSTALLDIR=bin
# Where to place FPGA bitstreams
HWBUILDDIR=rbf

all: tools emulator patmos

tools: patsim elf2bin javatools

# Build simulator and assembler
patsim:
	-mkdir -p simulator/build
	cd simulator/build && cmake ..
	cd simulator/build && make
	-mkdir -p $(INSTALLDIR)
	cp simulator/build/src/pa* $(INSTALLDIR)

# Build tool to transform elf to binary
elf2bin:
	-mkdir -p ctools/build
	cd ctools/build && cmake ..
	cd ctools/build && make
	-mkdir -p $(INSTALLDIR)
	cp ctools/build/src/elf2bin $(INSTALLDIR)

# Build various Java tools
javatools: java/lib/patmos-tools.jar

PATSERDOW_SRC=$(shell find java/src/patserdow/ -name *.java)
PATSERDOW_CLASS=$(patsubst java/src/%.java,java/classes/%.class,$(PATSERDOW_SRC))
JAVAUTIL_SRC=$(shell find java/src/util/ -name *.java)
JAVAUTIL_CLASS=$(patsubst java/src/%.java,java/classes/%.class,$(JAVAUTIL_SRC))

java/lib/patmos-tools.jar: $(PATSERDOW_CLASS) $(JAVAUTIL_CLASS)
	-mkdir -p java/lib
	cd java/classes && jar cf ../lib/patmos-tools.jar $(subst java/classes/,,$^)

java/classes/%.class: java/src/%.java
	-mkdir -p java/classes
	javac -classpath lib/java-binutils-0.1.0.jar:lib/jssc.jar \
		-sourcepath java/src -d java/classes $<

# Build the Chisel emulator
emulator:
	-mkdir -p chisel/build
	$(MAKE) -C chisel BUILDDIR=$(CURDIR)/chisel/build APP=$(APP) emulator
	-mkdir -p $(INSTALLDIR)
	cp chisel/build/emulator $(INSTALLDIR)

# Create binary, no matter how
binary: $(BUILDDIR)/$(APP).bin

# Assemble a program
asm: asm-$(APP)
asm-% $(BUILDDIR)/%.bin: asm/%.s
	-mkdir -p $(dir $(BUILDDIR)/$*)
	$(INSTALLDIR)/paasm $< $(BUILDDIR)/$*.bin

# Compile a program
comp: comp-$(APP)
comp-% $(BUILDDIR)/%.bin: $(BUILDDIR)/%.elf
	bin/elf2bin $< $(BUILDDIR)/$*.bin
$(BUILDDIR)/%.elf: .FORCE
	-mkdir -p $(dir $@)
	$(MAKE) -C c BUILDDIR=$(CURDIR)/$(BUILDDIR) APP=$* compile

# Compile a program with flags for booting
bootcomp: comp-bootable-$(APP)

# High-level pasim simulation
hsim: $(BUILDDIR)/$(APP).bin
	bin/pasim --debug --debug-fmt=short $(BUILDDIR)/$(APP).bin

# C simulation of the Chisel version of Patmos
csim:
	$(MAKE) -C chisel test APP=$(APP)

# Testing
test:
	testsuite/run.sh

# Compile Patmos and download
patmos: synth config

# configure the FPGA
config:
ifeq ($(USB),true)
	make config_usb
else
ifeq ($(XFPGA),true)
	make config_xilinx
else
	make config_byteblaster
endif
endif

synth: csynth

csynth:
	$(MAKE) -C chisel qsyn APP=$(APP) QPROJ=$(QPROJ)

config_byteblaster:
	quartus_pgm -c $(BLASTER_TYPE) -m JTAG quartus/$(QPROJ)/patmos.cdf

config_usb:
	$(USBRUNNER) $(HWBUILDDIR)/$(QPROJ).rbf

download: $(BUILDDIR)/$(APP).elf
	java -cp lib/*:java/lib/* patserdow.Main $(COM_PORT) $<

# TODO: no Xilinx Makefiles available yet
config_xilinx:
	$(MAKE) -C xilinx/$(XPROJ) config

# cleanup
CLEANEXTENSIONS=rbf rpt sof pin summary ttf qdf dat wlf done qws

clean:
	-rm -rf $(BUILDDIR) $(INSTALLDIR) $(HWBUILDDIR)
	-rm -rf java/classes java/lib
	for ext in $(CLEANEXTENSIONS); do \
		find `ls` -name \*.$$ext -print -exec rm -r -f {} \; ; \
	done
	-find `ls` -name patmos.pof -print -exec rm -r -f {} \;
	-find `ls` -name db -print -exec rm -r -f {} \;
	-find `ls` -name incremental_db -print -exec rm -r -f {} \;
	-find `ls` -name patmos_description.txt -print -exec rm -r -f {} \;

# Dummy target to force the execution of recipies for things that are not really phony
.FORCE:
