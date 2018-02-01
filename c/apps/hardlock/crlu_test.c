#include <string.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"
#include "crlu.h"


_UNCACHED char data[20] = "AAAAAAAAAAAAAAAAAAAA";
_UNCACHED volatile int cnt;
_UNCACHED int times[20];

int _main()
{
  const char *msg = "Hello, World!\n";

  volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;

  for(int k = 0; k < 4; k++)
  {
    lock(0);
    for (int j = 0; j < 100000; j++) {
      asm volatile("");
    }
    data[cnt++] = get_cpuid() + 48;
    if(get_cpuid() == 0) {
      for (int i = 0; i < strlen(msg); i++) {
        *uart_ptr = msg[i];
        for (int j = 0; j < 100000; j++) {
          asm volatile("");
        }
      }
      for (int i = 0; i < 20; i++) {
        *uart_ptr = data[i];
        for (int j = 0; j < 100000; j++) {
          asm volatile("");
        }
      }
      *uart_ptr = '\n';
    }
    unlock(0);
  }
  return 0;
}

void worker_func(void* arg) {
  int worker_param = *((int*)arg);
  int ret = _main();
  corethread_exit(&ret);
  return;
}

int main() {

  id threads[20];
  int len = sizeof threads / sizeof *threads;
  if(get_cpucnt() < len)
    len = get_cpucnt();

  for(int i = 1; i < len; i++)
  {
    threads[i] = i;
    int worker_param = 1;
    corethread_create(threads[i],&worker_func,&worker_param);
  }

  int ret = _main();
  for(int i = 1; i < len; i++)
  {
    void * res;
    corethread_join(threads[i], &res);
  }
  return ret;
}




