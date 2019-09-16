/*
    InterNoC Tokens LED Demo
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
#include "internoc_net.h"

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
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;
/**
 * Macros
 */
#define DEAD_CALC(WAITTIME, CPU_PERIOD) (WAITTIME * 0.5 * USEC_TO_NS / CPU_PERIOD)

/**
 * Globals
 */
typedef struct {
  int cpuPeriod;
  int tokenLeader;
  int numOfTokens;
} ThreadArg;

typedef struct{
  short id;
  char* data;
} InterNoCToken;


unsigned int runAsPTPMode = PTP_SLAVE;
int userPTPSyncInterval = -3;

/**
 * Functions
 */
int wait_deadline(unsigned int waitUsecs, int cpuPeriod) {
  DEAD = DEAD_CALC(waitUsecs, cpuPeriod);
  return DEAD;
}

void print_general_info(){
	printf("\nGeneral info:\n");
	printf("\tMAC: %llx", get_mac_address());
	printf("\n\tIP: ");
	ipv4_print_my_ip();
	printf("\n");
	arp_table_print();
	printf("\n");
	return;
}

void printSegmentInt(unsigned number) {
    *(disp_ptr+0) = number & 0xF;
    *(disp_ptr+1) = (number >> 4) & 0xF;
    *(disp_ptr+2) = (number >> 8) & 0xF;
    *(disp_ptr+3) = (number >> 12) & 0xF;
    *(disp_ptr+4) = (number >> 16) & 0xF;
    *(disp_ptr+5) = (number >> 20) & 0xF;
    *(disp_ptr+6) = (number >> 24) & 0xF;
    *(disp_ptr+7) = (number >> 28) & 0xF;
}

void task(void *param) {
  ThreadArg threadArg = *((ThreadArg *)param);
  int waitPeriod = get_cpuid()+1 * 500 * MS_TO_USEC;
  unsigned char init = 0;
  int token = 0;
  qpd_t *chanRx = mp_create_qport(get_cpuid(), SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  qpd_t *chanTx = mp_create_qport((get_cpuid()+1) % (get_cpucnt()-1), SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports();

  if (get_cpuid() == threadArg.tokenLeader && init == 0) {
    for (int i = threadArg.numOfTokens; i != 0; --i) {
      *(volatile int _SPM *)(chanTx->write_buf) = 1;
      mp_send(chanTx, 0);
    }
    init = 1;
  }
  while (1) {
    mp_recv(chanRx, 0);
    token = *((volatile int _SPM *)(chanRx->read_buf));
    mp_ack(chanRx, 0);
    LEDCMP = token;
    wait_deadline(waitPeriod, threadArg.cpuPeriod);
    *(volatile int _SPM *)(chanTx->write_buf) = 1;
    mp_send(chanTx, 0);
    LEDCMP = ~token;
  }
  return;
}

void gateway(void* param){
  ThreadArg threadArg = *((ThreadArg *)param);
  int waitPeriod = get_cpuid()+1 * 500 * MS_TO_USEC;
  unsigned char init = 0;
  int token = 0;
  qpd_t* chanRx = mp_create_qport(0, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  qpd_t* chanTx = mp_create_qport(1, SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports();

  LED &= 0x0FF;
  
  if (get_cpuid() == threadArg.tokenLeader && init == 0) {
    if(runAsPTPMode == PTP_MASTER){
      for (int i = threadArg.numOfTokens; i != 0; --i) {
        *(volatile int _SPM *)(chanTx->write_buf) = 1;
        mp_send(chanTx, 0);
      }
    } else {
      for (int i = threadArg.numOfTokens; i != 0; --i) {
        while(receiveAndHandleFrame(0) != INTERNOC_MSG){};
        *(volatile int _SPM *)(chanTx->write_buf) = 1;
        mp_send(chanTx, 0);
      }
    }
    init = 1;
  }

  LED |= 0x100;

  while(1){
    if(runAsPTPMode == PTP_MASTER){
      mp_recv(chanRx, 0);
      token = *((volatile int _SPM *)(chanRx->read_buf));
      mp_ack(chanRx, 0);
      udp_send_mac(tx_addr, rx_addr, PTP_BROADCAST_MAC, (unsigned char[4]){192,168,2,128}, 696, 696, (unsigned char[]) {'1'}, 1, 0);
    } else {
        while(receiveAndHandleFrame(0) != INTERNOC_MSG){};
        *(volatile int _SPM *)(chanTx->write_buf) = 1;
        mp_send(chanTx, 0);
    }
    LEDCMP = token;
    wait_deadline(waitPeriod, threadArg.cpuPeriod);
    *(volatile int _SPM *)(chanTx->write_buf) = 1;
    mp_send(chanTx, 0);
    LEDCMP = ~token;
  }
}

/**
 * MAIN
 */
int main() {
  int ptpSyncPeriod = 0;
  unsigned int ptpSeqId = 0;
  unsigned int startTime = 0;
  LED = 0x1FF;
  puts("InterNoC is up n' running!");
  puts("Select PTP mode (PTP_MASTER=0, PTP_SLAVE=1):");
	scanf("%u", &runAsPTPMode);
	// puts("Enter a sync interval period in 2^n seconds:");
	// scanf("%d", &userPTPSyncInterval);
	// ptpSyncPeriod = SYNC_INTERVAL_OPTIONS[abs(userPTPSyncInterval)]*USEC_TO_NS;
	// printf("PTP node assigned role of %s\n", runAsPTPMode == 0 ? "PTP_MASTER" : "PTP_SLAVE");
	// printf("PTP sync interval (0x%02x) configured at %d ns\n", (unsigned char) userPTPSyncInterval, ptpSyncPeriod);
  // printf("PTP follow up delay configured at %d ns\n", PTP_DELAY_FOLLOWUP * USEC_TO_NS);
	// //MAC controller settings
	eth_iowr(0x40, 0xEEF0DA42);
	eth_iowr(0x44, 0x000000FF);
	eth_iowr(0x00, 0x0000A423);
	ipv4_set_my_ip((unsigned char[4]) {192, 168, 2, 64*(runAsPTPMode+1)});
	arp_table_init();
	print_general_info();
  // //Configure PTP
	// thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, runAsPTPMode, my_mac, my_ip, 1, userPTPSyncInterval);

  //Start Threads
  ThreadArg threadArg = {
    .cpuPeriod = (1.0f / get_cpu_freq()) * SEC_TO_NS,
    .tokenLeader = 0,
    .numOfTokens = 2
  };
  LED = 0xFF & get_cpucnt();
  for (int i = 0; i < get_cpucnt(); i++) {
    if (i != NOC_MASTER) {
      if (corethread_create(i, &task, (void *)&threadArg) != 0) {
        printf("Corethread %d not created\n", i);
      }
    }
  }
  
  puts("I started the threads!");
  gateway((void *)&threadArg);

  return 0;
}

