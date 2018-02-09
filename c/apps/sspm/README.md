# Benchmarking T-CREST

These applications are used for the evaluation section of following submitted paper:

A Shared Scratchpad Memory with Synchronization Support

For the general build instructions of T-CREST please look into the
[Main README](../../../README.md).

The C program for the evaluations can be found in:

* noc_write_bench.c: Benchmarks how many cycles writing to the Argo NOC takes.
* noc_roundtrip_bench.c: Benchmakrs how many cycles a roundtrip of a message takes using the Argo NOC.
* sspm_write_bench.c: Benchmarks how many cycles writing to the SSPM takes.
* sspm_roundtrip_bench.c: Benchmarks how many cycles a roundtrip of massage takes using the SSPM.
* sspm_locking_bench.c: Benchmarks how many cycles it takes to get a lock while other cores are also trying to get a lock. I.e. synchronization under synchronization traffik.
* sspm_write_with_lock_contention_bench.c: Benchmarks how many cycles it takes to write to the SSPM while other cores are trying to get a lock. I.e. write under synchronization traffik.

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
make app APP=sspm MAIN=example download
```

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2017-10-17" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos` and `aegean`, as we plan to restructure the generation of the
multicore version of Patmos/T-CREST in the near future where the above described
steps to build a multicore may be different.

