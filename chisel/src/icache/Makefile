# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder
SBT = java -Xmx1024M -Xss8M -XX:MaxPermSize=2G -jar sbt/sbt-launch.jar

hdldir=build
simdir=build
basedir=../../
binfile=../tmp/patmos_in

# what is --checkPorts?
SBTARGS_SIM = --backend c --vcd --compile --debug --test --targetDir $(simdir) --genHarness
SBTARGS_HDL = --backend v --targetDir $(hdldir) --genHarness

MCACHEMAIN=patmos.MCPatmosMain
MCACHETOP=MCPatmos
SSRAMMAIN=patmos.SsramMain
SSRAMTOP=Ssram

# Generate Verilog code and C++ for Emulation
all:	mcache msim

mcache:
	cd $(basedir); $(SBT) "run-main $(MCACHEMAIN) $(binfile) $(SBTARGS_HDL)"

msim:
	cd $(basedir); $(SBT) "run-main $(DIR)$(MCACHEMAIN) $(binfile) $(SBTARGS_SIM)"

ssram:
	cd $(basedir); $(SBT) "run-main $(SSRAMMAIN) $(SBTARGS_HDL)"

ssramsim:
	cd $(basedir); $(SBT) "run-main $(DIR)$(SSRAMMAIN) $(binfile) $(SBTARGS_SIM)"

# clean all
clean:
	cd $(basedir); rm -rf $(simdir) $(hdldir) $(ICACHETOP).vcd $(MCACHETOP).vcd

.PHONY: all clean
