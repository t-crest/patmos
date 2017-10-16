#include "include/patio.h"
#include "include/bootable.h"
#include <string.h>
#include <machine/spm.h>


static volatile _UNCACHED char data[20] = "99999999999999999999";

int main() {

  const char *msg = "Hello, World!\n";
  unsigned i, k;

  volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;
  int id = get_cpuid();
  int cnt = get_cpucnt();
  char cid = id + 48;

  data[id] = cid;
  for(;;)
  {
    if(id == 0) {
      for (i = 0; i < strlen(msg); i++) {
        *uart_ptr = msg[i];
        for (k = 0; k < 1000000; k++) {
          asm volatile("");
        }
      }
      for (i = 0; i < cnt; i++) {
        *uart_ptr = data[i];
        for (k = 0; k < 1000000; k++) {
          asm volatile("");
        }
      }
    }
  }
}
