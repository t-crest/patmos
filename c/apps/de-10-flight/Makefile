PATMOSHOME=~/t-crest/patmos
SERIAL?=/dev/ttyS4
BAUD_RATE?=115200
# LIBCORETHREAD=$(PATMOSHOME)/c/libcorethread
# LIBNOC=$(PATMOSHOME)/c/libnoc
# LIBMP=$(PATMOSHOME)/c/libmp
# NOCINIT?=$(PATMOSHOME)/c/cmp/nocinit.c

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c/include	\
	-I $(PATMOSHOME)/c	\
	-I $(PATMOSHOME)/c/libelf/ \
	-mpatmos-disable-vliw \
	$(DEFINES)


wcet:
	mkdir -p output
	
	patmos-clang -O2 $(CFLAGS) hello_i2c.c bmi055.c bmi055_accel.c bmi055_gyro.c i2c.c -mserialize=tpip.pml -D WCET -o de-10-flight_wcet.elf
	platin wcet --disable-ait -i tpip.pml -b de-10-flight_wcet.elf -e atan2_fp --report output/report_atan.txt
	platin wcet --disable-ait -i tpip.pml -b de-10-flight_wcet.elf -e calc_pith --report output/report_pitch.txt
	platin wcet --disable-ait -i tpip.pml -b de-10-flight_wcet.elf -e calc_roll --report output/report_roll.txt


accel_demo:
	patmos-clang -O2 $(CFLAGS) hello_i2c.c bmi055.c bmi055_accel.c bmi055_gyro.c i2c.c -o de-10-flight.elf

download:
	patserdow -v $(SERIAL) de-10-flight.elf $(BAUD_RATE)

clean:
	rm -f *.out *.pcap *.pml *.png *.elf
