#include "include/patio.h"
#include "include/bootable.h"
#include <string.h>
#include <machine/spm.h>


static volatile _UNCACHED char data[20] = "99999999999999999999";

int main() {

  const char *msg = "\nLoop Start:";
  unsigned i, k;

  volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;
  volatile _SPM int *crlu = (volatile _SPM int *) 0xE0000000;
  int id = get_cpuid();
  int ccnt = get_cpucnt();
  char cnt = 0;
  
  for(;;)
  {
    *crlu = 1;
    if(id == 0) {
      for (i = 0; i < strlen(msg); i++) {
        *uart_ptr = msg[i];
        for (k = 0; k < 100000; k++) {
          asm volatile("");
        }
      }
      for (i = 0; i < ccnt; i++) {
        *uart_ptr = data[i];
        for (k = 0; k < 100000; k++) {
          asm volatile("");
        }
      }
      for (k = 0; k < 5000000; k++) {
          asm volatile("");
      }
    }
    else {
      cnt++;
      if(cnt == 10)
        cnt = 0;
      data[id] = cnt + 48;
      for (k = 0; k < 10000000; k++) {
          asm volatile("");
      }
    }
  }
}
