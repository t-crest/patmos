# DE2-115 Configuration for Patmos

Set the board variable `BOARD=de2-all` (best done in a local config.mk)

The board is configured with 2 additional serial ports, both connected
to GPIO pins (see Figure 4-15 GPIO Pin Arrangement in the DE2 user manual on page 46).

Connect the USB/TTL serial cables to the following pins on GPIO:

```
               | * * |  GND
               | * * |
               | * * |
               | * * |
 RX3-- 37/AF20 | * * |  38/AH26 -- TX3
 RX2-- 39/AH23 | * * |  40/AG26 -- TX2
               +-----+
                GPIO
```

RX and TX names are from Patmos view. Connect TX from your USB/TTL
device to RX of Patmos, and RX to TX.




