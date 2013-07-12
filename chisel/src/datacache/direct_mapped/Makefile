#
# Building a Hello World Chisel without too much sbt/scala/... stuff
#
# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder

SBT = java -Xmx1024M -Xss8M -XX:MaxPermSize=128M -jar sbt/sbt-launch.jar

# This generates the Verilog and C++ files by invoking main from
# class HelloMain in package example.
# The source directory is configured in project/Build.scala.
# The Scala/Java build directory is configured in project/Build.scala

# The first two arguments are consumed by sbt, the others are
# forwarded to the Scala/Chisel main().

hdldir=sc_generated
simdir=sc_emulator
basedir=../../../

test:
#	-rm -rf generated
	cd $(basedir); $(SBT) "run-main dc.HelloMain --backend c --compile --vcd --test --genHarness --targetDir src/datacache/direct_mapped/generated"

# Generare Verilog code
hdl:
	cd $(basedir); $(SBT) "run-main dc.HelloMain --targetDir src/datacache/direct_mapped/generated --backend v"

# C++ genertion
cpp:
	cd $(basedir); $(SBT) "run-main example.HelloMain --targetDir src/datacache/direct_mapped/generated"

###### Minimum Makefile ends here ########

view:
	gtkwave Test.vcd wave.gtkw

intsbt:
	$(SBT)
