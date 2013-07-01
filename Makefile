#
# Main make file for Patmos
#

# COM port for downloader
COM_PORT=/dev/ttyUSB0

# Application to be stored in boot ROM
BOOTAPP=basic
#BOOTAPP=bootable-bootloader

# Application to be downloaded
APP=hello

# Altera FPGA configuration cable
#BLASTER_TYPE=ByteBlasterMV
BLASTER_TYPE=USB-Blaster

# Path delimiter for Wdoz and others
ifeq ($(WINDIR),)
	S=:
else
	S=\;
endif

# The Quartus project
# MeMicro missing
QPROJ=altde2-70

# Where to put elf files and binaries
BUILDDIR=tmp
# Where to install tools
INSTALLDIR=bin

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
emulator: elf2bin
	-mkdir -p chisel/build
	$(MAKE) -C chisel BUILDDIR=$(CURDIR)/chisel/build BOOTAPP=$(BOOTAPP) emulator
	-mkdir -p $(INSTALLDIR)
	cp chisel/build/emulator $(INSTALLDIR)

# Assemble a program
asm: asm-$(BOOTAPP)

asm-% $(BUILDDIR)/%.bin: asm/%.s
	-mkdir -p $(dir $(BUILDDIR)/$*)
	$(INSTALLDIR)/paasm $< $(BUILDDIR)/$*.bin

# Compile a program with flags for booting
bootcomp: bin-$(BOOTAPP)

# Convert elf file to binary
bin-% $(BUILDDIR)/%.bin: $(BUILDDIR)/%.elf
	bin/elf2bin $< $(BUILDDIR)/$*.bin

# Compile a program to an elf file
comp: comp-$(APP)

comp-% $(BUILDDIR)/%.elf: .FORCE
	-mkdir -p $(dir $@)
	$(MAKE) -C c BUILDDIR=$(CURDIR)/$(BUILDDIR) APP=$* compile

# High-level pasim simulation
hsim: $(BUILDDIR)/$(BOOTAPP).bin
	bin/pasim --debug --debug-fmt=short $(BUILDDIR)/$(BOOTAPP).bin

# C simulation of the Chisel version of Patmos
csim:
	$(MAKE) -C chisel test BOOTAPP=$(BOOTAPP)

# Testing
test:
	testsuite/run.sh

# Compile Patmos and download
patmos: synth config

# configure the FPGA
config:
ifeq ($(XFPGA),true)
	make config_xilinx
else
	make config_byteblaster
endif

synth: csynth

csynth:
	$(MAKE) -C chisel qsyn BOOTAPP=$(BOOTAPP) QPROJ=$(QPROJ)

config_byteblaster:
	quartus_pgm -c $(BLASTER_TYPE) -m JTAG chisel/quartus/$(QPROJ)/patmos.cdf

download: $(BUILDDIR)/$(APP).elf
	java -Dverbose=true -cp lib/*:java/lib/* patserdow.Main $(COM_PORT) $<

fpgaexec: $(BUILDDIR)/$(APP).elf
	java -Dverbose=false -cp lib/*:java/lib/* patserdow.Main $(COM_PORT) $<

# TODO: no Xilinx Makefiles available yet
config_xilinx:
	echo "No Xilinx Makefile"
#	$(MAKE) -C xilinx/$(XPROJ) config

# cleanup
CLEANEXTENSIONS=rbf rpt sof pin summary ttf qdf dat wlf done qws

clean:
	-rm -rf $(BUILDDIR) $(INSTALLDIR)
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
