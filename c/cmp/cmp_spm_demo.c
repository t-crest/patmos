const int NOC_MASTER = 0;

/**
 * Libraries
 */
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
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
#define MOCKUPDATA_100B  "_# Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce finibus luctus nibh id porttitor."
#define ALIGN_4B(addr) ((addr & (-4)))

/**
 * Types
 */
typedef struct
{   
    unsigned int cores;
    unsigned int master_core;
} CMPConfig;  // 8-byte

typedef struct{
  unsigned long long timestamp;
  unsigned long length;
} AMessageHeader; // 12-byte

typedef struct{
  AMessageHeader header;
  unsigned char* payload; // 4-byte pointer
} AMessage; // 16-byte

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
  volatile _SPM AMessage *message = (volatile _SPM AMessage*) (local_config+1);
  message->header.timestamp = get_cpu_cycles();
  message->header.length = strlen(MOCKUPDATA_100B)+1;
  message->payload = (unsigned char* volatile _SPM) (&message->header+1);
  volatile _SPM unsigned char* _payload = (volatile _SPM unsigned char*) (message->payload);

  // Each core modifies the message by putting its own ID in the beggining
  _payload[0] = (unsigned char) ((char) (get_cpuid() + '0'));
  // Everybody copy a common message
  for(unsigned i = 1; i < message->header.length; i++){
    _payload[i] = (unsigned char) (MOCKUPDATA_100B)[i];
  }

  // Print the information using a locked UART
  pthread_mutex_lock(&lock);
  printf("Core#%d is up\n", get_cpuid());
  printf("\tCONF(allocated_ptr = %p)\t= {%u-cores, %u-master} [%lu-byte] \n", 
        local_config,
        ((volatile _SPM CMPConfig*)local_config)->cores, 
        ((volatile _SPM CMPConfig*) local_config)->master_core, sizeof(CMPConfig));
  printf("\tMSG (allocated_ptr = %p)\t= {%s %c %c, %lu-byte, @%llu-cycles}\n",
        _payload, 
        ( _SPM char*) _payload, (char) _payload[0], (char) _payload[1],
        ((volatile _SPM AMessage*) message)->header.length, 
        ((volatile _SPM AMessage*) message)->header.timestamp);
  pthread_mutex_unlock(&lock);

  return NULL;
}

/**
 * MAIN
 */
int main() {
  pthread_t tid[NOC_CORES];

  LED = 0x100;
  puts("\nTesting the local SPMs in a multi-threaded example:\n");
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