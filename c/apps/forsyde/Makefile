EXAMPLE?=dsp

all: $(EXAMPLE)_comp 

wcet: $(EXAMPLE)_wcet

adas_comp:
	make -C adas
	cp adas/adas.elf forsyde.elf

adas_wcet: 
	make wcet -C adas 

adas_clean:
	make clean -C adas

dsp_comp:
	make -C dsp
	cp dsp/dsp.elf forsyde.elf

dsp_wcet: 
	make wcet -C dsp 

dsp_clean: 
	make clean -C dsp

clean: adas_clean dsp_clean
	rm -rf *.out *.pml *.elf *.txt	 
