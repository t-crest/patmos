/*
 * Copyright (c) 2009 Johns Hopkins University.
 * All rights reserved.
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
 * - Neither the name of the copyright holder nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Simple test program for SAM3U's 12 bit ADC Read with LCD
 * @author Chieh-Jan Mike Liang
 * @author JeongGil Ko
 */

#include <color.h>
#include <lcd.h>

module MoteP
{
  uses {
    interface Boot;
    interface Leds;
    interface Read<uint16_t>;
    interface SplitControl as SerialSplitControl;
    interface Packet;
    interface Timer<TMilli>;
    interface Lcd;
    interface Draw;
  }
}

implementation
{
  event void Boot.booted()
  {
    while (call SerialSplitControl.start() != SUCCESS);
    call Lcd.initialize();
  }

  event void Lcd.initializeDone(error_t err)
  {
    if(err != SUCCESS)
    {
    }
    else
    {
      call Draw.fill(COLOR_RED);
      call Lcd.start();
    }
  }

  event void Lcd.startDone(){
  }

  event void SerialSplitControl.startDone(error_t error)
  {
    if (error != SUCCESS) {
      while (call SerialSplitControl.start() != SUCCESS);
    }else{
      call Timer.startPeriodic(256);
    }
  }

  event void SerialSplitControl.stopDone(error_t error) {}


  task void sample()
  {
    const char *start = "Start Sampling";
    call Leds.led0Toggle();
    call Draw.fill(COLOR_BLUE);
    call Draw.drawString(10,50,start,COLOR_RED);
    if(call Read.read() != SUCCESS)
    {
      const char *fail = "Read Failed";
      call Draw.drawString(10,70, fail, COLOR_RED);
    }

  }

  event void Read.readDone(error_t result, uint16_t value)
  {
    const char *fail = "Read done error";
    const char *good = "Read done success";
    call Draw.fill(COLOR_GREEN);
    if (result != SUCCESS) {
      call Draw.drawString(10,70,fail,COLOR_BLACK);
    }else{
      call Draw.drawString(10,70,good,COLOR_BLACK);
      call Draw.drawInt(100,100,value,1,COLOR_BLACK);
    }
  }

  event void Timer.fired() {
    post sample();
  }
}
