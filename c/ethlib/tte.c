#include "tte.h"

unsigned int integration_period; //clock cycles at 80Mhz (12.5 ns)
unsigned int integration_cycle; //integer
static unsigned long long start_time; //clock cycles at 80MHz (12.5 ns)
static unsigned long long timer_time; //clock cycles at 80MHz (12.5 ns)
static unsigned long long receive_pit; //clock cycles at 80MHz (12.5 ns)
static unsigned long long scheduled_pit; //clock cycles at 80MHz (12.5 ns)

unsigned char frame[16] = {
    0xab, 0xad, 0xba, 0xbe, 0x0f, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xDC, 0x04, 0x00
}; 

unsigned char sending = 0;
void tte_clock_tick(void) __attribute__((naked));


unsigned char is_pcf(unsigned int addr){
	unsigned type_1 = mem_iord_byte(addr + 12);
	unsigned type_2 = mem_iord_byte(addr + 13);
	if(type_1 == 0x89 && type_2 == 0x1D){ 
		return 1;
	}
	return 0;
}

int handle_integration_frame(unsigned int addr,unsigned long long rec_start,unsigned long long r_pit[],unsigned long long p_pit[],unsigned long long s_pit[],unsigned int int_pd[],unsigned long long trans_clk[],int i){
	unsigned long long permanence_pit;
	unsigned long long sched_rec_pit;
	unsigned long long trans_clock; // weird 2^(-16) ns format

	receive_pit=get_cpu_cycles();
	//printf("r: %llu\n",receive_pit);
	r_pit[i]=receive_pit;

	trans_clock = mem_iord_byte(addr + 34);
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 35));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 36));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 37));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 39));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 40));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(addr + 41));
	trans_clock = transClk_to_clk(trans_clock);
	trans_clock += TTE_WIRE_DELAY + TTE_STATIC_RECIEVE_DELAY + (receive_pit-rec_start);

	//printf("transparent clock: %llu\n",trans_clock);

	trans_clk[i] = trans_clock;

	integration_cycle = mem_iord_byte(addr + 14);
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(addr + 15));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(addr + 16));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(addr + 17));

	//printf("rec_start: %llu, r_pit: %llu, dyn rec delay: %llu\n",rec_start,receive_pit,(receive_pit-rec_start));
	//printf("transClock after convert and delay: %llu\n",trans_clock);

	permanence_pit = receive_pit + (TTE_MAX_TRANS_DELAY-trans_clock); 
	if(start_time==0){
		start_time=permanence_pit-(2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY);
	}

	sched_rec_pit = start_time + 2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY;
	s_pit[i]=sched_rec_pit;

	//printf("p: %llu\n",permanence_pit);
	p_pit[i]=permanence_pit;

	if(permanence_pit>(sched_rec_pit-TTE_PRECISION) &&
	    permanence_pit<(sched_rec_pit+TTE_PRECISION)){
		for(unsigned long long i=get_cpu_cycles();i<(sched_rec_pit+2*TTE_PRECISION);){
		  i=get_cpu_cycles();
		  //busy wait until we're out of clock correction delay (should do this differently?)
		}

		integration_period = integration_period + permanence_pit - sched_rec_pit;
		int_pd[i] = integration_period;

		//printf("new integration period: %u\n",integration_period);

		start_time += integration_period;
		if(sending==0 && integration_cycle==1){
		  timer_time = start_time+((integration_period/100)*26);
		  arm_clock_timer(timer_time);
		  sending=1;
		}

		//printf("next tick should be at: %llu\n",start_time);

		return 1;
	}
	printf("out: %llu , %llu\n",
		(sched_rec_pit-TTE_PRECISION),
		(sched_rec_pit+TTE_PRECISION));
	return 0;
}

unsigned long long transClk_to_clk (unsigned long long transClk){
	return (transClk/12)>> 16;
}

unsigned char is_tte(unsigned int addr){
	unsigned CT = mem_iord(addr + 0);
	if(CT == TTE_CT_MARKER){ 
		return 1;
	}
	return 0;
}

void tte_initialize(unsigned int int_period){ 
	integration_period=int_period;
	eth_iowr(0x40, 0x1D000400);
	eth_iowr(0x44, 0x00000289);
	eth_iowr(0x00, 0x0000A023); //exactly like eth_mac_initialize, but with pro-bit set
	
	exc_register(16, &tte_clock_tick);

  	intr_unmask_all();
  	intr_clear_all_pending();
  	intr_enable();

  	start_time = 0; 

	return;
}

void tte_prepare_test_data(unsigned int tx_addr, unsigned char data){
  for(int i=0; i<16; i++){
    mem_iowr_byte(tx_addr + i, frame[i]);
  }
  for(int i=16; i<1514; i++){
    mem_iowr_byte(tx_addr + i, data);
  }
}

void tte_stop_sending(){
  sending=0;
}

void tte_send_test_data(unsigned int tx_addr){ 
  static unsigned char num = 0;
  num++;
  if(num%2==0){
    mem_iowr_byte(0x800 + 14,num);
    eth_mac_send(0x800, 1514);
  }
  else{
    mem_iowr_byte(0xdea + 14,num);
    eth_mac_send(0xdea, 1514);
  }
  return;
}

void tte_clock_tick(void) {
  exc_prologue();
  
  if(sending==1){
    timer_time += ((integration_period/5)*2);
    arm_clock_timer(timer_time);
    tte_send_test_data(0x800);
  }

  exc_epilogue();
}
