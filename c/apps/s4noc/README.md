# The S4NOC

These applications are used to explore the usage of S4NOC with Patmos.

If you are looking for the initial evaluation of the S4NOC, please refer to the [s4noc-2019](../s4noc-2019/README.md) folder.

We use the T-CREST multicore to evaluate the S4NOC.
General build instructions of T-CREST can be found in the [Main README](../../../README.md).

Before building the Patmos processor, add the following lines after `<frequency Hz="80000000"/>` in 
[altde2-115.xml](../../../hardware/config/altde2-115.xml):
```
<cores count="4" />
<pipeline dual="false" />

<CmpDevs>
  <CmpDev name="S4NoC" />
</CmpDevs>
```

with cores count either 4 or 9. Use just 4 for running the emulator.
This configuration might be default as the writing of this README.

A simple C program for a first test can be found at 
[hello_noc.c](hello_noc.c)

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
patemu tmp/s4nocx.elf
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

### Running out of Heap

It can happen when many cores are constructed the JVM runs out of heap.
Increase the possible heap size with:
```bash
export _JAVA_OPTIONS=-Xmx4096m
```
