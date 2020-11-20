/*
 * Copyright (c) 2008 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Stanford University nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STANFORD
 * UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * This application is used to test the threaded version of the API for accessing
 * the software based SineSensor usable by any platform for demonstration purposes.
 * 
 * This application simply takes sensor readings in an infinite loop from the
 * SineSensor and forwards them over the serial interface.  Upon successful
 * transmission, LED0 is toggled.
 *
 * @author Kevin Klues (klueska@cs.stanford.edu)
 */

module TestSineSensorC {
  uses {
    interface Boot;
    interface Thread as MainThread;
    interface BlockingRead<uint16_t>;
    interface BlockingStdControl as AMControl;
    interface BlockingAMSend;
    interface Packet;
    interface Leds;
  }
}

implementation {
  event void Boot.booted() {
    call MainThread.start(NULL);
  }

  event void MainThread.run(void* arg) {
    uint16_t* var;
    message_t msg;
    var = call Packet.getPayload(&msg, sizeof(uint16_t));

    while( call AMControl.start() != SUCCESS );    
    for(;;){
      while( call BlockingRead.read(var) != SUCCESS );
      while( call BlockingAMSend.send(AM_BROADCAST_ADDR, &msg, sizeof(uint16_t)) != SUCCESS );
      call Leds.led0Toggle();
    }
  }
}

