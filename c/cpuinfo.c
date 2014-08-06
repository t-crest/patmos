/*
	Test CPU info.

	Author: 
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>

#define DELAY 10000*1

int main() {
  int val;

  for (int i=0; i<10; ++i) {
  printf("Time: %llu usec\n", get_cpu_usecs());
  }

  putchar('0');
  val = get_cpu_usecs()+DELAY;
  while ((long long)get_cpu_usecs()-val < 0)
    ;
  putchar('1');
  val = get_cpu_usecs()+DELAY;
  while ((long long)get_cpu_usecs()-val < 0)
    ;
  putchar('2');
  val = get_cpu_usecs()+DELAY;
  while ((long long)get_cpu_usecs()-val < 0)
    ;
  putchar('X');
  puts("hello");
  printf("CPU ID: %d \n", get_cpuid());
  printf("CPU frequency: %d MHz\n", get_cpu_freq()/1000000);
  
  return 0;
}
