

# cleanup
EXTENSIONS=class rbf rpt sof pin summary ttf qdf dat wlf done qws

#
#	Set USB to true for an FTDI chip based board (dspio, usbmin, lego)
#
USB=false


# Assembler files
APP=test
# Altera FPGA configuration cable
#BLASTER_TYPE=ByteBlasterMV
BLASTER_TYPE=USB-Blaster

ifeq ($(WINDIR),)
	USBRUNNER=./USBRunner
	S=:
else
	USBRUNNER=USBRunner.exe
	S=\;
endif

# The VHDL project for Quartus
QPROJ=dspio
QPROJ=altde2-70

all: directories tools rom
	make patmos

directories:
	-mkdir rbf

patsim:
	-mkdir simulator/build
	cd simulator/build && cmake ..
	cd simulator/build && make
	-mkdir bin
	cp simulator/build/src/pa* bin

tools:
	make patsim
	-rm -rf java/classes
	-rm -rf java/lib
	-rm -rf java/src/patmos/asm/generated
	mkdir java/classes
	mkdir java/lib
	mkdir java/src/patmos/asm/generated
	java -classpath lib/antlr-3.3-complete.jar org.antlr.Tool \
		-fo java/src/patmos/asm/generated java/src/grammar/PatGram.g
	javac -classpath lib/antlr-3.3-complete.jar \
		-d java/classes java/src/patmos/asm/generated/*.java \
		java/src/patmos/asm/*.java
	javac java/src/simulator/*.java \
		-d java/classes
	cd java/classes && jar cf ../lib/patmos-tools.jar *

rom: tools
	-rm -rf vhdl/generated
	mkdir vhdl/generated
	-mkdir tmp
	bin/paasm asm/$(APP).asm tmp/$(APP).bin
	java -cp java/lib/patmos-tools.jar \
		patmos.asm.Bin2Vhdl -s tmp -d vhdl/generated $(APP).bin

old_rom: tools
	-rm -rf vhdl/generated
	mkdir vhdl/generated
	java -cp java/lib/patmos-tools.jar$(S)lib/antlr-3.3-complete.jar \
		patmos.asm.PatAsm -s asm -d vhdl/generated $(APP).asm

# ModelSim
sim:
	cd modelsim; make

old_sim:
	java -cp java/lib/patmos-tools.jar \
		simulator.SimPat

# Compile Patmos and download
patmos:
	make synth
	make config

# configure the FPGA
config:
ifeq ($(USB),true)
	make config_usb
else
ifeq ($(XFPGA),true)
	make config_xilinx
else
	make config_byteblaster
endif
endif

synth:
	@echo $(QPROJ)
	for target in $(QPROJ); do \
		make qsyn -e QBT=$$target || exit; \
#		cd quartus/$$target && quartus_cpf -c patmos.sof ../../rbf/$$target.rbf; \
	done

#
#	Quartus build process
#		called by jopser, jopusb,...
#
qsyn:
	echo $(QBT)
	echo "building $(QBT)"
	-rm -rf quartus/$(QBT)/db
	-rm -f quartus/$(QBT)/patmos.sof
	-rm -f jbc/$(QBT).jbc
	-rm -f rbf/$(QBT).rbf
	quartus_map quartus/$(QBT)/patmos
	quartus_fit quartus/$(QBT)/patmos
	quartus_asm quartus/$(QBT)/patmos
	quartus_sta quartus/$(QBT)/patmos

config_byteblaster:
	cd quartus/$(QPROJ) && quartus_pgm -c $(BLASTER_TYPE) -m JTAG patmos.cdf

config_usb:
	cd rbf && ../$(USBRUNNER) $(QPROJ).rbf

# TODO: no Xilinx Makefiles available yet
config_xilinx:
	cd xilinx/$(XPROJ) && make config

clean:
	for ext in $(EXTENSIONS); do \
		find `ls` -name \*.$$ext -print -exec rm -r -f {} \; ; \
	done
	-find `ls` -name patmos.pof -print -exec rm -r -f {} \;
	-find `ls` -name db -print -exec rm -r -f {} \;
	-find `ls` -name incremental_db -print -exec rm -r -f {} \;
	-find `ls` -name patmos_description.txt -print -exec rm -r -f {} \;
	-rm -rf asm/generated
	-rm -f vhdl/*.vhd
	-rm -rf $(TOOLS)/dist
	-rm -rf $(PCTOOLS)/dist
	-rm -rf $(TARGET)/dist
	-rm -rf modelsim/work
	-rm -rf modelsim/transcript
	-rm -rf modelsim/gaisler
	-rm -rf modelsim/grlib
	-rm -rf modelsim/techmap
