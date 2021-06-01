# DE10 Nano Configuration for Patmos

Set the board variable `BOARD=de10-nano-drone` (best done in a local patmos/config.mk)

Change switches for FPGA configuration to:

```
+------+
|* ** *|
| *  * |
+------+
```

Probably add USB blaster permissions for: Bus 001 Device 005: ID 09fb:6810 Altera and 09fb:6010

A TTL UART is connected to GPIO pins 1 and 2 of GPIO 0. The pins 38, 39, and 40 for the DTU I2C controller. Pins 6 and 5 are for the second UART and pins 24 - 13 are for the actuators and propdrives. Additionally, the UART3 and ADC Modules were added for the PREDICT Project. See below.

```
GPIO 0
____________________________
SCL 40 * * 39 SDA
AD0 38 * *
       * *
       * *
    34 * * 33 AD0
SDA 32 * * 31 SCL
GND 30 * * 29 3.3V
       * *
txd 26 * * 25 rxd UART3
AC3 24 * *
AC3 22 * *
AC3 20 * * 19 PROP3
AC2 18 * * 17 PROP2
AC1 16 * * 15 PROP1
AC0 14 * * 13 PROP0
GND 12 * * 11
       * *
       * *
txd  6 * *  5 rxd UART2
     4 * *  3
txd  2 * *  1 rxd UART
____________________________

ADC Module
____________________________
         9 * *  10 GND
         7 * *
         6 * *  5  GND
         4 * *  3
battery  2 * *  1  VCC5 
feedback
____________________________

```

rxd and txd are from the Patmos view, therefore TTL UART rxd needs to
be connected to txd and the other way around.

FPGA configuration has to be done via Quartus (instead of make config).

The de10-nano-drone.xml is configured to use the external 1 GB DDR3 memory.
This requires an SD card containing instructions for the ARM processor to enable FPGA access to the memory.
If your SD card is not already setup, download the "DE10-Nano_v.1.3.8_HWrevC_SystemCD.zip" demo cd from 
```
http://download.terasic.com/downloads/cd-rom/de10-nano/
```
and write the SD card image from Demonstrations/FPGA/SdcardImage
to the SD card. Insert it into the SD card slot of the de10 Nano.
Power on the board and wait a few seconds before programming the FPGA.

Compile and download that example with:

```
make app APP=de10-nano download
```

This example uses the DTU controller to print our the values of the accelerometer, thermometer, and gyroscope. You can use the program that reads out from the AAU
interface with:
```
make app APP=de10-nano MAIN=blinking download
```

The acts_props_uart2 application provides the functions to interact with the actuators, propdrive, and to the second UART. N.B. only the second UART was tested. I have no way to test the actuators and propdrive. Compile and download that example with:

```
make app APP=de10-nano MAIN=acts_props_uart2 download
```

## Comments

 * The I2C interface is an adaption of the [I2C Master](https://www.digikey.com/eewiki/pages/viewpage.action?pageId=10125324) from Digi-Key,
   where further information on the interfacing can be found
 * Register definitions in C for the MPU-6050 can be found at https://playground.arduino.cc/Main/MPU-6050

## TODO

