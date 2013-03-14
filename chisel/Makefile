#
# Building Patmos with Chisel without too much sbt/scala/... stuff
#
# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder

SBT = java -Xmx1024M -Xss8M -XX:MaxPermSize=128M -jar sbt/sbt-launch.jar

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

all:
	echo what to do?

asm:
	cd .. && make rom -e APP=$(APP)

test: asm
	-rm -rf generated
	$(SBT) "run-main $(MAIN) ../tmp/asm --test --compile --vcd --genHarness --targetDir generated"

# the Verilog version
verilog: asm
	$(SBT) "run-main $(MAIN) ../tmp/asm --targetDir generated --backend v"

###### Minimum Makefile ends here ########

view:
	gtkwave Patmos.vcd wave.gtkw

intsbt:
	$(SBT)
