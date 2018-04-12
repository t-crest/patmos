# The S4NOC

These applications are used for the evaluation section of the following submitted paper:

Martin, Luca, and Jens, A Simple Network Interface for a Simple Network-on-Chip,
submitted to NOCS 2018.

## Stand Alone Evalution

The network interface and the S4NOC are written in Chisel and the
source can be found at: `patmos/hardware/src/main/scala/s4noc`.

The tests can run from within folder `patmos/hardware`, e.g.:

	sbt "test:runMain s4noc.ScheduleTester"
	sbt "test:runMain s4noc.RouterTester"
	sbt "test:runMain s4noc.NetworkTester"
	sbt "test:runMain s4noc.NetworkCompare"
	sbt "test:runMain oneway.OneWayMemTester"

or from your favorite Scala IDE (e.g., InelliJ or Eclipse) or from this folder with

```bash
make test-all
```

## Evaluation with T-CREST

We use the T-CREST multicore to evaluate the one-way shared memory.
General build instructions of T-CREST in [Main README](../../../README.md).

Before building the Patmos processor,  tests, add the following lines after `<frequency Hz="80000000"/>` in 
[altde2-115.xml](../../../hardware/config/altde2-115.xml):
```
<cores count="4" />
<cmp device="7" />
<pipeline dual="false" />
```

The C programs for the tests are found at 
[hello_s4noc.c](hello_s4noc.c)

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
Change `MAIN` to the appropriate test.

To ensure that you have the exact version of T-CREST that we have used in the
evaluation section of the paper, use the following `git` command to checkout that version:

```bash
git checkout `git rev-list -n 1 --before="2018-05-02" master`
```

This can be done in all T-CREST repositories. However, it is most important
in `patmos`.
