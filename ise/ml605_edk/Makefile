
#default target, this shows help from system.make
all:


%: system.make force
	make -f system.make $@

force: ;

system.make: pcores_proprietary
	echo Creating Makefile from XPS project...
	echo exit | xps -nw system.xmp # this open the project and creates the makefile	
	
pcores_proprietary:
	echo Downloading SDRAM controller EDK cores...
	scp -r tipca.imm.dtu.dk:/home/edgarlakis/TUE_memctrl/pcores $@

