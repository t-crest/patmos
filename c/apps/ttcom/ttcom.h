
#define TDM_P_COUNTER  *((volatile int _SPM *) NOC_TDM_BASE+1)
#define NOC_SPM_COMP_BASE ((volatile int _SPM *) 0x00000000)

#ifndef DATA_LEN
#define DATA_LEN 4096 //words
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 64 // words
#endif

#ifndef MP_CHAN_NUM_BUF
#define MP_CHAN_NUM_BUF 2 //double buffering
#endif

#define MSG_SIZE BUFFER_SIZE //words 
#define MP_CHAN_BUF_SIZE MSG_SIZE*4 //byte
#define LOOP_COUNT (DATA_LEN/BUFFER_SIZE/MP_CHAN_NUM_BUF) 
// memory requirement to store the measurements in the local Data SPM
#define MEASUREMENT_SIZE (((LOOP_COUNT+2)*MP_CHAN_NUM_BUF*5)+2)//words
#define MEASUREMENT_MEM_TRACE (MEASUREMENT_SIZE*2) //words

///////////////////////////////////////////////////////////////////////////////////////////////
// Following definitions represent different numeric values for each `BUFFER_SIZE` and `MP_CHAN_NUM_BUF` configuration pair.
// Values are obtained from a set of measuments,and represents the maximum computation time for both of producer and consumer.
// `Figure X` in the paper includes a full set of period numbers. 
///////////////////////////////////////////////////////////////////////////////////////////////////

//Define a numeric value of minor period for time-triggered double buffering. 
#ifndef MINOR_PERIOD
#define MINOR_PERIOD 188 // clock cycles
#endif
//Define maximum computation period for time-triggered single buffering.
#ifndef WCET_COMP
#define WCET_COMP 458 // clock cycles
#endif
//Define maximum communication period for time-triggered single buffering.
#ifndef WCET_COMM
#define WCET_COMM 1112 // clock cycles
#endif

#define TRIGGER_PERIOD (WCET_COMP+WCET_COMM)// represents Worst-Casehopewel
#define SYNC_INIT 5200// tdm rounds- a large enough value to cover all initializaitons

///////////////////////////////////////////////////////////////////////////////////////////////
// These preprocessor directives defines different execution modes 
// that use the Data SPM memory layout for different purpose
// - "MEASUREMENT_MODE" is used to store the measurements in the Data SPM
// - "LATENCY_CALC_MODE" is used to store only 2  measuremnts to calculate total latency for sending 16KB data
// - "DATA_CHECK_MODE" is used to store and later print the data at the receiver side for sanity check 
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEASUREMENT_MODE
#ifndef DATA_CHECK_MODE
#ifndef LATENCY_CALC_MODE
#define LATENCY_CALC_MODE
#endif
#endif
#endif
