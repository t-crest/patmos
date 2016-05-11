============
AUDIO INTERFACE FOR WM-8731 Audio Codec & PATMOS running on Altera altDE2-115
============

Added hardware modules:
============
hardware/src/io/AudioADC.scala
hardware/src/io/AudioDAC.scala
hardware/src/io/AudioClkGen.scala
hardware/src/io/AudioI2C.scala
hardware/src/io/AudioInterface.scala

Added C libraries and programs:
============
c/audio.c
c/audio.h
c/audio_delay.c
c/audio_inout.c
c/audio_i2c_test.c
c/audio_regrw_test.c
c/audio_sin.c

Added configuration files:
===========
hardware/config/altde2-115-audio.xml
hardware/quartus/altde2-115-audio/patmos.qsf
hardware/vhdl/patmos_de2-115-audio.vhdl

============
============
CAUTION!! folder of patmos.qsf file has changed. Might need to specify on Makefile
============
============

Useful commands:
============
Build patmos for FPGA:
>make BOARD=altde2-115-audio

Run a program on FPGA: (may also need to add: BOARD=altde2-115-audio)
>cd ..
>make -C patmos APP=__appName__ comp
>make -C patmos APP=__appName__ download

Simple simulation of a program (printings):
>cd .. (not sure of this)
>make comp APP=__appName__
>make emulator
>install/bin/emulator tmp/__appName__.elf

Waveform simulation of a program (gtkwave):
>cd .. (not sure of this)
>make comp APP=__appName__
>make emulator
>install/bin/emulator tmp/__appName__.elf
>/./install/bin/emulator -v -l __simulationLength__ tmp/__appName__.elf
>gtkwave Patmos.vcd &

Get emulator help:
>./install/bin/emulator -h