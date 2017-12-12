Building and testing the shared scratchpad memory
=====================================

The shared scratchpad memory is a mechanism for centralized memory sharing on a multiprocessor platform.
Consequently this project is only useful when used on a multiprocessor platform, but it is possible to test some aspects using a single core patmos instance -- for example, using the emulator.

Setup
--------

 1. Setup the T-CREST platform according to [the Patmos Handbook] chapter 6 "Build instructions", but with minor modifications
    1. In the 'aegean' project, checkout the 'sspm' branch of `https://github.com/A-T-Kristensen/aegean/tree/SSPM`
    2. In the 'patmos' project, checkout the 'SSPM-device' branch of `https://github.com/henrikh/patmos/tree/SSPM-device`

It is now possible to build, synthesize and configure the entire project and use the shared scratchpad memory.

An alternative arbiter for the shared scratchpad is also available in the branch 'singleSyncSlot' of `https://github.com/henrikh/patmos/tree/SSPM-device`.
This arbiter gives, in general terms, better read/write performance at the cost of potentially longer delays associated with locking.

Reading the code
-------------------------

The hardware description is mostly located in `https://github.com/henrikh/patmos/tree/SSPM-device/hardware/src/sspm`.

 - `SSPMAegean.scala` is the arbiter and core of the implementation
 - `SSPMConnector.scala` is the core interface
 - `memSPM.scala` is the memory

Running hardware test benches
--------------------------------------------

A number of hardware test benches are included to confirm the behavior of the implementation.

To use these, navigate to the `hardware` folder of `patmos` and execute:

 - `make SSPMAegean-test` for a test of the overall behavior of the implementation
 - `make SSPMConnector-test` for a test of the connector's behavior

Both tests export `.vcd` files in the `generated` folder. These files can be opened in a waveform viewer.

Running software benchmarks
--------------------------------------------

Several programs for benchmarking the real-world performance are available in `patmos/c/libsspm/bencharks`:
* noc_write_bench.c: Benchmarks the execution time of writing to the Argo NoC local scratchpads.
* noc_roundtrip_bench.c: Benchmarks the execution time of sending a burst of data and waiting for an acknowledgement using the Argo NoC.
* sspm_write_bench.c: Benchmarks the execution time of writing to the SSPM.
* sspm_roundtrip_bench.c: Benchmarks the execution time of sending a burst of data and waiting for an acknowledgement using the SSPM.
* sspm_locking_bench.c: Benchmarks the execution time of acquiring a lock using extended time slots while other cores also use extended time slots.
* sspm_write_with_lock_contenttion_bench.c: Benchmarks the execution time of writing to the shared scratchpad memory while other cores use extended time slots.



[the Patmos Handbook]: http://patmos.compute.dtu.dk/patmos_handbook.pdf
