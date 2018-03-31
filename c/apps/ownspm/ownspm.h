/*
  Include file for all SPM programs.

*/

#define DATA_LEN  4096 // words
#define BUFFER_SIZE 128 // words

volatile int _SPM *spm_ptr = (( volatile _SPM int *)0xE8000000);

#define NEXT 0x10000/4 // SPMs are placed every 64 KB 
