/*
    A small test program testing the communication between patmos processors
    through the NoC

    Author: Rasmus Bo Sorensen
    Copyright: DTU, BSD License
*/

#include <machine/spm.h>

#define UART_STATUS *((volatile _SPM int *) 0xF0000800)
#define UART_DATA   *((volatile _SPM int *) 0xF0000804)
#define CPU_ID      *((volatile _SPM int *) 0xF0000000)

#define WRITE(data,len) do { \
  unsigned i; \
  for (i = 0; i < (len); i++) {        \
    while ((UART_STATUS & 0x01) == 0); \
    UART_DATA = (data)[i];             \
  } \
} while(0)

struct network_interface
{
    volatile _SPM int *dma;
    volatile _SPM int *dma_p;
    volatile _SPM int *st;
    volatile _SPM int *spm;
} ni = {
    (volatile _SPM int *) 0xE0000000,
    (volatile _SPM int *) 0xE1000000,
    (volatile _SPM int *) 0xE2000000,
    (volatile _SPM int *) 0xE8000000
};

int main() {

volatile int *dummy = (int *) 0x123;

    volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000900;

    char msg[] = "Hello world, from core: ";
    char cid = (char)(CPU_ID + (int)'0');

    WRITE(msg,24);
    WRITE(&cid,1);
    WRITE("\n",1);

    if (CPU_ID == 0)
    {

        int k;
        for(k = 0; k < 8; k++){
            *(ni.spm+k) = k+CPU_ID; // Fill numbers in communication scratch pad
        }

        for (k = 0; k < 8; k++)
        {
            *(ni.st+k) = 0  | (1 << 2); // Slot table use dma 0 and set valid bit.
        }

        *ni.dma_p = 13; // Route L -> E -> L
        *(ni.dma+1) = (0 << 16) | 4; // Read pointer and write pointer in the dma table
        *ni.dma = 4 | (1 << 15) ; // DWord count and valid bit

    }


    int i, j;

    for (;;) {
        UART_DATA = '1';
        for (i=2000; i!=0; --i)
            for (j=2000; j!=0; --j)
                *led_ptr = 1;


        UART_DATA = '0';
        for (i=2000; i!=0; --i)
            for (j=2000; j!=0; --j)
                *led_ptr = 0;

    }

    return 0;
}
