# Get the hardware build dir from the parent make
HWBUILDDIR?=$(CURDIR)/build

BOOTAPP?=basic

BOOTBUILDROOT?=$(CURDIR)/..
BOOTBUILDDIR?=$(BOOTBUILDROOT)/tmp
BOOTBIN?=$(BOOTBUILDDIR)/$(BOOTAPP).bin
BOOTDAT?=$(BOOTBUILDDIR)/$(BOOTAPP).dat

BOARD?=altde2-70

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
	INC_EXTRA=-I /opt/local/include -I /usr/local/include/libelf -I /opt/local/include/libelf -L/opt/local/lib
else
	INC_EXTRA=
endif

# consider everything a source file
SRC=$(shell find src -name '*.scala')

# main class for Patmos
MAIN=patmos.PatmosMain

# main classes for core, memory controller and arbiter
COREMAIN=patmos.PatmosCoreMain
MEMCTRLMAIN32=io.SSRam32Main
MEMCTRLMAIN16=io.SRamMain
MEMCTRLMAINOC=io.OCRamMain
ARBITERMAIN=ocp.ArbiterMain

# config file for Patmos -- now just frequency
#CONFIG=default
CONFIGFILE?=$(BOOTBUILDROOT)/hardware/config/$(BOARD).xml
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
CGEN=$(HWBUILDDIR)/.cgen
VGEN=$(HWBUILDDIR)/.vgen
VGENCORE=$(HWBUILDDIR)/.vgencore

# build the C++ version
emulator: $(HWBUILDDIR)/emulator

$(HWBUILDDIR)/emulator: emulator.cpp $(HWBUILDDIR)/Patmos.cpp
	$(CXX) $(CXXFLAGS) $(INC_EXTRA) -I $(HWBUILDDIR) -o $@ $^ -lelf

$(HWBUILDDIR)/Patmos.cpp: $(CGEN)
	if [ x`cat $(CGEN)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)" ]; then \
		rm $(CGEN); $(MAKE) $(CGEN); \
	fi

$(CGEN): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --vcd --targetDir $(HWBUILDDIR) --backend c"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT) > $(CGEN)

# run emulator
test: emulator
	$(HWBUILDDIR)/emulator -r -n -v -l 1000000

# view detailed emulation results
view:
	gtkwave Patmos.vcd wave.gtkw

# build the Verilog version
verilog: $(HWBUILDDIR)/$(HWMODULEPREFIX)Patmos.v

modules: \
	$(HWBUILDDIR)/$(HWMODULEPREFIX)PatmosCore.v \
	$(HWBUILDDIR)/SsramBurstRW.v

$(HWBUILDDIR)/$(HWMODULEPREFIX)Patmos.v: $(VGEN)
	if [ x`cat $(VGEN)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX)" ]; then \
		rm $(VGEN); $(MAKE) $(VGEN); \
	fi

$(VGEN): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --targetDir $(HWBUILDDIR) --backend v $(HWMODULEPREFIXOPT)"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX) > $(VGEN)

$(HWBUILDDIR)/$(HWMODULEPREFIX)PatmosCore.v: $(VGENCORE)
	if [ x`cat $(VGENCORE)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX)" ]; then \
		rm $(VGENCORE); $(MAKE) $(VGENCORE); \
	fi

$(VGENCORE): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "run-main $(COREMAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --targetDir $(HWBUILDDIR) --backend v $(HWMODULEPREFIXOPT)"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX) > $(VGENCORE)

$(HWBUILDDIR)/SSRam32Ctrl.v: $(SRC)
	$(SBT) "run-main $(MEMCTRLMAIN32) $(MEMCTRL_ADDR_WIDTH) --targetDir $(HWBUILDDIR) --backend v"

$(HWBUILDDIR)/SRamCtrl.v: $(SRC)
	$(SBT) "run-main $(MEMCTRLMAIN16) $(MEMCTRL_ADDR_WIDTH) --targetDir $(HWBUILDDIR) --backend v"

$(HWBUILDDIR)/OCRamCtrl.v: $(SRC)
	$(SBT) "run-main $(MEMCTRLMAINOC) $(MEMCTRL_ADDR_WIDTH) --targetDir $(HWBUILDDIR) --backend v"

$(HWBUILDDIR)/Arbiter.v: $(SRC)
	$(SBT) "run-main $(ARBITERMAIN) $(ARBITER_CNT) $(ARBITER_ADDR_WIDTH) $(ARBITER_DATA_WIDTH) $(ARBITER_BURST_LENGTH) --targetDir $(HWBUILDDIR) --backend v"

# run Verilog simulation
vsim: verilog
	$(MAKE) -C modelsim sim

# synthesize
qsyn: verilog
	quartus_map quartus/$(BOARD)/patmos
	quartus_fit quartus/$(BOARD)/patmos
	quartus_asm quartus/$(BOARD)/patmos
	quartus_sta quartus/$(BOARD)/patmos

# build the boot binary
$(BOOTDAT): $(BOOTBIN)
$(BOOTBIN): .FORCE
	$(MAKE) -C $(BOOTBUILDROOT) BUILDDIR=$(BOOTBUILDDIR) $(BOOTBIN) $(BOOTDAT)

# for module tests
MODULE=example.FsmMain
MODULE=ocp.test.OcpTester
MODULE=ocp.test.ArbiterTester
#MODULE=ocp.test.NodeTdmArbiterTester
#MODULE=ocp.test.TdmArbiterTester

modtest:
#	-rm -rf generated
	$(SBT) "run-main $(MODULE) --backend c --compile --vcd --test --genHarness --targetDir build"

modcomp:
#	-rm -rf generated
	$(SBT) "run-main $(MODULE) --backend v --compile --vcd --targetDir build"

.FORCE:

.PHONY: all emulator test view verilog vsim
