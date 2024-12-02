/*
    Mulit-core tokens LED example
        Author: Eleftherios Kyriakakis
*/

/**
 * Libraries
 */
#include "libcorethread/corethread.h"
#include "libmp/mp.h"
#include <machine/patmos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Defs
 */
#define NS_TO_SEC 0.000000001
#define NS_TO_USEC 0.001
#define USEC_TO_NS 1000
#define USEC_TO_SEC 0.000001
#define MS_TO_NS 1000000
#define MS_TO_USEC 1000
#define MS_TO_SEC 0.001
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778
#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define LEDCMP (*((volatile _IODEV unsigned *)PATMOS_IO_LEDSCMP))
#define DEAD (*((volatile _IODEV unsigned *)PATMOS_IO_DEADLINE))
#define UART *((volatile _SPM unsigned int *) (PATMOS_IO_UART+0x4))
/**
 * Macros
 */
#define DEAD_CALC(WAITTIME, CPU_PERIOD) (WAITTIME * 0.5 * USEC_TO_NS / CPU_PERIOD)

typedef struct {
  int numOfTokens;
  float cpuPeriod;
} ThreadArg;

/**
 * Functions
 */
int wait_deadline(unsigned int waitUsecs, float cpuPeriod) {
  DEAD = DEAD_CALC(waitUsecs, cpuPeriod);
  return DEAD;
}

void slave(void *param) {
  ThreadArg threadArg = *((ThreadArg *)param);
  int readPeriod = 100000;
  unsigned char init = 0;
  qpd_t *chan1 = mp_create_qport(get_cpuid(), SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  qpd_t *chan2 = mp_create_qport((get_cpuid() % (get_cpucnt() - 1)) + 1, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports();

  if (get_cpuid() == 1 && init == 0) {
    for (int i = threadArg.numOfTokens; i != 0; --i) {
      *(volatile int _SPM *)(chan1->write_buf) = (get_cpuid()+1) * 1000;
      mp_send(chan1, 0);
    }
    init = 1;
  }
  while (1) {
    mp_recv(chan2, 0);
    readPeriod = *((volatile int _SPM *)(chan1->read_buf));
    mp_ack(chan2, 0);
    UART = get_cpuid() + '0';
    wait_deadline(readPeriod, threadArg.cpuPeriod);
    *(volatile int _SPM *)(chan1->write_buf) = (readPeriod + (get_cpuid()+1) * 100*MS_TO_USEC) % SEC_TO_USEC;
    mp_send(chan1, 0);
    UART = get_cpuid() + 'A';
  }
  return;
}

/**
 * MAIN
 */
int main() {
  puts("Master is up n' running!");
  ThreadArg threadArg = {
    .numOfTokens = 2,
    .cpuPeriod = (1.0f / get_cpu_freq()) * SEC_TO_NS
  };
  LED = get_cpucnt();
  for (int i = 0; i < get_cpucnt(); i++) {
    if (i != NOC_MASTER) {
      if (corethread_create(i, &slave, (void *)&threadArg) != 0) {
        printf("Corethread %d not created\n", i);
        return 1;
      }
    }
  }
  puts("I started the threads!");
  while (1) {
    LED = get_cpucnt();
    wait_deadline(get_cpucnt()*10000, 12.5);
    LED = 0;
    wait_deadline(get_cpucnt()*10000, 12.5);
  };

  return 0;
}