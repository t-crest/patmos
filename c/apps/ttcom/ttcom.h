
#define TDM_P_COUNTER  *((volatile int _SPM *) NOC_TDM_BASE+1)
#define NOC_SPM_COMP_BASE ((volatile int _SPM *) 0x00000000)

#ifndef DATA_LEN
#define DATA_LEN 4096 //words
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 32// words
#endif

#ifndef MP_CHAN_NUM_BUF
#define MP_CHAN_NUM_BUF 2 //double buffering
#endif

#define MSG_SIZE BUFFER_SIZE //words 
#define MP_CHAN_BUF_SIZE MSG_SIZE*4 //byte
#define LOOP_COUNT (DATA_LEN/BUFFER_SIZE) 
// memory requirements to store the measurements in the local Data SPM
#define MEASUREMENT_SIZE (((LOOP_COUNT+5)*5)+2)//words
#define MEASUREMENT_MEM_TRACE (MEASUREMENT_SIZE*2) //words

/*/////////////////////////////////////////////////////////////////////////////////////////////
The computation/communication time frame values are obtained via statical analysis,
and represent the worst-case computation/communication time frames for any core in the communication chain.
`Table 3` in the paper includes a full set of global period numbers for each `BUFFER_SIZE` and `MP_CHAN_NUM_BUF` configuration pair.
*//////////////////////////////////////////////////////////////////////////////////////////////////

// For Double Buffering
// the global period for time-triggered double buffering for a specific 'BUFFER_SIZE'
#ifndef GLOBAL_PERIOD
#define GLOBAL_PERIOD 685 // clock cycles
#endif
// For Single Buffering
// time-triggered single buffering for a specific 'BUFFER_SIZE'
// worst-case computation time for any core
#ifndef WCET_COMP
#define WCET_COMP 647 // clock cycles
#endif
// worst-case communication time 
#ifndef WCET_COMM
#define WCET_COMM 607 // clock cycles
#endif
// the global period for time-triggered single buffering.
#define TRIGGER_PERIOD (WCET_COMP+WCET_COMM)
#define SYNC_INIT 7000// A statically obtained value that is large enough to cover all initializaitons

///////////////////////////////////////////////////////////////////////////////////////////////
// These preprocessor directives defines different execution modes 
// that use the Data SPM memory layout for different purpose
// - "MEASUREMENT_MODE" is used to store the measurements in the Data SPM
// - "LATENCY_CALC_MODE" is used to store only 2  measuremnts to calculate end-to-end latency for sending 16KB data
// - "DATA_CHECK_MODE" is used to store and later print the data at the receiver side for sanity check 
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEASUREMENT_MODE
#ifndef LATENCY_CALC_MODE
#define LATENCY_CALC_MODE
#endif
#endif

//#ifndef DATA_CHECK_MODE
//#define DATA_CHECK_MODE
//#endif