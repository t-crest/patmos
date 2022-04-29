# SoundFX application

This application is a demonstration of the provided SoundFX coprocessor in comparison to a purely software-based implementation.
It is used for the evaluation section of following submitted paper:<br/>
C. Pircher, A. Baranyai, C. Lehr and M. Schoeberl, "Coprocessor Interface for the Patmos Processor", submitted to Elsevier Microprocessors and Microsystems 2022.

The Chisel sources for the SoundFX coprocessor can be found under [cop/SoundFX.scala](../../../hardware/src/main/scala/cop/SoundFX.scala).
The implementation is based on building blocks provided by [soundbytes](https://github.com/schoeberl/soundbytes) or modifications thereof which can be found in [soundbytes](../../../hardware/src/main/scala/soundbytes)
Before building the Patmos processor and running the benchmarks, please make sure that you have successfully built T-CREST (see [Main README](../../../README.md)).

## Patmos Configuration
To add the AudioInterface to Patmos, add the following lines to the [altde2-115.xml](../../../hardware/config/altde2-115.xml) in the sections `<IOs>` and `<Devs>` respectively:
```
    <IO DevTypeRef="AudioInterface" offset="12"/>
```
```
    <Dev DevType="AudioInterface" entity="AudioInterface" iface="OcpCore">
      <params>
        <param name="audioLength"	value="16"/>  <!-- either 16, 20, 24 or 32 -->
        <param name="audioFsDivider"	value="256"/> <!-- depending on chosen Fs (see datasheet) -->
        <param name="audioClkDivider"	value="6"/>   <!-- for the bclk/xlk: patmosclk / audioClkDivider-->
        <param name="maxDacBufferPower" value="8"/>  <!-- for a maximum DAC buffer of 2^8 = 256 -->
        <param name="maxAdcBufferPower" value="8"/>  <!-- for a maximum ADC buffer of 2^8 = 256 -->
      </params>
    </Dev>
```
You must also add the corresponding top-level signals and pin assignments.
Also make sure to add a tristate buffer for the I²C dataline!

In order to add the SoundFX coprocessor (and round-robin arbiter), add the following lines to the configuration file:
```
  <bus roundRobinArbiter="true" />
  
  <Coprocessors>
    <Coprocessor Name="SoundFX" CoprocessorID="0" requiresMemoryAccess="True" />
  </Coprocessors>
```

You can now build the Patmos emulator or synthesize Patmos for FPGA as required (e.g. by calling `make emulator` or `make synth` in the [patmos](../../../) directory).

