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

Useful commands:
============
Build patmos for FPGA:

    make BOARD=altde2-115-audio gen # only generation of verilog
    make BOARD=altde2-115-audio synth # Quartus synthesis
    make BOARD=altde2-115-audio config # load binary into board
    make BOARD=altde2-115-audio # all 3 steps of before

Run a program on FPGA:

    cd ..
    make -C patmos APP=__appName__ comp
    make -C patmos APP=__appName__ download

Simple simulation of a program (printings):

    make comp APP=__appName__
    make BOARD=altde2-115-audio emulator
    install/bin/emulator tmp/__appName__.elf

Waveform simulation of a program (gtkwave):

    make comp APP=__appName__
    make BOARD=altde2-115-audio emulator
    ./install/bin/emulator -v -l __simulationLength__ tmp/__appName__.elf
    gtkwave Patmos.vcd &

Get emulator help:

    ./install/bin/emulator -h