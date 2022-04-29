# SHA-256 Benchmarks

These benchmarks are used for the evaluation section of the following papers:</br>
[C. Pircher, A. Baranyai, C. Lehr and M. Schoeberl, "Accelerator Interface for Patmos"](https://ieeexplore.ieee.org/document/9599856), IEEE NorCAS 2021.</br>
C. Pircher, A. Baranyai, C. Lehr and M. Schoeberl, "Coprocessor Interface for the Patmos Processor", submitted to Elsevier Microprocessors and Microsystems 2022.

Results listed in the paper can be found in the `*.stdout.ref` files.
The Chisel sources for the SHA-256 IO device and coprocessor can be found under [io/Sha256.scala](../../../hardware/src/main/scala/io/sha256.scala) and [cop/Sha256.scala](../../../hardware/src/main/scala/cop/sha256.scala) respectively.
All benchmark source files are contained within this folder.
Before building the Patmos processor and running the benchmarks, please make sure that you have successfully built T-CREST (see [Main README](../../../README.md)).

## Patmos Configuration
To build Patmos with the SHA-256 IO device, add the following lines to the [altde2-115.xml](../../../hardware/config/altde2-115.xml) in the sections `<IOs>` and `<Devs>` respectively:
```
    <IO DevTypeRef="Sha256" offset="11"/>
```
```
    <Dev DevType="Sha256" entity="Sha256" iface="OcpCore"/>
```

In order to build with the SHA-256 coprocessor (and round-robin arbiter), add the following lines instead:
```
  <bus roundRobinArbiter="true" />
  
  <Coprocessors>
    <Coprocessor Name="Sha256" CoprocessorID="0" requiresMemoryAccess="True" />
  </Coprocessors>
```

You can now build the Patmos emulator or synthesize Patmos for FPGA as required (e.g. by calling `make emulator` or `make synth` in the [patmos](../../../) directory).

**Note:** you can also add both units to the design but the presence of the arbiter may slow down the software and IO implementations!

## Benchmarking
Compilation of the benchmarks can be achieved by calling `make compile`.
This will generate the files `sw_benchmark.elf`, `io_benchmark.elf` and `cop_benchmark.elf`.
The Makefile also provides targets for benchmarking with the Patmos emulator.
In order to run all three benchmarks you can call `make benchmark`.
The results will be written to the files `sw.stdout`, `io.stdout` and `cop.stdout`.

If you want to run a benchmark on an FPGA you can use the `patserdow` utility (e.g. `patserdow /dev/ttyUSB0 c/apps/sha256-bench/cop_benchmark.elf`).
Be sure to configure the FPGA in advance (e.g. by calling `make config` in the [patmos](../../../) directory).
