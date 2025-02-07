.PHONY: all clean wcet
all:
	patmos-clang -O2 dsp.c -o dsp.elf 

wcet: 
	patmos-clang -mserialize-pml=dsp.pml -DWCET -O2 dsp.c -o dsp.elf 
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e dspFilter --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e windowSY --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e dspSMA --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e dspRMS --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e dspGain --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e dspMonitor --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e dspTag --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e dspAudio --report report.txt
	platin wcet --disable-ait -i dsp.pml -b dsp.elf -e main --report report.txt

clean: 
	rm -f *.out *.pml *.elf *.txt
