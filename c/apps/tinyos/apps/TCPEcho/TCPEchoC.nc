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

#include <lib6lowpan/6lowpan.h>

configuration TCPEchoC {

} implementation {
  components MainC, LedsC;
  components TCPEchoP;

  TCPEchoP.Boot -> MainC;
  TCPEchoP.Leds -> LedsC;

  components new TimerMilliC();
  components IPStackC;

  TCPEchoP.RadioControl -> IPStackC;
  components new UdpSocketC() as Echo,
    new UdpSocketC() as Status;
  TCPEchoP.Echo -> Echo;

  components new TcpSocketC() as TcpEcho;
  TCPEchoP.TcpEcho -> TcpEcho;

  components new TcpSocketC() as TcpWeb, HttpdP;
  HttpdP.Boot -> MainC;
  HttpdP.Leds -> LedsC;
  HttpdP.Tcp -> TcpWeb;

  TCPEchoP.Status -> Status;

  TCPEchoP.StatusTimer -> TimerMilliC;

  components UdpC, IPDispatchC;

  TCPEchoP.IPStats -> IPDispatchC.BlipStatistics;
  TCPEchoP.UDPStats -> UdpC;

  components RandomC;
  TCPEchoP.Random -> RandomC;

  components UDPShellC;

  components StaticIPAddressTosIdC; // Use TOS_NODE_ID in address
  //components StaticIPAddressC; // Use LocalIeee154 in address
}
