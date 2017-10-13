# Benchmarking T-CREST

This application is used to provide the evaluation for the submitted paper:

Martin Schoeberl, Design of a Time-predictable Multicore Processor:
The T-CREST Project, submitted to DATE 2018.

For the general build instructions of T-CREST please look into the
[Main README](../../../README.md).

To build the 9-core version for the DE2-115 FPGA board execute in t-crest/aegean:
```bash
make platform synth
```

After building the multicore, the FPGA can be configured with:
```bash
make config
```

The benchmark application is build and downloaded from within t-crest/patmos:
```bash
make app download APP=bench
```