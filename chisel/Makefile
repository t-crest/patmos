# Get the chisel build dir from the parent make
CHISELBUILDDIR?=$(CURDIR)/build

BOOTAPP?=basic

BOOTBUILDROOT?=$(CURDIR)/..
BOOTBUILDDIR?=$(BOOTBUILDROOT)/tmp
BOOTBIN?=$(BOOTBUILDDIR)/$(BOOTAPP).bin
BOOTDAT?=$(BOOTBUILDDIR)/$(BOOTAPP).dat

QPROJ?=altde2-70

HWMODULEPREFIX?=

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
CXXFLAGS=-O2

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

# main classes for core, memory controller and arbiter
COREMAIN=patmos.PatmosCoreMain
MEMCTRLMAIN=patmos.SsramMain
ARBITERMAIN=ocp.ArbiterMain

# config file for Patmos -- now just frequency
CONFIG=default
CONFIGFILE?=$(BOOTBUILDROOT)/chisel/config/$(CONFIG).xml
SRC+=$(CONFIGFILE)

# optionally pass module prefix when generating Verilog
ifeq ($(HWMODULEPREFIX),)
	HWMODULEPREFIXOPT=
else
	HWMODULEPREFIXOPT=--moduleNamePrefix $(HWMODULEPREFIX)
endif

# build C++ and Verilog versions by default
all: emulator verilog

# Temporary files to remember the ROM we built into the C++ and Verilog versions
CGEN=$(CHISELBUILDDIR)/.cgen
VGEN=$(CHISELBUILDDIR)/.vgen
VGENCORE=$(CHISELBUILDDIR)/.vgencore

# build the C++ version
emulator: $(CHISELBUILDDIR)/emulator

$(CHISELBUILDDIR)/emulator: emulator.cpp $(CHISELBUILDDIR)/Patmos.cpp
	$(CXX) $(CXXFLAGS) $(INC_EXTRA) -I $(CHISELBUILDDIR) -o $@ $^ -lelf

$(CHISELBUILDDIR)/Patmos.cpp: $(CGEN)
	if [ x`cat $(CGEN)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)" ]; then \
		rm $(CGEN); $(MAKE) $(CGEN); \
	fi

$(CGEN): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --vcd --targetDir $(CHISELBUILDDIR) --backend c"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT) > $(CGEN)

# run emulator
test: emulator
	$(CHISELBUILDDIR)/emulator -r -n -v -l 1000000

# view detailed emulation results
view:
	gtkwave Patmos.vcd wave.gtkw

# build the Verilog version
verilog: $(CHISELBUILDDIR)/$(HWMODULEPREFIX)Patmos.v

modules: \
	$(CHISELBUILDDIR)/$(HWMODULEPREFIX)PatmosCore.v \
	$(CHISELBUILDDIR)/SsramBurstRW.v

$(CHISELBUILDDIR)/$(HWMODULEPREFIX)Patmos.v: $(VGEN)
	if [ x`cat $(VGEN)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX)" ]; then \
		rm $(VGEN); $(MAKE) $(VGEN); \
	fi

$(VGEN): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --targetDir $(CHISELBUILDDIR) --backend v $(HWMODULEPREFIXOPT)"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX) > $(VGEN)

$(CHISELBUILDDIR)/$(HWMODULEPREFIX)PatmosCore.v: $(VGENCORE)
	if [ x`cat $(VGENCORE)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX)" ]; then \
		rm $(VGENCORE); $(MAKE) $(VGENCORE); \
	fi

$(VGENCORE): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(COREMAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --targetDir $(CHISELBUILDDIR) --backend v $(HWMODULEPREFIXOPT)"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX) > $(VGENCORE)

$(CHISELBUILDDIR)/SsramBurstRW.v: $(SRC)
	$(SBT) "run-main $(MEMCTRLMAIN) $(MEMCTRL_ADDR_WIDTH) --targetDir $(CHISELBUILDDIR) --backend v"

$(CHISELBUILDDIR)/Arbiter.v: $(SRC)
	$(SBT) "run-main $(ARBITERMAIN) $(ARBITER_CNT) $(ARBITER_ADDR_WIDTH) $(ARBITER_DATA_WIDTH) $(ARBITER_BURST_LENGTH) --targetDir $(CHISELBUILDDIR) --backend v"

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
