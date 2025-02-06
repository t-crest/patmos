.PHONY: all clean wcet
all:
	patmos-clang -O2 adas.c -o adas.elf 

wcet:
	patmos-clang -mserialize-pml=adas.pml -DWCET -O2 adas.c -o adas.elf 
	platin wcet --disable-ait -i adas.pml -b adas.elf -e createSignal --report report.txt
	platin wcet --disable-ait -i adas.pml -b adas.elf -e zipWithProcessor --report report.txt
	platin wcet --disable-ait -i adas.pml -b adas.elf -e brake --report report.txt
	platin wcet --disable-ait -i adas.pml -b adas.elf -e display --report report.txt
	platin wcet --disable-ait -i adas.pml -b adas.elf -e main --report report.txt

clean: 
	rm -f *.out *.pml *.elf *.txt
