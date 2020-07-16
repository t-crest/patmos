PATMOSHOME=~/t-crest/patmos
SERIAL?=/dev/ttyUSB0
LIBETH=$(PATMOSHOME)/c/ethlib

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c/include	\
	-I $(PATMOSHOME)/c	\
	-I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBETH) \
	-mpatmos-disable-vliw \
	$(DEFINES)

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

# WCET analysis

wcet_tte_sensor:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c common.c i2c.c tteconfig.c mpu9250.c tte_sensor.c -mserialize=tte_sensor.pml -o tte_sensor_wcet.elf
	platin wcet --disable-ait --enable-wca -i tte_sensor.pml -b tte_sensor_wcet.elf -e task_sync
	platin wcet --disable-ait --enable-wca -i tte_sensor.pml -b tte_sensor_wcet.elf -e task_sense
	platin wcet --disable-ait --enable-wca -i tte_sensor.pml -b tte_sensor_wcet.elf -e task_send
	platin wcet --disable-ait --enable-wca -i tte_sensor.pml -b tte_sensor_wcet.elf -e sort_asc_ttetasks
	platin wcet --disable-ait --enable-wca -i tte_sensor.pml -b tte_sensor_wcet.elf -e cyclic_executive_loop
	platin wcet --disable-ait --enable-wca -i tte_sensor.pml -b tte_sensor_wcet.elf -e get_tte_aligned_time

wcet_tte_control:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c common.c i2c.c tteconfig.c tte_control.c -mserialize=tte_control.pml -o tte_control_wcet.elf
	platin wcet --disable-ait --enable-wca -i tte_control.pml -b tte_control_wcet.elf -e task_sync
	platin wcet --disable-ait --enable-wca -i tte_control.pml -b tte_control_wcet.elf -e task_recv
	platin wcet --disable-ait --enable-wca -i tte_control.pml -b tte_control_wcet.elf -e task_send
	platin wcet --disable-ait --enable-wca -i tte_control.pml -b tte_control_wcet.elf -e task_calc
	platin wcet --disable-ait --enable-wca -i tte_control.pml -b tte_control_wcet.elf -e sort_asc_ttetasks
	platin wcet --disable-ait --enable-wca -i tte_control.pml -b tte_control_wcet.elf -e cyclic_executive_loop
	platin wcet --disable-ait --enable-wca -i tte_control.pml -b tte_control_wcet.elf -e get_tte_aligned_time

wcet_tte_actuator:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c common.c i2c.c tteconfig.c tte_actuator.c -mserialize=tte_actuator.pml -o tte_actuator_wcet.elf
	platin wcet --disable-ait --enable-wca -i tte_actuator.pml -b tte_actuator_wcet.elf -e task_sync
	platin wcet --disable-ait --enable-wca -i tte_actuator.pml -b tte_actuator_wcet.elf -e task_recv
	platin wcet --disable-ait --enable-wca -i tte_actuator.pml -b tte_actuator_wcet.elf -e task_pulse
	platin wcet --disable-ait --enable-wca -i tte_actuator.pml -b tte_actuator_wcet.elf -e sort_asc_ttetasks
	platin wcet --disable-ait --enable-wca -i tte_actuator.pml -b tte_actuator_wcet.elf -e cyclic_executive_loop
	platin wcet --disable-ait --enable-wca -i tte_actuator.pml -b tte_actuator_wcet.elf -e get_tte_aligned_time

# running examples

tte_sensor:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c common.c i2c.c tteconfig.c mpu9250.c tte_sensor.c -o ttecps.elf

tte_control:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c common.c i2c.c tteconfig.c tte_control.c -o ttecps.elf

tte_actuator:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c common.c i2c.c tteconfig.c tte_actuator.c -o ttecps.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) ttecps.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf
