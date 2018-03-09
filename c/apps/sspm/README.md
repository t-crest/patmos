# Shared Scratchpad Memory with Synchronization Support

This source code and the example applications are used for the evaluation section of following paper:

Henrik Enggaard Hansen, Emad Jacob Maroun, Andreas Toftegaard Kristensen, Jimmi Marquart, and Martin Schoeberl.
Time-predictable Synchronization Support with a Shared Scratchpad Memory.
submitted to the special issue for NorCAS in MICPRO.

## Build Instructions

For the general build instructions of T-CREST please look into the
[Main README](../../../README.md).

For the multicore configuration the build and compile process is split
into two sub-projects in folders `t-crest/aegean` and `t-crest/patmos`.
In `aegean` the multicore hardware is built and the FPGA configured with it.
In `patmos` the C program is compiled and downloaded.
Best open two terminal windows, each for one project.

To build the 9-core version for the DE2-115 FPGA board first create a
`config.mk` in `t-crest/aegean` and put following line into it
```
AEGEAN_PLATFORM?=altde2-115-9core
```
to select the 9-core platform then execute
```bash
make platform synth
```

After building the multicore, the FPGA can be configured with:
```bash
make config
```

## Benchmarking

The C programs for the evaluation can be found in the current folder.

The benchmark application is build and downloaded from within `t-crest/patmos`:
```bash
make app download APP=sspm MAIN=sspm_write_bench
```

The source code for all benchmarks can be found in `c/apps/sspm`.
The main file is selected with the variable `MAIN`.
Following targets are available:

```bash
noc_multi_channel_bench.c
noc_parallel_channel_bench.c
noc_roundtrip_bench.c
noc_write_bench.c
sspm_locking_bench.c
sspm_multi_channel_bench.c
sspm_parallel_channel_bench.c
sspm_roundtrip_bench.c
sspm_write_bench.c
sspm_write_with_lock_contention_bench.c
```

Note that for the parallel benchmarks from one sender to multiple receiver,
the Argo NoC scratchpad memory size of core 0 needs to be increased.

### Benchmark Description

* noc_write_bench.c: Benchmarks the execution time of writing to the Argo NoC local scratchpads.
* noc_roundtrip_bench.c: Benchmarks the execution time of sending a burst of data and waiting for an acknowledgement using the Argo NoC.
* sspm_write_bench.c: Benchmarks the execution time of writing to the SSPM.
* sspm_roundtrip_bench.c: Benchmarks the execution time of sending a burst of data and waiting for an acknowledgement using the SSPM.
* sspm_locking_bench.c: Benchmarks the execution time of acquiring a lock using extended time slots while other cores also use extended time slots.
* sspm_write_with_lock_contenttion_bench.c: Benchmarks the execution time of writing to the shared scratchpad memory while other cores use extended time slots.

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-02-20" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos` and `aegean`, as we plan to restructure the generation of the
multicore version of Patmos/T-CREST in the near future where the above described
steps to build a multicore may be different.

## Hardware Setup and Emulation

The shared scratchpad memory is a mechanism for centralized memory sharing on a multiprocessor platform.
Consequently this project is only useful when used on a multiprocessor platform, but it is possible to test some aspects using a single core Patmos instance, for example, using the emulator.

### Setup

Setup the T-CREST platform according to
the [Patmos Handbook](http://patmos.compute.dtu.dk/patmos_handbook.pdf)
Chapter 6 "Build instructions"

It is now possible to build, synthesize, and configure the entire project and use the shared scratchpad memory.

When using the emulator, use the board configuration file `altde2-115-sspm.xml`.

### Configuration

A number of settings are available on the SSPM:

 - Extended slot size: Change how many cycles are allocated for extended slots
 - Single extended slot mode: Limit extended slots to only occur once per round In general terms, this gives better read/write performance at the cost of potentially longer delays associated with locking.

When using the SSPM as a device on the emulator the above is set in the configuration file as:

    <param name="extendedSlotSize" value="5" />
    <param name="singleExtendedSlot" value="false" />

When instantiating the SSPM through the Scala built system, the following calling convention is used:

    $(SBT) "runMain sspm.SSPMAegeanMain $(CORE_CNT) $(EXT_SLOT_SIZE) $(SINGLE_EXT_SLOT)"

To change the configuration in the multicore setup, those constants are set in
the source in object SSPMAegeanMain (SSPMAegean.scala

### Reading the Code

The hardware description is mostly located in `https://github.com/t-crest/patmos/tree/master/hardware/src/main/scala/sspm`.

 - `SSPMAegean.scala` is the arbiter and core of the implementation
 - `SSPMConnector.scala` is the core interface
 - `memSPM.scala` is the memory

### Running Hardware Tests

A number of hardware test benches are included to confirm the behavior of the implementation.

To use these, navigate to the `hardware` folder of `patmos` and execute:

 - `make SSPMAegean-test` for a test of the overall behavior of the implementation
 - `make SSPMConnector-test` for a test of the connector's behavior

Both tests export `.vcd` files in the `generated` folder. These files can be opened in a waveform viewer.


