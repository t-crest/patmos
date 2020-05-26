
#include "internoc_driver.h"

unsigned int internoc_packed_id = 1;

InterNoCConfig internoc_init_config(unsigned int nr_cores, unsigned char gateway_ip[4], unsigned int gateway_core_id)
{
  InterNoCConfig new_config;
  new_config.cores = nr_cores;
  new_config.my_ip[0] = gateway_ip[0];
  new_config.my_ip[1] = gateway_ip[1];
  new_config.my_ip[2] = gateway_ip[2];
  new_config.my_ip[3] = get_cpuid();
  new_config.gateway_ip[0] = gateway_ip[0];
  new_config.gateway_ip[1] = gateway_ip[1];
  new_config.gateway_ip[2] = gateway_ip[2];
  new_config.gateway_ip[3] = gateway_ip[3];
  new_config.gateway_core = gateway_core_id;
  new_config.core_links_num = nr_cores-1;
  new_config.core_links = (core_link_t*) malloc(new_config.core_links_num * sizeof(core_link_t));

  unsigned j = 0;
  for(unsigned i=0; i<nr_cores; i++){
    if(get_cpuid() != i){
      unsigned rxChannId = i + get_cpuid()*nr_cores*2;
      unsigned txChannId = get_cpuid() + i*nr_cores*2;
      new_config.core_links[j].chanRx = mp_create_qport(rxChannId, SINK, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
      new_config.core_links[j].chanTx = mp_create_qport(txChannId, SOURCE, MP_CHAN_BUF_SIZE, MP_CHAN_NUM_BUF);
      j++;
    }
  }

  mp_init_ports();
  return new_config;
}

int internoc_ip_to_corelinkid(InterNoCConfig config, const unsigned char ip_addr[4])
{
  if(ip_addr[2] == config.gateway_ip[2])
  {
    for(unsigned i = 0; i < config.core_links_num; i++)
    {
      if(config.core_links[i].chanTx->remote == ip_addr[3]){
        return i;
      }
    }
  } else {
    for(unsigned i = 0; i < config.core_links_num; i++)
    {
      if(config.core_links[i].chanTx->remote == config.gateway_core){
        return i;
      }
    }
  }
  return -1;
}

qpd_t * internoc_get_txbuffer(InterNoCConfig config, const unsigned char dst_ip[4])
{
  unsigned int index = internoc_ip_to_corelinkid(config, dst_ip);
  return config.core_links[index].chanTx;
}

qpd_t * internoc_get_rxbuffer(InterNoCConfig config, const unsigned char src_ip[4])
{
  unsigned int index = internoc_ip_to_corelinkid(config, src_ip);
  return config.core_links[index].chanRx;
}

_SPM udp_t* internoc_build_packet(InterNoCConfig config, 
                          unsigned char src_ip[4], unsigned char dst_ip[4], 
                          unsigned short src_port, unsigned short dst_port, 
                          unsigned char* data, unsigned short data_length)
{
  qpd_t* packet_channel = internoc_get_txbuffer(config, dst_ip);
  _SPM udp_t *packet_ptr = (_SPM udp_t *) packet_channel->write_buf;

	packet_ptr->ip_head.ver_headlen = 0x45;
  packet_ptr->ip_head.dscp_ecn = 0x00;
  packet_ptr->ip_head.length = 20 + sizeof(udphead_t) + data_length;
  packet_ptr->ip_head.identification = internoc_packed_id;
  packet_ptr->ip_head.flags_fragmentoff = 0x4000;
  packet_ptr->ip_head.ttl = 0x40;
  packet_ptr->ip_head.protocol = 0x11;
  packet_ptr->ip_head.head_checksum = 0x0;
  packet_ptr->ip_head.checksum = 0x0;
  packet_ptr->ip_head.source_ip[0] = src_ip[0];
  packet_ptr->ip_head.source_ip[1] = src_ip[1];
  packet_ptr->ip_head.source_ip[2] = src_ip[2];
  packet_ptr->ip_head.source_ip[3] = src_ip[3];
  packet_ptr->ip_head.destination_ip[0] = dst_ip[0];
  packet_ptr->ip_head.destination_ip[1] = dst_ip[1];
  packet_ptr->ip_head.destination_ip[2] = dst_ip[2];
  packet_ptr->ip_head.destination_ip[3] = dst_ip[3];
  packet_ptr->udp_head.source_port = src_port;
  packet_ptr->udp_head.destination_port = dst_port;
  packet_ptr->udp_head.data_length = data_length;
  packet_ptr->udp_head.checksum = 0x0;
  packet_ptr->data = ((unsigned char * _SPM) (packet_channel->write_buf)) + 20 + sizeof(udphead_t);  //set the data address of the assigned SPM buffer after the IP/UDP head
  for(unsigned i=0; i<data_length; i+=1)
  {
      packet_ptr->data[i] = data[i];
  }
  internoc_packed_id++;
  return packet_ptr;
}

void internoc_send(InterNoCConfig config,  const unsigned char dst_ip[4] , unsigned int timeout)
{
  mp_send(internoc_get_txbuffer(config, dst_ip), timeout);
}

_SPM udp_t* internoc_recv(InterNoCConfig config, const unsigned char src_ip[4], unsigned int timeout)
{
  qpd_t *packet_channel = internoc_get_rxbuffer(config, src_ip);
  mp_recv(packet_channel, timeout);
  _SPM udp_t* packet_ptr = (_SPM udp_t* ) packet_channel->read_buf;
  mp_ack(packet_channel, timeout);
  return packet_ptr;
}

void prin_core_channels(InterNoCConfig config)
{
  printf("-------------------------\n");
  printf("Core %u assigned RX channels are:\n", get_cpuid());
  for(unsigned i = 0; i < config.core_links_num; i++){
      printf("link[%u] from %u, receive at buffer %p (size = %lu bytes)\n", i, config.core_links[i].chanRx->remote, config.core_links[i].chanRx->write_buf, MP_CHAN_BUF_SIZE);
  }
  printf("-------------------------\n");
  printf("Core %u assigned TX channels are:\n", get_cpuid());
  for(unsigned i = 0; i < config.core_links_num; i++){
      printf("link[%u] to %u, receive at buffer %p (size = %lu bytes)\n", i, config.core_links[i].chanTx->remote, config.core_links[i].chanTx->write_buf, MP_CHAN_BUF_SIZE);
  }
  printf("-------------------------\n\n");
}