#include <machine/spm.h>

int main() {

volatile int *dummy = (int *) 0x123;

    volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000900;
    volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0000804;
    volatile _SPM int *cpu_id = (volatile _SPM int *) 0xF0000000;

    volatile _SPM int *ni_dma = (volatile _SPM int *) 0xE0000000;
    volatile _SPM int *ni_dma_p = (volatile _SPM int *) 0xE1000000;
    volatile _SPM int *ni_st = (volatile _SPM int *) 0xE2000000;
    volatile _SPM int *com_spm_base = (volatile _SPM int *) 0xE8000000;

    int core_id;
    core_id = *cpu_id;

    if (core_id == 0)
    {

        int k;
        for(k = 0; k < 8; k++){
            *(com_spm_base+k) = k+core_id; // Fill numbers in communication scratch pad
        }

        for (k = 0; k < 8; k++)
        {
            *ni_st = 0  | (1 << 3); // Slot table use dma 0 and set valid bit.
        }

        *ni_dma_p = 13; // Route L -> E -> L
        *(ni_dma+1) = (0 << 16) | 4; // Read pointer and write pointer in the dma table
        *ni_dma = 4 | (1 << 15) ; // DWord count and valid bit

    }

    int i, j;

    for (;;) {
        *uart_ptr = '1';
        for (i=2000; i!=0; --i)
            for (j=2000; j!=0; --j)
                *led_ptr = 1;


        *uart_ptr = '0';
        for (i=2000; i!=0; --i)
            for (j=2000; j!=0; --j)
                *led_ptr = 0;

    }
}
