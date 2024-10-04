/*
    This is a multicore program in which threats communicate in the time-triggered fashion
    by reading TDM period counter.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libnoc/noc.h"
#include "include/patio.h"
#include "libcorethread/corethread.h"
#include "libmp/mp.h"
#include "libmp/mp_internal.h"


#define TDM_P_COUNTER (*(NOC_TDM_BASE+1))
#define MP_CHAN_NUM_BUF 2  //  The number of buffers in the SPM
#define MP_CHAN_BUF_SIZE 2044 // The size of a buffer in bytes (max 2048 Bytes) + 4 Bytes for FLAG_SIZE
#define MSG_SIZE (MP_CHAN_BUF_SIZE/4) // Message in words 

//Triggering definitions
#define MEASUREMENT_MODE_N

///////////////////////////////////////////////////////////////////////////////
// These preprocessor directives will map to a schedule table later 
///////////////////////////////////////////////////////////////////////////////
// Triggering period calculation
#define TASK_ACTIVATION 3960 // Initial overhead coming from task activations
#define INITIALIZATION_SLOT 1000 // shows the number of TDM periods required for task initialization. obtained by measurements, showing the Worst-Case Execution Time
//producer
#define TRIGGER_PROD_COMP  (TASK_ACTIVATION+INITIALIZATION_SLOT)
#define WCET_PROD_COMP 1000// dummy number for now
#define TRIGGER_PROD_COMM (TRIGGER_PROD_COMP+WCET_PROD_COMP)
#define WCET_PROD_COMM 10//dummy number
#define PROD_PERIOD (WCET_PROD_COMP+WCET_PROD_COMM)
//consumer
#define TRIGGER_CONS_COMM (TRIGGER_PROD_COMM+WCET_PROD_COMM)
#define WCET_CONS_COMM 10// dummy number for now
#define TRIGGER_CONS_COMP (TRIGGER_CONS_COMM+WCET_CONS_COMM)
#define WCET_CONS_COMP 1000//dummpy number
#define CONS_PERIOD (WCET_CONS_COMP+WCET_CONS_COMM)


// Measure execution time
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED unsigned int timeStamps_master[4]={0};
volatile _UNCACHED unsigned int timeStamps_slave1[4]={0};
volatile _UNCACHED unsigned int timeStamps_slave2[4]={0};

//For debugging
volatile _UNCACHED int debug_print[MSG_SIZE]= {0};

//the schedule table
//static const unsigned int schedule_table[]{};


//Producer Core
void producer(void *arg) {

  #ifdef MEASUREMENT_MODE
  // start initialization measurement
    timeStamps_slave1[0] = TDM_P_COUNTER;//3955 cycles
  #endif

  int id = get_cpuid();

  // Data initialization
  int volatile data_wr[MSG_SIZE];

  ///////////////////////////////////////////////////////////////////////////////
  // This section of the task that handles with initializations for buffering
  ///////////////////////////////////////////////////////////////////////////////

  // allocating the data into SPM
  qpd_t * chan1 = mp_create_qport(1, SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 

  //remote address calculation at the receiver side
  int rmt_addr_offset = (chan1->buf_size + FLAG_SIZE) * chan1->send_ptr;
  volatile void _SPM *calc_rmt_addr = &chan1->recv_addr[rmt_addr_offset];

	
  
  #ifdef MEASUREMENT_MODE
    timeStamps_slave1[1] = TDM_P_COUNTER; // stop the initialization measurement at 4888 cycles
  #endif


  for(;;){

    ///////////////////////////////////////////////////////////////////////////////
    // Computation part of the Task. Data production/manipulation
    ///////////////////////////////////////////////////////////////////////////////
    
    if((TDM_P_COUNTER-TRIGGER_PROD_COMP)%PROD_PERIOD == 0 ){

        // Data production. at the moment we use dummy data
         for (int i=0;i<MSG_SIZE;i++){
          data_wr[i]=i;
        }

        // writing the data to the write buffer
        for (int i=0;i<MSG_SIZE;i++){
          *(volatile int _SPM*)((int*)chan1->write_buf+i) = data_wr[i];
        }
    } 

    ///////////////////////////////////////////////////////////////////////////////
    // Communication part of the Task. 
    ///////////////////////////////////////////////////////////////////////////////
  	
      // when the communication time is triggered
  	  if(((TDM_P_COUNTER-TRIGGER_PROD_COMM)%PROD_PERIOD) == 0 ){
    	    
      	  #ifdef MEASUREMENT_MODE
      	    timeStamps_slave1[2] = TDM_P_COUNTER; //start the communication measurement
    	  	#endif

          //nonblocking write transaction
    	  	noc_nbwrite( (id+1),calc_rmt_addr,chan1->write_buf,chan1->buf_size + FLAG_SIZE, 0);

          #ifdef MEASUREMENT_MODE
    	  	  timeStamps_slave1[3] = TDM_P_COUNTER; //stop the communication measurement
    	  	#endif

  	  }// if

   }//for

}



// Consumer Core
void consumer(void *arg) {

  #ifdef MEASUREMENT_MODE
  // start initialization measurement
  timeStamps_slave2[0] = TDM_P_COUNTER;	
  #endif
  
  int id = get_cpuid();
  int cnt = get_cpucnt();

  int volatile data_rd[MSG_SIZE];

  ///////////////////////////////////////////////////////////////////////////////
  // This section of the task handles with initializations for buffering
  ///////////////////////////////////////////////////////////////////////////////
  // allocating data to SPM
  qpd_t * chan1 = mp_create_qport(1, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); // mp init ports


  #ifdef MEASUREMENT_MODE
  timeStamps_slave2[1] = TDM_P_COUNTER; // stop the initialization measurement
  #endif

  for(;;){

    ///////////////////////////////////////////////////////////////////////////////
    // Communication part of the Task. 
    ///////////////////////////////////////////////////////////////////////////////
      // when the time is triggered
  	  if((TDM_P_COUNTER-TRIGGER_CONS_COMM)%CONS_PERIOD == 0 ){

    	  	#ifdef MEASUREMENT_MODE
    	  	timeStamps_slave2[2] = TDM_P_COUNTER;// start the communication measurement
            #endif

    		  // reading the data to the read buffer
    	  	for (int i=0;i<MSG_SIZE;i++){

    	  	   data_rd[i] = *(volatile int _SPM*)((int*)chan1->read_buf+i); // read the data
    	  	}

    	  	#ifdef MEASUREMENT_MODE
    	    timeStamps_slave2[3] = TDM_P_COUNTER; //stop the communication measurement
    	  	#endif
      }

    ///////////////////////////////////////////////////////////////////////////////
    // Computation part of the Task. Perhaps for an actuator
    ///////////////////////////////////////////////////////////////////////////////

      if(((TDM_P_COUNTER-TRIGGER_CONS_COMP)%CONS_PERIOD) == 0 ){


	     }//if
    
    ///////////////////////////////////////////////////////////////////////////////
    //Print the received data for debuging
    ///////////////////////////////////////////////////////////////////////////////
      #define DEBUG_PRINT
      #ifdef DEBUG_PRINT

      #define PRINT_ARRAY

          for(int i=0;i<MSG_SIZE;i++){
              debug_print[i] = data_rd[i];

          }
      #endif

	}//for
     

}






int main() {

  #ifdef MEASUREMENT_MODE
  //timeStamps_master[0] = TDM_P_COUNTER; //start master initialization measurement
  #endif

  noc_configure();
  noc_enable();

  unsigned i;
  int slave_param = 1;
  int id = get_cpuid();
  int cnt = get_cpucnt();
  #ifdef MEASUREMENT_MODE
  //timeStamps_master[1] = TDM_P_COUNTER; //stop master initialization measurement
  #endif

  #define PROD_CONS
  #ifdef PROD_CONS
  corethread_create(1, &producer, (void*)slave_param);
  corethread_create(2, &consumer, (void*)slave_param);
  #endif
    
  #define MULTICORE_N
  #ifdef MULTICORE
	  for (i=2; i<cnt; ++i) {
	    int core_id = i; // The core number
	    corethread_create(core_id, &slave, (void*)slave_param);  
	  }
  #endif
  
  printf("Threats are started!\n");

  
for(;;){

  //#define PRINT_ARRAY
  #ifdef PRINT_ARRAY

   for(int i=0;i<MSG_SIZE;i++){
	
      printf("The value at %d is: %d \n",i, debug_print[i]);
  }
  #endif

  #ifdef MEASUREMENT_MODE
  // Producer timing metrics
  printf("-----------Producer Timing Metrics--------------------\n");
  printf("Producer starts at %d TDM cycles\n", timeStamps_slave1[0]);
  printf("Producer end of computation at %d TDM cycles\n", timeStamps_slave1[1]);
  printf("Producer initialization Latency is %d TDM cycles\n", timeStamps_slave1[1]-timeStamps_slave1[0]);
  printf("Producer triggered at %d TDM cycles\n", timeStamps_slave1[2]);
  printf("Producer polls for %d TDM cycles\n", timeStamps_slave1[2]-timeStamps_slave1[1]);
  printf("Producer stops at %d TDM cycles\n", timeStamps_slave1[3]);
  printf("Producer communication Latency is %d TDM cycles\n", timeStamps_slave1[3]-timeStamps_slave1[2]);
  
  // Consumer timing metrics
  printf("-----------Consumer Timing Metrics--------------------\n");
  printf("Consumer starts at %d TDM cycles\n", timeStamps_slave2[0]);
  printf("Consumer end of computation at %d TDM cycles\n", timeStamps_slave2[1]);
  printf("Consumer initialization Latency is %d TDM cycles\n", timeStamps_slave2[1]-timeStamps_slave2[0]);
  printf("Consumer triggered at %d TDM cycles\n", timeStamps_slave2[2]);
  printf("Consumer polls for %d TDM cycles\n", timeStamps_slave2[2]-timeStamps_slave2[1]);
  printf("Consumer stops at %d TDM cycles\n", timeStamps_slave2[3]);
  printf("Consumer communication Latency is %d TDM cycles\n", timeStamps_slave2[3]-timeStamps_slave2[2]);

  //master
  printf("-----------Master Timing Metrics--------------------\n");
  printf("Master starts at %d TDM cycles\n", timeStamps_master[0]);
  printf("The master initialization Latency is %d TDM cycles\n", timeStamps_master[1]-timeStamps_master[0]);
  printf("The End to End latency is %d TDM cycles\n", timeStamps_slave2[2]-timeStamps_master[0]);

  #endif

 }

 return 0;

}
