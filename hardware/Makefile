# Get the hardware build dir from the parent make
HWBUILDDIR?=$(CURDIR)/build

BOOTAPP?=bootable-bootloader

BOOTBUILDROOT?=$(CURDIR)/..
BOOTBUILDDIR?=$(BOOTBUILDROOT)/tmp
BOOTBIN?=$(BOOTBUILDDIR)/$(BOOTAPP).bin
BOOTDAT?=$(BOOTBUILDDIR)/$(BOOTAPP).dat

BOARD?=altde2-115
#BOARD?=altde10-NANO-oc
BOOTSPM_BASE?=0x00020000

HWMODULEPREFIX?=

#SBT=java -Xmx1024M -Xss8M -jar sbt/sbt-launch.jar
SBT= sbt

# C++ build flags
CXX=g++
CXXFLAGS=-O1

# Quick fix for Mac OS X
# How are include paths handled these days in *nix? CMake?
ifeq ($(TERM_PROGRAM),$(filter $(TERM_PROGRAM), Apple_Terminal iTerm.app))
	INC_EXTRA=-I /opt/local/include -I /usr/local/include/libelf -I /opt/local/include/libelf -L /opt/local/lib
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
TDMARBITERMAIN=ocp.TdmArbiterWrapperMain
MEMSSPM?=sspm.SSPMAegeanMain

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

# Ensure configuration for SSPM is defined
CORE_CNT?=4
EXT_SLOT_SIZE?=5

# build the C++ version
emulator: $(HWBUILDDIR)/emulator

$(HWBUILDDIR)/emulator: emulator.cpp $(HWBUILDDIR)/Patmos.cpp $(HWBUILDDIR)/emulator_config.h
	$(CXX) $(CXXFLAGS) $(INC_EXTRA) -I $(HWBUILDDIR) -o $@ $(filter %.cpp,$^) -lelf

