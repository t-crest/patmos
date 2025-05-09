#
# Main make file for Patmos
#
##############################################################
# Include user makefile for local configurations
-include config.mk
# COM port for downloader
COM_PORT?=/dev/ttyUSB0

# Application to be stored in boot ROM
BOOTAPP?=bootable-bootloader

# Application to be downloaded
APP?=hello_puts

# Altera FPGA configuration cables
#BLASTER_TYPE=ByteBlasterMV
#BLASTER_TYPE=Arrow-USB-Blaster
#BLASTER_TYPE?=DE-SoC
BLASTER_TYPE?=USB-Blaster

# Path delimiter for Wdoz and others
ifeq ($(WINDIR),)
	S=:
else
	S=\;
endif

# The FPGA vendor (Altera, Xilinx, XilinxVivado)
#VENDOR?=Xilinx
#VENDOR?=XilinxVivado
VENDOR?=Altera

# The Quartus/ISE project
#BOARD=ml605oc
#BOARD=bemicro
#BOARD?=altde2-70
BOARD?=altde2-115
#BOARD?=altde10-NANO-oc

# Where to put elf files and binaries
BUILDDIR?=$(CURDIR)/tmp
# Build directories for various tools
CTOOLSBUILDDIR?=$(CURDIR)/tools/c/build
JAVATOOLSBUILDDIR?=$(CURDIR)/tools/java/build
SCRIPTSBUILDDIR?=$(CURDIR)/tools/scripts/build
HWBUILDDIR?=$(CURDIR)/hardware/build
HWEMUBUILDDIR?=$(CURDIR)/hardware/buildemu
# Where to install tools
INSTALLDIR?=$(CURDIR)/../local
HWINSTALLDIR?=$(INSTALLDIR)
export LF_PROJECT_ROOT:=$(CURDIR)/c/apps/lf-workspace/hello
export LF_MAIN_TARGET:=$(APP)

# Quick fix for Mac OS X
# How are include paths handled these days in *nix? CMake?
#ifeq ($(TERM_PROGRAM),$(filter $(TERM_PROGRAM), Apple_Terminal iTerm.app))
	INC_EXTRA=-I /opt/homebrew/include -I /opt/homebrew/include/libelf -L /opt/homebrew/lib
#else
#	INC_EXTRA=
#endif

all: tools emulator patmos

tools: elf2bin javatools scripttools

# Build tool to transform elf to binary
elf2bin:
	-mkdir -p $(CTOOLSBUILDDIR)
	gcc $(INC_EXTRA) -o $(INSTALLDIR)/bin/elf2bin tools/c/src/elf2bin.c -lelf 

# Target for dependencies: build elf2bin only if it does not exist.
$(INSTALLDIR)/bin/elf2bin:
	$(MAKE) elf2bin

# Build various Java tools
javatools: $(JAVATOOLSBUILDDIR)/lib/patmos-tools.jar \
		tools/lib/java-binutils-0.1.0.jar tools/lib/jssc.jar
	-mkdir -p $(INSTALLDIR)/lib/java
	cp $(JAVATOOLSBUILDDIR)/lib/patmos-tools.jar $(INSTALLDIR)/lib/java
	cp tools/lib/java-binutils-0.1.0.jar $(INSTALLDIR)/lib/java
	cp tools/lib/jssc.jar $(INSTALLDIR)/lib/java

# Patch and install scripts
scripttools:
	-mkdir -p $(SCRIPTSBUILDDIR)
	make -C tools/scripts BUILDDIR=$(SCRIPTSBUILDDIR) \
		PATMOS_HOME=$(CURDIR) COM_PORT=$(COM_PORT) all
	-mkdir -p $(INSTALLDIR)/bin
	cp $(SCRIPTSBUILDDIR)/config_altera $(INSTALLDIR)/bin
	cp $(SCRIPTSBUILDDIR)/config_xilinx $(INSTALLDIR)/bin
	cp $(SCRIPTSBUILDDIR)/patserdow $(INSTALLDIR)/bin
	cp $(SCRIPTSBUILDDIR)/patex $(INSTALLDIR)/bin

