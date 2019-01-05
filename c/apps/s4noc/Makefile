MAIN?=hello_s4noc

# various parameters can be set via COPTS and $defines for the compilation, e.g.,:
# make app APP=s4noc MAIN=prodcons_flow COPTS="-D BUF_LEN=8 -D NR_CREDITS=4"

OPT=-mpatmos-max-subfunction-size=4096 -mpatmos-preferred-subfunction-size=1024
all:
	patmos-clang -O2 $(MAIN).c -I ../.. -I ../../include ../../libcorethread/*.c $(OPT) $(COPTS) -o s4noc.elf


clean:
	rm *.elf

test-bubble:
	cd ../../../hardware; sbt "runMain s4noc.FifoTester"

test:
	cd ../../../hardware; sbt "test:runMain s4noc.S4nocTester"

test-all:
	cd ../../../hardware; sbt "test:runMain s4noc.ScheduleTester"
	cd ../../../hardware; sbt "test:runMain s4noc.RouterTester"
	cd ../../../hardware; sbt "test:runMain s4noc.NetworkTester"
	cd ../../../hardware; sbt "test:runMain s4noc.NetworkCompare"
