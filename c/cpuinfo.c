/*
	Test CPU info.

	Author: 
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>

int main() {

  volatile _SPM int *cpuinfo_ptr = (volatile _SPM int *) 0xF0000000;
  volatile _SPM int *cpufrequ_ptr = (volatile _SPM int *) 0xF0000004;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0000804;
  int val;

  *uart_ptr = 'X';
// TODO wait for UART to be ready
  *uart_ptr = *cpuinfo_ptr + '0';
  val = *cpufrequ_ptr + '0';
  val = val >> 20; // about divide by a million, now it is in MHz
  val = val >> 3; // now /8
  *uart_ptr = val+'0';
  *uart_ptr = '\n';
  
}
