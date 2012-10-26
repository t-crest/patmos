

#temporary default target
notice:
	@echo
	@echo 'To use this SDRAM test some changes are needed in patmos source:'
	@echo '  * patmos_core.vhd: uncomment the lines containing the dma_; change uart clock freq to 100MHz; increase the uart fifo depth to 5'
	@echo '  * patmos_fetch.vhd: increase the size of instruction memory'
	@echo '  * match the size of generated rom in java/src/patmos/asm/Bin2Vhdl.java and recompile it'
	@echo '  * generate the rom in patmos base_dir: make rom APP=test_sdram'
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

# work around for BMM file:
# The platgen doesn't know that we want to use BRAM inside the patmos core for updating the programs
# So we have to find the placement of our bram in the synthesized file.
# NOTE: for this to work, the application compiled with 'make rom APP=test_app' in base directory
# must be big enougth, for the tool to infer the Block RAM (and not just LUT distributed ROM)
patmos.bmm: bits
	xdl -ncd2xdl implementation/system
	echo -e 'ADDRESS_SPACE fet_rom_evn RAMB36 [0x00000000:0x000011FF]\n BUS_BLOCK\n  patmos_top_0/patmos_top_0/fet_rom_evn/Mram_data_mem1 [35:0] PLACED = '`grep 'inst.*fet_rom' system.xdl | sed 's/^.*RAMB36_//; s/ *,$$//'`';\n END_BUS_BLOCK;\nEND_ADDRESS_SPACE;' > patmos.bmm

