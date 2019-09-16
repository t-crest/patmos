#ifndef INTERNOC_NET_H
#define INTERNOC_NET_H

#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/eth_mac_driver.h"
#include "ethlib/ptp1588.h"

PTPPortInfo thisPtpPortInfo;

extern const unsigned int rx_addr;
extern const unsigned int tx_addr;

int receiveAndHandleFrame(const unsigned int timeout);
int demoMasterLoop(unsigned int syncPeriod);
int demoSlaveLoop();

#endif