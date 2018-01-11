//#include "include/patio.h"
//#include "include/bootable.h"
#include <string.h>
#include <machine/patmos.h>
#include <machine/spm.h>


static volatile _UNCACHED char data[20] = "AAAAAAAAAAAAAAAAAAAA";

int main() {

  const char *msg = "Hello, World!\n";
  unsigned i, k;

  volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;

  
  for(;;)
  {
    data[get_cpuid()] = get_cpuid() + 48;
    if(get_cpuid() == 0) {
      for (i = 0; i < strlen(msg); i++) {
        *uart_ptr = msg[i];
        for (k = 0; k < 1000000; k++) {
          asm volatile("");
        }
      }
      for (i = 0; i < get_cpucnt(); i++) {
        *uart_ptr = data[i];
        for (k = 0; k < 1000000; k++) {
          asm volatile("");
        }
      }
      *uart_ptr = '\n';
    }
    return 10;
  }
}
