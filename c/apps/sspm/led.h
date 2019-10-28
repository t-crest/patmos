
#ifndef _LED_H_
#define _LED_H_

#include <machine/patmos.h>

#define LED ( *( ( volatile _IODEV unsigned * ) PATMOS_IO_LED))
#define MS_CLOCK (18000)

void led_on();

void led_off();

void led_on_for(int ms);

void led_off_for(int ms);

#endif
