#include "internoc_net.h"

unsigned int ptpSeqId = 0;
unsigned int startTime = 0;

const unsigned int rx_addr = 0x000;
const unsigned int tx_addr = 0x800;

const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "INTERNOC"};

const unsigned char multicastip[4] = {224, 0, 0, 255};

int rxPacketCount = 0;
int txPacketCount = 0;

int receiveAndHandleFrame(const unsigned int timeout){
  unsigned short dest_port;
  if(eth_mac_receive(rx_addr, timeout)){
        switch (mac_packet_type(rx_addr)) {
            case ICMP:
                return (icmp_process_received(rx_addr, tx_addr)==0) ? -ICMP : ICMP;
            case UDP:
                dest_port = udp_get_destination_port(rx_addr);
                if((dest_port==PTP_EVENT_PORT) || (dest_port==PTP_GENERAL_PORT)){
                    return PTP;
                } else if (dest_port==5353){
                    return MDNS;
                } else if(dest_port==696){
                    return INTERNOC_MSG;
                }
            case TCP:
                return TCP;
            case ARP:
                return (arp_process_received(rx_addr, tx_addr)==0) ? -ARP : ARP;
            case TTE_PCF:
                return TTE_PCF;
            default:
                return 0x0F;
        }
    } else {
    return -1;
  }
}

int demoMasterLoop(unsigned int syncPeriod){
  unsigned int dead_val;
  unsigned char ans = 0;
  if(get_ptp_usecs(thisPtpPortInfo.eth_base) - startTime >= syncPeriod){
    startTime = get_ptp_usecs(thisPtpPortInfo.eth_base);
    ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_MULTICAST_MAC, PTP_MULTICAST_IP, ptpSeqId, PTP_SYNC_MSGTYPE); //30 us measured @ 80MHz
    *((volatile _IODEV unsigned *)PATMOS_IO_DEADLINE) = 30400;  //wcet of demoSlaveLoop() wait otherwise is too fast for slave patmos  
    dead_val = *((volatile _IODEV unsigned *)PATMOS_IO_DEADLINE);
    ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_MULTICAST_MAC, PTP_MULTICAST_IP, ptpSeqId, PTP_FOLLOW_MSGTYPE);
    if((receiveAndHandleFrame(PTP_REQ_TIMEOUT)) == PTP){
      if((ptpv2_process_received(thisPtpPortInfo, tx_addr, rx_addr)) == PTP_DLYREQ_MSGTYPE){
        ptpSeqId = rxPTPMsg.head.sequenceId++;
        ans = 1;
      }
    }
  }
  return ans;
}

int demoSlaveLoop(){
  unsigned char ans = 0;
  if((receiveAndHandleFrame(PTP_SYNC_TIMEOUT)) == PTP){
    if((ptpv2_process_received(thisPtpPortInfo, tx_addr, rx_addr)) == PTP_DLYRPLY_MSGTYPE){
      ans = 1;
    }
  }
  return ans;
}