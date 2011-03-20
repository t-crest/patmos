

# cleanup
EXTENSIONS=class rbf rpt sof pin summary ttf qdf dat wlf done qws

#
#	Set USB to true for an FTDI chip based board (dspio, usbmin, lego)
#
USB=false


# Assembler fils
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
	make rbf
	make config

directories:
	-mkdir rbf

assembler:
	java -cp lib/antlr-3.3-complete.jar org.antlr.Tool -fo . java/src/grammar/PatGram.g
	javac -cp lib/antlr-3.3-complete.jar -d . java/src/Test.java PatGramLexer.java PatGramParser.java
	make run

run:
	java -cp .:lib/antlr-3.3-complete.jar Test < asm/test.asm


tools:
	-rm -rf java/classes
	-rm -rf java/lib
	mkdir java/classes
	mkdir java/lib
	javac -d java/classes -sourcepath java/src java/src/patmos/asm/*.java
	cd java/classes && jar cf ../lib/patmos-tools.jar *

rom:
	-rm -rf vhdl/generated
	mkdir vhdl/generated
	java -cp java/lib/patmos-tools.jar patmos.asm.PatAsm -s asm -d vhdl/generated $(APP).asm

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

rbf:
	@echo $(QPROJ)
	for target in $(QPROJ); do \
		make qsyn -e QBT=$$target || exit; \
		cd quartus/$$target && quartus_cpf -c patmos.sof ../../rbf/$$target.rbf; \
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
