/*
    This is a multicore program in which a producer-consumer task pair communicating
    time-triggered  with single buffering

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/

const int NOC_MASTER = 0;
#include <stdio.h>
#include <machine/patmos.h>
#include "libnoc/noc.h"
#include "libcorethread/corethread.h"
#include "libmp/mp.h"
#include "libmp/mp_internal.h"

#include "ttcom.h"

// a timer for measuring the execution time
volatile int _SPM *timer_ptr = (volatile int _SPM *) (PATMOS_IO_TIMER+4);

//For printing measurements
volatile _UNCACHED int debug_print_slave1[MEASUREMENT_SIZE]= {0};
volatile _UNCACHED int debug_print_slave2[MEASUREMENT_SIZE]= {0};
volatile _UNCACHED int debug_print_slave1_P[MEASUREMENT_SIZE]= {0}; //for period measurment
volatile _UNCACHED int debug_print_slave2_P[MEASUREMENT_SIZE]= {0}; //for period mesurement
//For printing data
volatile _UNCACHED int debug_print_data[DATA_LEN+BUFFER_SIZE]= {0};
//For printing the overall latency
volatile _UNCACHED int start_transm_slave1= 0;
volatile _UNCACHED int stop_transm_slave2= 0;

//Producer Core
void producer(void *arg) {

  #ifdef MEASUREMENT_MODE   
      // recording timestamps locally in the data SPM
      volatile int _SPM *timeStamps_slave1 = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      volatile int _SPM *timeStamps_slave1_P = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE/2);
      // local pointers for bookkeeping the computation time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+1);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+2);
      // local pointers for bookkeeping the communication time triggering instants
      volatile int _SPM *trigger_comm = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+3);
      volatile int _SPM *next_trigger_comm = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+4);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+5);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_wr = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+6);
  #endif

  #ifdef LATENCY_CALC_MODE
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      // local pointers for bookkeeping the computation time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+1);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+2);
      // local pointers for bookkeeping the communication time triggering instants
      volatile int _SPM *trigger_comm = ((volatile int _SPM *) NOC_SPM_COMP_BASE+3);
      volatile int _SPM *next_trigger_comm = ((volatile int _SPM *) NOC_SPM_COMP_BASE+4);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_wr = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+5);
  #endif

    #ifdef MEASUREMENT_MODE
      timeStamps_slave1[0] = *timer_ptr;
      timeStamps_slave1_P[0] = TDM_P_COUNTER;
    #endif

  int id = get_cpuid();
  ///////////////////////////////////////////////////////////////////////////////
  // -Channel declation at tranmitter side.  
  // -buffer allocations
  // -initial remote buffer address calculations 
  // -no flow control is needed due to single buffering
  ///////////////////////////////////////////////////////////////////////////////

  // buffer allocation into SPM
  qpd_t * chan1 = mp_create_qport(1, SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 

  //remote address calculation at the receiver side
  int rmt_addr_offset = (chan1->buf_size + FLAG_SIZE) * chan1->send_ptr;
  volatile void _IODEV *calc_rmt_addr = &chan1->recv_addr[rmt_addr_offset];

  ///////////////////////////////////////////////////////////////////////////////
  // This section: 
  //  1- syncronizes the tasks by looping around an initial value from TDM counter
  //  2- after syncronization, the tasks take a mesurement from clock cycle counter 
  //     to determine the current instance of time. The error margin between the tasks is 1-2 clock cycles,
  //     that is considered as negligible.
  //  3- adding a large enough offset value (in clock cycles) to to determine the initial triggering time instances for
  //     computation and communication. The offset value is choosen big enough to toletate the cache misses for the 1st loop
  //     iteration, thus, measurements in the 1st loop iteration are garbage
  //     
  ///////////////////////////////////////////////////////////////////////////////

   //Sync the tasks by Busy waiting over a TDM counter value
   while( !( TDM_P_COUNTER >= SYNC_INIT ) ){ 
      ;
    }

   // read the clock counter to determine current instance of time
   *next_trigger_comp = *timer_ptr + 7000 ; //7000cc to tolerate cache misses up to first triggereing point
   *next_trigger_comm = *next_trigger_comp + WCET_COMP ;

    #ifdef MEASUREMENT_MODE
      timeStamps_slave1[1] = *timer_ptr;
      timeStamps_slave1_P[1] = TDM_P_COUNTER; 
    #endif

  for(int i=0;i<LOOP_COUNT+1;i++){ 
        // the first iteration in the loop is dedicated to warming up the cache
        // therefore measurements in the first loop are not considered 

        #ifdef MEASUREMENT_MODE
           timeStamps_slave1[2+(i*5)+0] = *timer_ptr; 
           timeStamps_slave1_P[2+(i*5)+0] = TDM_P_COUNTER;
        #endif

        ///////////////////////////////////////////////////////////////////////////////
        // Bookkeeping for the triggering time instances
        ///////////////////////////////////////////////////////////////////////////////
        
        if(i==0){//for the first loop iteration
              // computation   
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += 6000;// a value large enough to deal with cache misses for the first period
              // communication
              *trigger_comm = *next_trigger_comm;
              *next_trigger_comm += 6000; // a value large enough to deal with cache misses for the first period


        }else{
              //  computation 
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += TRIGGER_PERIOD;
              // communication
              *trigger_comm = *next_trigger_comm;
              *next_trigger_comm += TRIGGER_PERIOD;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // Computation part of the Task. Data production/manipulation
        ///////////////////////////////////////////////////////////////////////////////
        while( !( *timer_ptr >= *trigger_comp ) ){ 
          ;
        }

              // to measure the computation triggering time
              #ifdef MEASUREMENT_MODE
                 timeStamps_slave1[2+(i*5)+1] = *timer_ptr;  
                 timeStamps_slave1_P[2+(i*5)+1] = TDM_P_COUNTER;
              #endif

              // Produce Data into the write buffer.
              for (int j=0;j<MSG_SIZE;j++){
                  *(volatile int _IODEV*)((int*)chan1->write_buf+j) = (i+1);
              }   
          
              #ifdef MEASUREMENT_MODE
                timeStamps_slave1[2+(i*5)+2] = *timer_ptr;  
                timeStamps_slave1_P[2+(i*5)+2] = TDM_P_COUNTER;
              #endif
        

        ///////////////////////////////////////////////////////////////////////////////
        // Communication part of the Task. 
        ///////////////////////////////////////////////////////////////////////////////

        while(  !( *timer_ptr >= *trigger_comm ) ) {
            ;
        }
            #ifdef LATENCY_CALC_MODE
                if(i==1){//Start the latency calculation when the first data is sent
                  *start_transmission = *timer_ptr; 
                }
            #endif

            // to measure the communication triggering time
            #ifdef MEASUREMENT_MODE
                if(i==1){
                  *start_transmission = *timer_ptr; 
                }
               timeStamps_slave1[2+(i*5)+3] = *timer_ptr;  
               timeStamps_slave1_P[2+(i*5)+3] = TDM_P_COUNTER;
            #endif

            //nonblocking write transaction, will fail if the DMA controller is not available.
            // assuming that DMA available
        	  noc_nbwrite( (id+1),calc_rmt_addr,chan1->write_buf,chan1->buf_size + FLAG_SIZE, 0);

            // to measure the latency of the code section of the communication 
            #ifdef MEASUREMENT_MODE
        	  	  timeStamps_slave1[2+(i*5)+4] = *timer_ptr;  
                timeStamps_slave1_P[2+(i*5)+4] = TDM_P_COUNTER;
        	  #endif

   }//for


   ///////////////////////////////////////////////////////////////////////////////
   //copy the measurement values from local memory to global main-mem for print debugging
   ///////////////////////////////////////////////////////////////////////////////
        
        #ifdef MEASUREMENT_MODE
            start_transm_slave1 = *start_transmission;
            for(int i=0;i<MEASUREMENT_SIZE;i++){
                debug_print_slave1[i] = timeStamps_slave1[i];
                debug_print_slave1_P[i] = timeStamps_slave1_P[i];

            }
        #endif

        #ifdef LATENCY_CALC_MODE
              start_transm_slave1 = *start_transmission;
        #endif

}


// Consumer Core
void consumer(void *arg) {

  #ifdef MEASUREMENT_MODE
      // recording timestamps locally in the data SPM 
      volatile int _IODEV *timeStamps_slave2 = ((volatile int _IODEV *) NOC_SPM_COMP_BASE);
      volatile int _IODEV *timeStamps_slave2_P = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE/2);
      // local pointers for bookkeeping the computation time triggering instants
      volatile int _IODEV *trigger_comp = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+1);
      volatile int _IODEV *next_trigger_comp = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+2);
      // local pointers for bookkeeping the communication time triggering instants
      volatile int _IODEV *trigger_comm = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+3);
      volatile int _IODEV *next_trigger_comm = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+4);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *stop_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+5);
      // Data pointer to read buffer
      volatile int _IODEV *data_rd = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+6);
  #endif

  #ifdef LATENCY_CALC_MODE
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *stop_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      // local pointers for bookkeeping the computation time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+1);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+2);
      // local pointers for bookkeeping the communication time triggering instants
      volatile int _SPM *trigger_comm = ((volatile int _SPM *) NOC_SPM_COMP_BASE+3);
      volatile int _SPM *next_trigger_comm = ((volatile int _SPM *) NOC_SPM_COMP_BASE+4);
      // Data pointer to read buffer
      volatile int _SPM *data_rd = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+5);
  #endif

  #ifdef MEASUREMENT_MODE
    timeStamps_slave2[0] = *timer_ptr; 
    timeStamps_slave2_P[0] = TDM_P_COUNTER; 
  #endif

  ///////////////////////////////////////////////////////////////////////////////
  // -Channel declation at tranmitter side.  
  // -buffer allocations
  // -initial remote buffer address calculations 
  // -no flow control is needed due to single buffering
  ///////////////////////////////////////////////////////////////////////////////

  // buffer allocation into SPM
  qpd_t * chan1 = mp_create_qport(1, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 

  ///////////////////////////////////////////////////////////////////////////////
  // This section: 
  //  1- syncronizes the tasks by looping around an initial value from TDM counter
  //  2- after syncronization, the tasks take a mesurement from clock cycle counter 
  //     to determine the current instance of time. The error margin between the tasks is 1-2 clock cycles,
  //     that is considered as negligible.
  //  3- adding a large enough offset value (in clock cycles) to to determine the initial triggering time instances for
  //     computation and communication. The offset value is choosen big enough to toletate the cache misses for the 1st loop
  //     iteration, thus, measurements in the 1st loop iteration are garbage
  //     
  ///////////////////////////////////////////////////////////////////////////////

   //Sync the tasks by Busy waiting over a TDM counter value
   while( !( TDM_P_COUNTER >= SYNC_INIT ) ){ //sync
      ;
   }

   // read the clock counter to determine current instance of time
   *next_trigger_comp = *timer_ptr + 7000 ; //7000cc to tolerate cache misses up to first triggereing point
   *next_trigger_comm = *next_trigger_comp + WCET_COMP ;


     #ifdef MEASUREMENT_MODE
      timeStamps_slave2[1] = *timer_ptr; 
      timeStamps_slave2_P[1] = TDM_P_COUNTER;
     #endif

  for(int i=0;i<LOOP_COUNT+2;i++){ 
     // LOOP_COUNT+2 as the consumer should run one more period than the producer to get the last message sent 
      // the first iteration in the loop is dedicated to warming up the cache
      // therefore measurements in the first loop are not considered 

      #ifdef MEASUREMENT_MODE
         timeStamps_slave2[2+(i*5)+0] = *timer_ptr; 
         timeStamps_slave2_P[2+(i*5)+0] = TDM_P_COUNTER;
      #endif

        ///////////////////////////////////////////////////////////////////////////////
        // Bookkeeping for the triggering time instances
        ///////////////////////////////////////////////////////////////////////////////
        if(i==0){
              //Computation
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += 6000;
              // communication
              *trigger_comm = *next_trigger_comm;
              *next_trigger_comm += 6000;

        }else{
              //  computation 
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += TRIGGER_PERIOD;
              // communication
              *trigger_comm = *next_trigger_comm;
              *next_trigger_comm += TRIGGER_PERIOD;
        }

      ///////////////////////////////////////////////////////////////////////////////
      // Computation part of the Task. Data production/manipulation
      ///////////////////////////////////////////////////////////////////////////////
      
      while( !( *timer_ptr >= *trigger_comp ) ){ 
        ;
      }     //Stop the latency calculation when the last data is received
            #ifdef LATENCY_CALC_MODE
                if(i==LOOP_COUNT+1){
                  *stop_transmission = *timer_ptr; 
                }
            #endif
         
            #ifdef MEASUREMENT_MODE
                if(i==LOOP_COUNT+1){
                  *stop_transmission = *timer_ptr; 
                }
                timeStamps_slave2[2+(i*5)+1] = *timer_ptr; 
                timeStamps_slave2_P[2+(i*5)+1] = TDM_P_COUNTER;
            #endif

      		  // consuming/reading the data from the read buffers into local SPM
      	  	for (int j=0;j<MSG_SIZE;j++){

              #ifdef DATA_CHECK_MODE
      	  	      data_rd[j+i*MSG_SIZE] = *(volatile int _IODEV*)((int*)chan1->read_buf+j); // for printing
              #else
                  data_rd[j] = *(volatile int _IODEV*)((int*)chan1->read_buf+j); 
              #endif

      	  	}

            #ifdef MEASUREMENT_MODE
               timeStamps_slave2[2+(i*5)+2] = *timer_ptr;  
               timeStamps_slave2_P[2+(i*5)+2] = TDM_P_COUNTER;
            #endif
        

      ///////////////////////////////////////////////////////////////////////////////
      // Communication  part of the Task. Perhaps for an actuator
      ///////////////////////////////////////////////////////////////////////////////

       while( !( *timer_ptr >= *trigger_comm) ){ 
        ;
      }
            #ifdef MEASUREMENT_MODE
               timeStamps_slave2[2+(i*5)+3] = *timer_ptr;  
               timeStamps_slave2_P[2+(i*5)+3] = TDM_P_COUNTER;
            #endif

            // do something usefull here (Computation). 

            #ifdef MEASUREMENT_MODE
               timeStamps_slave2[2+(i*5)+4] = *timer_ptr;  
               timeStamps_slave2_P[2+(i*5)+4] = TDM_P_COUNTER;
            #endif

	}//for

      ///////////////////////////////////////////////////////////////////////////////
      //Print the received data for debuging
      ///////////////////////////////////////////////////////////////////////////////
        #ifdef DATA_CHECK_MODE
            for(int i=0;i<(DATA_LEN+BUFFER_SIZE);i++){
                debug_print_data[i] = data_rd[i];
            }
        #endif

   ///////////////////////////////////////////////////////////////////////////////
   //copy the measurement values from local memory to global main-mem for print debugging
   ///////////////////////////////////////////////////////////////////////////////
        #ifdef MEASUREMENT_MODE

            for(int i=0;i<MEASUREMENT_SIZE;i++){
                debug_print_slave2[i] = timeStamps_slave2[i];
                debug_print_slave2_P[i] = timeStamps_slave2_P[i];
            }
            stop_transm_slave2 = *stop_transmission;
        #endif

        #ifdef LATENCY_CALC_MODE
              stop_transm_slave2 = *stop_transmission;
        #endif

}




int main() {

  noc_configure();
  noc_enable();

  int slave_param = 1;
  int id = get_cpuid();
  int cnt = get_cpucnt();

  #ifdef MEASUREMENT_MODE
    timeStamps_master[1] = *timer_ptr;
    timeStamps_master_P[1] = TDM_P_COUNTER; 
  #endif

    corethread_create(1, &producer, (void*)slave_param);
    corethread_create(2, &consumer, (void*)slave_param);
    
    corethread_join(1, (void*)slave_param);
    corethread_join(2, (void*)slave_param);

  //Print the measurements
  #ifdef MEASUREMENT_MODE

        printf("------Latency Metrics--------------------\n");
        printf("The End-to-End Latency : %d/ Clock Cycle,\
                 for DATA_LEN=%d/words \
          and BUFFER_SIZE:%d/words \n", stop_transm_slave2-start_transm_slave1, DATA_LEN, BUFFER_SIZE);
        printf("The End-to-End Latency for DATA_LEN %d words of data \
          and %d words of BUFFER_SIZE:  %d  PERIOD \n", DATA_LEN, BUFFER_SIZE, debug_print_slave2[2+(LOOP_COUNT*5)+1]-debug_print_slave1[10]);

        printf("--------------------------------------------------------------\n");

        printf("------Period SLAVE 1--------------------\n");
        printf("T0 \t (Task Created) \t = %d PERIOD \n",debug_print_slave1_P[0]);
        printf("T0 \t (Task Created) \t = %d CC \n",debug_print_slave1[0]);
        printf("T1 \t (End of Initializations)\t = %d PERIOD \n",debug_print_slave1_P[1]);
        printf("T1 \t (End of Initializations)\t = %d CC \n",debug_print_slave1[1]);
            for(int j=0;j<LOOP_COUNT;j++){
                  printf(" \t------Message %d--------------------\n",j+1);
                  printf("T%d \t (After FOR loop) \t= %d PERIOD \n",(2+j*5+0),debug_print_slave1_P[2+j*5+0]);
                  printf("T%d \t (After FOR loop) \t= %d CC \n",(2+j*5+0),debug_print_slave1[2+j*5+0]);

                  printf("T%d \t (Trigger Computation) \t = %d PERIOD \n",(2+j*5+1),debug_print_slave1_P[2+j*5+1]);
                  printf("T%d \t (Trigger Computation) \t = %d CC \n",(2+j*5+1),debug_print_slave1[2+j*5+1]);

                  printf("T%d \t (End of Computation)\t= %d PERIOD \n",(2+j*5+2),debug_print_slave1_P[2+j*5+2]);
                  printf("T%d \t (End of Computation)\t= %d CC \n",(2+j*5+2),debug_print_slave1[2+j*5+2]);

                  printf("- \t (Computation Duration) \t = %d PERIOD \n",debug_print_slave1_P[2+j*5+2]-debug_print_slave1_P[2+j*5+1]);
                  printf("- \t (Computation Duration) \t = %d CC \n",debug_print_slave1[2+j*5+2]-debug_print_slave1[2+j*5+1]);

                  printf("T%d \t (Trigger Communication) \t = %d PERIOD \n",(2+j*5+3),debug_print_slave1_P[2+j*5+3]);
                  printf("T%d \t (Trigger Communication) \t = %d CC \n",(2+j*5+3),debug_print_slave1[2+j*5+3]);

                  printf("T%d \t (End of Communication) \t = %d PERIOD \n",(2+j*5+4),debug_print_slave1_P[2+j*5+4]);
                  printf("T%d \t (End of Communication) \t = %d CC \n",(2+j*5+4),debug_print_slave1[2+j*5+4]);

                  printf("- \t (Communication Duration) \t = %d PERIOD \n",debug_print_slave1_P[2+j*5+4]-debug_print_slave1_P[2+j*5+3]);
                  printf("- \t (Communication Duration) \t = %d CC \n",debug_print_slave1[2+j*5+4]-debug_print_slave1[2+j*5+3]);
              
              }

        printf("------Period SLAVE 2--------------------\n");
        printf("T0 \t (Task Created) \t = %d PERIOD \n",debug_print_slave2_P[0]);
        printf("T0 \t (Task Created) \t = %d CC \n",debug_print_slave2[0]);

        printf("T1 \t (End of Initializations)\t = %d PERIOD \n",debug_print_slave2_P[1]);
        printf("T1 \t (End of Initializations)\t = %d CC \n",debug_print_slave2[1]);

            for(int j=0;j<LOOP_COUNT+1;j++){
                  printf("\t------Message %d--------------------\n",j+1);
                  printf("T%d \t (After FOR loop) \t= %d PERIOD \n",(2+j*5+0),debug_print_slave2_P[2+j*5+0]);
                  printf("T%d \t (After FOR loop) \t= %d CC \n",(2+j*5+0),debug_print_slave2[2+j*5+0]);

                  printf("T%d \t (Trigger Computation) \t = %d PERIOD \n",(2+j*5+1),debug_print_slave2_P[2+j*5+1]);
                  printf("T%d \t (Trigger Computation) \t = %d CC \n",(2+j*5+1),debug_print_slave2[2+j*5+1]);


                  printf("T%d \t (End of Computation)\t= %d PERIOD \n",(2+j*5+2),debug_print_slave2_P[2+j*5+2]);
                  printf("T%d \t (End of Computation)\t= %d CC \n",(2+j*5+2),debug_print_slave2[2+j*5+2]);

                  printf("- \t (Computation Duration) \t = %d PERIOD \n",debug_print_slave2_P[2+j*5+2]-debug_print_slave2_P[2+j*5+1]);
                  printf("- \t (Computation Duration) \t = %d CC \n",debug_print_slave2[2+j*5+2]-debug_print_slave2[2+j*5+1]);

                  printf("T%d \t (Trigger Communication) \t = %d PERIOD \n",(2+j*5+3),debug_print_slave2_P[2+j*5+3]);
                  printf("T%d \t (Trigger Communication) \t = %d CC \n",(2+j*5+3),debug_print_slave2[2+j*5+3]);

                  printf("T%d \t (End of Communication) \t = %d PERIOD \n",(2+j*5+4),debug_print_slave2_P[2+j*5+4]);
                  printf("T%d \t (End of Communication) \t = %d CC \n",(2+j*5+4),debug_print_slave2[2+j*5+4]);

                  printf("- \t (Communication Duration) \t = %d PERIOD \n",debug_print_slave2_P[2+j*5+4]-debug_print_slave2_P[2+j*5+3]);
                  printf("- \t (Communication Duration) \t = %d CC \n",debug_print_slave2[2+j*5+4]-debug_print_slave2[2+j*5+3]);
              }

        printf("--------------------------------------------------------------\n");
    #endif

//Print the data received by the consumer
  #ifdef DATA_CHECK_MODE
     printf("Data received at the Consumer Side: \n");
     printf("--------------------------\n");
     for(int i=0;i<(DATA_LEN+BUFFER_SIZE);i++){
        printf("Data at %d is: %d \n",i, debug_print_data[i]);     
     }
  #endif

  #ifdef LATENCY_CALC_MODE

      printf("------Analitical Calculation of Latency --------------------\n");
      int analitical_latency=(DATA_LEN/BUFFER_SIZE*TRIGGER_PERIOD)-WCET_COMP;
      printf("The End-to-End Latency= %d/Clock Cycle, \
          for DATA_LEN=%d/words \
          and BUFFER_SIZE:%d/words \n", analitical_latency ,DATA_LEN, BUFFER_SIZE);// "-1" is use
      printf("------Latency by Measurements--------------------\n");
      int measurement_latency =stop_transm_slave2-start_transm_slave1;
      printf("The End-to-End Latency= %d/Clock Cycle, \
          for DATA_LEN=%d/words \
          and BUFFER_SIZE:%d/words \n",  measurement_latency,DATA_LEN, BUFFER_SIZE);
      printf("------Throughput Calculation from Measurements--------------------\n");
      printf("The Throughput= %d.%d/Clock Cycle/words, \
          for DATA_LEN=%d/words \
          and BUFFER_SIZE:%d/words \n", measurement_latency/DATA_LEN ,measurement_latency*10/DATA_LEN%10 ,DATA_LEN, BUFFER_SIZE);
  #endif

 return 0;

}
