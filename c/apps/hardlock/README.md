# Hardlock Tests

These applications are used for the evaluation section of the following submitted paper:

Tórur, Jens and Martin, Hardlock: Real-Time Multicore Locking, submitted to Journal of Systems Architecture 2018.

If the T-CREST platform is not built, please first follow the general build 
instructions of T-CREST in [Main README](../../../README.md).

Before running tests, add the following lines after `<frequency Hz="80000000"/>` in 
[altde2-115.xml](../../../hardware/config/altde2-115.xml):
```
<cores count="8" />
<pipeline dual="false" />
<CmpDevs>
	<CmpDev name="Hardlock" />
	<CmpDev name="CASPM" />
	<CmpDev name="AsyncLock" />
</CmpDevs>
```

If you wish to run a test with a lower core count, simply update the line.
Note, that your JVM might run out of heap space when creating 8 Patmos
cores. To increase the heap space, run the following in your shell:
```
export _JAVA_OPTIONS=-Xmx4096m
```

Also add 
```
set_global_assignment -name VERILOG_FILE ../../src/main/scala/cmp/AsyncMutex.v
set_global_assignment -name VERILOG_FILE ../../src/main/scala/cmp/AsyncArbiter.v
```
after `set_global_assignment -name VERILOG_FILE ../../build/Patmos.v` in
[patmos.qsf](../../../hardware/quartus/altde2-115/patmos.qsf)

The C programs for the tests are found at 
[nocontention_test.c](nocontention_test.c)
and
[contention_test.c](contention_test.c)

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
make download app APP=hardlock MAIN=nocontention_test COPTS="-D _HARDLOCK_"
```
This compiles and downloads the program that tests the Hardlock without contention. 
Change `MAIN` to the appropriate test. Change `_HARDLOCK_` to either `_ASYNCLOCK_`
or `_CASPM_` to use one of the other locking units. Add ` -D USE_PTHREAD_MUTEX`
after `_HARDLOCK_` to use the locking unit with the POSIX mutex.

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use the following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-09-26" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos`.
