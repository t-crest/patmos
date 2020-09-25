#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include <machine/patmos.h>
//#include "libcorethread/corethread.h"
//#include "libmp/mp.h"

//LEDs
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))

//Actuators and Propulsion controller
#define ACTUATORS ((volatile _IODEV unsigned *)PATMOS_IO_ACT)
#define PROPULSION ((volatile _IODEV unsigned *)PATMOS_IO_ACT + 0x10)

//UART2
#define UART2 ((volatile _IODEV unsigned *)PATMOS_IO_UART2)

//SPI
#define ADC ((volatile _IODEV unsigned *)0xf00e0000)

const unsigned int CPU_PERIOD = 20; //CPU period in ns.

//Writes to actuator specified by actuator ID (0 to 4)
// void actuator_write(unsigned int actuator_id, unsigned int data)
// {
//   *(ACTUATORS + actuator_id) = data;
// }

//Writes to propulsion specified by propulsion ID (0 to 4)
void propulsion_write(unsigned int propulsion_id, unsigned int data)
{
  *(PROPULSION + propulsion_id) = data;
}

//Reads from propulsion specified by propulsion ID (0 to 4)
int propulsion_read(unsigned int propulsion_id)
{
  return *(PROPULSION + propulsion_id); // this is clearly wrong
}

//Returns the time of an active pulse in microseconds.
//Use actuator pins as defined in de10-nano readme AC(0,3)
unsigned int actuator_read(unsigned int actuator_id)
{
  unsigned int clock_cycles_counted = *(ACTUATORS + actuator_id);
  unsigned int pulse_high_time = (clock_cycles_counted * CPU_PERIOD) / 1000;

  return pulse_high_time;
}

//Writes a byte to the uart2 (to be sent)
//Returns 0 is a character was sent, -1 otherwise.
int uart2_write(unsigned char data)
{
  if ((*UART2 & 0x00000001) != 0)
  {
    *UART2 = (unsigned int)data;
    return 0;
  }
  else
  {
    //*data = 0;
    return -1;
  }
}

//Reads a byte from uart2 (from received data) and places it int the variable
//specified by the pointer * data.
//Returns 0 is a character was read, -1 otherwise.
int uart2_read(unsigned char *data)
{
  if ((*UART2 & 0x00000002) != 0)
  {
    *data = (unsigned char)(*(UART2 + 1) & 0x000000FF);
    return 0;
  }
  else
  {
    //*data = 0;
    return -1;
  }
}

unsigned int reverseBits(unsigned int n)
{
  unsigned int rev = 0;
  while(n > 0)
  {
    rev <<= 1;
    if((n&1) == 1)
    {
      rev ^= 1;
    }
    n >>= 1;
  }

  return rev;
}

void write_adc()
{
  unsigned int config_word = 0;
  *(ADC) = config_word;
}

int read_adc()
{
  return reverseBits(*(ADC));
}



//Blinks the LEDs once
void blink_once()
{
  int i, j;
  for (i = 2000; i != 0; --i)
    for (j = 2000; j != 0; --j)
      LED = 0x0001;
  uart2_write('H');
  for (i = 2000; i != 0; --i)
    for (j = 2000; j != 0; --j)
      LED = 0x0000;
  uart2_write('L');
  return;
}

int main(int argc, char **argv)
{
  unsigned char uart2_data;
  unsigned int adc_val;
  printf("Hello actuators, propellers, and UART2!\n");

  blink_once();

  while (1)
  {
    write_adc();
    printf("Writing...\n");
    adc_val = read_adc();
    printf("%d\n",adc_val);
    // blink_once();
    // unsigned int rec0 = actuator_read(0);
    // printf("PWM cycles: %d = %d\n", 0, rec0);
    // blink_once();
    // unsigned int rec1 = actuator_read(1);
    // printf("PWM cycles: %d = %d\n", 1, rec1);
    // blink_once();
    // unsigned int rec2 = actuator_read(2);
    // printf("PWM cycles: %d = %d\n", 2, rec2);
    // blink_once();
    // unsigned int rec3 = actuator_read(3);
    // printf("PWM cycles: %d = %d\n", 3, rec3);

    // blink_once();
    // propulsion_write(0, 0);
    // printf("Propulsion %d = %d\n", 0, 0);
    // blink_once();
    // propulsion_write(1, 50);
    // printf("Propulsion %d = %d\n", 1, 50);
    // blink_once();
    // propulsion_write(2, 100);
    // printf("Propulsion %d = %d\n", 2, 100);
    // blink_once();
    // propulsion_write(3, 150);
    // printf("Propulsion %d = %d\n", 3, 150);
    // blink_once();
    // if (uart2_read(&uart2_data) == -1)
    // {
    //   printf("Nothing received form UART2\n");
    // }
    // else
    // {
    //   printf("UART2 received: %d\n", uart2_data);
    // }

    

    blink_once();
  }

  return 0;
}
