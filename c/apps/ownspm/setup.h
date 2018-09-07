#include "ownspm.h"
#include "spmpool.h"

#ifndef DATA_LEN
#define DATA_LEN 4096 // words
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8 //128 // words
#endif

#ifndef MAX_CPU_CNT
#define MAX_CPU_CNT 16
#endif

#define CNT 4 //cores
#define STATUS_OFFSET (CNT-1) // no of status flags for buffer1
#define STATUS_LEN (STATUS_OFFSET*2) // no of status flags for buffer1
#define FLAG_OFFSET STATUS_LEN
