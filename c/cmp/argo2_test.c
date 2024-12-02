/*
   C test application for the Argo 2.0 NOC.
   Author: Luca Pezzarossa (lpez@dtu.dk) Copyright: DTU, BSD License
*/

#define CORETHREAD_INIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <machine/patmos.h>
#include "libnoc/noc.h"
#include "libcorethread/corethread.h"
#include <math.h>

#define SEND_TIMEOUT_DEFAULT 800000	// in cc
#define REMOTE_IRQ_IDX 19		// bit n.3 -- the forth (shifted 16)
#define LOCAL_IRQ_IDX 18		// bit n.2 -- the third (shifted 16)

volatile long long unsigned int send_timeout = SEND_TIMEOUT_DEFAULT;
volatile int mode = 0;

volatile _UNCACHED int **bandwidth_results = NULL;
volatile _UNCACHED int **correctness_results = NULL;
volatile _UNCACHED unsigned char **interrupt_status = NULL;
volatile _UNCACHED unsigned char **remote_irq_results = NULL;
volatile _UNCACHED unsigned char **interrupt_occ = NULL;
volatile _UNCACHED unsigned int **interrupt_results = NULL;
volatile _UNCACHED int *spm_sizes = NULL;
volatile _UNCACHED int **reconfiguration_results = NULL;
volatile _UNCACHED int s, d;	// global sender and destnation
volatile _UNCACHED int block_size;
volatile _UNCACHED unsigned int block_base;
volatile _UNCACHED unsigned char *random_array = NULL;


int c_find_mem_size(volatile unsigned int _SPM * mem_addr)
{
	int init = *(mem_addr);
	int tmp;
	*(mem_addr) = 0xFFEEDDCC;
	int i = 2;
	int j = 0;
	for (j = 0; j < 28; j++)
	{
		tmp = *(mem_addr + i);
		*(mem_addr + i) = 0;
		if (*(mem_addr) == 0)
		{
			*(mem_addr + i) = tmp;
			*(mem_addr) = init;
			return i * 4;
		}
		i = i << 1;
		if (*(mem_addr) != 0xFFEEDDCC)
		{
			*(mem_addr + i) = tmp;
			*(mem_addr) = init;
			return -1;
		}
		*(mem_addr + i) = tmp;
	}
	*(mem_addr) = init;
	return -1;
}

void m_find_spm_size()
{
	spm_sizes[get_cpuid()] =
		c_find_mem_size((volatile unsigned int _SPM *)NOC_SPM_BASE);
	return;
}

void s_find_spm_size(void *arg)
{
	spm_sizes[get_cpuid()] =
		c_find_mem_size((volatile unsigned int _SPM *)NOC_SPM_BASE);
	int ret = 0;
	corethread_exit(&ret);
	return;
}

void m_collect_platform_info()
{
	int slave_param = 0;
	int *retval;
	int ct;
	for (int i = 0; i < NOC_CORES; i++)
	{
		if (i == NOC_MASTER)
		{
			m_find_spm_size();
		}
		else
		{
			ct = (int) i;
			if (corethread_create(ct, &s_find_spm_size, (void *)&slave_param))
			{
				// printf("Corethread not created.\n");
			}
			else
			{
				// printf("Corethread created.\n");
			}
			if (corethread_join(ct, (void **)&retval))
			{
				// printf("Corethread not joined.\n");
			}
			else
			{
				// printf("Corethread joined.\n");
			}
		}
	}
	return;
}

int m_get_max_spm_sizes()
{
	int max = spm_sizes[0];
	for (int i = 1; i < NOC_CORES; i++)
	{
		if (spm_sizes[i] > max)
		{
			max = spm_sizes[i];
		}
	}
	return max;
}

int m_get_min_spm_sizes()
{
	int min = spm_sizes[0];
	for (int i = 1; i < NOC_CORES; i++)
	{
		if (spm_sizes[i] < min)
		{
			min = spm_sizes[i];
		}
	}
	return min;
}

void m_print_test_parameters()
{
	printf("The tests are executed with the following parameters:\n\n");
	printf("Mode of operation: %d - (total modes: %d)\n", mode,
		   (int)NOC_CONFS);
	printf("Data block base: 0x%08X - (default: 0x%08X)\n", block_base,
		   (unsigned int)NOC_SPM_BASE);
	printf("Data block size: %u bytes - (default: %u bytes)\n", block_size,
		   m_get_min_spm_sizes());
	printf("Send timeout: %llu clock cycles - (default: %d clock cycles)\n",
		   send_timeout, SEND_TIMEOUT_DEFAULT);
	return;
}

