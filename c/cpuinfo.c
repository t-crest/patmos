/*
	Test CPU info.

	Author: 
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>
#include <stdio.h>
#include "patio.h"

#define DELAY 1000000*1

int main() {

  volatile _SPM int *t_ptr = (volatile _SPM int *) 0xF0000204;
  volatile _SPM int *cpuinfo_ptr = (volatile _SPM int *) 0xF0000000;
  volatile _SPM int *cpufrequ_ptr = (volatile _SPM int *) 0xF0000004;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0000804;
  int val;

for (int i=0; i<10; ++i) {
printf("%d\n", TIMER_US_LOW);
}

  *uart_ptr = '0';
  val = TIMER_US_LOW+DELAY;
  while (TIMER_US_LOW-val < 0)
    ;
  *uart_ptr = '1';
  val = TIMER_US_LOW+DELAY;
  while (TIMER_US_LOW-val < 0)
    ;
  *uart_ptr = '2';
  val = TIMER_US_LOW+DELAY;
  while (TIMER_US_LOW-val < 0)
    ;
  *uart_ptr = 'X';
puts("hello");
// TODO wait for UART to be ready
  *uart_ptr = *cpuinfo_ptr + '0';
  val = *cpufrequ_ptr + '0';
  val = val >> 20; // about divide by a million, now it is in MHz
  val = val >> 3; // now /8
  *uart_ptr = val+'0';
  *uart_ptr = '\n';
  
}
