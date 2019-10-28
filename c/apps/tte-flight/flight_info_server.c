#include "flight_info_server.h"

fsim_msg_circbuf_t fsim_msg_circbuf = {
  .len = 0,
  .rdId = 0,
  .wrId = 0
};

fsim_msg_t fsimDataoutput;
acf_orientation_t acfOrientation;

ethif_t ttethif = {
  .base_addr = PATMOS_IO_ETH,
  .rx_addr = 0x000,
  .tx_addr = 0x800
};

unsigned char CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };
unsigned char VL0[] = { 0x0F, 0xA1 };
unsigned char VL1[] = { 0x0F, 0xA2 };

int sched_errors = 0;
unsigned beaconReceived = 0;

volatile _IODEV int *led_ptr = (volatile _IODEV int *) PATMOS_IO_LED;
volatile _IODEV int *key_ptr = (volatile _IODEV int *) PATMOS_IO_KEYS;
volatile _IODEV unsigned* disp_ptr = (volatile _IODEV unsigned*) PATMOS_IO_SEGDISP;

int main(){
    *led_ptr = 0xFF;
    puts("FLISERV (Flight Info Server) started");
    puts("Initializing Ethernet communication");
    eth_mac_initialize();
    set_mac_address(0x1D000400, 0x00000289);
    eth_iowr(ttethif.rx_addr, 0x00006000); // set NOT empty, IRQ and wrap
    // int_period = 10ms, cluster cycle=20ms, CT, 2 VLs sending, max_delay , comp_delay, precision (in clock cycles)
    tte_initialize(INT_PERIOD*MS_TO_MAJA, CYC_PERIOD*MS_TO_MAJA, CT, 2, TTE_MAX_TRANS_DELAY, TTE_COMP_DELAY, TTE_PRECISION);
    tte_init_VL(0, 8, 40);  // VL 4001 starts at 0.8ms and has a period of 4ms
    tte_init_VL(1, 10, 20); // VL 4002 starts at 1ms and has a period of 2ms
    ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 253});
		arp_table_init();
		print_comm_info();
    ttethif.base_addr = PATMOS_IO_ETH;
    ttethif.rx_addr = 0x000;
    ttethif.tx_addr = 0x800;
    *led_ptr = 0xF0;
    puts("Start serving");
    beaconReceived = 0;
    // waitInitArp();
    waitFsimBeacon();
    execute();
    puts("Server stopped");
}

void waitInitArp(){
  unsigned char arpReceived = 0;
  while (!arpReceived){ //Wait here for the beacon
    switch(checkForFrame(ttethif, 0)){
      case ARP:
          arpReceived = 1;
        break;
    }
  }
}

void waitFsimBeacon(){
  while (beaconReceived == 0){ //Wait here for the beacon
      switch(checkForFrame(ttethif, 0)){
        case UDP:
          if((udp_get_destination_port(ttethif.rx_addr)==FSIM_BEACON_PORT)){
            beaconReceived++;
          }
          break;
      }
    }
}

void execute(){
  volatile unsigned long long startTime = get_cpu_usecs();
  unsigned short udpPort = 0x0;
  while(*key_ptr != 0xE){
    if(get_cpu_usecs() - startTime >= FSIM_PERIOD - FSIM_MSG_RXWIN_TIMEOUT/2){
      startTime = get_cpu_usecs();
      switch(*led_ptr = checkForFrame(ttethif, FSIM_MSG_RXWIN_TIMEOUT/2)){
        case UDP:
          if((udpPort = udp_get_destination_port(ttethif.rx_addr))==FSIM_DATAOUT_PORT){
            handle_fsim_msg(ttethif, &fsim_msg_circbuf);
            printf("0x%x\t%3.4f\t%3.4f\t%3.4f\n", beaconReceived, acfOrientation.pitch.val, acfOrientation.roll.val, acfOrientation.headT.val);
            tte_schedsend_flightdata(ttethif, &fsim_msg_circbuf);
          }
          else
          if(udpPort==FSIM_BEACON_PORT){
            beaconReceived++;
            print_segment(beaconReceived);
          } else {
            print_segment(0xEE000000 | udpPort);
          }
          break;
        default:
          startTime = 0; //if we receive non-TT frame in a TT slot we shall reset the time to immediately catch the next
      }
    }
  }
  printf("received_beacons = %d\n", beaconReceived);
}

