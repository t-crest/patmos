# This Makefile is used from the main Patmos folder to run all stack test measurements
rm log.txt
for dev in "-D _CAS_" "-D _HTMRTS_" "-D _HARDLOCK_"; \
do \
	for cpucnt in 2 4 6 8; \
	do \
		#for elms in 1 2 4 8 16; \
		for elms in 2; \
		do \
			for iter in 1 10 100 1000; \
			do \
				make app APP=htmrts MAIN=stack_test COPTS="$dev -D MAX_CPU_CNT=$cpucnt -D ELEMENTS_PER_CORE=$elms -D ITERATIONS=$iter"; \
				patemu tmp/htmrts.elf >> log.txt; \
				#Comment the line above and uncomment the two lines below to use the FPGA
				#make config
				#make APP=htmrts download >> log.txt; \
			done
		done
	done
done
grep measure log.txt > results.txt
cat results.txt
