/*
 * Copyright (c) 2008 Johns Hopkins University.
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
*/

/**
 * This application is used to test the threaded version of the API for performing
 * block storage.
 * 
 * This application first checks the size of the block storage volume, and
 * erases it. Then, it randomly writes records, followed by a verification
 * with read.
 * 
 * Successful running of this application results in LED0 being ON
 * throughout the duration of the erase, write, and read sequence. Finally,
 * if all tests pass, LED1 is turned ON. Otherwise, all three LEDs are
 * turned ON to indicate problems.
 *
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 */

#include "Storage.h"

module TestBlockStorageP
{
  uses {
    interface Boot;
    interface Leds;
    interface Thread as TinyThread1;
    interface BlockingBlock as BlockingBlock1;
    interface Random;
  }
}

implementation
{
  event void Boot.booted() {
    call TinyThread1.start(NULL);
  }
  
  event void TinyThread1.run(void* arg)
  {
    int i;
    error_t error;
#if defined USE_AT45DB
    storage_len_t expectedVolumeSize = 262144;
#elif defined USE_STM25P
    storage_len_t expectedVolumeSize = 1048576;
#endif

    call Leds.set(1);

    if (call BlockingBlock1.getSize() != expectedVolumeSize) {
      call Leds.set(7);
      return;
    }
    
    error = call BlockingBlock1.erase();
    if (error != SUCCESS) {
      call Leds.set(7);
      return;
    }
    
    for (i = 0; i < 50; i++) {
      storage_addr_t writeAddr = call Random.rand32() % (call BlockingBlock1.getSize() - sizeof(storage_addr_t));
      storage_len_t len = sizeof(storage_addr_t);
      storage_addr_t readBuf;
    
      error = call BlockingBlock1.write(writeAddr, &writeAddr, &len);
      if (error == SUCCESS) {
        len = sizeof(storage_addr_t);
        call BlockingBlock1.read(writeAddr, &readBuf, &len);
        if (readBuf != writeAddr) {
          call Leds.set(7);
          return;
        }
      } else {
        call Leds.set(7);
        return;
      }
    }
    
    call Leds.set(2);
  }
}
