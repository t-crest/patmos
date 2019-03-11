# Scratchpad Memories with Ownership

These applications are used for the evaluation section of the following paper:

Martin Schoeberl, Torur Biskopsto Strom, Oktay Baris, and Jens Sparso,
*Scratchpad Memories with Ownership*, DATE 2019.


We use the T-CREST multicore to evaluate the shared memory with ownership.
General build instructions of T-CREST in [Main README](../../../README.md).

Before building the Patmos processor (hardware or emulator) add the following lines after `<frequency Hz="80000000"/>` in 
[altde2-115.xml](../../../hardware/config/altde2-115.xml):
```
<cores count="4" />
<pipeline dual="false" />
  <CmpDevs>
		<CmpDev name="SharedSPM" />
		<CmpDev name="OwnSPM" />
		<CmpDev name="SPMPool" />
  </CmpDevs>
```

The number of SPMs with ownership
is currently configured to be the same as the number of processor cores.
This can be changed in `Patmos.scala`. Note that `pc.c` requires
the number of SPMs to be (number of cores - 1)*2.
The individual SPMs are displaced every 4 KB.

The C programs for the tests are found here, e.g., for the shared SPM: 
[hello_spm.c](hello_spm.c)

For other test programs set the variable `MAIN` at your compile make call.
Following test programs are available:

 * `hello_spm.c` does simple checks on a shared SPM
 * `timing.c` measures access times (using the deadline device with random delays)
 * `single_owner.c` does a multicore test on a single SPM with ownership
 * `test_owner.c` does a multicore test with two SPMs with ownership
 * `pc.c` the producer/consumer test program

For the evaluation section in the paper we have written a producer/consumer
based synthetic benchmark: `pc.c`. It can be configured for the different
versions of the SPM. The default version uses main memory for the buffers.
The configuration is via C based defines, which can also be passed to the
compiler with command line arguments. E.g., compilation for the shared SPM
is:

```bash
make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D _SSPM"
```

The experiments can be execute on the Patmos emulator or with the real
hardware on an FPGA board.

## Emulator Based Testing

*Note that the emulator currently supports only a maximum of 4 cores.*

To compile the emulator with the selected hardware configuration run

```bash
make emulator
```

Compile the application with

```bash
make app APP=ownspm 
```

If you want to use a different test program add the `MAIN` variable, such as

```bash
make app APP=ownspm MAIN=single_owner 
```

Both make commands lead to an `.elf` file in the `tmp` folder and can be
executed with the emulator as follows:

```bash
patemu tmp/ownspm.elf
```

## FPGA Based Testing

To run the tests on a DE2-115 board, first connect it, 
and then from `t-crest/patmos` run 
```bash
make gen synth
```
This creates Patmos. To configure the FPGA with Patmos run
```bash
make config
```
Afterwards run:
```bash
make app download APP=ownspm 
```

This compiles and downloads a simple test for the one-way shared memory.
Change `MAIN` to the appropriate test.


## The Version of the Submitted Paper

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use the following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-09-19" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos`.
