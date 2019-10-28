/*
  Small test program for the distributed shared memory
*/

#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#define CNT 4
#define WORDS 128

#define LED (*((volatile _IODEV unsigned *) PATMOS_IO_LED))

// The main function for the other threads on the other cores
void work(void *arg)
{

	volatile _SPM int *mem = (volatile _SPM int *)(0xE8000000);

	int id = get_cpuid();

	//Slave cores write to their own memory.
	for (int i = 0; i < CNT; i++)
	{
		mem[id * WORDS + i] = id * 0x10000 + 0x100 + i;
	}

	while (1)
	{
		//Wait for token
		while (mem[id * WORDS] == 0)
			;

		//Use token
		for (volatile int i = 4000000; i != 0; i--)
			;

		//Pass token
		int next = id + 1;
		if (next >= 4)
			next = 1;

		mem[next * WORDS] = (mem[id * WORDS] + 3) & 1;

		//Delete own token:
		mem[id * WORDS] = 0;
	}
}

int main()
{

	volatile _SPM int *rxMem = (volatile _SPM int *)(0xE8000000);

	for (int i = 1; i < get_cpucnt(); ++i)
	{
		corethread_create(i, &work, NULL);
	}

	printf("\n");
	printf("Number of cores: %d\n", get_cpucnt());

	volatile _SPM int *led_ptr = (volatile _SPM int *) PATMOS_IO_LED;

	for (int i = 0; i < 10000; i++)
	{
		*led_ptr = 1;
	}
	*led_ptr = 0;

	//Print content of all the slaves memory.
	for (int i = 0; i < CNT; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			printf("%08x\n", rxMem[i * WORDS + j]);
		}
	}

	//Initiate token
	rxMem[WORDS] = 5;

	while (1)
	{
		for (int i = 1; i < CNT; ++i)
		{
			printf("Core:%d, %08x\n", i, rxMem[i * WORDS]);
		}
		for (volatile int i = 0; i < 1000000; i++)
			;
		printf("\n");
	}

	return 0;
}
