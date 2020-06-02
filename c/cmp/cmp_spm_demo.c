const int NOC_MASTER = 0;

/**
 * Libraries
 */
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include <pthread.h>
#include <math.h>
#include "libnoc/noc.h"

/**
 * Defs
 */
#define CPU_PERIOD 12.5f
#define LOCAL_SPM_BASE  0x0
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define LEDCMP (*((volatile _IODEV unsigned *)PATMOS_IO_LEDSCMP))
#define SEGDISP (*((volatile _IODEV unsigned *)PATMOS_IO_SEGDISP))
#define DEAD *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))
#define MOCKUPDATA_100B "#_ Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce finibus luctus nibh id porttitor."
#define ALIGN_4B(addr) ((addr & (-4)) * 8); 

/**
 * Types
 */
typedef struct
{   
    unsigned int cores;
    unsigned int master_core;
} CMPConfig;

typedef struct{
  unsigned long long timestamp; // 8-byte
  unsigned int length; // 4-byte
} AMessageHeader;

typedef struct{
  AMessageHeader header;
  unsigned char* payload; // 4-byte pointer
} AMessage;

/**
 * Globals
 */              
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *worker_thread(void *param) {
  // Initialize the local configuration, this should be passed to the thread from the dispatcher
  volatile _SPM CMPConfig* local_config = (volatile _SPM CMPConfig*) (LOCAL_SPM_BASE);
  local_config->cores = NOC_CORES;
  local_config->master_core = NOC_MASTER;
  
  // Create an application message on the SPM
  volatile _SPM AMessage *message = (volatile _SPM AMessage*) ALIGN_4B(sizeof(CMPConfig));
  message->header.timestamp = get_cpu_cycles();
  message->header.length = sizeof(MOCKUPDATA_100B);
  message->payload = (volatile unsigned char* _SPM) ALIGN_4B(sizeof(CMPConfig) + sizeof(AMessageHeader));
  for(unsigned i = 0; i < sizeof(MOCKUPDATA_100B); i++){
    message->payload[i] = (unsigned char) MOCKUPDATA_100B[i];
  }
  message->payload[1] = (unsigned char) ((char) (get_cpuid() + '0'));

  // Print the information using a locked UART
  pthread_mutex_lock(&lock);
  printf("Core#%d is up \n\tCONF(allocated_ptr = %p) = {%u, %u} [%lu-byte] \n\tMSG (allocated_ptr = %p) = {%s} [%lu-byte])\n",
        get_cpuid(), (volatile _SPM CMPConfig*) local_config, ((volatile _SPM CMPConfig*)local_config)->cores, 
        ((volatile _SPM CMPConfig*) local_config)->master_core, sizeof(CMPConfig),
        message, (_SPM char*) ((volatile _SPM AMessage*) message)->payload, sizeof(MOCKUPDATA_100B));
  pthread_mutex_unlock(&lock);

  return NULL;
}

/**
 * MAIN
 */
int main() {
  pthread_t tid[NOC_CORES];

  LED = 0x100;
  puts("\nTesting the local SPMs in a multi-threaded example");
  LED = 0xF0;

  unsigned j = 0;
  for (unsigned i = 0; i < NOC_CORES; i++) {
    if (i != NOC_MASTER) {
      if (pthread_create(&tid[i], NULL ,&worker_thread, NULL) != 0) {
        printf("\n Compute thread %d not created\n", i);
        return 1;
      }
    }
    LED |= i;
  }

  LED = 0x0;
  worker_thread(NULL);
  LED = 0x1FF;

  return 0;
}