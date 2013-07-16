# Get the chisel build dir from the parent make
CHISELBUILDDIR?=$(CURDIR)/build

BOOTAPP?=basic

BOOTBUILDROOT?=$(CURDIR)/..
BOOTBUILDDIR?=$(BOOTBUILDROOT)/tmp
BOOTFILE?=$(BOOTAPP).bin
BOOTBIN?=$(BOOTBUILDDIR)/$(BOOTFILE)

QPROJ?=altde2-70

# Building Patmos with Chisel without too much sbt/scala/... stuff
#
# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder
#
# Source directory is configured in project/Build.scala
# Scala/Java build directory is configured in project/Build.scala
SBT=java -Xmx1024M -Xss8M -XX:MaxPermSize=128M -jar sbt/sbt-launch.jar

# C++ build flags
CXX=g++
CXXFLAGS=-O2 -g

# Quick fix for Mac OS X
# How are include paths handled these days in *nix? CMake?
ifeq ($(TERM_PROGRAM),Apple_Terminal)
	INC_EXTRA=-I /opt/local/include -I /opt/local/include/libelf -L/opt/local/lib
else
	INC_EXTRA=
endif

# consider everything a source file
SRC=$(shell find src -name '*.scala')

# main class for Patmos
MAIN=patmos.PatmosMain

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
	if [ x`cat $(CGEN)` != x"$(BOOTBIN)" ]; then \
		rm $(CGEN); $(MAKE) $(CGEN); \
	fi

$(CGEN): $(BOOTBIN) $(SRC)
	$(SBT) "run-main $(MAIN) $< --vcd --targetDir $(CHISELBUILDDIR) --backend c"
	echo $(BOOTBIN) > $(CGEN)

# run emulator
test: emulator
	$(CHISELBUILDDIR)/emulator -v -l 1000000

# view detailed emulation results
view:
	gtkwave Patmos.vcd wave.gtkw

# build the Verilog version
verilog: $(CHISELBUILDDIR)/Patmos.v

$(CHISELBUILDDIR)/Patmos.v: $(VGEN)
	if [ x`cat $(VGEN)` != x"$(BOOTBIN)" ]; then \
		rm $(VGEN); $(MAKE) $(VGEN); \
	fi

$(VGEN): $(BOOTBIN) $(SRC)
	$(SBT) "run-main $(MAIN) $< --targetDir $(CHISELBUILDDIR) --backend v"
	echo $(BOOTBIN) > $(VGEN)

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
$(BOOTBIN): .FORCE
	$(MAKE) -C $(BOOTBUILDROOT) BUILDDIR=$(BOOTBUILDDIR) $(BOOTBIN)

.FORCE:

.PHONY: all emulator test view verilog vsim
