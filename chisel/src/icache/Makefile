# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder
SBT = java -Xmx1024M -Xss8M -XX:MaxPermSize=2G -jar sbt/sbt-launch.jar

hdldir=icache_generated
simdir=icache_emulator
basedir=../../
binfile=pclang_files/patmos_in

# what is --checkPorts?
SBTARGS_SIM = --backend c --vcd --compile --test --targetDir $(simdir) --genHarness
SBTARGS_HDL = --backend v --targetDir $(hdldir) --genHarness

#MAIN=icache.TopMain
#TOP=Top
MAIN=icache.PatmosMain
TOP=Patmos

# Generate Verilog code and C++ for Emulation
all:	hdl sim

# Generare Verilog code
hdl:
	cd $(basedir); $(SBT) "run-main $(MAIN) $(binfile) $(SBTARGS_HDL)"

# .vcd generation
sim:
	cd $(basedir); $(SBT) "run-main $(DIR)$(MAIN) $(SBTARGS_SIM)"

# with input bin file
test:
	cd $(basedir); $(SBT) "run-main $(DIR)$(MAIN) $(binfile) $(SBTARGS_SIM)"

# clean all
clean:
	rm -rf $(basedir)$(simdir) $(basedir)$(hdldir) $(basedir)$(TOP).vcd

.PHONY: all clean
