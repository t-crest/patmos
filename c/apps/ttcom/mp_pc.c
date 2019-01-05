/*
    This is a multicore program in which a producer-consumer task pair
    communicate with message passing.

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

// Measure execution time
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);

//For printing measurements
volatile _UNCACHED int debug_print_slave1[MEASUREMENT_SIZE]= {0};
volatile _UNCACHED int debug_print_slave2[MEASUREMENT_SIZE]= {0};
volatile _UNCACHED int debug_print_slave1_P[MEASUREMENT_SIZE]= {0}; //for period measurment
volatile _UNCACHED int debug_print_slave2_P[MEASUREMENT_SIZE]= {0}; //for period mesurement
//For printing data
volatile _UNCACHED int debug_print_data[DATA_LEN]= {0};
//to calculate the overall latency
volatile _UNCACHED int start_transm_slave1 = 0;
volatile _UNCACHED int stop_transm_slave2 = 0;

//Producer Core
void producer(void *arg) {

   #ifdef MEASUREMENT_MODE   
        // recording timestamps locally in the data SPM
        volatile  int _IODEV *timeStamps_slave1 = ((volatile   int _IODEV *) NOC_SPM_COMP_BASE);
        volatile  int _IODEV *timeStamps_slave1_P = ((volatile  int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE/2);
        //To calculate overall latency for the bulk data communication
        volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+1);
        // a pointer on the local SPM for handling the data manupulation locally 
        volatile int _IODEV *data_wr = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+2); 
    #endif

    #ifdef LATENCY_CALC_MODE   
        //To calculate overall latency for the bulk data communication
        volatile int _SPM *start_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
        // a pointer on the local SPM for handling the data manupulation locally 
        volatile int _IODEV *data_wr = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+1); 
    #endif

    #ifdef MEASUREMENT_MODE
    // start initialization measurement
      timeStamps_slave1[0] = *timer_ptr;
      timeStamps_slave1_P[0] = TDM_P_COUNTER;
    #endif

  ///////////////////////////////////////////////////////////////////////////////
  // -Channel declation at tranmitter side.  
  // -buffer allocations
  // -remote buffer address calculations (at the receiver side)
  ///////////////////////////////////////////////////////////////////////////////

  // buffer allocation into SPM
  qpd_t * chan1 = mp_create_qport(1, SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 

   //Sync the tasks by Busy waiting over a TDM counter value
    while( !( TDM_P_COUNTER >= SYNC_INIT ) ){ 
        ;
    }

      #ifdef MEASUREMENT_MODE
      timeStamps_slave1[1] = *timer_ptr;
      timeStamps_slave1_P[1] = TDM_P_COUNTER; 
      #endif

 for(int i=0;i<LOOP_COUNT+1;i++){ 
      // the first iteration in the loop is dedicated to warming up the cache
      // therefore measurements in the first loop are not considered 

        //send for each buffer
        for(int k=0;k<MP_CHAN_NUM_BUF;k++){ 

                  if (k==0){
                    //syncronize the tasks for the 1st iteration of the loop
                      while( !( TDM_P_COUNTER >= SYNC_INIT+100 ) ){ 
                            ;
                      }
                  }

              #ifdef MEASUREMENT_MODE
                  timeStamps_slave1[2+(i*MP_CHAN_NUM_BUF*3)+k*3] = *timer_ptr;  
                  timeStamps_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+k*3] = TDM_P_COUNTER;
              #endif

            ///////////////////////////////////////////////////////////////////////////////
            // Computation.Produce Data.
            /////////////////////////////////////////////////////////////////////////////// 

              // produce data into the write buffer number #k
              for (int j=0;j<BUFFER_SIZE;j++){
                  *(volatile int _IODEV*)((int*)chan1->write_buf+j) = (i+1)*100+(k*10);
              }

              #ifdef LATENCY_CALC_MODE
                  if(i==1 && k==0){ //Start the latency calculation when the first is sent
                    *start_transmission = *timer_ptr; 
                  }
              #endif

              #ifdef MEASUREMENT_MODE
                  if(i==1 &&k ==0){
                    *start_transmission = *timer_ptr; 
                  }
                  timeStamps_slave1[2+(i*MP_CHAN_NUM_BUF*3)+1+k*3] = *timer_ptr;
                  timeStamps_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+1+k*3] = TDM_P_COUNTER; 
              #endif

            ///////////////////////////////////////////////////////////////////////////////
            // Communication
            /////////////////////////////////////////////////////////////////////////////// 

              // blocking send 1
              mp_send(chan1,0); 

              #ifdef MEASUREMENT_MODE
                   timeStamps_slave1[2+(i*MP_CHAN_NUM_BUF*3)+2+k*3] = *timer_ptr;
                   timeStamps_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+2+k*3] = TDM_P_COUNTER; 
              #endif 
        }

        if (i==0){
          //syncronize the tasks for the 1st iteration of the loop
            while( !( TDM_P_COUNTER >= SYNC_INIT+700 ) ){ 
                ;
            }
         }

 }//for

   ///////////////////////////////////////////////////////////////////////////////
   //copy the measurement values from local memory to global main-mem for debug printing
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
        volatile  int _IODEV *timeStamps_slave2 = ((volatile   int _IODEV *) NOC_SPM_COMP_BASE);
        volatile  int _IODEV *timeStamps_slave2_P = ((volatile  int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE/2);
        //To calculate overall latency for the bulk data communication
        volatile int _SPM *stop_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+1);
        // a pointer on the local SPM for handling the data manupulation locally 
        volatile int _IODEV *data_rd = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+MEASUREMENT_MEM_TRACE+2); 
    #endif

    #ifdef LATENCY_CALC_MODE
       //To calculate overall latency for the bulk data communication   
        volatile int _SPM *stop_transmission = ((volatile int _SPM *) NOC_SPM_COMP_BASE);
        // a pointer on the local SPM for handling the data manupulation locally 
        volatile int _IODEV *data_rd = ((volatile int _IODEV *) NOC_SPM_COMP_BASE+1); 
    #endif

  #ifdef MEASUREMENT_MODE
    // start initialization measurement
    timeStamps_slave2[0] = *timer_ptr; 
    timeStamps_slave2_P[0] = TDM_P_COUNTER; 
  #endif

  ///////////////////////////////////////////////////////////////////////////////
  // -Channel declation at tranmitter side.  buffer allocations
  // -remote buffer address calculations (at the receiver side)
  ///////////////////////////////////////////////////////////////////////////////
  // allocating data to SPM
  qpd_t * chan1 = mp_create_qport(1, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
  mp_init_ports(); 
     
   //Sync the tasks by Busy waiting over a TDM counter value
  while( !( TDM_P_COUNTER >= SYNC_INIT ) ){ 
      ;
  }
  
     #ifdef MEASUREMENT_MODE
      timeStamps_slave2[1] = *timer_ptr;  
      timeStamps_slave2_P[1] = TDM_P_COUNTER;
     #endif

  for(int i=0;i<LOOP_COUNT+1;i++){ 
      // the first iteration in the loop is dedicated to warming up the cache
      // therefore measurements in the first loop are not considered

        //receive and ack for each buffer
        for(int k=0;k<MP_CHAN_NUM_BUF;k++){


                    if (k==0){
                        //syncronize the tasks for the 1st iteration of the loop
                          while( !( TDM_P_COUNTER >= SYNC_INIT+100 ) ){ 
                                ;
                          }
                      }

                #ifdef MEASUREMENT_MODE
                   timeStamps_slave2[2+(i*MP_CHAN_NUM_BUF*4)+0+k*4] = *timer_ptr; 
                   timeStamps_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+0+k*4] = TDM_P_COUNTER;
                #endif

                // blocking receive function.
                // checks if the next buffer, in the buffer queue, has received a complete message.
                // if a message is received, read_buf pointer is moved to the beginning of the message  
                mp_recv(chan1,0);


                #ifdef MEASUREMENT_MODE
                   timeStamps_slave2[2+(i*MP_CHAN_NUM_BUF*4)+1+k*4] = *timer_ptr;
                   timeStamps_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+1+k*4] = TDM_P_COUNTER;
                #endif

                // reading the data to the read buffer 1
                for (int j=0;j<BUFFER_SIZE;j++){
                    #ifdef DATA_CHECK_MODE
                        data_rd[j+k*BUFFER_SIZE+i*BUFFER_SIZE*MP_CHAN_NUM_BUF] = *(volatile int _IODEV*)((int*)chan1->read_buf+j); // for printing
                    #else
                        data_rd[j] = *(volatile int _IODEV*)((int*)chan1->read_buf+j); 
                    #endif
                }
            
                 #ifdef MEASUREMENT_MODE
                    timeStamps_slave2[2+(i*MP_CHAN_NUM_BUF*4)+2+k*4] = *timer_ptr; 
                    timeStamps_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+2+k*4] = TDM_P_COUNTER;
                 #endif

                /// Blocking acknowledment. It is called to release space in the receiving
                /// buffer when the received data is no longer used.
                 mp_ack(chan1,0,1);

                //Stop the latency calculation when the ack is sent from the receiver
                // The measurement is taken after the mp_ack, which possible compansates the noc traversal time.
                #ifdef LATENCY_CALC_MODE
                    if((i == LOOP_COUNT) && (k == MP_CHAN_NUM_BUF-1) ){
                      *stop_transmission = *timer_ptr; 
                    }
                #endif

                 #ifdef MEASUREMENT_MODE
                    if((i == LOOP_COUNT) && (k == MP_CHAN_NUM_BUF-1) ){
                      *stop_transmission = *timer_ptr; 
                    }
                    timeStamps_slave2[2+(i*MP_CHAN_NUM_BUF*4)+3+k*4] = *timer_ptr; 
                    timeStamps_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+3+k*4] = TDM_P_COUNTER;
                 #endif
            }


            if (i==0 ){
                //syncronize the tasks
                while( !( TDM_P_COUNTER >= SYNC_INIT+700 ) ){ 
                   ;
                }
            }
        
	}//for

      ///////////////////////////////////////////////////////////////////////////////
      //Print the received data for debuging
      ///////////////////////////////////////////////////////////////////////////////
        #ifdef DATA_CHECK_MODE
            for(int i=0;i<DATA_LEN;i++){
                debug_print_data[i] = data_rd[i];
            }
        #endif

   ///////////////////////////////////////////////////////////////////////////////
   // copy the measurement values from local memory to global main-mem
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

    corethread_create(1, &producer, (void*)slave_param);
    corethread_create(2, &consumer, (void*)slave_param);

    corethread_join(1, (void*)slave_param);
    corethread_join(2, (void*)slave_param);
  
  #ifdef MEASUREMENT_MODE

        printf("------ PRODUCER--------------------\n");
        printf("T0 \t (Task Created) \t = %d PERIOD \n",debug_print_slave1_P[0]);
        printf("T0 \t (Task Created) \t = %d CC \n",debug_print_slave1[0]);
        printf("T1 \t (End of Initializations)\t = %d PERIOD \n",debug_print_slave1_P[1]);
        printf("T1 \t (End of Initializations)\t = %d CC \n",debug_print_slave1[1]);
            for(int i=0;i<LOOP_COUNT+1;i++){

                for(int j=0;j<MP_CHAN_NUM_BUF;j++){
                      printf(" \t------Message %d--------------------\n",(i)*MP_CHAN_NUM_BUF+(j+1));
                      printf("T%d \t (Begin Produce Data) \t= %d PERIOD \n",(2+i*MP_CHAN_NUM_BUF*3+0+j*3),debug_print_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+j*3]);
                      printf("T%d \t (Begin Produce Data) \t= %d CC \n",(2+i*MP_CHAN_NUM_BUF*3+0+j*3),debug_print_slave1[2+(i*MP_CHAN_NUM_BUF*3)+j*3]);  
                      printf("T%d \t (Begin mp_send) \t= %d PERIOD \n",(2+i*MP_CHAN_NUM_BUF*3+1+j*3),debug_print_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+1+j*3]);
                      printf("T%d \t (Begin mp_send) \t= %d CC \n",(2+i*MP_CHAN_NUM_BUF*3+1+j*3),debug_print_slave1[2+(i*MP_CHAN_NUM_BUF*3)+1+j*3]);
                      printf("T%d \t (Finish mp_send) \t = %d PERIOD \n",(2+i*MP_CHAN_NUM_BUF*3+2+j*3),debug_print_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+2+j*3]);
                      printf("T%d \t (Finish mp_send) \t = %d CC \n",(2+i*MP_CHAN_NUM_BUF*3+2+j*3),debug_print_slave1[2+(i*MP_CHAN_NUM_BUF*3)+2+j*3]);
                      printf("- \t (mp_send Duration) \t = %d PERIOD \n",debug_print_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+2+j*3]-debug_print_slave1_P[2+(i*MP_CHAN_NUM_BUF*3)+1+j*3]);
                      printf("- \t (mp_send Duration) \t = %d CC \n",debug_print_slave1[2+(i*MP_CHAN_NUM_BUF*3)+2+j*3]-debug_print_slave1[2+(i*MP_CHAN_NUM_BUF*3)+1+j*3]);
                }
            }

        printf("------CONSUMER--------------------\n");
        printf("T0 \t (Task Created) \t = %d PERIOD \n",debug_print_slave2_P[0]);
        printf("T0 \t (Task Created) \t = %d CC \n",debug_print_slave2[0]);
        printf("T1 \t (End of Initializations)\t = %d PERIOD \n",debug_print_slave2_P[1]);
        printf("T1 \t (End of Initializations)\t = %d CC \n",debug_print_slave2[1]);
            for(int i=0;i<LOOP_COUNT+1;i++){

                for(int j=0;j<MP_CHAN_NUM_BUF;j++){              
                      printf(" \t------Message %d--------------------\n",(i)*MP_CHAN_NUM_BUF+(j+1));

                      printf("T%d \t (Begin mp_recv) \t = %d PERIOD \n",(2+i*MP_CHAN_NUM_BUF*4+0+j*4),debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+0+j*4]);
                      printf("T%d \t (Begin mp_recv) \t = %d CC \n",(2+i*MP_CHAN_NUM_BUF*4+0+j*4),debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+0+j*4]);

                      printf("T%d \t (End mp_recv)\t= %d PERIOD \n",(2+i*MP_CHAN_NUM_BUF*4+1+j*4),debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+1+j*4]);
                      printf("T%d \t (End mp_recv)\t= %d CC \n",(2+i*MP_CHAN_NUM_BUF*4+1+j*4),debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+1+j*4]);

                      printf("- \t (mp_recv Duration) \t = %d PERIOD \n",debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+1+j*4]-debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+0+j*4]);
                      printf("- \t (mp_recv Duration) \t = %d CC \n",debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+1+j*4]-debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+0+j*4]);

                      printf("T%d \t (Begin mp_ack) \t = %d PERIOD \n",(2+(i*MP_CHAN_NUM_BUF*4)+2+j*4),debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+2+j*4]);
                      printf("T%d \t (Begin mp_ack) \t = %d CC \n",(2+(i*MP_CHAN_NUM_BUF*4)+2+j*4),debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+2+j*4]);

                      printf("T%d \t (End mp_ack)\t= %d PERIOD \n",(2+(i*MP_CHAN_NUM_BUF*4)+3+j*4),debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+3+j*4]);
                      printf("T%d \t (End mp_ack)\t= %d CC \n",(2+(i*MP_CHAN_NUM_BUF*4)+3+j*4),debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+3+j*4]);

                      printf("- \t (mp_ack Duration) \t = %d PERIOD \n",debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+3+j*4]-debug_print_slave2_P[2+(i*MP_CHAN_NUM_BUF*4)+2+j*4]);
                      printf("- \t (mp_ack Duration) \t = %d CC \n",debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+3+j*4]-debug_print_slave2[2+(i*MP_CHAN_NUM_BUF*4)+2+j*4]);
                }

            }
        printf("------Latency Metrics--------------------\n");
        printf("The End-to-End Latency : %d/ Clock Cycle,\
                 for DATA_LEN=%d/words \
          and BUFFER_SIZE:%d/words \n", stop_transm_slave2-start_transm_slave1, DATA_LEN, BUFFER_SIZE);
        printf("The End-to-End Latency : %d/ Clock Cycle,\
               for DATA_LEN= %d/words  \
          and %d words of BUFFER_SIZE:  %d  PERIOD \n", debug_print_slave2[2+(LOOP_COUNT)*MP_CHAN_NUM_BUF*4+2+4*(MP_CHAN_NUM_BUF-1)]-debug_print_slave1[3], DATA_LEN, BUFFER_SIZE );
        printf("--------------------------------------------------------------\n");
    #endif

  #ifdef DATA_CHECK_MODE

     printf("------Data at the Receiver Side--------------------\n");
     for(int i=0;i<DATA_LEN;i++){
        printf("Data at %d is: %d \n",i, debug_print_data[i]);   
     }
  #endif

  #ifdef LATENCY_CALC_MODE

      printf("------Latency by Measurements--------------------\n");
      int measurement_latency = stop_transm_slave2-start_transm_slave1;
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
