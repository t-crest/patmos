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

typedef union 
{
  unsigned char b[4];
  float val;
} fsimFloat;


int main(){
    puts("FLISERV (Flight Info Server) started");
    puts("Initializing Ethernet communication");
    eth_mac_initialize();
    ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 253});
		arp_table_init();
		print_comm_info();
    ttethif.base_addr = PATMOS_IO_ETH;
    ttethif.rx_addr = 0x000;
    ttethif.tx_addr = 0x800;
    puts("Start serving");
    execute();
}

void execute(){
  volatile unsigned long long elapsedTime = 0;
  volatile unsigned long long startTime = get_cpu_usecs();
  while(1){
    elapsedTime = get_cpu_usecs() - startTime;                                      //calculate elapsed time
    if(elapsedTime >= FSIM_PERIOD - 40){
      startTime = get_cpu_usecs();
      *led_ptr ^= ~1UL << 7;
      switch(*led_ptr = checkForFrame(ttethif, FSIM_MSG_TIMEOUT)){
        case UDP:
          if((udp_get_destination_port(ttethif.rx_addr)==FSIM_PORT)){
            handle_fsim_msg(ttethif, &fsim_msg_circbuf);
              printf("Xplane-dataPrologue (bytes=%lu): %s\n", sizeof(fsimDataoutput), (char*) &fsimDataoutput.prologue);
              printf("Xplane-dataID = %x\n", fsimDataoutput.index[0]);
              printf("Xplane-aircraft-pitch = %3.4f\n", acfOrientation.pitch.val);
              print_hex_bytes(fsimDataoutput.pitch, 4);
            send_sensor_data(ttethif, &fsim_msg_circbuf);
          }
          break;
        default:
          continue;
      }
    }
  }
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
			return UNSUPPORTED;
		}
	} else {
		return UNSUPPORTED;
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

void send_sensor_data(ethif_t ethif, fsim_msg_circbuf_t *fsim_msg_buf){
  udp_send(ethif.tx_addr, ethif.rx_addr, (unsigned char[4]){192, 168, 2, 254}, 666, 999, acfOrientation.pitch.bytes, 4, 0);
  
  //TODO: pop values from circular buffer and distribute to flight controller nodes
}

void print_segment(unsigned number)
{
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