$(HWBUILDDIR)/Patmos.cpp $(HWBUILDDIR)/emulator_config.h: $(CGEN)
	if [ x`cat $(CGEN)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)" ]; then \
		rm $(CGEN); $(MAKE) $(CGEN); \
	fi

$(CGEN): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "runMain $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --vcd --targetDir $(HWBUILDDIR) --cse --backend c"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT) > $(CGEN)

# run emulator
test: emulator
	$(HWBUILDDIR)/emulator -r -i -l 1000000 -O /dev/null; exit 0

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
	$(SBT) "runMain $(MAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --targetDir $(HWBUILDDIR) --cse --backend v $(HWMODULEPREFIXOPT)"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX) > $(VGEN)

$(HWBUILDDIR)/$(HWMODULEPREFIX)PatmosCore.v: $(VGENCORE)
	if [ x`cat $(VGENCORE)` != x"$(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX)" ]; then \
		rm $(VGENCORE); $(MAKE) $(VGENCORE); \
	fi

$(VGENCORE): $(BOOTBIN) $(BOOTDAT) $(SRC)
	$(SBT) "runMain $(COREMAIN) $(CONFIGFILE) $(BOOTBIN) $(BOOTDAT) --targetDir $(HWBUILDDIR) --cse --backend v $(HWMODULEPREFIXOPT)"
	echo $(CONFIGFILE)+$(BOOTBIN)+$(BOOTDAT)+$(HWMODULEPREFIX) > $(VGENCORE)

$(HWBUILDDIR)/SSRam32Ctrl.v: $(SRC)
	$(SBT) "runMain $(MEMCTRLMAIN32) $(MEMCTRL_ADDR_WIDTH) --targetDir $(HWBUILDDIR) --cse --backend v"

$(HWBUILDDIR)/SRamCtrl.v: $(SRC)
	$(SBT) "runMain $(MEMCTRLMAIN16) $(MEMCTRL_ADDR_WIDTH) --targetDir $(HWBUILDDIR) --cse --backend v"

$(HWBUILDDIR)/OCRamCtrl.v: $(SRC)
	$(SBT) "runMain $(MEMCTRLMAINOC) $(MEMCTRL_ADDR_WIDTH) --targetDir $(HWBUILDDIR) --cse --backend v"

$(HWBUILDDIR)/Arbiter.v: $(SRC)
	$(SBT) "runMain $(ARBITERMAIN) $(ARBITER_CNT) $(ARBITER_ADDR_WIDTH) $(ARBITER_DATA_WIDTH) $(ARBITER_BURST_LENGTH) --targetDir $(HWBUILDDIR) --cse --backend v"

$(HWBUILDDIR)/TdmArbiterWrapper.v: $(SRC)
	$(SBT) "runMain $(TDMARBITERMAIN) $(ARBITER_CNT) $(ARBITER_ADDR_WIDTH) $(ARBITER_DATA_WIDTH) $(ARBITER_BURST_LENGTH) --targetDir $(HWBUILDDIR) --cse --backend v"

$(HWBUILDDIR)/SSPMAegean.v: $(SRC)
	$(SBT) "runMain $(MEMSSPM) $(CORE_CNT) --targetDir $(HWBUILDDIR) --cse --backend v"

# run Verilog simulation
vsim: verilog
	echo '`timescale 1 ns / 100 ps' > $(HWBUILDDIR)/Patmos.v.tmp
	cat $(HWBUILDDIR)/Patmos.v >> $(HWBUILDDIR)/Patmos.v.tmp
	mv $(HWBUILDDIR)/Patmos.v.tmp $(HWBUILDDIR)/Patmos.v
	$(MAKE) -C modelsim sim

# synthesize with Quartus
synth_quartus: verilog
	quartus_map quartus/$(BOARD)/patmos
	quartus_fit quartus/$(BOARD)/patmos
	quartus_asm quartus/$(BOARD)/patmos
	quartus_sta quartus/$(BOARD)/patmos

# synthesize with ISE
synth_ise: verilog
	echo "project open ise/$(BOARD)/patmos.xise; process run \"Generate Programming File\" -force rerun_all" | xtclsh -

# synthesize with Viviado
synth_vivado: verilog
	vivado -mode batch -source vivado/$(BOARD)/synth.tcl

# build the boot binary
$(BOOTDAT): $(BOOTBIN)
$(BOOTBIN): .FORCE
	$(MAKE) -C $(BOOTBUILDROOT) BUILDDIR=$(BOOTBUILDDIR) BOOTSPM_BASE=$(BOOTSPM_BASE) $(BOOTBIN) $(BOOTDAT)

# for module tests
MODULE=example.FsmMain
MODULE=ocp.test.OcpTester
MODULE=ocp.test.ArbiterTester
MODULE=io.test.BranchPredictTester
#MODULE=ocp.test.TdmArbiterWrapperTester
#MODULE=ocp.test.TdmArbiterTester

modtest:
#	-rm -rf generated
	$(SBT) "runMain $(MODULE) --cse --backend c --compile --vcd --test --genHarness --targetDir build"

modcomp:
#	-rm -rf generated
	$(SBT) "runMain $(MODULE) --cse --backend v --compile --vcd --targetDir build"

.FORCE:

.PHONY: all emulator test view verilog vsim

# SSPM make targets

SSPM-test:
	$(SBT) "runMain io.SSPMTester"

SSPMAegean:
	$(SBT) "runMain sspm.SSPMAegeanMain $(CORE_CNT) $(EXT_SLOT_SIZE) false --targetDir $(HWGENDIR) --cse --backend v"

SSPMAegean-test:
	$(SBT) "runMain sspm.SSPMAegeanTester"

SSPM-scheduler:
	$(SBT) "runMain sspm.SchedulerMain"

SSPM-scheduler-test:
	$(SBT) "runMain sspm.SchedulerTester"

SSPM-connector:
	$(SBT) "runMain sspm.SSPMConnectorMain"

SSPM-connector-test:
	$(SBT) "runMain sspm.SSPMConnectorTester"

SSPM-memmodule:
	$(SBT) "runMain sspm.memModuleMain"

SSPM-memmodule-test:
	$(SBT) "runMain sspm.memModuleTester"

SSPM-memspm:
	$(SBT) "runMain sspm.memSPMMain"

SSPM-memspm-test:
	$(SBT) "runMain sspm.memSPMTester"
