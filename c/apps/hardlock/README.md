# Hardware Locking

This application is used for the evaluation section of following submitted paper:

Torur and Martin, xxxx, submitted to ISORC 2018.

For the general build instructions of T-CREST please look into the
[Main README](../../../README.md).

The C program for the evaluation can be found in [abc.c](abc.c).

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
make app download APP=hardlock
```

To measure the single core version, disable any NoC related code in the
benchmark and configure the FPGA from the `t-crest/patmos` folder:
```bash
make app config download APP=hardlock
```

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-mm-dd" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos`.

We plan to restructure the generation of the
multicore version of Patmos/T-CREST in the near future where the above described
steps to build a multicore may be different. TBD

