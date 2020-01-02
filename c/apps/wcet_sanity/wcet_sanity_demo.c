#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include <ethlib/tte.h>

#define NS_TO_SEC 0.000000001
#define NS_TO_USEC 0.001
#define USEC_TO_NS 1000
#define USEC_TO_MS 0.0001
#define USEC_TO_SEC 0.000001
#define MS_TO_USEC 1000
#define MS_TO_NS 1000*MS_TO_USEC
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778

#define INT_PERIOD 100 //clocks
#define TTE_MAX_TRANS_DELAY 11 //clock cycles from net_config (eclipse project)
#define TTE_COMP_DELAY 0
#define TTE_PRECISION 2000 //clock cycles from network_description (eclipse project)

#define RUNS 20

unsigned long wcet_integration_max_time = 0;
unsigned long wcet_mem_rd_max_time = 0;

unsigned int startTick;
unsigned char schedplace;

unsigned long long current_time = 0;
unsigned long long timer_time = 0;
unsigned long long int_err = 0;

unsigned long wcet_mem_rd(unsigned int addr, unsigned i){
	unsigned long initMeasurement = (*(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK));
    unsigned long trans_clock = mem_iord_byte(addr + 34);
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 35));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 36));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 37));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 39));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 40));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 41));
	trans_clock = transClk_to_clk(trans_clock) + 11*i; //fake reading so that wcet estimates correctly the mem_iord_byte while preserving a correct emulation
    unsigned long diffTemp = (*(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK)) - initMeasurement;
    if(diffTemp > wcet_mem_rd_max_time)
	{
      wcet_mem_rd_max_time =  diffTemp;
	}
    return trans_clock;
}

__attribute__((noinline))
int wcet_integration(unsigned int addr, unsigned long receive_pit, signed long error[],int i){
	unsigned long long permanence_pit;
	unsigned long long sched_rec_pit;
	unsigned long long trans_clock;
    unsigned long long integration_cycle; 
	signed long long err;
	signed long long der_err;
	unsigned char ans = 0;
	unsigned long initMeasurement = (*(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK));

	trans_clock = mem_iord_byte(addr + 34);
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 35));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 36));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 37));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 39));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 40));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 41));
	trans_clock = transClk_to_clk(trans_clock) + 10; //fake reading so that wcet estimates correctly the mem_iord_byte while preserving a correct emulation

	integration_cycle = mem_iord_byte(addr + 14);
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(addr + 15));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(addr + 16));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(addr + 17));
    integration_cycle += i; //fake reading so that wcet estimates correctly the mem_iord_byte while preserving a correct emulation

	permanence_pit = receive_pit + (TTE_MAX_TRANS_DELAY-trans_clock); 

	if(current_time==0)
    {
		current_time=permanence_pit-(2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY);
	}
    // This else statement halfs the measured execution time but does not affect the WCET analysis
    // else
    // {
        sched_rec_pit = current_time + 2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY;

        error[i]=err=(permanence_pit - sched_rec_pit);

        //check scheduled receive window
        if(permanence_pit>(sched_rec_pit-TTE_PRECISION) && permanence_pit<(sched_rec_pit+TTE_PRECISION))
        {	
            int_err = int_err + err;
            current_time = current_time + (300*err >> 10) + (700*int_err >> 10);
            if(integration_cycle==0)
            {
            schedplace=0;
            timer_time = current_time+(CYCLES_PER_UNIT*startTick); 
            arm_clock_timer(timer_time);
            }
            current_time += INT_PERIOD;
            ans = 1;
        } 
        else 
        {
            current_time = 0;
        }
    // }
	unsigned long diffTemp = (*(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK)) - initMeasurement;
    if(diffTemp > wcet_integration_max_time)
	{
      wcet_integration_max_time =  diffTemp;
	}
	return ans;
}

int main()
{
    volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
    volatile _IODEV int *led_ptr = (volatile _SPM int *) PATMOS_IO_LED;
    signed long error_log[RUNS];
    unsigned long dead_read;
    // puts("time, error, wcet");
    for(int i=0; i<RUNS; i++)
    {
        *dead_ptr = INT_PERIOD;
        dead_read = *dead_ptr;
        wcet_integration(0x000, (*(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK)), error_log, i);
        wcet_mem_rd(0x000, i);
    }
    printf("Max_time{ wcet_integration() } = %lu clock cycles\n", wcet_integration_max_time);
    printf("Max_time{     wcet_mem_rd () } = %lu clock cycles\n", wcet_mem_rd_max_time);
    return 0;
}