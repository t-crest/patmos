/*
    This is a multicore program in which a pipelined chain of tasks
    communicate in a time-triggered fashion
    using double buffering

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/

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
volatile _UNCACHED int debug_print_slave3[MEASUREMENT_SIZE]= {0};
volatile _UNCACHED int debug_print_slave1_P[MEASUREMENT_SIZE]= {0}; 
volatile _UNCACHED int debug_print_slave2_P[MEASUREMENT_SIZE]= {0};
volatile _UNCACHED int debug_print_slave3_P[MEASUREMENT_SIZE]= {0};  
//For printing the data
volatile _UNCACHED int debug_print_data[DATA_LEN+BUFFER_SIZE*5]= {0};
//For printing the overall latency
volatile _UNCACHED int start_transm_slave1= 0;
volatile _UNCACHED int stop_transm_slave3= 0;

//Producer Core
void producer(void *arg) {

  #ifdef MEASUREMENT_MODE   
      // recording timestamps locally in the data SPM
      volatile int _SPM *timeStamps_slave1 = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      volatile int _SPM *timeStamps_slave1_P = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE/2);
      // local pointers for bookkeeping the time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+1);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+2);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+3);
      //receiver address calculation at the receiver buffer
      volatile int _SPM *rmt_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+4);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_wr = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+5);
  #endif

  #ifdef LATENCY_CALC_MODE
      // local pointers for bookkeeping the time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+1);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+2);
      //receiver address calculation at the receiver buffer
      volatile int _SPM *rmt_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+3);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_wr = ((volatile int _SPM *) NOC_SPM_COMP_BASE+4);
  #endif

    #ifdef MEASUREMENT_MODE
      timeStamps_slave1[0] = *timer_ptr;
      timeStamps_slave1_P[0] = TDM_P_COUNTER;
    #endif

  int id = get_cpuid();
  ///////////////////////////////////////////////////////////////////////////////
  // -Channel declation at tranmitter side.  
  // -buffer allocations
  // -remote buffer address calculations (at the receiver side)
  ///////////////////////////////////////////////////////////////////////////////

  // buffer allocation into SPM for the channel
  qpd_t * chan1 = mp_create_qport(1, SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 

  //initial receiver address calculation at the receiver buffer
  *rmt_addr_offset = (chan1->buf_size + FLAG_SIZE) * chan1->send_ptr;
  volatile void _SPM *calc_rmt_addr = &chan1->recv_addr[*rmt_addr_offset];

  ///////////////////////////////////////////////////////////////////////////////
  // In this section: 
  //  1- syncronizes the tasks before entering to the loop. For the syncronization TDM period counter
  //     is used due to its courser granularity.
  //  2- after syncronization, tasks take a measurement sample from clock cycle counter 
  //     to determine the current instance of time. 
  //     The error margin between the tasks is 1-2 clock cycles,that is considered as negligible.
  //  3- adding a large enough offset value (7000 clock cycles) to cover possible cache misses till the
  //     1st triggering time instance.
  //  4- the first iteration of the for loop is dedicated to warming up the cache
  //     therefore measurements taken in the first loop iteration are not considered. 
  //     For the first loop iteration time triggering period is taken large enough (5000 Clock cycles)
  //     to cover possible cache misses.
  //     
  ///////////////////////////////////////////////////////////////////////////////

   //Sync the tasks by Busy waiting over a a statically determined TDM counter value
   while( !( TDM_P_COUNTER >= SYNC_INIT ) ){ 
      ;
    }

   // read the clock counter to determine current instance of time
   *next_trigger_comp = *timer_ptr + 7000 ; //7000cc to tolerate cache misses up to first triggereing point

    #ifdef MEASUREMENT_MODE
      timeStamps_slave1[1] = *timer_ptr;
      timeStamps_slave1_P[1] = TDM_P_COUNTER; 
    #endif

  for(int i=0;i<LOOP_COUNT+2;i++){ 
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
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += 10000;// a value large enough to deal with cache misses for the iteration of the loop
        }else{
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += GLOBAL_PERIOD;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // Trigger the Computation/Communication. Data production/manipulation
        ///////////////////////////////////////////////////////////////////////////////
        while( !( *timer_ptr >= *trigger_comp ) ){ 
          ;
        }
           //Start the latency calculation when the first data is sent
            #ifdef LATENCY_CALC_MODE
                if(i==1){
                  *start_transmission = *timer_ptr; 
                }
            #endif

            #ifdef MEASUREMENT_MODE
                if(i==1){
                  *start_transmission = *timer_ptr; 
                }
               timeStamps_slave1[2+(i*5)+1] = *timer_ptr;  
               timeStamps_slave1_P[2+(i*5)+1] = TDM_P_COUNTER;
            #endif

              //Setting the dma controller
              //nonblocking write transaction, will fail if the DMA controller is not available.
              // Time-triggering commucation guarantees that the DMA is available
              noc_nbwrite( (id+1),calc_rmt_addr,chan1->write_buf,chan1->buf_size + FLAG_SIZE, 0);


              #ifdef MEASUREMENT_MODE
                timeStamps_slave1[2+(i*5)+2] = *timer_ptr;  
                timeStamps_slave1_P[2+(i*5)+2] = TDM_P_COUNTER;
              #endif

              ///////////////////////////////////////////////////////////////////////////////
              // Swap input/output buffer pointers
              ///////////////////////////////////////////////////////////////////////////////  

              // Swap the send pointer 
              if (chan1->send_ptr == chan1->num_buf-1) {
                chan1->send_ptr = 0;
              } else {
                chan1->send_ptr++;  
              }
              //calculate the receiver buffer address 
              *rmt_addr_offset = (chan1->buf_size + FLAG_SIZE) * chan1->send_ptr;
              calc_rmt_addr = &chan1->recv_addr[*rmt_addr_offset];
              //Swap the write (output) buffer pointers
              volatile void _SPM * tmp = chan1->write_buf;
              chan1->write_buf = chan1->shadow_write_buf;
              chan1->shadow_write_buf = tmp;

              #ifdef MEASUREMENT_MODE
                timeStamps_slave1[2+(i*5)+3] = *timer_ptr;  
                timeStamps_slave1_P[2+(i*5)+3] = TDM_P_COUNTER;
              #endif

              ///////////////////////////////////////////////////////////////////////////////
              // Computation.Produce Data.
              /////////////////////////////////////////////////////////////////////////////// 

              // Produce Data into the write buffer.
              // 3 MAC operations for every word in the message.
              for (int j=0;j<MSG_SIZE;j++){

                  for (int k=0;k<3;k++){

                       *(volatile int _IODEV*)((int*)chan1->write_buf+j) += (j+1)*(k+1); 

                  }
              }   
          
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




//intermediate Core
void intermediate(void *arg) {

  #ifdef MEASUREMENT_MODE   
      // recording timestamps locally in the data SPM
      volatile int _SPM *timeStamps_slave2 = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      volatile int _SPM *timeStamps_slave2_P = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE/2);
      // local pointers for bookkeeping the time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+1);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+2);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+3);
      //receiver address calculation at the receiver buffer
      volatile int _SPM *rmt_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+4);
             // For Calculating the address of the local receiving buffer
      volatile int _SPM *locl_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+5);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_rd = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+6);
  #endif

  #ifdef LATENCY_CALC_MODE
      // local pointers for bookkeeping the time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+1);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+2);
      //receiver address calculation at the receiver buffer
      volatile int _SPM *rmt_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+3);
            // For Calculating the address of the local receiving buffer
      volatile int _SPM *locl_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+4);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_rd = ((volatile int _SPM *) NOC_SPM_COMP_BASE+5);
  #endif

    #ifdef MEASUREMENT_MODE
      timeStamps_slave2[0] = *timer_ptr;
      timeStamps_slave2_P[0] = TDM_P_COUNTER;
    #endif

  int id = get_cpuid();
  ///////////////////////////////////////////////////////////////////////////////
  // -Channel declation at tranmitter side.  
  // -buffer allocations
  // -remote buffer address calculations (at the receiver side)
  ///////////////////////////////////////////////////////////////////////////////

  // buffer allocation into SPM for the channel
  qpd_t * chan1 = mp_create_qport(1, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);

  qpd_t * chan2 = mp_create_qport(2, SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 

  //set initial Output (write) buffer pointers
  *rmt_addr_offset = (chan2->buf_size + FLAG_SIZE) * chan2->send_ptr;
  volatile void _SPM *calc_rmt_addr = &chan2->recv_addr[*rmt_addr_offset];


  //set initial Input(read) buffer pointers
  //chan1->recv_ptr = (MP_CHAN_NUM_BUF-1);
  //*locl_addr_offset = (chan1->buf_size + FLAG_SIZE) * chan1->recv_ptr;
  //volatile void _SPM *calc_locl_addr = &chan1->recv_addr[*locl_addr_offset];
  // Set the new read buffer pointer
  //chan1->read_buf = calc_locl_addr;

  ///////////////////////////////////////////////////////////////////////////////
  // In this section: 
  //  1- syncronizes the tasks before entering to the loop. For the syncronization TDM period counter
  //     is used due to its courser granularity.
  //  2- after syncronization, tasks take a measurement sample from clock cycle counter 
  //     to determine the current instance of time. 
  //     The error margin between the tasks is 1-2 clock cycles,that is considered as negligible.
  //  3- adding a large enough offset value (7000 clock cycles) to cover possible cache misses till the
  //     1st triggering time instance.
  //  4- the first iteration of the for loop is dedicated to warming up the cache
  //     therefore measurements taken in the first loop iteration are not considered. 
  //     For the first loop iteration time triggering period is taken large enough (5000 Clock cycles)
  //     to cover possible cache misses.
  //     
  ///////////////////////////////////////////////////////////////////////////////

   //Sync the tasks by Busy waiting over a TDM counter value
   while( !( TDM_P_COUNTER >= SYNC_INIT ) ){ 
      ;
    }

   // read the clock counter to determine current instance of time
   *next_trigger_comp = *timer_ptr + 7000 ; //7000cc to tolerate cache misses up to first triggereing point

    #ifdef MEASUREMENT_MODE
      timeStamps_slave2[1] = *timer_ptr;
      timeStamps_slave2_P[1] = TDM_P_COUNTER; 
    #endif

  for(int i=0;i<LOOP_COUNT+4;i++){ 
        // the first iteration in the loop is dedicated to warming up the cache
        // therefore measurements in the first loop are not considered  

        #ifdef MEASUREMENT_MODE
           timeStamps_slave2[2+(i*5)+0] = *timer_ptr; 
           timeStamps_slave2_P[2+(i*5)+0] = TDM_P_COUNTER;
        #endif

        ///////////////////////////////////////////////////////////////////////////////
        // Bookkeeping for the triggering time instances
        ///////////////////////////////////////////////////////////////////////////////
        
        if(i==0){//for the first loop iteration   
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += 10000;// a value large enough to deal with cache misses for the iteration of the loop
        }else{
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += GLOBAL_PERIOD;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // Trigger the Computation/Communication. Data production/manipulation
        ///////////////////////////////////////////////////////////////////////////////
        while( !( *timer_ptr >= *trigger_comp ) ){ 
          ;
        }

            #ifdef MEASUREMENT_MODE
               timeStamps_slave2[2+(i*5)+1] = *timer_ptr;  
               timeStamps_slave2_P[2+(i*5)+1] = TDM_P_COUNTER;
            #endif

              //Setting the dma controller
              //nonblocking write transaction, will fail if the DMA controller is not available.
              // Time-triggering commucation guarantees that the DMA is available
              noc_nbwrite( (id+1),calc_rmt_addr,chan2->write_buf,chan2->buf_size + FLAG_SIZE, 0);


              #ifdef MEASUREMENT_MODE
                timeStamps_slave2[2+(i*5)+2] = *timer_ptr;  
                timeStamps_slave2_P[2+(i*5)+2] = TDM_P_COUNTER;
              #endif


              ///////////////////////////////////////////////////////////////////////////////
              // Swap Output Buffer pointers
              ///////////////////////////////////////////////////////////////////////////////  

              // Swap the send pointer
              if (chan2->send_ptr == chan2->num_buf-1) {
                chan2->send_ptr = 0;
              } else {
                chan2->send_ptr++;  
              }
              //calculate the receiver buffer address 
              *rmt_addr_offset = (chan2->buf_size + FLAG_SIZE) * chan2->send_ptr;
              calc_rmt_addr = &chan2->recv_addr[*rmt_addr_offset];
              // Swap the write buffer pointer
              volatile void _SPM * tmp = chan2->write_buf;
              chan2->write_buf = chan2->shadow_write_buf;
              chan2->shadow_write_buf = tmp;

              ///////////////////////////////////////////////////////////////////////////////
              // Swap Input Buffer pointers
              /////////////////////////////////////////////////////////////////////////////// 

                // Swap the receive pointer
              if (chan1->recv_ptr == chan1->num_buf-1) {
                chan1->recv_ptr = 0;
              } else {
                chan1->recv_ptr++;
              }

               // Calculate the address of the local receiving buffer
              *locl_addr_offset = (chan1->buf_size + FLAG_SIZE) * chan1->recv_ptr;
              volatile void _SPM * calc_locl_addr = &chan1->recv_addr[*locl_addr_offset];

              // Set the new read buffer pointer
              chan1->read_buf = calc_locl_addr;


              #ifdef MEASUREMENT_MODE
                 timeStamps_slave2[2+(i*5)+3] = *timer_ptr;  
                 timeStamps_slave2_P[2+(i*5)+3] = TDM_P_COUNTER;
              #endif

              ///////////////////////////////////////////////////////////////////////////////
              // Computation.Produce Data.
              /////////////////////////////////////////////////////////////////////////////// 

              // Produce Data into the write buffer.
              // 3 MAC operations for every word in the message.
              for (int j=0;j<MSG_SIZE;j++){

                  for (int k=0;k<3;k++){

                       *(volatile int _IODEV*)((int*)chan2->write_buf+j) += (*(volatile int _IODEV*)((int*)chan1->read_buf+j))*(k+1); 

                  }
              }  

              #ifdef MEASUREMENT_MODE
                timeStamps_slave2[2+(i*5)+4] = *timer_ptr;  
                timeStamps_slave2_P[2+(i*5)+4] = TDM_P_COUNTER;
              #endif

   }

      ///////////////////////////////////////////////////////////////////////////////
      //copy the measurement values from local memory to global main-mem for print debugging
      ///////////////////////////////////////////////////////////////////////////////
        
        #ifdef MEASUREMENT_MODE

            for(int i=0;i<MEASUREMENT_SIZE;i++){
                debug_print_slave2[i] = timeStamps_slave2[i];
                debug_print_slave2_P[i] = timeStamps_slave2_P[i];
            }
        #endif
}




// Consumer Core
void consumer(void *arg) {

  #ifdef MEASUREMENT_MODE
      // recording timestamps locally in the data SPM 
      volatile int _SPM *timeStamps_slave2 = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      volatile int _SPM *timeStamps_slave2_P = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE/2);
       // local pointers for bookkeeping the time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+1);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+2);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *stop_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+3);
       // For Calculating the address of the local receiving buffer
      volatile int _SPM *locl_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+4);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_rd = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+5);
  #endif

  #ifdef LATENCY_CALC_MODE

      // local pointers for bookkeeping the time triggering instants
      volatile int _SPM *trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
      volatile int _SPM *next_trigger_comp = ((volatile int _SPM *) NOC_SPM_COMP_BASE+1);
      //To calculate overall latency for the bulk data communication
      volatile int _SPM *stop_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+2);
      // For Calculating the address of the local receiving buffer
      volatile int _SPM *locl_addr_offset = ((volatile int _SPM *) NOC_SPM_COMP_BASE+3);
      // a pointer on the local SPM for handling the data manupulation locally 
      volatile int _SPM *data_rd = ((volatile int _SPM *) NOC_SPM_COMP_BASE+4);
  #endif

  #ifdef MEASUREMENT_MODE

    timeStamps_slave2[0] = *timer_ptr; 
    timeStamps_slave2_P[0] = TDM_P_COUNTER; 
  #endif


  ///////////////////////////////////////////////////////////////////////////////
  // - Channel decleration at the receiver side.  
  // - buffer allocations
  // 
  ///////////////////////////////////////////////////////////////////////////////
  // buffer allocation into SPM
  qpd_t * chan2 = mp_create_qport(2, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 

    //initial receiver local address calculation 
  chan2->recv_ptr = (MP_CHAN_NUM_BUF-1);
  *locl_addr_offset = (chan2->buf_size + FLAG_SIZE) * chan2->recv_ptr;
  volatile void _SPM *calc_locl_addr = &chan2->recv_addr[*locl_addr_offset];
  // Set the new read buffer pointer
  chan2->read_buf = calc_locl_addr;

  ///////////////////////////////////////////////////////////////////////////////
  // In this section: 
  //  1- syncronizes the tasks before entering to the loop. For the syncronization TDM period counter
  //     is used due to its courser granularity.
  //  2- after syncronization, tasks take a measurement sample from clock cycle counter 
  //     to determine the current instance of time. 
  //     The error margin between the tasks is 1-2 clock cycles,that is considered as negligible.
  //  3- adding a large enough offset value (7000 clock cycles) to cover possible cache misses till the
  //     1st triggering time instance.
  //  4- the first iteration of the for loop is dedicated to warming up the cache
  //     therefore measurements taken in the first loop iteration are not considered. 
  //     For the first loop iteration time triggering period is taken large enough (5000 Clock cycles)
  //     to cover possible cache misses.
  //     
  ///////////////////////////////////////////////////////////////////////////////

   //Sync the tasks by Busy waiting over a TDM counter value
   while( !( TDM_P_COUNTER >= SYNC_INIT ) ){ 
      ;
   }

   // read the clock counter counter to determine current instance of time
   *next_trigger_comp = *timer_ptr + 7000 ; //7000cc to tolerate cache misses up to first triggereing point


     #ifdef MEASUREMENT_MODE
      timeStamps_slave2[1] = *timer_ptr; 
      timeStamps_slave2_P[1] = TDM_P_COUNTER;
     #endif

  for(int i=0;i<LOOP_COUNT+5;i++){ 
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
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += 10000; // a value large enough to deal with cache misses for the first iteration of the loop

        }else{
              *trigger_comp = *next_trigger_comp;
              *next_trigger_comp += GLOBAL_PERIOD;
        }

      ///////////////////////////////////////////////////////////////////////////////
      // Trigger the Computation/Communication. Data production/manipulation
      ///////////////////////////////////////////////////////////////////////////////
      
      while( !( *timer_ptr >= *trigger_comp ) ){ 
        ;
      }
            //Stop the latency calculation when the last data appears in the receiver buffer
            #ifdef LATENCY_CALC_MODE
                if(i==LOOP_COUNT+4){
                  *stop_transmission = *timer_ptr; 
                }
            #endif

         
            #ifdef MEASUREMENT_MODE
                if(i==LOOP_COUNT+4){
                  *stop_transmission = *timer_ptr; 
                }
                timeStamps_slave2[2+(i*5)+1] = *timer_ptr; 
                timeStamps_slave2_P[2+(i*5)+1] = TDM_P_COUNTER;
            #endif
        
            ///////////////////////////////////////////////////////////////////////////////
            // Computation.Consume Data.
            /////////////////////////////////////////////////////////////////////////////// 

      		  // consuming/reading the data from the read buffers into local SPM
      	  	for (int j=0;j<MSG_SIZE;j++){

              #ifdef DATA_CHECK_MODE
      	  	      data_rd[j+i*MSG_SIZE] = *(volatile int _IODEV*)((int*)chan2->read_buf+j);// for debug printing 
              #else
                  data_rd[j] = *(volatile int _IODEV*)((int*)chan2->read_buf+j); 
              #endif

      	  	}

            #ifdef MEASUREMENT_MODE
               timeStamps_slave2[2+(i*5)+2] = *timer_ptr;  
               timeStamps_slave2_P[2+(i*5)+2] = TDM_P_COUNTER;
            #endif

            ///////////////////////////////////////////////////////////////////////////////
            // Swap buffer pointers
            /////////////////////////////////////////////////////////////////////////////// 

              // Swap the receive pointer
            if (chan2->recv_ptr == chan2->num_buf-1) {
              chan2->recv_ptr = 0;
            } else {
              chan2->recv_ptr++;
            }

             // Calculate the address of the local receiving buffer
            *locl_addr_offset = (chan2->buf_size + FLAG_SIZE) * chan2->recv_ptr;
            volatile void _SPM * calc_locl_addr = &chan2->recv_addr[*locl_addr_offset];

            // Set the new read buffer pointer
            chan2->read_buf = calc_locl_addr;

            #ifdef MEASUREMENT_MODE
               timeStamps_slave2[2+(i*5)+3] = *timer_ptr;  
               timeStamps_slave2_P[2+(i*5)+3] = TDM_P_COUNTER;
            #endif

  }//for

      ///////////////////////////////////////////////////////////////////////////////
      //Print the received data for debuging
      ///////////////////////////////////////////////////////////////////////////////
        #ifdef DATA_CHECK_MODE
            for(int i=0;i<DATA_LEN+BUFFER_SIZE*5;i++){
                debug_print_data[i] = data_rd[i];
            }
        #endif
        
   ///////////////////////////////////////////////////////////////////////////////
   //copy the measurement values from local memory to global main-mem for print debugging
   ///////////////////////////////////////////////////////////////////////////////
        #ifdef MEASUREMENT_MODE

            for(int i=0;i<MEASUREMENT_SIZE;i++){
                debug_print_slave3[i] = timeStamps_slave2[i];
                debug_print_slave3_P[i] = timeStamps_slave2_P[i];
            }
            stop_transm_slave3 = *stop_transmission;
        #endif

        #ifdef LATENCY_CALC_MODE
              stop_transm_slave3 = *stop_transmission;
        #endif

}



int main() {

  noc_configure();
  noc_enable();

  int slave_param = 1;
  int id = get_cpuid();
  int cnt = get_cpucnt();


    corethread_create(1, &producer, (void*)slave_param);
    corethread_create(2, &intermediate, (void*)slave_param);
    corethread_create(3, &consumer, (void*)slave_param);
    
    corethread_join(1, (void*)slave_param);
    corethread_join(2, (void*)slave_param);
    corethread_join(3, (void*)slave_param);

  //Print the measurements
  #ifdef MEASUREMENT_MODE

        printf("------SLAVE 1--------------------\n");
        printf("T0 \t (Task Created) \t = %d PERIOD \n",debug_print_slave1_P[0]);
        printf("T0 \t (Task Created) \t = %d CC \n",debug_print_slave1[0]);
        printf("T1 \t (End of Initializations)\t = %d PERIOD \n",debug_print_slave1_P[1]);
        printf("T1 \t (End of Initializations)\t = %d CC \n",debug_print_slave1[1]);
            for(int i=0;i<LOOP_COUNT+1;i++){

                  printf(" \t------Message %d--------------------\n", (i+1) );


                  printf("T%d \t (After FOR loop) \t= %d PERIOD \n",(2+i*5+0),debug_print_slave1_P[2+i*5+0]);
                  printf("T%d \t (After FOR loop) \t= %d CC \n",(2+i*5+0),debug_print_slave1[2+i*5+0]);

                  printf("T%d \t (Trigger Period) \t = %d PERIOD \n",(2+i*5+1),debug_print_slave1_P[2+i*5+1]);
                  printf("T%d \t (Trigger Period) \t = %d CC \n",(2+i*5+1),debug_print_slave1[2+i*5+1]);

                  printf("T%d \t (End of 'Send' Code Section)\t= %d PERIOD \n",(2+i*5+2),debug_print_slave1_P[2+i*5+2]);
                  printf("T%d \t (End of 'Send' Code Section)\t= %d CC \n",(2+i*5+2),debug_print_slave1[2+i*5+2]);

                  printf("- \t ('Send' Code Section Duration) \t = %d PERIOD \n",debug_print_slave1_P[2+i*5+2]-debug_print_slave1_P[2+i*5+1]);
                  printf("- \t ('Send' Code Section Duration) \t = %d CC \n",debug_print_slave1[2+i*5+2]-debug_print_slave1[2+i*5+1]);

                  printf("T%d \t (End of Flow Control Code Section) \t = %d PERIOD \n",(2+i*5+3),debug_print_slave1_P[2+i*5+3]);
                  printf("T%d \t (End of Flow Control code Section) \t = %d CC \n",(2+i*5+3),debug_print_slave1[2+i*5+3]);

                  printf("- \t (Flow Control Code Duration) \t = %d PERIOD \n",debug_print_slave1_P[2+i*5+3]-debug_print_slave1_P[2+i*5+2]);
                  printf("- \t (Flow Control Code Duration) \t = %d CC \n",debug_print_slave1[2+i*5+3]-debug_print_slave1[2+i*5+2]);

                  printf("T%d \t (End of Computation) \t = %d PERIOD \n",(2+i*5+4),debug_print_slave1_P[2+i*5+4]);
                  printf("T%d \t (End of Computation) \t = %d CC \n",(2+i*5+4),debug_print_slave1[2+i*5+4]);

                  printf("- \t (Computation Duration) \t = %d PERIOD \n",debug_print_slave1_P[2+i*5+4]-debug_print_slave1_P[2+i*5+3]);
                  printf("- \t (Computation Duration) \t = %d CC \n",debug_print_slave1[2+i*5+4]-debug_print_slave1[2+i*5+3]);
              
            }

        printf("------SLAVE 2--------------------\n");
        printf("T0 \t (Task Created) \t = %d PERIOD \n",debug_print_slave2_P[0]);
        printf("T0 \t (Task Created) \t = %d CC \n",debug_print_slave2[0]);

        printf("T1 \t (End of Initializations)\t = %d PERIOD \n",debug_print_slave2_P[1]);
        printf("T1 \t (End of Initializations)\t = %d CC \n",debug_print_slave2[1]);

            for(int i=0;i<LOOP_COUNT+2;i++){

      
                  printf(" \t------Message %d--------------------\n",(i+1) );

                  printf("T%d \t (After FOR loop) \t= %d PERIOD \n",(2+i*5+0),debug_print_slave2_P[2+i*5+0]);
                  printf("T%d \t (After FOR loop) \t= %d CC \n",(2+i*5+0),debug_print_slave2[2+i*5+0]);

                  printf("T%d \t (Trigger Period) \t = %d PERIOD \n",(2+i*5+1),debug_print_slave2_P[2+i*5+1]);
                  printf("T%d \t (Trigger Period) \t = %d CC \n",(2+i*5+1),debug_print_slave2[2+i*5+1]);


                  printf("T%d \t (End of Computation)\t= %d PERIOD \n",(2+i*5+2),debug_print_slave2_P[2+i*5+2]);
                  printf("T%d \t (End of Computation)\t= %d CC \n",(2+i*5+2),debug_print_slave2[2+i*5+2]);

                  printf("- \t (Computation Duration) \t = %d PERIOD \n",debug_print_slave2_P[2+i*5+2]-debug_print_slave2_P[2+i*5+1]);
                  printf("- \t (Computation Duration) \t = %d CC \n",debug_print_slave2[2+i*5+2]-debug_print_slave2[2+i*5+1]);

                  printf("T%d \t (End of Flow Control Code Section) \t = %d PERIOD \n",(2+i*5+3),debug_print_slave2_P[2+i*5+3]);
                  printf("T%d \t (End of Flow Contol Code Section) \t = %d CC \n",(2+i*5+3),debug_print_slave2[2+i*5+3]);

                  printf("- \t (Flow Control Code Duration) \t = %d PERIOD \n",debug_print_slave2_P[2+i*5+3]-debug_print_slave2_P[2+i*5+2]);
                  printf("- \t (Flow Control Code Duration) \t = %d CC \n",debug_print_slave2[2+i*5+3]-debug_print_slave2[2+i*5+2]);
              
            }

        printf("------Latency Metrics--------------------\n");
        printf("The End-to-End Latency : %d/ Clock Cycle,\
                 for DATA_LEN=%d/words \
          and BUFFER_SIZE:%d/words \n", stop_transm_slave3-start_transm_slave1, DATA_LEN, BUFFER_SIZE);
        printf("The End-to-End Latency for DATA_LEN %d words of data \
          and %d words of BUFFER_SIZE:  %d  PERIOD \n", DATA_LEN, BUFFER_SIZE, debug_print_slave2[2+(LOOP_COUNT+1)*5+1]-debug_print_slave1[2+1]);
        printf("--------------------------------------------------------------\n");
    #endif

//Print the data received by the consumer
  #ifdef DATA_CHECK_MODE
     printf("Data received at the Consumer Side: \n");
     printf("--------------------------\n");
     for(int i=0;i<(DATA_LEN+BUFFER_SIZE*5);i++){
        printf("Data at %d is: %d \n",i, debug_print_data[i]);  
     }
  #endif

  #ifdef LATENCY_CALC_MODE

      printf("------Analitical Calculation of Latency --------------------\n");
      int analitical_latency =(((DATA_LEN/BUFFER_SIZE)+3)*GLOBAL_PERIOD);

      printf("The End-to-End Latency= %d/Clock Cycle, \
          for DATA_LEN=%d/words \
          and BUFFER_SIZE:%d/words \n", analitical_latency ,DATA_LEN, BUFFER_SIZE);

      printf("------Latency by Measurements--------------------\n");
      int measurement_latency =stop_transm_slave3-start_transm_slave1;
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
