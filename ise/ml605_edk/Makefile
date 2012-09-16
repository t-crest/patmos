

#temporary default target
notice:
	@echo
	@echo 'To use this SDRAM test some changes are needed in patmos source:'
	@echo '  * uncomment the lines containing the dma_ in patmos_core.vhd'
	@echo '  * increase the size of instruction memory in patmos_fetch.vhd'
	@echo '  * match the size of generated rom in java/src/patmos/asm/Bin2Vhdl.java and recompile it'
	@echo '  * generate the rom in patmos base_dir: make rom APP=test_bram'
	@echo 'When done one can build this project: make bits'
	@echo


#default target, this shows help from system.make
all:


%: system.make force
	make -f system.make $@

force: ;

system.make: pcores_proprietary
	@echo Creating Makefile from XPS project...
	echo exit | xps -nw system.xmp # this open the project and creates the makefile	
	
pcores_proprietary:
	@echo Downloading SDRAM controller EDK cores...
	scp -r tipca.imm.dtu.dk:/home/edgarlakis/TUE_memctrl/pcores $@

program: bits
	program_ML605 implementation/system.bit
