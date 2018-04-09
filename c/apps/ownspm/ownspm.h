/*
  Include file for all SPM programs.

*/

#ifndef DATA_LEN
#define DATA_LEN  32 // words
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4 // words
#endif

volatile int _SPM *spm_ptr = ((volatile _SPM int *) 0xE8000000);

#define NEXT 0x10000/4 // SPMs are placed every 64 KB 
