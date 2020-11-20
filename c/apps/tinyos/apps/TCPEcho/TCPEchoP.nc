/*
 * Copyright (c) 2008 The Regents of the University  of California.
 * All rights reserved."
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <IPDispatch.h>
#include <lib6lowpan/lib6lowpan.h>
#include <lib6lowpan/ip.h>

#include "UDPReport.h"

#define REPORT_PERIOD 75L

module TCPEchoP {
  uses {
    interface Boot;
    interface SplitControl as RadioControl;

    interface UDP as Echo;
    interface UDP as Status;
    interface Tcp as TcpEcho;

    interface Leds;

    interface Timer<TMilli> as StatusTimer;

    interface BlipStatistics<ip_statistics_t> as IPStats;
    interface BlipStatistics<udp_statistics_t> as UDPStats;

    interface Random;

  }

} implementation {

  bool timerStarted;
  nx_struct udp_report stats;
  struct sockaddr_in6 route_dest;

#ifndef SIM
#define CHECK_NODE_ID
#else
#define CHECK_NODE_ID if (TOS_NODE_ID == BASESTATION_ID) return
#endif

  event void Boot.booted() {
    CHECK_NODE_ID;
    call RadioControl.start();
    timerStarted = FALSE;

    call IPStats.clear();


#ifdef REPORT_DEST
    route_dest.sin6_port = hton16(7000);
    inet_pton6(REPORT_DEST, &route_dest.sin6_addr);
    call StatusTimer.startOneShot(call Random.rand16() % (1024 * REPORT_PERIOD));
#endif

    dbg("Boot", "booted: %i\n", TOS_NODE_ID);
    call Echo.bind(7);
    call TcpEcho.bind(7);
    call Status.bind(7001);
  }

  event void RadioControl.startDone(error_t e) {

  }

  event void RadioControl.stopDone(error_t e) {

  }

  event void Status.recvfrom(struct sockaddr_in6 *from, void *data,
                             uint16_t len, struct ip6_metadata *meta) {

  }

  event void Echo.recvfrom(struct sockaddr_in6 *from, void *data,
                           uint16_t len, struct ip6_metadata *meta) {
    CHECK_NODE_ID;
    call Echo.sendto(from, data, len);
  }

  enum {
    STATUS_SIZE = sizeof(ip_statistics_t) + sizeof(udp_statistics_t),
  };


  event void StatusTimer.fired() {

    if (!timerStarted) {
      call StatusTimer.startPeriodic(1024 * REPORT_PERIOD);
      timerStarted = TRUE;
    }

    stats.seqno++;
    stats.sender = TOS_NODE_ID;

    call IPStats.get(&stats.ip);
    call UDPStats.get(&stats.udp);

    call Status.sendto(&route_dest, &stats, sizeof(stats));
  }

  /*
   * Example code for setting up a TCP echo socket.
   */

  bool sock_connected = FALSE;
  char tcp_buf[150];

  event bool TcpEcho.accept(struct sockaddr_in6 *from,
                            void **tx_buf, int *tx_buf_len) {
    *tx_buf = tcp_buf;
    *tx_buf_len = 150;
    return TRUE;
  }
  event void TcpEcho.connectDone(error_t e) {

  }
  event void TcpEcho.recv(void *payload, uint16_t len) {
    if (call TcpEcho.send(payload,len) != SUCCESS)
      call Leds.led2Toggle();
  }
  event void TcpEcho.closed(error_t e) {
    call Leds.led0Toggle();
    call TcpEcho.bind(7);
  }
  event void TcpEcho.acked() {}

}
