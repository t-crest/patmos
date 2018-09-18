# This Makefile is used from the main Patmos folder to run all measurements
rm log.txt
#for pipeline in "" "-D _PIPELINE"; \
for pipeline in ""; \
do \
#	for cpucnt in 2 3 4 5 6 7 8; \
	for cpucnt in 2 3; \
	do \
		for bufsize in 4 8 16 32 64 128; \
		do \
			#for dev in "-D _MAINMEM" "-D _SSPM" "-D _OWN -D _OWNMAINMEM" "-D _OWN" "-D _SPMPOOL"; \
			for dev in "-D _MAINMEM" "-D _SSPM" "-D _OWN" "-D _SPMPOOL"; \
			do \
				make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$bufsize -D MAX_CPU_CNT=$cpucnt $dev $pipeline"; \
				patemu tmp/ownspm.elf >> log.txt; \
				#Comment the line above and uncomment the two lines below to use the FPGA
				#make config
				#make APP=ownspm download >> log.txt; \
			done
		done
	done
done
grep measure log.txt > results.txt
cat results.txt
