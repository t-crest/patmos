# DE10 Nano Configuration for Patmos

Set the board variable `BOARD=de10-nano` (best done in a local config.mk)

Change switches for FPGA configuration to:

```
+------+
|* ** *|
| *  * |
+------+
```

Probably add USB blaster permissions for: Bus 001 Device 005: ID 09fb:6810 Altera and 09fb:6010

A TTL UART is connected to GPIO pins 1 and 2 of GPIO 0. The MPU sensor is connected to GND and 3.3 V and pins 31, 32, and 33. See below.

```
    40 * * 39
       * *
       * *
       * *
    34 * * 33 AD0
SDA 32 * * 31 SCL
GND 30 * * 29 3.3V
       * *
       * *
       * *
       * *
       * *
       * *
       * *
       * *
GND 12 * * 11
       * *
       * *
       * *
     4 * *  3
txd  2 * *  1 rxd (pin 1)
```

rxd and txd are from the Patmos view, therefore TTL UART rxd needs to
be connected to txd and the other way around.

FPGA configuration has to be done via Quartus (instead of make config).

The on-chip memory is 512 KB (instead of typical 2 MB on the DE2-115).
Therefore, the stack start needs to be set accordingly with following
linker options:

```
        -mpatmos-stack-base=0x080000 -mpatmos-shadow-stack-base=0x078000 \
        -Xgold --defsym -Xgold __heap_end=0x070000
```

Best see in the example in c/apps/de10-nano. Compile and download that example with:

```
make app APP=de10-nano download
```

## TODO

 * Add Shibarchi's sensor interface
 * If needed add external memory (the ARM boot program needs to be changed to allow access to the memory from the FPGA fabric, Florian has the solution)



