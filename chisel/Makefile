#
# Building Patmos with Chisel without too much sbt/scala/... stuff
#
# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder

SBT = java -Xmx1024M -Xss8M -XX:MaxPermSize=128M -jar sbt/sbt-launch.jar

CXX=g++
CXXFLAGS=-O2 -g

# Quick fix for Mac OS X
# How are include paths handled these days in *nix? CMake?
ifeq ($(TERM_PROGRAM),Apple_Terminal)
	INC_EXTRA=-I /opt/local/include -I /opt/local/include/libelf -L/opt/local/lib
else
	INC_EXTRA=
endif

# This generates the C++ files by invoking main from
# class HelloMain in package example
# Source directory is configured in project/Build.scala
# Scala/Java build directory is configured in project/Build.scala

# The first two arguments are consumed by sbt, the others are
# forwarded to the 

# command to compile the C++ files is missing

MAIN=example.FsmMain
MAIN=patmos.PatmosMain

APP=basic

SRC=$(shell find src -name '*.scala')

all:
	echo what to do?

asm:
	cd .. && make rom -e APP=$(APP)

comp:
	cd .. && make comp -e APP=$(APP)


oldtest:
	$(SBT) "run-main $(MAIN) ../tmp/aout --genHarness --compile --test --targetDir generated --backend c"

test: generated/emulator
	./generated/emulator -l 1000000

generated/Patmos.cpp: $(SRC) ../tmp/aout.bin
	$(SBT) "run-main $(MAIN) ../tmp/aout --vcd --targetDir generated --backend c"

generated/emulator: emulator.cpp generated/Patmos.cpp
	$(CXX) $(CXXFLAGS) $(INC_EXTRA) -I generated -o $@ $^ -lelf

# the Verilog version
verilog: generated/Patmos.v

generated/Patmos.v:	$(SRC) ../tmp/aout.bin
	$(SBT) "run-main $(MAIN) ../tmp/aout --targetDir generated --backend v"

vsim: verilog
	make -C modelsim sim

###### Minimum Makefile ends here ########

view:
	gtkwave Patmos.vcd wave.gtkw

intsbt:
	$(SBT)
