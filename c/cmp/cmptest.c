//#include "include/patio.h"
//#include "include/bootable.h"
#include <string.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"


_UNCACHED char data[20] = "AAAAAAAAAAAAAAAAAAAA";

int _main()
{
  const char *msg = "Hello, World!\n";
  unsigned i, k;

  volatile _SPM int *led_ptr = (volatile _SPM int *) PATMOS_IO_LED;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) (PATMOS_IO_UART+0x04);

  
  for(int j = 0; j < 5; j++)
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
  }
  return 0;
}

void func_worker_1(void* arg) {
  int worker_1_param = *((int*)arg);
  int ret = _main();
  corethread_exit(&ret);
  return;
}

int main() {

  for(int j = 1; j < get_cpucnt(); j++)
  {
    int worker_1 = 1;
    int worker_1_param = 1;
    corethread_create(worker_1,&func_worker_1,&worker_1_param);
  }
  return _main();
}




