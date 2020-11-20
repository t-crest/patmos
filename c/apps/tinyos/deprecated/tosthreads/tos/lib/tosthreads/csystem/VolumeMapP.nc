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
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 * @author J� �gila Bitsch Link <jo.bitsch@cs.rwth-aachen.de>
 */

module VolumeMapP {
  provides { 
    interface BlockRead[uint8_t volume_id]; 
    interface BlockWrite[uint8_t volume_id];
    interface LogRead[uint8_t volume_id];
    interface LogWrite[uint8_t volume_id];
    interface ConfigStorage[uint8_t volume_id];
    interface Mount[uint8_t volume_id];
  }
  
  uses { 
    interface BlockRead as SubBlockRead[uint8_t volume_id]; 
    interface BlockWrite as SubBlockWrite[uint8_t volume_id]; 
    interface LogRead as SubLogRead[uint8_t volume_id];
    interface LogWrite as SubLogWrite[uint8_t volume_id];
    interface ConfigStorage as SubConfigStorage[uint8_t volume_id];
    interface Mount as SubMount[uint8_t volume_id];
  }
}

implementation {
  command error_t BlockRead.read[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return call SubBlockRead.read[volume_id](addr, buf, len);
  }

  event void SubBlockRead.readDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {
    signal BlockRead.readDone[volume_id](addr, buf, len, error);
  }

  command error_t BlockRead.computeCrc[uint8_t volume_id](storage_addr_t addr, storage_len_t len, uint16_t crc) {
    return call SubBlockRead.computeCrc[volume_id](addr, len, crc);
  }

  event void SubBlockRead.computeCrcDone[uint8_t volume_id](storage_addr_t addr, storage_len_t len, uint16_t crc, error_t error) {
    signal BlockRead.computeCrcDone[volume_id](addr, len, crc, error);
  }

  command storage_len_t BlockRead.getSize[uint8_t volume_id]() {
    return call SubBlockRead.getSize[volume_id]();
  }
  
  command error_t BlockWrite.write[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return call SubBlockWrite.write[volume_id](addr, buf, len);
  }

  event void SubBlockWrite.writeDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {
    signal BlockWrite.writeDone[volume_id](addr, buf, len, error);
  }

  command error_t BlockWrite.erase[uint8_t volume_id]() {
    return call SubBlockWrite.erase[volume_id]();
  }

  event void SubBlockWrite.eraseDone[uint8_t volume_id](error_t error) {
    signal BlockWrite.eraseDone[volume_id](error);
  }

  command error_t BlockWrite.sync[uint8_t volume_id]() {
    return call SubBlockWrite.sync[volume_id]();
  }

  event void SubBlockWrite.syncDone[uint8_t volume_id](error_t error) {
    signal BlockWrite.syncDone[volume_id](error);
  }
  
  default command error_t SubBlockRead.read[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return FAIL;
  }

  default command error_t SubBlockRead.computeCrc[uint8_t volume_id](storage_addr_t addr, storage_len_t len, uint16_t crc) {
    return FAIL;
  }

  default command storage_len_t SubBlockRead.getSize[uint8_t volume_id]() {
    return FAIL;
  }
  
  default command error_t SubBlockWrite.write[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return FAIL;
  }

  default command error_t SubBlockWrite.erase[uint8_t volume_id]() {
    return FAIL;
  }

  default command error_t SubBlockWrite.sync[uint8_t volume_id]() {
    return FAIL;
  }
  
  default event void BlockRead.readDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {}
  default event void BlockRead.computeCrcDone[uint8_t volume_id](storage_addr_t addr, storage_len_t len, uint16_t crc, error_t error) {}
  default event void BlockWrite.writeDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {}
  default event void BlockWrite.eraseDone[uint8_t volume_id](error_t error) {}
  default event void BlockWrite.syncDone[uint8_t volume_id](error_t error) {}
  
  command error_t LogRead.read[uint8_t volume_id](void* buf, storage_len_t len) {
    return call SubLogRead.read[volume_id](buf, len);
  }

  event void SubLogRead.readDone[uint8_t volume_id](void* buf, storage_len_t len, error_t error) {
    signal LogRead.readDone[volume_id](buf, len, error);
  }

  command storage_cookie_t LogRead.currentOffset[uint8_t volume_id]() {
    return call SubLogRead.currentOffset[volume_id]();
  }

  command error_t LogRead.seek[uint8_t volume_id](storage_cookie_t offset) {
    return call SubLogRead.seek[volume_id](offset);
  }

  event void SubLogRead.seekDone[uint8_t volume_id](error_t error) {
    signal LogRead.seekDone[volume_id](error);
  }

  command storage_len_t LogRead.getSize[uint8_t volume_id]() {
    return call SubLogRead.getSize[volume_id]();
  }
  
  command error_t LogWrite.append[uint8_t volume_id](void* buf, storage_len_t len) {
    return call SubLogWrite.append[volume_id](buf, len);
  }

  event void SubLogWrite.appendDone[uint8_t volume_id](void* buf, storage_len_t len, bool recordsLost, error_t error) {
    signal LogWrite.appendDone[volume_id](buf, len, recordsLost, error);
  }

  command storage_cookie_t LogWrite.currentOffset[uint8_t volume_id]() {
    return call SubLogWrite.currentOffset[volume_id]();
  }

  command error_t LogWrite.erase[uint8_t volume_id]() {
    return call SubLogWrite.erase[volume_id]();
  }

  event void SubLogWrite.eraseDone[uint8_t volume_id](error_t error) {
    signal LogWrite.eraseDone[volume_id](error);
  }

  command error_t LogWrite.sync[uint8_t volume_id]() {
    return call SubLogWrite.sync[volume_id]();
  }

  event void SubLogWrite.syncDone[uint8_t volume_id](error_t error) {
    signal LogWrite.syncDone[volume_id](error);
  }
  
  default command error_t SubLogRead.read[uint8_t volume_id](void* buf, storage_len_t len) {
    return FAIL;
  }

  default command storage_cookie_t SubLogRead.currentOffset[uint8_t volume_id]() {
    return 0;
  }

  default command error_t SubLogRead.seek[uint8_t volume_id](storage_cookie_t offset) {
    return FAIL;
  }

  default command storage_len_t SubLogRead.getSize[uint8_t volume_id]() {
    return 0;
  }
  
  default command error_t SubLogWrite.append[uint8_t volume_id](void* buf, storage_len_t len) {
    return FAIL;
  }

  default command storage_cookie_t SubLogWrite.currentOffset[uint8_t volume_id]() {
    return 0;
  }

  default command error_t SubLogWrite.erase[uint8_t volume_id]() {
    return FAIL;
  }

  default command error_t SubLogWrite.sync[uint8_t volume_id]() {
    return FAIL;
  }
  
  default event void LogRead.readDone[uint8_t volume_id](void* buf, storage_len_t len, error_t error) {}
  default event void LogRead.seekDone[uint8_t volume_id](error_t error) {}
  default event void LogWrite.appendDone[uint8_t volume_id](void* buf, storage_len_t len, bool recordsLost, error_t error) {}
  default event void LogWrite.eraseDone[uint8_t volume_id](error_t error) {}
  default event void LogWrite.syncDone[uint8_t volume_id](error_t error) {}
  
  command error_t ConfigStorage.read[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return call SubConfigStorage.read[volume_id](addr, buf, len);
  }
  
  event void SubConfigStorage.readDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {
    signal ConfigStorage.readDone[volume_id](addr, buf, len, error);
  }
  
  command error_t ConfigStorage.write[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return call SubConfigStorage.write[volume_id](addr, buf, len);
  }
  
  event void SubConfigStorage.writeDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {
    signal ConfigStorage.writeDone[volume_id](addr, buf, len, error);
  }
  
  command error_t ConfigStorage.commit[uint8_t volume_id]() {
    return call SubConfigStorage.commit[volume_id]();
  }
  
  event void SubConfigStorage.commitDone[uint8_t volume_id](error_t error) {
    signal ConfigStorage.commitDone[volume_id](error);
  }

  command storage_len_t ConfigStorage.getSize[uint8_t volume_id]() {
    return call SubConfigStorage.getSize[volume_id]();
  }
  
  command bool ConfigStorage.valid[uint8_t volume_id]() {
    return call SubConfigStorage.valid[volume_id]();
  }
  
  command error_t Mount.mount[uint8_t volume_id]() {
    return call SubMount.mount[volume_id]();
  }
  
  event void SubMount.mountDone[uint8_t volume_id](error_t error) {
    signal Mount.mountDone[volume_id](error);
  }
  
  default command error_t SubConfigStorage.read[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return FAIL;
  }

  default command error_t SubConfigStorage.write[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len) {
    return FAIL;
  }
  
  default command error_t SubConfigStorage.commit[uint8_t volume_id]() {
    return FAIL;
  }

  default command storage_len_t SubConfigStorage.getSize[uint8_t volume_id]() {
    return FAIL;
  }
  
  default command bool SubConfigStorage.valid[uint8_t volume_id]() {
    return FAIL;
  }
  
  default command error_t SubMount.mount[uint8_t volume_id]() {
    return FAIL;
  }

  default event void ConfigStorage.commitDone[uint8_t volume_id](error_t error) {}
  default event void ConfigStorage.writeDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {}  
  default event void ConfigStorage.readDone[uint8_t volume_id](storage_addr_t addr, void* buf, storage_len_t len, error_t error) {}
  default event void Mount.mountDone[uint8_t volume_id](error_t error) {}
}
