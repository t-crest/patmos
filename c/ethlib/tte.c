#include "tte.h"

unsigned int integration_period; //clock cycles at 80Mhz (12.5 ns)
unsigned int integration_cycle; //integer
static unsigned long long start_time; //clock cycles at 80MHz (12.5 ns)
static unsigned long long timer_time; //clock cycles at 80MHz (12.5 ns)
static unsigned long long receive_pit; //clock cycles at 80MHz (12.5 ns)
static unsigned long long scheduled_pit; //clock cycles at 80MHz (12.5 ns)
unsigned char CT_marker[4];
unsigned char mac[6] = {
    0x02, 0x89, 0x1D, 0x00, 0x04, 0x00
}; 

struct VL{
   unsigned char max_queue;
   unsigned int queue[10]; //magic number, arrays too big for VL0
   unsigned int sizeQueue[10]; //not sure how to get rid of them though
   unsigned char addplace;
   unsigned char rmplace;
};

struct VL *VLarray;
unsigned int sched[MAX_SCHED] = {16,4,20,16,4,20,16,4,20,16,4,20,16,4,20};
unsigned int VLsched[MAX_SCHED] = {1,1,0,1,1,0,1,1,0,1,1,0,1,1,0};
unsigned char schedplace;
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

		if(integration_cycle==0){
		  schedplace=0;
		  timer_time = start_time+((integration_period/100)*10);
		  arm_clock_timer(timer_time);
		}
		start_time += integration_period;

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
	for(int i=0;i<4;i++){
	  if(mem_iord_byte(addr + i)!=CT_marker[i]){
	    return 0;
	  }
	}
	return 1;
}

void tte_initialize(unsigned int int_period, unsigned char CT[]){ 
	integration_period=int_period;
	for(int i=0;i<4;i++){
	  CT_marker[i]=CT[i];
        }
	eth_iowr(0x40, 0x1D000400);
	eth_iowr(0x44, 0x00000289);
	eth_iowr(0x00, 0x0000A023); //exactly like eth_mac_initialize, but with pro-bit set
	
	exc_register(16, &tte_clock_tick);

  	intr_unmask_all();
  	intr_clear_all_pending();
  	intr_enable();

  	start_time = 0; 

	VLarray = realloc(VLarray, 2 * sizeof(struct VL));

	VLarray[0].addplace=0;
	VLarray[0].rmplace=0;
	VLarray[0].max_queue=5;
	VLarray[1].addplace=0;
	VLarray[1].rmplace=0;
	VLarray[1].max_queue=10;

	return;
}

void tte_prepare_header(unsigned int tx_addr, unsigned char VL[], unsigned char ethType[]){
  for(int i=0; i<4; i++){
    mem_iowr_byte(tx_addr + i, CT_marker[i]);
  }
  mem_iowr_byte(tx_addr + 4, VL[0]);
  mem_iowr_byte(tx_addr + 5, VL[1]);

  for(int i=6; i<12; i++){
    mem_iowr_byte(tx_addr + i, mac[i-6]);
  }

  mem_iowr_byte(tx_addr + 12, ethType[0]);
  mem_iowr_byte(tx_addr + 13, ethType[1]);
}

void tte_prepare_test_data(unsigned int tx_addr, unsigned char VL[], unsigned char data, int length){
  unsigned char ethType[2];
  ethType[0]=((length-14)>>8) & 0xFF;
  ethType[1]=(length-14) & 0xFF;

  tte_prepare_header(tx_addr,VL,ethType);

  for(int i=14; i<length; i++){
    mem_iowr_byte(tx_addr + i, data);
  }
}

void tte_stop_ticking(){
  for(int i=0;i<VLarray[0].max_queue;i++){
    VLarray[0].queue[i]=0;
    VLarray[0].sizeQueue[i]=0;
  }
  for(int i=0;i<VLarray[1].max_queue;i++){
    VLarray[1].queue[i]=0;
    VLarray[1].sizeQueue[i]=0;
  }
}

void tte_schedule_send(unsigned int addr,unsigned int size,unsigned char i){
  if(VLarray[i].queue[VLarray[i].addplace]==0){
    VLarray[i].queue[VLarray[i].addplace]=addr;
    VLarray[i].sizeQueue[VLarray[i].addplace]=size;
    VLarray[i].addplace++;
    if(VLarray[i].addplace==VLarray[i].max_queue){
      VLarray[i].addplace=0;
    }
    return;
  } 
  printf("scheduling error: queue[%d]: %d\n",VLarray[i].addplace,VLarray[i].queue[VLarray[i].addplace]);
}

void tte_send_data(unsigned char i){ 
  //static unsigned char num = 0;
  //num++;
  int tx_addr=VLarray[i].queue[VLarray[i].rmplace];
  VLarray[i].queue[VLarray[i].rmplace]=0;
  mem_iowr_byte(tx_addr + 14,VLarray[i].rmplace);
  eth_mac_send(tx_addr, VLarray[i].sizeQueue[VLarray[i].rmplace]);
  return;
}

void tte_clock_tick(void) {
  exc_prologue();
  
  timer_time += ((integration_period/100)*sched[schedplace]);
  schedplace++;
  if(schedplace==MAX_SCHED){
    schedplace=0;
  }
  else{
    arm_clock_timer(timer_time);
  }
  int i=VLsched[schedplace];
  if(VLarray[i].queue[VLarray[i].rmplace]>0){
    tte_send_data(i);
    VLarray[i].rmplace++;
    if(VLarray[i].rmplace==VLarray[i].max_queue){
      VLarray[i].rmplace=0;
    }
  }
  exc_epilogue();
}
