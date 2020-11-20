/* Copyright (c) 2007 Johns Hopkins University.
*  All rights reserved.
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
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 * @author Razvan Musaloiu-E. <razvanm@cs.jhu.edu>
 */

generic module FakeBlockWriterP(uint32_t size)
{
  provides interface BlockWrite;
  uses interface Leds;
}

implementation
{
  enum {
    S_IDLE,
    S_BUSY
  };
  
  storage_addr_t saddr;
  uint8_t *oribuf;
  uint8_t *sbuf;
  storage_len_t slen;
  uint8_t state;
  
  task void task_write()
  {
    error_t error = SUCCESS;
    
    while (slen > 0) {
      if (*sbuf != (saddr & 0xFF)) {
        error = FAIL;
call Leds.led2Toggle();
      }
      saddr++;
      sbuf++;
      slen--;
    }
    
    signal BlockWrite.writeDone(saddr, oribuf, slen, error);
    state = S_IDLE;
  }
  
  command error_t BlockWrite.write(storage_addr_t addr,
				   void* buf,
				   storage_len_t len)
  {
    if (state != S_IDLE) {
      return FAIL;
    }
    
    state = S_BUSY;
    saddr = addr;
    sbuf = buf;
    oribuf = buf;
    slen = len;
    post task_write();
    return SUCCESS;
  }
  
  task void task_erase()
  {
    signal BlockWrite.eraseDone(SUCCESS);
  }
  
  command error_t BlockWrite.erase()
  {
    post task_erase();
    return SUCCESS;
  }
  
  task void task_sync()
  {
    signal BlockWrite.syncDone(SUCCESS);
  }
  
  command error_t BlockWrite.sync()
  {
    post task_sync();
    return SUCCESS;
  }
}
