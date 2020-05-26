#pragma once

#include "libmp/mp.h"
#include "udp.h"

#define INTERNOC_PACKET_SIZE(data_len) 20 + sizeof(udphead_t) + data_len

#define INTERNOC_MAX_PAYLOAD_SIZE 372 * sizeof(char)
#define INTERNOC_MAX_PACKET_SIZE INTERNOC_PACKET_SIZE(INTERNOC_MAX_PAYLOAD_SIZE)

#define MP_CHAN_NUM_BUF 1
#define MP_CHAN_BUF_SIZE INTERNOC_MAX_PACKET_SIZE

typedef struct {
  qpd_t *chanRx;
  qpd_t *chanTx;
} core_link_t;

typedef struct
{   
    unsigned int cores;
    unsigned char my_ip[4];
    unsigned char gateway_ip[4];
    unsigned int gateway_core;
    unsigned int core_links_num;
    core_link_t *core_links;
} InterNoCConfig;

typedef struct{
  unsigned long long timestamp; // 8-byte
  unsigned int length; // 4-byte
} InterNoCMessageHead;

typedef struct{
  InterNoCMessageHead header;
  unsigned char* payload; // 4-byte
} InterNoCMessage;

extern unsigned int internoc_packed_id;

InterNoCConfig internoc_init_config(unsigned int nr_cores, unsigned char gateway_ip[4], unsigned int gateway_core_id);

int internoc_ip_to_corelinkid(InterNoCConfig config, const unsigned char ip_addr[4]);

_SPM udp_t* internoc_build_packet(InterNoCConfig config, 
                                  unsigned char src_ip[4], unsigned char dst_ip[4], 
                                  unsigned short src_port, unsigned short dst_port, 
                                  unsigned char* data, unsigned short data_length);

qpd_t * internoc_get_txbuffer(InterNoCConfig config, const unsigned char dst_ip[4]);

qpd_t * internoc_get_rxbuffer(InterNoCConfig config, const unsigned char src_ip[4]);

void internoc_send(InterNoCConfig config,  const unsigned char dst_ip[4] , unsigned int timeout);

_SPM udp_t* internoc_recv(InterNoCConfig config, const unsigned char src_ip[4], unsigned int timeout);

void prin_core_channels(InterNoCConfig config);