void m_print_bandwidth_results()
{
	printf("\n\nBandwith results:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++)
		{
			if (bandwidth_results[i][j] == -1)
			{
				printf("N/A\t");
			}
			else if (bandwidth_results[i][j] == 0)
			{
				printf("-\t");
			}
			else
			{
				printf("%d\t", bandwidth_results[i][j]);
			}
		}
		printf("\n");
	}
	printf
		("\nNotes: The table shows the clock cycles needed to send %d bytes\nbetween every couple of cores. 'N/A' means that the channel does not\nexist or the trasmission timed-out.\n\n",
		 block_size);
	return;
}

void m_print_reconfiguration_results()
{
	printf("Reconfiguration results:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CONFS; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CONFS; i++)
	{
		printf("%d\t", i);
		for (int j = 0; j < NOC_CONFS; j++)
		{
			if (i != j)
			{
				printf("%d\t", reconfiguration_results[i][j]);
			}
			else
			{
				printf("-\t");
			}
		}
		printf("\n");
	}
	printf
		("\nNotes: The table shows the clock cycles needed to switch between every\ncouple of configurations from a system point of view.\n");
	return;
}

void m_print_correctness_results()
{
	printf("\nCorrectness results:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++)
		{
			if (correctness_results[i][j] == 0)
			{
				printf("-\t");
			}
			else if (correctness_results[i][j] == 1)
			{
				printf("OK\t");
			}
			else if (correctness_results[i][j] == 2)
			{
				printf("ERR.\t");
			}
			else
			{
				printf("?\t");
			}
		}
		printf("\n");
	}
	printf
		("\nNotes: The table shows if the transmission of a random generated data\nblock of %d bytes is correctly performed.\n\n",
		 block_size);
	return;
}

void m_print_spm_sizes()
{
	printf("CPU #:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	printf("Size:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", spm_sizes[i]);
	}
	printf("\n");
	return;
}

void m_print_interrupt_results()
{
	printf("\nInterrupt results:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++)
		{
			if (interrupt_status[i][j] > 1)
			{
				printf("%04X\t", interrupt_results[i][j]);
			}
			else
			{
				printf("-\t");
			}
		}
		printf("\n");
	}
	printf
		("\nNotes: The table shows, in hexadecimal, the last value read from\nthe data irq fifo. This should be the word-based address of\nthe last received word.\n");
	return;
}

void m_print_interrupt_status()
{
	printf("\nInterrupt status:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++)
		{
			if (interrupt_status[i][j] == 0)
			{
				printf("-\t");
			}
			else if (interrupt_status[i][j] == 1)
			{
				printf("NO IRQ\t");
			}
			else if (interrupt_status[i][j] == 2)
			{
				printf("NO DEA.\t");
			}
			else
			{
				printf("DEA.\t");
			}
		}
		printf("\n");
	}
	printf
		("\nNotes: The table shows the data irq status. 'DEA.' means that the\ninterrupt was deasserted after reading the fifo. 'NO DEA.' means the\nopposite. 'NO IRQ' menas that the interrupt was never asseted.\n\n");
	return;
}

void m_print_remote_irq_results()
{
	printf("Remote interrupt results:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++)
		{
			if (i == j)
			{
				printf("-\t");
			}
			else if (remote_irq_results[i][j] == 0)
			{
				printf("NO\t");
			}
			else if (remote_irq_results[i][j] == 1)
			{
				printf("NO DEA.\t");
			}
			else if (remote_irq_results[i][j] == 2)
			{
				printf("ERR.1\t");
			}
			else if (remote_irq_results[i][j] == 3)
			{
				printf("ERR.2\t");
			}
			else
			{
				printf("OK\t");
			}
		}
		printf("\n");
	}
	printf
		("\nNotes: The table shows the remote interrupt results. 'NO DEA' means\nthat the interrupt was not deasserted after reading from the fifo.\n'ERR.1' and 'ERR.2' report an error in the received value read from\nthe fifo and in the data in the SPM, respectively.\n\n");
	return;
}

void m_print_interrupt_occ()
{
	printf("\nInterrupt fifo elements:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++)
	{
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++)
		{
			if (interrupt_status[i][j] <= 1)
			{
				printf("-\t");
			}
			else
			{
				printf("%d\t", interrupt_occ[i][j]);
			}
		}
		printf("\n");
	}
	printf
		("\nNotes: The table shows how many elements were in the data fifo before\nthe interrupt de-asserted (1 is the expected value).\n\n");
	return;
}

void m_print_platform_info()
{
	printf("General platform information:\n\n");
	printf("Number of cores: %d\n", get_cpucnt());
	printf("Master core: %d\n", get_cpuid());
	printf("Operating frequency: %d Hz\n", get_cpu_freq());
	printf("NOC master: %d\n", NOC_MASTER);
	printf("NOC cores: %d\n", NOC_CORES);
	printf("Number of configurations: %d\n", NOC_CONFS);
	printf("SPM size by CPU (min: %d - max: %d):\n", m_get_max_spm_sizes(),
		   m_get_min_spm_sizes());
	m_print_spm_sizes();
	return;
}

void m_clear_interrupt_status()
{
	for (int i = 0; i < NOC_CORES; i++)
	{
		for (int j = 0; j < NOC_CORES; j++)
		{
			interrupt_status[i][j] = 0;
		}
	}
	return;
}

void m_clear_remote_irq_results()
{
	for (int i = 0; i < NOC_CORES; i++)
	{
		for (int j = 0; j < NOC_CORES; j++)
		{
			remote_irq_results[i][j] = 0;
		}
	}
	return;
}

void m_clear_interrupt_occ()
{
	for (int i = 0; i < NOC_CORES; i++)
	{
		for (int j = 0; j < NOC_CORES; j++)
		{
			interrupt_occ[i][j] = 0;
		}
	}
	return;
}

void m_clear_interrupt_results()
{
	for (int i = 0; i < NOC_CORES; i++)
	{
		for (int j = 0; j < NOC_CORES; j++)
		{
			interrupt_results[i][j] = 0;
		}
	}
	return;
}

void m_clear_bandwidth_results()
{
	for (int i = 0; i < NOC_CORES; i++)
	{
		for (int j = 0; j < NOC_CORES; j++)
		{
			bandwidth_results[i][j] = -1;
		}
	}
	return;
}


void m_clear_correctness_results()
{
	for (int i = 0; i < NOC_CORES; i++)
	{
		for (int j = 0; j < NOC_CORES; j++)
		{
			correctness_results[i][j] = 0;
		}
	}
	return;
}

void m_generate_random_array()
{
	for (int i = 0; i < block_size; i++)
	{
		random_array[i] = ((unsigned char)(rand() & 0x000000FF));
	}
}



void c_generate_bandwidth_results()
{								// bandwith results need to be cleared
	volatile _SPM unsigned char *block =
		((volatile _SPM unsigned char *)block_base);
	// Initialize memorycontent with incremental values
  // printf("Initializing memory\n");
	for (int i = 0; i < block_size; i++)
	{
		block[i] = (unsigned char)(i & 0x000000FF);
	}
	long long unsigned int t1, t2, tmax;
	bool timeout = false;
	// I need to put nocsend in the cache therefore every measure is repeated
	// twice

	for (int c = 0; c < 2 * NOC_CORES; c++)
	{
		int i = c / 2;
		timeout = false;
		if (i == get_cpuid())
		{
			bandwidth_results[i][i] = 0;
		}
		else
		{
			t1 = get_cpu_cycles();
			tmax = t1 + send_timeout;
			noc_write((unsigned)i, ((volatile _SPM void *)block_base),
					  ((volatile _SPM void *)block), (size_t) block_size, 0);

			while (!(timeout || (noc_dma_done((unsigned)i))))
			{
				if (get_cpu_cycles() > tmax)
				{
					timeout = true;
					noc_dma_clear((unsigned)i);
				}
			}
			if (!timeout)
			{
				t2 = get_cpu_cycles();
				bandwidth_results[get_cpuid()][i] = t2 - t1;
			}
		}
	}
  // printf("timeout=%d\n", timeout);
	return;
}

void m_generate_bandwidth_results()
{
	c_generate_bandwidth_results();
	return;
}

void s_generate_bandwidth_results(void *arg)
{
	c_generate_bandwidth_results();
	int ret = 0;
	corethread_exit(&ret);
	return;
}



void c_send_random_array()
{
	volatile _SPM unsigned char *block =
		((volatile _SPM unsigned char *)block_base);
	for (int i = 0; i < block_size; i++)
	{
		block[i] = random_array[i];
	}
	noc_write((unsigned)d, ((volatile _SPM void *)block_base),
			  ((volatile _SPM void *)block), (size_t) block_size, 1);
	while (!(noc_dma_done((unsigned)d)))
	{;
	}
	return;
}

void m_send_random_array()
{
	c_send_random_array();
	return;
}

void s_send_random_array(void *arg)
{
	c_send_random_array();
	int ret = 0;
	corethread_exit(&ret);
	return;
}

void c_check_correctness()
{
	volatile _SPM unsigned char *block =
		((volatile _SPM unsigned char *)block_base);
	// Initialize memory content with incremental values
	bool correct = true;
	for (int i = 0; i < block_size; i++)
	{
		if (block[i] != random_array[i])
		{
			correct = false;
			break;
		}
	}
	if (correct)
	{
		correctness_results[s][d] = 1;
	}
	else
	{
		correctness_results[s][d] = 2;
	}
	// Check if the local IRQ addr is correct ()
	if ((intr_get_pending() & (1 << LOCAL_IRQ_IDX)) != 0)
	{

		for (int i = 1; i <= 256; i++)
		{
			interrupt_occ[s][d] = (unsigned char)i;
			// read fifo
			interrupt_results[s][d] = (unsigned int)(*(NOC_IRQ_BASE + 1));
			// clear the pending
			intr_clear_pending(LOCAL_IRQ_IDX);
			if ((intr_get_pending() & (1 << LOCAL_IRQ_IDX)) == 0)
			{
				break;
			}
		}
		if ((intr_get_pending() & (1 << LOCAL_IRQ_IDX)) == 0)
		{
			// deasserted
			interrupt_status[s][d] = 3;
		}
		else
		{
			// not deasserted
			interrupt_status[s][d] = 2;
		}
	}
	else
	{
		// no interrupt
		interrupt_status[s][d] = 1;
	}
	return;
}

void m_check_correctness()
{
	c_check_correctness();
	return;
}

void s_check_correctness(void *arg)
{
	c_check_correctness();
	int ret = 0;
	corethread_exit(&ret);
	return;
}

void m_bandwidth_correctness_test()
{
	m_clear_bandwidth_results();
	int slave_param = 0;		// not used now
	int *retval;
	int ct;
	for (int i = 0; i < NOC_CORES; i++)
	{
		if (i == NOC_MASTER)
		{
			// The master does the measure
			// printf("Bandwidth test: master core (%d) is testing.\n", NOC_MASTER);
			// NOC_MASTER);
			m_generate_bandwidth_results();
		}
		else
		{
			// printf("Bandwidth test: core %d is testing.\n", i);
			ct = (int) i;
			if (corethread_create
				(ct, &s_generate_bandwidth_results, (void *)&slave_param))
			{
				// printf("Corethread not created.\n");
			}
			if (corethread_join(ct, (void **)&retval))
			{
				// printf("Corethread not joined.\n");
			}
		}
	}

	m_clear_correctness_results();
	m_clear_interrupt_status();
	m_clear_interrupt_results();
	m_clear_interrupt_occ();
	s = 0;
	d = 0;
	for (s = 0; s < NOC_CORES; s++)
	{
		for (d = 0; d < NOC_CORES; d++)
		{
			if (bandwidth_results[s][d] > 0)
			{
				// printf("Correctness test: form core %d to core %d.\n", s, d);
				m_generate_random_array();
				if (s == NOC_MASTER)
				{
					m_send_random_array();	// pass the destination
				}
				else
				{
					ct = (int) (s);
					if (corethread_create
						(ct, &s_send_random_array, ((void *)&slave_param)))
					{
						// printf("Corethread send not created.\n");
					}
					else
					{
						// printf("Corethread send created.\n");
					}

					if (corethread_join(ct, (void **)&retval))
					{
						// printf("Corethread send not joined.\n");
					}
					else
					{
						// printf("Corethread send joined.\n");
					}
				}

				if (d == NOC_MASTER)
				{
					m_check_correctness();
				}
				else
				{
					ct = (int) (d);
					if (corethread_create
						(ct, &s_check_correctness, ((void *)&slave_param)))
					{
						// printf("Corethread receiver not created.\n");
					}
					else
					{
						// printf("Corethread receiver created.\n");
					}
					if (corethread_join(ct, (void **)&retval))
					{
						// printf("Corethread receiver not joined.\n");
					}
					else
					{
						// printf("Corethread receiver joined.\n");
					}
				}
			}
		}
	}
	m_print_test_parameters();
	m_print_bandwidth_results();
	m_print_correctness_results();
	m_print_interrupt_status();
	m_print_interrupt_occ();
	m_print_interrupt_results();
	return;
}

void m_change_test_pars()
{
	bool loop = true;
	char c = 'v';
	while (loop)
	{
		m_print_test_parameters();
		printf
			("\nParameters:\n1 -> Change mode of operation\n2 -> Change data block base address\n3 -> Change data block size\n4 -> Change send timeout value\nb -> Back to main menu\n");
		printf("\nSelect parameter: ");
		scanf(" %c", &c);
		while ((c != 'b') && (c != '1') && (c != '2') && (c != '3')
			   && (c != '4'))
		{
			printf("\nSelection not valid! Select operation: ");
			scanf(" %c", &c);
		};
		switch (c)
		{
		case 'b':
			loop = false;
			break;
		case '1':
			printf
				("\n-------------------------------------------------------------------------------\n");
			printf("Insert new mode of operation (between 0 and %u): ",
				   (unsigned int)NOC_CONFS - 1);
			int new_mode = -1;
			scanf("%d", &new_mode);
			if (new_mode == mode)
			{
				printf("\nMode not changed.\n\n");
			}
			else if (new_mode >= 0 && new_mode < NOC_CONFS)
			{
				mode = new_mode;
				noc_sched_set(mode);
				printf("\nMode changed.\n\n");
			}
			else
			{
				printf
					("\nThe inserted mode is not valid. Mode not changed.\n\n");
			}
			break;
		case '2':
			printf
				("\n-------------------------------------------------------------------------------\n");
			printf("Insert new block base address in hexadecimal: 0x%04X",
				   ((unsigned int)NOC_SPM_BASE) >> 16);
			unsigned int new_block_base = 0;
			scanf("%x", &new_block_base);
			if (new_block_base == block_base)
			{
				printf("\nBlock base not changed.\n\n");
			}
			else if (new_block_base <= 0x0000FFFF)
			{
				block_base = ((unsigned int)NOC_SPM_BASE + new_block_base);
				printf("\nBlock base changed.\n\n");
			}
			else
			{
				printf
					("\nThe inserted value is not valid. Block base not changed.\n\n");
			}
			break;
		case '3':
			printf
				("\n-------------------------------------------------------------------------------\n");
			printf("Insert new block size in bytes (between 1 and %u): ",
				   m_get_min_spm_sizes());
			int new_block_size = 0;
			scanf("%d", &new_block_size);
			if (new_block_size == block_size)
			{
				printf("\nBlock size not changed.\n\n");
			}
			else if (new_block_size >= 1
					 && new_block_size <= m_get_min_spm_sizes())
			{
				block_size = new_block_size;
				printf("\nBlock size changed.\n\n");
			}
			else
			{
				printf
					("\nThe inserted value is not valid. Block size not changed.\n\n");
			}
			break;
		case '4':
			printf
				("\n-------------------------------------------------------------------------------\n");
			printf("Insert new timeout value: ");
			long long unsigned int new_timeout = 0;
			scanf("%llu", &new_timeout);
			if (new_timeout == send_timeout)
			{
				printf("\nTimeout not changed.\n\n");
			}
			else
			{
				send_timeout = new_timeout;
				printf("\nTimeout changed. Timeout not changed.\n\n");
			}
			break;
		}
	}
	return;
}

void m_test_mode_change()
{
	if (NOC_CONFS == 1)
	{
		printf("There is only one mode of operation.\n");
		return;
	}
	reconfiguration_results =
		(volatile _UNCACHED int **)malloc(NOC_CONFS *
										  sizeof(*reconfiguration_results));
	if (reconfiguration_results == NULL)
	{
		printf("Dynamic memory allocation failed.\n");
		return;
	}
	for (int i = 0; i < NOC_CONFS; i++)
	{
		reconfiguration_results[i] =
			(volatile _UNCACHED int *)malloc(NOC_CONFS *
											 sizeof
											 (**reconfiguration_results));
		if (reconfiguration_results[i] == NULL)
		{
			printf("Dynamic memory allocation failed.\n");
			return;
		}
	}
	long long unsigned int t1, t2;
	for (int f = 0; f < NOC_CONFS; f++)
	{
		noc_sched_set(f);
		for (int t = f + 1; t < NOC_CONFS; t++)
		{
			noc_sched_set(f);
			t1 = get_cpu_cycles();
			noc_sched_set(t);
			t2 = get_cpu_cycles();
			reconfiguration_results[f][t] = t2 - t1;
			noc_sched_set(t);
			t1 = get_cpu_cycles();
			noc_sched_set(f);
			t2 = get_cpu_cycles();
			reconfiguration_results[t][f] = t2 - t1;
		}
	}
	m_print_reconfiguration_results();

	for (int i = 0; i < NOC_CONFS; i++)
	{
		free((void *)reconfiguration_results[i]);
		reconfiguration_results[i] = NULL;
	}
	free((void *)reconfiguration_results);
	reconfiguration_results = NULL;
	// Restore the original mode
	noc_sched_set(mode);
	return;
}



void c_send_irq()
{
	volatile _SPM unsigned char *block =
		((volatile _SPM unsigned char *)block_base);
	volatile _SPM unsigned char *irq_dst_addr =
		((volatile _SPM unsigned char *)block_base);
	block[0] = random_array[0];
	noc_irq((unsigned)d, (volatile void _SPM *)irq_dst_addr,
			(volatile void _SPM *)block);
	while (!(noc_dma_done((unsigned)d)))
	{;
	}
	return;
}

void m_send_irq()
{
	c_send_irq();
	return;
}

void s_send_irq(void *arg)
{
	c_send_irq();
	int ret = 0;
	corethread_exit(&ret);
	return;
}


void c_receive_irq()
{
	volatile _SPM unsigned char *block =
		((volatile _SPM unsigned char *)block_base);
	volatile _SPM unsigned char *irq_dst_addr =
		((volatile _SPM unsigned char *)block_base);
	unsigned int tmp_irq_addr;
	remote_irq_results[s][d] = 0;
	if ((intr_get_pending() & (1 << REMOTE_IRQ_IDX)) != 0)
	{
		int i;
		for (i = 1; i <= 256; i++)
		{
			// read fifo
			tmp_irq_addr = (unsigned int)noc_fifo_irq_read();
			// clear the pending
			intr_clear_pending(REMOTE_IRQ_IDX);
			if ((intr_get_pending() & (1 << REMOTE_IRQ_IDX)) == 0)
			{
				break;
			}
		}
		if ((intr_get_pending() & (1 << REMOTE_IRQ_IDX)) == 0)
		{
			// irq deasserted
			if (i != 1)
			{
				// irq deasserted but more than 1 entry in the fifo
				remote_irq_results[s][d] = 1;
			}
			else if ((tmp_irq_addr & 0x00003FFF) !=
					 ((((unsigned int)((void *)(irq_dst_addr))) & 0x0000FFFF)
					  >> 2))
			{
				// irq deasserted but fifo content is wrong
				remote_irq_results[s][d] = 2;
			}
			else if ((*irq_dst_addr) != random_array[0])
			{
				// irq deasserted but memory content is wrong
				remote_irq_results[s][d] = 3;
			}
			else
			{
				remote_irq_results[s][d] = 4;
			}
		}
		else
		{
			// irq not deasserted
			remote_irq_results[s][d] = 1;
		}
	}
	else
	{
		// no interrupt
		remote_irq_results[s][d] = 0;
	}
	return;
}

void m_receive_irq()
{
	c_receive_irq();
	return;
}

void s_receive_irq(void *arg)
{
	c_receive_irq();
	int ret = 0;
	corethread_exit(&ret);
	return;
}

void m_test_remote_irq()
{
	m_clear_bandwidth_results();
	int slave_param = 0;
	int *retval;
	int ct;
	for (int i = 0; i < NOC_CORES; i++)
	{
		if (i == NOC_MASTER)
		{
			// printf("Bandwidth test: master core (%d) is testing.\n",
			// NOC_MASTER);
			m_generate_bandwidth_results();
		}
		else
		{
			// printf("Bandwidth test: core %d is testing.\n", i);
			ct = (int) i;
			if (corethread_create
				(ct, &s_generate_bandwidth_results, (void *)&slave_param))
			{
				// printf("Corethread not created.\n");
			}
			if (corethread_join(ct, (void **)&retval))
			{
				// printf("Corethread not joined.\n");
			}
		}
	}
	m_clear_remote_irq_results();
	s = 0;
	d = 0;
	for (s = 0; s < NOC_CORES; s++)
	{
		for (d = 0; d < NOC_CORES; d++)
		{
			if (bandwidth_results[s][d] > 0)
			{
				m_generate_random_array();
				if (s == NOC_MASTER)
				{
					m_send_irq();	// pass the destination
				}
				else
				{
					ct = (int) (s);
					if (corethread_create
						(ct, &s_send_irq, ((void *)&slave_param)))
					{
						// printf("Corethread send not created.\n");
					}
					else
					{
						// printf("Corethread send created.\n");
					}

					if (corethread_join(ct, (void **)&retval))
					{
						// printf("Corethread send not joined.\n");
					}
					else
					{
						// printf("Corethread send joined.\n");
					}
				}
				if (d == NOC_MASTER)
				{
					m_receive_irq();	// pass the sender id
				}
				else
				{
					ct = (int) (d);
					if (corethread_create
						(ct, &s_receive_irq, ((void *)&slave_param)))
					{
						// printf("Corethread receiver not created.\n");
					}
					else
					{
						// printf("Corethread receiver created.\n");
					}
					if (corethread_join(ct, (void **)&retval))
					{
						// printf("Corethread receiver not joined.\n");
					}
					else
					{
						// printf("Corethread receiver joined.\n");
					}
				}
			}
		}
	}
	m_print_remote_irq_results();
	return;
}

int main()
{
	long long unsigned int t1, t2;
	// Print the header
	printf
		("\n-------------------------------------------------------------------------------\n");
	printf
		("-                   C test application for the Argo 2.0 NOC                   -\n");
	printf
		("-                                  Enjoy! ;-)                                 -\n");
	printf
		("-------------------------------------------------------------------------------\n");

	// Dynamically allocate main memory

	spm_sizes =
		(volatile _UNCACHED int *)malloc(NOC_CORES * sizeof(*spm_sizes));

	random_array =
		(volatile _UNCACHED unsigned char *)malloc(4096 *
												   sizeof(*random_array));

	bandwidth_results =
		(volatile _UNCACHED int **)malloc(NOC_CORES *
										  sizeof(*bandwidth_results));
	correctness_results =
		(volatile _UNCACHED int **)malloc(NOC_CORES *
										  sizeof(*correctness_results));
	interrupt_status =
		(volatile _UNCACHED unsigned char **)malloc(NOC_CORES *
													sizeof(*interrupt_status));
	remote_irq_results =
		(volatile _UNCACHED unsigned char **)malloc(NOC_CORES *
													sizeof
													(*remote_irq_results));
	interrupt_occ =
		(volatile _UNCACHED unsigned char **)malloc(NOC_CORES *
													sizeof(*interrupt_occ));
	interrupt_results =
		(volatile _UNCACHED unsigned int **)malloc(NOC_CORES *
												   sizeof(*interrupt_results));

	if (bandwidth_results == NULL
		|| correctness_results == NULL || interrupt_status == NULL
		|| remote_irq_results == NULL || interrupt_occ == NULL
		|| interrupt_results == NULL || spm_sizes == NULL)
	{
		printf("Dynamic memory allocation failed.\n");
		return -1;
	}
	for (int i = 0; i < NOC_CORES; i++)
	{
		bandwidth_results[i] =
			(volatile _UNCACHED int *)malloc(NOC_CORES *
											 sizeof(**bandwidth_results));
		correctness_results[i] =
			(volatile _UNCACHED int *)malloc(NOC_CORES *
											 sizeof(**correctness_results));
		interrupt_status[i] =
			(volatile _UNCACHED unsigned char *)malloc(NOC_CORES *
													   sizeof
													   (**interrupt_status));
		remote_irq_results[i] =
			(volatile _UNCACHED unsigned char *)malloc(NOC_CORES *
													   sizeof
													   (**remote_irq_results));
		interrupt_occ[i] =
			(volatile _UNCACHED unsigned char *)malloc(NOC_CORES *
													   sizeof
													   (**interrupt_occ));
		interrupt_results[i] =
			(volatile _UNCACHED unsigned int *)malloc(NOC_CORES *
													  sizeof
													  (**interrupt_results));
		if (bandwidth_results[i] == NULL || correctness_results[i] == NULL
			|| interrupt_status[i] == NULL || remote_irq_results[i] == NULL
			|| interrupt_occ[i] == NULL || interrupt_results[i] == NULL)
		{
			printf("Dynamic memory allocation failed.\n");
			return -1;
		}
	}

	m_collect_platform_info();
	block_size = m_get_min_spm_sizes();
	block_base = (unsigned int)NOC_SPM_BASE;
	m_print_platform_info();
	printf("\n");
	m_print_test_parameters();

/*	printf("%08X (%08X)", random_array, m_get_max_spm_sizes());
	random_array =
		(volatile _UNCACHED unsigned char *)malloc(m_get_max_spm_sizes() *
												   sizeof(*random_array));
	if (random_array == NULL)
	{
		printf("Dynamic memory allocation for 'random_array' failed.\n");
		//printf("%08X (%08X)", random_array, m_get_max_spm_sizes());
		return -1;
	}
*/

	// Main loop
	bool loop = true;
	char c = 'v';
	while (loop)
	{
		printf
			("\n-------------------------------------------------------------------------------");
		printf
			("\nAvailable operations:\n1 -> Test bandwidth, tramission correctness, and data interrupts.\n2 -> Test remote interrupts\n3 -> Test reconfiguration\n4 -> Print platform info\n5 -> Print test parameters\n6 -> Change test parameters\ne -> Exit\n");
		printf("\nSelect operation: ");
		scanf(" %c", &c);
		while ((c != 'e') && (c != '1') && (c != '2') && (c != '3')
			   && (c != '4') && (c != '5') && (c != '6'))
		{
			printf("\nOperation not valid! Select operation: ");
			scanf(" %c", &c);
		};
		printf
			("\n-------------------------------------------------------------------------------");
		switch (c)
		{
		case '1':
			printf
				("\nOperation 1: Test bandwidth, tramission correctness, and data interrupts.\n\n");
			m_bandwidth_correctness_test();
			break;
		case '2':
			printf("\nOperation 2: Test remote interrupts\n\n");
			m_test_remote_irq();
			break;
		case '3':
			printf("\nOperation 3: Test reconfiguration\n\n");
			m_test_mode_change();
			break;
		case '4':
			printf("\nOperation 4: Print platform info\n\n");
			m_print_platform_info();
			break;
		case '5':
			printf("\nOperation 5: Print test parameters\n\n");
			m_print_test_parameters();
			break;
		case '6':
			printf("\nOperation 6: Change test parameters\n\n");
			m_change_test_pars();
			break;
		case 'e':
			loop = false;
			break;
		}
	}

	// De-allocate dynamic memory
	for (int i = 0; i < NOC_CORES; i++)
	{
		free((void *)bandwidth_results[i]);
		free((void *)correctness_results[i]);
		free((void *)interrupt_status[i]);
		free((void *)remote_irq_results[i]);
		free((void *)interrupt_occ[i]);
		free((void *)interrupt_results[i]);
		bandwidth_results[i] = NULL;
		correctness_results[i] = NULL;
		interrupt_status[i] = NULL;
		remote_irq_results[i] = NULL;
		interrupt_occ[i] = NULL;
		interrupt_results[i] = NULL;
	}
	free((void *)random_array);
	free((void *)bandwidth_results);
	free((void *)correctness_results);
	free((void *)interrupt_status);
	free((void *)remote_irq_results);
	free((void *)interrupt_occ);
	free((void *)interrupt_results);
	free((void *)spm_sizes);
	random_array = NULL;
	bandwidth_results = NULL;
	correctness_results = NULL;
	interrupt_status = NULL;
	remote_irq_results = NULL;
	interrupt_occ = NULL;
	interrupt_results = NULL;
	spm_sizes = NULL;

	printf("\nGoodbye!\n");
	return 0;
}