int checkForFrame(ethif_t ethif, const unsigned int timeout){
	if(eth_mac_receive(ethif.rx_addr, timeout)){
		// printf("%d", mac_packet_type(rx_addr));
		switch (mac_packet_type(ethif.rx_addr)) {
		case ICMP:
			return (icmp_process_received(ethif.rx_addr, ethif.tx_addr)==0) ? -ICMP : ICMP;
		case UDP:
			return UDP;
		case ARP:
			return (arp_process_received(ethif.rx_addr, ethif.tx_addr)==0) ? -ARP : ARP;
		default:
			return 0xF0;
		}
	} else {
		return 0x80;
	}
}

__attribute__((noinline))
void handle_fsim_msg(ethif_t ethif, fsim_msg_circbuf_t *fsim_msg_buf){
  udp_get_data(ethif.rx_addr, (unsigned char*) &fsimDataoutput, udp_get_data_length(ethif.rx_addr));
  //fix ugly endianness
  acfOrientation.pitch.bytes[0] = fsimDataoutput.pitch[3];
  acfOrientation.pitch.bytes[1] = fsimDataoutput.pitch[2];
  acfOrientation.pitch.bytes[2] = fsimDataoutput.pitch[1];
  acfOrientation.pitch.bytes[3] = fsimDataoutput.pitch[0];

  acfOrientation.roll.bytes[0] = fsimDataoutput.roll[3];
  acfOrientation.roll.bytes[1] = fsimDataoutput.roll[2];
  acfOrientation.roll.bytes[2] = fsimDataoutput.roll[1];
  acfOrientation.roll.bytes[3] = fsimDataoutput.roll[0];

  acfOrientation.headT.bytes[0] = fsimDataoutput.headT[3];
  acfOrientation.headT.bytes[1] = fsimDataoutput.headT[2];
  acfOrientation.headT.bytes[2] = fsimDataoutput.headT[1];
  acfOrientation.headT.bytes[3] = fsimDataoutput.headT[0];

  acfOrientation.headM.bytes[0] = fsimDataoutput.headM[3];
  acfOrientation.headM.bytes[1] = fsimDataoutput.headM[2];
  acfOrientation.headM.bytes[2] = fsimDataoutput.headM[1];
  acfOrientation.headM.bytes[3] = fsimDataoutput.headM[0];

  //TODO: further pre-processing, put values in circular buffer for transmission
}

void tte_schedsend_flightdata(ethif_t ethif, fsim_msg_circbuf_t *fsim_msg_buf){
  tte_prepare_data(ethif.tx_addr, VL0, (unsigned char*) &acfOrientation, sizeof(acf_orientation_t));
  if (!tte_schedule_send(0x2000, 1514, 0))
    sched_errors++;
}

void print_segment(unsigned number){
  *(disp_ptr + 0) = number & 0xF;
  *(disp_ptr + 1) = (number >> 4) & 0xF;
  *(disp_ptr + 2) = (number >> 8) & 0xF;
  *(disp_ptr + 3) = (number >> 12) & 0xF;
  *(disp_ptr + 4) = (number >> 16) & 0xF;
  *(disp_ptr + 5) = (number >> 20) & 0xF;
  *(disp_ptr + 6) = (number >> 24) & 0xF;
  *(disp_ptr + 7) = (number >> 28) & 0xF;
}

void print_comm_info(){
	printf("\nGeneral info:\n");
	printf("\tMAC: %llx", get_mac_address());
	printf("\n\tIP: ");
	ipv4_print_my_ip();
	printf("\n");
	arp_table_print();
	printf("\n");
	return;
}

void print_hex_bytes(unsigned char byte_buffer[], unsigned int len){
	int i=0;
	while (i < len)
	{
		if(i > 0 && i % 16 == 0){
			printf("%02X\n",(unsigned)byte_buffer[i]);
		}else{
			printf("%02X ",(unsigned)byte_buffer[i]);
		}
	 	i++;
	}
	puts("\n");
}
