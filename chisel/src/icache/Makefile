# sbt looks for default into a folder project/ for build.sdt and Build.scala
# sbt creates per default a target folder
SBT = java -Xmx1024M -Xss8M -XX:MaxPermSize=2G -jar sbt/sbt-launch.jar

hdldir=cache_generated
simdir=cache_emulator
basedir=../../
binfile=pclang_files/patmos_in

# what is --checkPorts?
SBTARGS_SIM = --backend c --vcd --compile --test --targetDir $(simdir) --genHarness
SBTARGS_HDL = --backend v --targetDir $(hdldir) --genHarness

MCACHEMAIN=patmos.MCPatmosMain
MCACHETOP=MCPatmos
MAIN=icache.PatmosMain
TOP=Patmos

# Generate Verilog code and C++ for Emulation
all:	hdl sim

# Generare Verilog code
icache:
	cd $(basedir); $(SBT) "run-main $(MAIN) $(binfile) $(SBTARGS_HDL)"

# with input bin file and vcd generation
isim:
	cd $(basedir); $(SBT) "run-main $(DIR)$(MAIN) $(binfile) $(SBTARGS_SIM)"

mcache:
	cd $(basedir); $(SBT) "run-main $(MCACHEMAIN) $(binfile) $(SBTARGS_HDL)"

msim:
	cd $(basedir); $(SBT) "run-main $(DIR)$(MCACHEMAIN) $(binfile) $(SBTARGS_SIM)"

# clean all
clean:
	cd $(basedir); rm -rf $(simdir) $(hdldir) $(TOP).vcd $(MCACHETOP).vcd

.PHONY: all clean
