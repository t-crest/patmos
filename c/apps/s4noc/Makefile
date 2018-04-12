MAIN?=hello_s4noc

all:
	patmos-clang -O2 $(MAIN).c -I ../.. -I ../../include ../../libcorethread/*.c -o $(APP).elf

clean:
	rm *.elf

test:
	cd ../../../hardware; sbt "test:runMain s4noc.S4nocTester"

test-all:
	cd ../../../hardware; sbt "test:runMain s4noc.ScheduleTester"
	cd ../../../hardware; sbt "test:runMain s4noc.RouterTester"
	cd ../../../hardware; sbt "test:runMain s4noc.NetworkTester"
	cd ../../../hardware; sbt "test:runMain s4noc.NetworkCompare"
