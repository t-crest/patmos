# Benchmarking T-CREST

This application is used for the evaluation section of following submitted paper:

Martin Schoeberl, Design of a Time-predictable Multicore Processor:
The T-CREST Project, submitted to DATE 2018.

For the general build instructions of T-CREST please look into the
[Main README](../../../README.md).

The C program for the evaluation can be found in [bench.c](bench.c).

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

The benchmark application is build and downloaded from within `t-crest/patmos`:
```bash
make app download APP=bench
```

To measure the single core version, disable any NoC related code in the
benchmark and configure the FPGA from the `t-crest/patmos` folder:
```bash
make app config download APP=bench
```

