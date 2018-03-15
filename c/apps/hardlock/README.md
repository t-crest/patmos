# Hardlock Tests

These applications are used for the evaluation section of the following submitted paper:

Tórur and Martin, Hardlock: a Concurrent Real-Time Multicore
Locking Unit, submitted to ISORC 2018.

If the T-CREST platform is not built, please first follow the general build 
instructions of T-CREST in [Main README](../../../README.md).

Before running tests, add the following lines after `<frequency Hz="80000000"/>` in 
[altde2-115.xml](../../../hardware/config/altde2-115.xml):
```
<cores count="8" />
<pipeline dual="false" />
```
If you wish to run a test with a lower core count, simply update the line.
Note, that your JVM might run out of heap space when creating 8 Patmos
cores. To increase the heap space, run the following in your shell:
```
export _JAVA_OPTIONS=-Xmx4096m
```

The C programs for the Hardlock tests are found at 
[hardlock_nocontention_test.c](hardlock_nocontention_test.c),
[hardlock_contention_test.c](hardlock_contention_test.c),
and
[sspm_contention_test.c](sspm_nocontention_test.c)

To run the tests on a DE2-115 board, first connect it, 
and then from `t-crest/patmos` run 
```bash
make comp gen synth
```
This creates Patmos. To configure the FPGA with Patmos, run:
```bash
make config
```
Afterwards run:
```bash
make download app APP=hardlock MAIN=hardlock_nocontention_test
```
This compiles and downloads the program that tests the Hardlock without contention. 
Change `MAIN` to the appropriate test.

If you wish to run [sspm_contention_test.c](sspm_nocontention_test.c)
it is necessary to run it on Aegean. From `t-crest/aegean` run:
```bash
make platform synth
```
This creates Aegean. To configure the FPGA with Aegean, run: 
```bash
make config
```
Finally, to download the application, go to `t-crest/patmos` and run: 
```bash
make download app APP=hardlock MAIN=sspm_contention_test
```

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use the following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-02-14" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos`.

We plan to restructure the generation of Aegean in the near future 
where the above described steps to build a multicore may be different.

