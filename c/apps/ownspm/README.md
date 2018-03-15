# Shared Memory with Ownership

These applications are used for the evaluation section of the following submitted paper:

Scratchpad Memories with Ownership, submitted to CASES


We use the T-CREST multicore to evaluate the shared memory with ownership.
General build instructions of T-CREST in [Main README](../../../README.md).

Before building the Patmos processor (hardware or emulator) add the following lines after `<frequency Hz="80000000"/>` in 
[altde2-115.xml](../../../hardware/config/altde2-115.xml):
```
<cores count="4" />
<cmp device="n" />
<pipeline dual="false" />
```

where the cmp device is either `1` for the TDM arbitrated shared SPM or
`5` for the SPMs with ownership.

The C programs for the tests are found here, e.g., for the shared SPM: 
[hello_spm.c](hello_spm.c)

For other test programs set the variable `MAIN` at your compile make call.

The experiments can be execute on the Patmos emulator of with the real
hardware on an FPGA board.

## Emulator Based Testing

To compile the emulator with the selected hardware configuration run:

```bash
make emulator
```

Compile the application with

Afterwards run:
```bash
make app APP=ownspm 
```

If you want to use a different test program add the `MAIN` variable, such as:

```bash
make app APP=ownspm MAIN=single_owner 
```

Both make commands lead to an `.elf` file in the `tmp` folder and can be
executed with the emulator as follows:

```bash
patemu tmp/ownspm.elf
```

## FPGA Based Tesing

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
make app download APP=ownspm 
```

This compiles and downloads a simple test for the one-way shared memory.
Change `MAIN` to the appropriate test.

`hello_spm.c` does simple checks on a shared SPM
`timing.c` measures access times (with shifts with the deadline device)
`single_owner.c` does a multicore test on a single SPM with ownership
`test_owner.c` does a multicore test with two SPMs with ownership

## The Version of the Submitted Paper

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use the following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-xx-xx" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos`.