PATSERDOW_SRC=$(shell find tools/java/src/patserdow/ -name *.java)
PATSERDOW_CLASS=$(patsubst tools/java/src/%.java,$(JAVATOOLSBUILDDIR)/classes/%.class,$(PATSERDOW_SRC))
PATSERDOW_EXTRACLASS=patserdow/Main'$$'ShutDownHook.class patserdow/Main'$$'InputThread.class patserdow/Transmitter'$$'1.class
JAVAUTIL_SRC=$(shell find tools/java/src/util/ -name *.java)
JAVAUTIL_CLASS=$(patsubst tools/java/src/%.java,$(JAVATOOLSBUILDDIR)/classes/%.class,$(JAVAUTIL_SRC))

$(JAVATOOLSBUILDDIR)/lib/patmos-tools.jar: $(PATSERDOW_CLASS) $(JAVAUTIL_CLASS)
	-mkdir -p $(JAVATOOLSBUILDDIR)/lib
	cd $(JAVATOOLSBUILDDIR)/classes && jar cf ../lib/patmos-tools.jar $(subst $(JAVATOOLSBUILDDIR)/classes/,,$^) $(PATSERDOW_EXTRACLASS)

$(JAVATOOLSBUILDDIR)/classes/%.class: tools/java/src/%.java
	-mkdir -p $(JAVATOOLSBUILDDIR)/classes
	javac -classpath tools/lib/java-binutils-0.1.0.jar:tools/lib/jssc.jar \
		-sourcepath tools/java/src -d $(JAVATOOLSBUILDDIR)/classes $<

emulator: export HWBUILDDIR = $(HWEMUBUILDDIR)
emulator:
	-mkdir -p $(HWBUILDDIR)
	$(MAKE) -C hardware verilog BOOTAPP=$(BOOTAPP) BOARD=$(BOARD) GENEMU=true
	-cd $(HWBUILDDIR) && verilator --cc --exe --build -LDFLAGS "-L /opt/homebrew/lib -lelf" -CFLAGS "-I /opt/homebrew/include/libelf -I /opt/homebrew/include -Wno-undefined-bool-conversion -O3" --top-module Patmos -Mdir $(HWBUILDDIR) --trace-fst -j 0 Patmos.v ../Patmos-harness.cpp
	-cp $(HWBUILDDIR)/VPatmos $(HWBUILDDIR)/emulator
	-mkdir -p $(HWINSTALLDIR)/bin
	cp $(HWBUILDDIR)/VPatmos $(HWINSTALLDIR)/bin/patemu

# Assemble a program
asm: asm-$(BOOTAPP)

asm-% $(BUILDDIR)/%.bin $(BUILDDIR)/%.dat: asm/%.s
	-mkdir -p $(dir $(BUILDDIR)/$*)
	$(INSTALLDIR)/bin/paasm $< $(BUILDDIR)/$*.bin
	touch $(BUILDDIR)/$*.dat

# Compile a program with flags for booting
bootcomp: bin-$(BOOTAPP)

# Convert elf file to binary, using the address of the boot ROM as displacement
bin-% $(BUILDDIR)/%.bin $(BUILDDIR)/%.dat: $(BUILDDIR)/%.elf $(INSTALLDIR)/bin/elf2bin
	$(INSTALLDIR)/bin/elf2bin -d 0xf0008000 $< $(BUILDDIR)/$*.bin $(BUILDDIR)/$*.dat

# Convert elf file to flat memory image
img: img-$(APP)
img-% $(BUILDDIR)/%.img: $(BUILDDIR)/%.elf $(INSTALLDIR)/bin/elf2bin
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

# Compile an app that lives in the app folder
app:
	make -C c/apps/$(APP)
	mkdir -p $(BUILDDIR)
	cp c/apps/$(APP)/$(APP).elf $(BUILDDIR)

.PRECIOUS: $(BUILDDIR)/%.elf

app-wcet:
	make wcet -C c/apps/$(APP)

app-clean:
	make clean -C c/apps/$(APP)

pasim: 
	pasim $(BUILDDIR)/$(APP).elf

patemu: 
	patemu $(BUILDDIR)/$(APP).elf
	
# Compile an lf app that lives in the lf-workspace folder
lf-app:
	-rm -rf $(LF_PROJECT_ROOT)/bin
	-rm -rf $(LF_PROJECT_ROOT)/include
	-rm -rf $(LF_PROJECT_ROOT)/src-gen
	lfc $(LF_PROJECT_ROOT)/src/$(APP).lf
	chmod +x $(LF_PROJECT_ROOT)/src/scripts/patmos_build.sh
	$(LF_PROJECT_ROOT)/src/scripts/patmos_build.sh $(LF_PROJECT_ROOT) $(APP)
	make -C $(LF_PROJECT_ROOT)/src-gen/$(APP) 
	mkdir -p $(BUILDDIR)
	cp $(LF_PROJECT_ROOT)/src-gen/$(APP)/$(APP).elf $(BUILDDIR)

