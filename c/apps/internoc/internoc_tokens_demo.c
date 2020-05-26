const int NOC_MASTER = 0;
/**
 * Libraries
 */
#include <machine/patmos.h>
#include <pthread.h>
#include "internoc_driver.h"
#include <math.h>
/**
 * Defs
 */
#define CPU_PERIOD 12.5f
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
#define LED (*((volatile _IODEV unsigned *)PATMOS_IO_LED))
#define LEDCMP (*((volatile _IODEV unsigned *)PATMOS_IO_LEDSCMP))
#define SEGDISP (*((volatile _IODEV unsigned *)PATMOS_IO_SEGDISP))
#define DEAD *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))
/**
 * Macros
 */
#define DEAD_CALC(WAITTIME) (WAITTIME * 0.5 * USEC_TO_NS / CPU_PERIOD)

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void print_general_info()
{
	printf("\nGeneral info:\n");
	printf("\tMAC: %llx", get_mac_address());
	printf("\n\tIP: ");
	ipv4_print_my_ip();
	printf("\n");
	arp_table_print();
	printf("\n");
	return;
}

void *compute_thread(void *param) {
  // Initialize the local configuration, this should be passed to the thread from the dispatcher
  InterNoCConfig local_config = internoc_init_config(NOC_CORES, (unsigned char[4]) {192, 168, 1, NOC_MASTER}, NOC_MASTER);

  // Print the information using a lock UART
  pthread_mutex_lock(&lock);
  printf("InterNoCCore#%d is up\n", get_cpuid());
  prin_core_channels(local_config);
  pthread_mutex_unlock(&lock);

  // Create an application message on the stack to be sent
  InterNoCMessage message = {(InterNoCMessageHead) {get_cpu_cycles(), 14}, (unsigned char*) "hello gateway"};

  // Build the message on the proper buffer
  _SPM udp_t* packet_ptr = internoc_build_packet(local_config, local_config.my_ip, local_config.gateway_ip, 69, 69, 
                                                (unsigned char*) &message, sizeof(InterNoCMessageHead) + message.header.length);
  
  // Send the message
  internoc_send(local_config, local_config.gateway_ip, 0);

  // Print the information of the sent message using a lock to synchronize UART
  pthread_mutex_lock(&lock);
  printf("TX[%x]{%u:%u:%u:%u=>%u:%u:%u:%u}[%s] @ %llu cycles\n", get_cpuid(), 
        (unsigned int) packet_ptr->ip_head.source_ip[0], (unsigned int) packet_ptr->ip_head.source_ip[1], 
        (unsigned int) packet_ptr->ip_head.source_ip[2], (unsigned int) packet_ptr->ip_head.source_ip[3], 
        (unsigned int) packet_ptr->ip_head.destination_ip[0], (unsigned int) packet_ptr->ip_head.destination_ip[1], 
        (unsigned int) packet_ptr->ip_head.destination_ip[2], (unsigned int) packet_ptr->ip_head.destination_ip[3], 
        (char*) ((InterNoCMessage*) (packet_ptr->data))->payload,
        ((InterNoCMessage*)(packet_ptr->data))->header.timestamp);
  pthread_mutex_unlock(&lock);

  return NULL;
}

void *gateway_thread(void* param){
  // Initialize the local configuration, this should be passed to the thread from the dispatcher
  InterNoCConfig local_config = internoc_init_config(NOC_CORES, (unsigned char[4]) {192, 168, 1, NOC_MASTER}, NOC_MASTER);

  // Print the information using a lock UART
  pthread_mutex_lock(&lock);
  printf("InterNoCCore#%d is up\n", get_cpuid());
  prin_core_channels(local_config);
  pthread_mutex_unlock(&lock);

  // Assign an array on the stack to store the received packet pointers
  _SPM udp_t* packet_ptrs[local_config.core_links_num];
  unsigned long long timestamps[local_config.core_links_num];

  // Loop through all the cores to receive packets and timestamp reception
  unsigned j = 0;
  for(int i=0; i<local_config.cores; i++)
  {
    if(i != local_config.gateway_core)
    {
      packet_ptrs[j] = internoc_recv(local_config, (unsigned char[]){192,168,1,i}, 0);
      timestamps[j] = get_cpu_cycles();
      j++;
    }
  }

  // Loop through the received packets and timestamps and print the packet content
  for(int i=0; i<local_config.core_links_num; i++)
  {
    pthread_mutex_lock(&lock);
    printf("RX[%x]{%u:%u:%u:%u=>%u:%u:%u:%u}[#%u, %s] @ %llu cycles (e2e = %.2f us)\n", get_cpuid(),
          (unsigned int) packet_ptrs[i]->ip_head.source_ip[0], (unsigned int) packet_ptrs[i]->ip_head.source_ip[1], 
          (unsigned int) packet_ptrs[i]->ip_head.source_ip[2], (unsigned int) packet_ptrs[i]->ip_head.source_ip[3], 
          (unsigned int) packet_ptrs[i]->ip_head.destination_ip[0], (unsigned int) packet_ptrs[i]->ip_head.destination_ip[1], 
          (unsigned int) packet_ptrs[i]->ip_head.destination_ip[2], (unsigned int) packet_ptrs[i]->ip_head.destination_ip[3], 
          (unsigned int) packet_ptrs[i]->ip_head.identification, (char*) ((InterNoCMessage*) (packet_ptrs[i]->data))->payload,
          timestamps[i], (timestamps[i] - ((InterNoCMessage*)(packet_ptrs[i]->data))->header.timestamp) * CPU_PERIOD * NS_TO_USEC);
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

/**
 * MAIN
 */
int main() {
  pthread_t tid[NOC_CORES];

  LED = 0x100;
  puts("InterNoC is up n' running!");
	eth_iowr(0x40, 0xEEF0DA42);
	eth_iowr(0x44, 0x000000FF);
	eth_iowr(0x00, 0x0000A423);
	ipv4_set_my_ip((unsigned char[4]) {192, 168, 1, NOC_MASTER});
	arp_table_init();
	print_general_info();

  LED = 0xF0;

  for (unsigned i = 0; i < get_cpucnt(); i++) {
    if (i != NOC_MASTER) {
      if (pthread_create(&tid[i], NULL ,&compute_thread, NULL) != 0) {
        printf("\n Compute thread %d not created\n", i);
        return 1;
      }
    }
    LED |= i;
  }

  gateway_thread(NULL);
  LED = 0x1FF;

  return 0;
}