# The S4NOC

These applications are used for the evaluation section of the following paper:

<!---
Martin Schoeberl, Luca Pezzarossa, and Jens Sparso,
--->
*Blind*, A Simple Network Interface for a Simple Network-on-Chip, *submitted to DATE 2019*

## Stand Alone Evaluation

The network interface and the S4NOC are written in Chisel and the
source can be found at: `patmos/hardware/src/main/scala/s4noc`.

The tests can run from within folder `patmos/hardware`, e.g.:

	sbt "test:runMain s4noc.ScheduleTester"
	sbt "test:runMain s4noc.RouterTester"
	sbt "test:runMain s4noc.NetworkTester"
	sbt "test:runMain s4noc.NetworkCompare"
	sbt "test:runMain s4noc.S4nocTester"

or from your favorite Scala IDE (e.g., InelliJ or Eclipse) or from this folder with

```bash
make test-all
make test
```

A standalone version of the S4NoC with simple traffic generators can be built
with:

```bash
sbt "runMain s4noc.S4nocTrafficGen n"
```

where n is the number of cores (e.g., 4, 9, or 16).

The generated Verilog file can be found in ```generated/S4nocTrafficGen.v```
and can be synthesized to provide resource numbers and maximum
clocking frequency. An example project for Quartus can be found in this
[quartus](quartus) subfolder.

## Evaluation with T-CREST

We use the T-CREST multicore to evaluate the network interface with the S4NOC.
General build instructions of T-CREST in [Main README](../../../README.md).

Before building the Patmos processor, add the following lines after `<frequency Hz="80000000"/>` in 
[altde2-115.xml](../../../hardware/config/altde2-115.xml):
```
<cores count="4" />
<cmp device="7" />
<pipeline dual="false" />
```

A simple C program for a first test are found at 
[hello_s4noc.c](hello_s4noc.c)

### Evaluation with the Emulator

Build the emulator with:
```bash
make emulator
```
Build the test application with:
```bash
make app APP=s4noc
```

Execute with the emulator with:
```bash
patemu tmp/s4noc.elf
```

### Evaluation with the FPGA

To run the tests on a DE2-115 board, first connect it, 
and then from `t-crest/patmos` run 
```bash
make gen synth
```
This creates Patmos. To configure the FPGA with Patmos, run:
```bash
make config
```
Afterwards run:
```bash
make app download APP=s4noc 
```
This compiles and downloads a simple test for the S4NOC"
Change `MAIN` to the appropriate test.

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use the following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-09-19" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos`.

### Running out of Heap

It can happen when many cores are constructed the JVM runs out of heap.
Increase the possible heap size with:
```bash
export _JAVA_OPTIONS=-Xmx4096m
```