.PRECIOUS: $(BUILDDIR)/%.elf

lf-clean:
	make clean -C $(LF_PROJECT_ROOT)/src-gen/$(APP)

lf-wcet:
	make wcet -C c/apps/lf-workspace/hello/src-gen/$(APP)
# High-level pasim simulation
swsim: $(BUILDDIR)/$(BOOTAPP).bin
	$(INSTALLDIR)/bin/pasim --debug --debug-fmt=short $(BUILDDIR)/$(BOOTAPP).bin; exit 0

# ISA simulation with PatSim
isasim: $(BUILDDIR)/$(BOOTAPP).bin
	cd isasim; sbt "runMain patsim.PatSim $(BUILDDIR)/$(BOOTAPP).bin"

# C simulation of the Chisel version of Patmos
hwsim:
	$(MAKE) -C hardware test BOOTBUILDROOT=$(CURDIR) BOOTAPP=$(BOOTAPP)

# Testing
test: test_compile test_emu

test_compile:
	make clean
	make emulator
	make synth

test_emu:
	testsuite/run.sh
.PHONY: test test_emu

# Build documentation
doc:
	make -C doc all
.PHONY: doc

# Compile Patmos and download
patmos: gen synth config

# configure the FPGA
config:
ifeq ($(VENDOR),Xilinx)
	$(INSTALLDIR)/bin/config_xilinx hardware/ise/$(BOARD)/patmos_top.bit
endif
ifeq ($(VENDOR),XilinxVivado)
	vivado -mode batch -source hardware/vivado/$(BOARD)/config.tcl
endif
ifeq ($(VENDOR),Altera)
	$(INSTALLDIR)/bin/config_altera -b $(BLASTER_TYPE) hardware/quartus/$(BOARD)/patmos.sof
endif

gen:
	$(MAKE) -C hardware verilog BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)

elab:
ifeq ($(VENDOR),Altera)
	$(MAKE) -C hardware elab_quartus BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)
endif

rtlview:
ifeq ($(VENDOR),Altera)
	$(MAKE) -C hardware rtl_quartus BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)
endif

synth:
ifeq ($(VENDOR),Xilinx)
	$(MAKE) -C hardware synth_ise BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)
endif
ifeq ($(VENDOR),XilinxVivado)
	$(MAKE) -C hardware synth_vivado BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)
endif
ifeq ($(VENDOR),Altera)
	$(MAKE) -C hardware synth_quartus BOOTAPP=$(BOOTAPP) BOARD=$(BOARD)
endif

download: $(BUILDDIR)/$(APP).elf
	$(INSTALLDIR)/bin/patserdow -v $(COM_PORT) $<

fpgaexec: $(BUILDDIR)/$(APP).elf
	$(INSTALLDIR)/bin/patserdow $(COM_PORT) $<

# cleanup
CLEANEXTENSIONS=rbf rpt sof pin summary ttf qdf dat wlf done qws

mostlyclean:
	-rm -rf $(CTOOLSBUILDDIR) $(BUILDDIR) $(HWBUILDDIR) $(HWEMUBUILDDIR)
	-rm -rf $(JAVATOOLSBUILDDIR)/classes
	-rm -rf hardware/target
	-rm $(HWINSTALLDIR)/bin/patemu

clean: mostlyclean
#	-rm -rf $(INSTALLDIR)/bin
#	-rm -rf $(INSTALLDIR)/lib
	-rm -rf $(JAVATOOLSBUILDDIR)/lib
	-rm -rf $(HWBUILDDIR)
	-rm -rf $(HWEMUBUILDDIR)
	-rm -rf $(CURDIR)/hardware/emulator_config.h
	for ext in $(CLEANEXTENSIONS); do \
		find `ls` -name \*.$$ext -print -exec rm -r -f {} \; ; \
	done
	-find `ls` -name patmos.pof -print -exec rm -r -f {} \;
	-find `ls` -name db -print -exec rm -r -f {} \;
	-find `ls` -name incremental_db -print -exec rm -r -f {} \;
	-find `ls` -name patmos_description.txt -print -exec rm -r -f {} \;

# Dummy target to force the execution of recipies for things that are not really phony
.FORCE:
