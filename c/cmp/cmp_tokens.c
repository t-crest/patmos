/*
    Mulit-core LED example
        Author: Eleftherios Kyriakakis
*/

const int NOC_MASTER = 0;
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
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778
#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define LEDCMP (*((volatile _IODEV unsigned *)PATMOS_IO_LEDSCMP))
#define DEAD (*((volatile _IODEV unsigned *)PATMOS_IO_DEADLINE))
/**
 * Macros
 */
#define DEAD_CALC(WAITTIME, CPU_PERIOD)                                        \
  (WAITTIME * 0.5 * USEC_TO_NS / CPU_PERIOD)

/**
 * Functions
 */
int wait_deadline(unsigned int waitUsecs, float cpuPeriod) {
  DEAD = DEAD_CALC(waitUsecs, cpuPeriod);
  return DEAD;
}

void slave(void *param) {
  float cpuPeriod = (1.0f / get_cpu_freq()) * SEC_TO_NS;
  unsigned int numOfTokens = *((unsigned int *)param);
  int readPeriod;
  unsigned char init = 0;
  qpd_t *chan1 =
      mp_create_qport(get_cpuid(), SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  qpd_t *chan2 = mp_create_qport((get_cpuid() % (get_cpucnt() - 1)) + 1, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports();

  if (get_cpuid() == 1 && init == 0) {
    for (int i = numOfTokens; i != 0; --i) {
      *(volatile int _SPM *)(chan1->write_buf) = get_cpuid() * 1000;
      mp_send(chan1, 0);
    }
    init = 1;
  }
  while (1) {
    mp_recv(chan2, 0);
    readPeriod = *((volatile int _SPM *)(chan1->read_buf));
    mp_ack(chan2, 0);
    LEDCMP = 1;
    wait_deadline(readPeriod, cpuPeriod);
    *(volatile int _SPM *)(chan1->write_buf) = (readPeriod + get_cpuid() * 10000) % SEC_TO_USEC;
    mp_send(chan1, 0);
    LEDCMP = 0;
  }
  return;
}

/**
 * MAIN
 */
int main() {
  int numOfTokens = 1;
  puts("Master is up n' running!");
  for (int i = 0; i < get_cpucnt(); i++) {
    if (i != NOC_MASTER) {
      if (corethread_create(i, &slave, (void *)&numOfTokens) != 0) {
        printf("Corethread %d not created\n", i);
      }
    }
  }
  puts("I started the threads!");
  while (1) {
    continue;
  };

  return 0;
}