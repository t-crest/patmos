# Benchmarking the SSPM

This application is used for the evaluation section of following paper:

A Shared Scratchpad Memory with Synchronization Support (need to be changed),
submitted to the special issue of NorCAS 2017.

For the general build instructions of T-CREST please look into the
[Main README](../../../README.md).

The C programs for the evaluation can be found in the current folder.

For the multicore configuration the build and compile process is split
into two sub-projects in folders `t-crest/aegean` and `t-crest/patmos`.
In `aegean` the multicore hardware is built and the FPGA configured with it.
In `patmos` the C program is compiled and downloaded.
Best open two terminal windows, each for one project.

*Martin: are we doing the 4-core or the 9-core version?*

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
make app download APP=sspm MAIN=sspm_write_bench
```

The source code for all benchmarks can be found in `c/apps/sspm`.
The main file is selected with the variable `MAIN`.
Following targets are available:

```bash
noc_roundtrip_bench.c
noc_write_bench.c
noc_multi_channel_bench.c
sspm_locking_bench.c
sspm_roundtrip_bench.c
sspm_write_bench.c
sspm_write_with_lock_contention_bench.c
sspm_multi_channel_bench.c
```

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-02-20" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos` and `aegean`, as we plan to restructure the generation of the
multicore version of Patmos/T-CREST in the near future where the above described
steps to build a multicore may be different.

