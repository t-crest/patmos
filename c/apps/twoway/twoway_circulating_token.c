/*
    Small test program for the One-Way Shared Memory
    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "../../libcorethread/corethread.h"

#define BLOCKWIDTH 8

#define TOKEN 0x42

#define N_TOKENS 3 // Number of times a core sees a token

#define DELAY(time) for (volatile int i = time; i != 0; i--)

// Shared data in main memory for the return value
volatile _UNCACHED static int field;
volatile _UNCACHED static int end_time;

// The main function for the other thread on the another core
void work(void *arg)
{
    // Measure execution time with the clock cycle timer
    volatile _IODEV int *timer_ptr = (volatile _IODEV int *)(PATMOS_IO_TIMER + 4);
    volatile _SPM int *mem_ptr = (volatile _IODEV int *)(0xE8000000);

    // writing occurs on addresses 0x100, 0x200, 0x300, 0x400e

    //1 reads from 3, 2 from 1 and 3 from 2
    int partnerID = (get_cpuid() - 1) == 0 ? get_cpucnt() - 1 : get_cpuid() - 1;
    volatile _SPM int *readPtr = mem_ptr + (get_cpuid() << BLOCKWIDTH);
    volatile _SPM int *p_readPtr = mem_ptr + (partnerID << BLOCKWIDTH);

    int tokenCounter = 0;
    while (tokenCounter != N_TOKENS)
    {
        volatile int spinlock;
        while (*p_readPtr != TOKEN)
        {
            // wait until we observe our token
            spinlock++;
        }

        *p_readPtr = 0; // reset the token on the partner - we now own the token

        // Our turn to light the LEDS
        mem_ptr[get_cpuid()] = 1;
        DELAY(1000000);
        mem_ptr[get_cpuid()] = 0;
        // Write token to ourself (starting transfer of token to next core)
        *readPtr = TOKEN;

        // Increment token counter by 1:
        tokenCounter++;
    }

    return;
}

int main()
{
    volatile _SPM int *mem_ptr = (volatile _IODEV int *)(0xE8000000);

    printf("Number of cores: %d\n", get_cpucnt());
    for (int i = 1; i < get_cpucnt(); i++)
    {
        int parameter;
        corethread_create(i, (void *)&work, (void *)&parameter);
    }
    // Inject token into core 1
    mem_ptr[2 << BLOCKWIDTH] = TOKEN;

    // master reads its own memory to control the LED
    while (1)
    {
        // Monitor the "LED" sections of the memory and print if something was detected.
        // This also heavily stress the readback network
        DELAY(100000);
        for (int i = 1; i < get_cpucnt(); i++)
        {
            if (mem_ptr[i] != 0)
            {
                // turn on LED for the given core
                printf("%d on\n", i);
            }
        }
    }

    return 0;
}
