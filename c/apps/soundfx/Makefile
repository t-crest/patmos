DEFINES ?= -D USE_COP -D DETAILED_TIMING -D DOUBLE_DISPATCH

all:
	patmos-clang $(DEFINES) -O2 *.c -I ../.. -o soundfx.elf

sim:
	patemu -v soundfx.elf

down:
	patserdow -v /dev/ttyUSB0 soundfx.elf

bench_cop_art:
	make all DEFINES="-D USE_COP -D ARTIFICIAL -D DETAILED_TIMING -D DOUBLE_DISPATCH"
	make down > cop_art_det_dd.txt
	make all DEFINES="-D USE_COP -D ARTIFICIAL -D DOUBLE_DISPATCH"
	make down > cop_art_nodet_dd.txt
	make all DEFINES="-D USE_COP -D ARTIFICIAL -D DETAILED_TIMING"
	make down > cop_art_det_nodd.txt
	make all DEFINES="-D USE_COP -D ARTIFICIAL"
	make down > cop_art_nodet_nodd.txt

bench_cop_aa:
	make all DEFINES="-D USE_COP -D USE_ITERATIONS -D DETAILED_TIMING -D DOUBLE_DISPATCH"
	make down > cop_aa_det_dd.txt
	make all DEFINES="-D USE_COP -D USE_ITERATIONS -D DOUBLE_DISPATCH"
	make down > cop_aa_nodet_dd.txt
	make all DEFINES="-D USE_COP -D USE_ITERATIONS -D DETAILED_TIMING"
	make down > cop_aa_det_nodd.txt
	make all DEFINES="-D USE_COP -D USE_ITERATIONS"
	make down > cop_aa_nodet_nodd.txt

bench_sw_art:
	make all DEFINES="-D USE_SW -D ARTIFICIAL -D DETAILED_TIMING"
	make down > sw_art_det.txt
	make all DEFINES="-D USE_SW -D ARTIFICIAL"
	make down > sw_art_nodet.txt

bench_sw_aa:
	make all DEFINES="-D USE_SW -D USE_ITERATIONS -D DETAILED_TIMING"
	make down > sw_aa_det.txt
	make all DEFINES="-D USE_SW -D USE_ITERATIONS"
	make down > sw_aa_nodet.txt

