.PHONY: all clean wcet
all:
	patmos-clang -O2 dsp.c -o dsp.elf 

wcet: 
	patmos-clang -mserialize-pml=dsp.pml -DWCET -O2 dsp.c -o dsp.elf 
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e Clipper --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e windowSY --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e SMA --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e RMS --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e Gain --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e Monitor --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e Tag --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e Audio --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e main --report report.txt

clean: 
	rm -f *.out *.pml *.elf *.txt
