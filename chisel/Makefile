# Get the chisel build dir from the parent make
CHISELBUILDDIR?=$(CURDIR)/build

BOOTAPP?=basic

BOOTBUILDROOT?=$(CURDIR)/..
BOOTBUILDDIR?=$(BOOTBUILDROOT)/tmp
BOOTBIN?=$(BOOTBUILDDIR)/$(BOOTAPP).bin
BOOTDAT?=$(BOOTBUILDDIR)/$(BOOTAPP).dat

QPROJ?=altde2-70

# Building Patmos with Chisel without too much sbt/scala/... stuff
#
# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder
#
# Source directory is configured in build.sbt
# Scala/Java build directory is the default one
SBT=java -Xmx1024M -Xss8M -XX:MaxPermSize=128M -jar sbt/sbt-launch.jar

# C++ build flags
CXX=g++
CXXFLAGS=-O2 -g

# Quick fix for Mac OS X
# How are include paths handled these days in *nix? CMake?
ifeq ($(TERM_PROGRAM),$(filter $(TERM_PROGRAM), Apple_Terminal iTerm.app))
	INC_EXTRA=-I /opt/local/include -I /opt/local/include/libelf -L/opt/local/lib
else
	INC_EXTRA=
endif

# consider everything a source file
SRC=$(shell find src -name '*.scala')

# main class for Patmos
MAIN=patmos.PatmosMain

# config file for Patmos -- now yust in test mode
CONFIG=default
CONFIGFILE=config/$(CONFIG).xml

# build C++ and Verilog versions by default
all: emulator verilog

# Temporary files to remember the ROM we built into the C++ and Verilog versions
CGEN=$(CHISELBUILDDIR)/.cgen
VGEN=$(CHISELBUILDDIR)/.vgen

# build the C++ version
emulator: $(CHISELBUILDDIR)/emulator

$(CHISELBUILDDIR)/emulator: emulator.cpp $(CHISELBUILDDIR)/Patmos.cpp
	$(CXX) $(CXXFLAGS) $(INC_EXTRA) -I $(CHISELBUILDDIR) -o $@ $^ -lelf

$(CHISELBUILDDIR)/Patmos.cpp: $(CGEN)
	if [ x`cat $(CGEN)` != x"$(BOOTBIN)+$(BOOTDAT)" ]; then \
		rm $(CGEN); $(MAKE) $(CGEN); \
	fi

$(CGEN): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --vcd --targetDir $(CHISELBUILDDIR) --backend c"
	echo $(BOOTBIN)+$(BOOTDAT) > $(CGEN)

# run emulator
test: emulator
	$(CHISELBUILDDIR)/emulator -r -n -v -l 1000000

# view detailed emulation results
view:
	gtkwave Patmos.vcd wave.gtkw

# build the Verilog version
verilog: $(CHISELBUILDDIR)/Patmos.v

$(CHISELBUILDDIR)/Patmos.v: $(VGEN)
	if [ x`cat $(VGEN)` != x"$(BOOTBIN)+$(BOOTDAT)" ]; then \
		rm $(VGEN); $(MAKE) $(VGEN); \
	fi

$(VGEN): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --targetDir $(CHISELBUILDDIR) --backend v"
	echo $(BOOTBIN)+$(BOOTDAT) > $(VGEN)

# run Verilog simulation
vsim: verilog
	$(MAKE) -C modelsim sim

# synthesize
qsyn: verilog
	quartus_map quartus/$(QPROJ)/patmos
	quartus_fit quartus/$(QPROJ)/patmos
	quartus_asm quartus/$(QPROJ)/patmos
	quartus_sta quartus/$(QPROJ)/patmos

# build the boot binary
$(BOOTBIN) $(BOOTDAT): .FORCE
	$(MAKE) -C $(BOOTBUILDROOT) BUILDDIR=$(BOOTBUILDDIR) $(BOOTBIN) $(BOOTDAT)

# for module tests
MODUL=example.FsmMain
MODUL=ocp.test.OcpTester
MODUL=ocp.test.ArbiterTester
modtest:
#	-rm -rf generated
	$(SBT) "run-main $(MODUL) --backend c --compile --vcd --test --genHarness --targetDir generated"

.FORCE:

.PHONY: all emulator test view verilog vsim
