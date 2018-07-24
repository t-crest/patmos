# DE10 Nano project for Patmos

Set the board variable BOARD=de10-nano (best done in a local config.mk)

Change switches for FPGA configuration to:

```
+------+
|* ** *|
| *  * |
+------+
```

Probably add USB blaster permissions for: Bus 001 Device 005: ID 09fb:6810 Altera and 09fb:6010

A TTL UART is connected to GPIO pins 1 and 2 of GPIO 0.

GND * *
    * *
    * *
    * *
    * *
txd * * rxd (pin 1)

rxd and txd are from the Patmos view, therefore TTL UART rxd needs to
be connected to txd and the other way around.

FPGA configuration has to be done via Quartus (instead of make config).

## TODO

 * Add Shibarchi's sensor interface
 * If needed add external memory (the ARM boot program needs to be changed to allow access to the memory from the FPGA fabric, Florian has the solution)



