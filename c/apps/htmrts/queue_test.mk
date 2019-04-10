# This Makefile is used from the main Patmos folder to run all queue test measurements
rm log.txt
for dev in "-D _HARDLOCK_" "-D _CAS_" "-D _HTMRTS_"; \
do \
	for cpucnt in 2 4 6 8; \
	do \
		for elms in 10 20 40 60; \
		do \
			make app APP=htmrts MAIN=queue_test COPTS="$dev -D MAX_CPU_CNT=$cpucnt -D ELEMENTS_PER_CORE=$elms -D ITERATIONS=$iter"; \
			patemu tmp/htmrts.elf >> log.txt; \
			#Comment the line above and uncomment the two lines below to use the FPGA
			#make config
			#make APP=htmrts download >> log.txt; \
		done
	done
done
grep measure log.txt > results.txt
cat results.txt
