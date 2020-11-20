/*
* Copyright (c) 2011, University of Szeged
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* - Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following
* disclaimer in the documentation and/or other materials provided
* with the distribution.
* - Neither the name of University of Szeged nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Author: Andras Biro
*/

#include "Ms5607.h"

generic module Ms5607ConversionDtP(){
  uses interface Read<uint32_t> as ReadRawTemp;
  uses interface ReadRef<calibration_t> as ReadCalib;
  provides interface Read<int32_t>;
  provides interface Get<uint16_t>[uint8_t index];
}
implementation{
  calibration_t calib;
  bool calibReady=FALSE;
  
  command error_t Read.read(){
    if(calibReady)
      return call ReadRawTemp.read();
    else
      return call ReadCalib.read(&calib);
  }
  
  event void ReadCalib.readDone(error_t err, calibration_t *cal){
    if(err==SUCCESS){
      err = call ReadRawTemp.read();
    }
    if(err!=SUCCESS)
      signal Read.readDone(err, 0);
  }
  
  event void ReadRawTemp.readDone(error_t err, uint32_t value){
    if(err!=SUCCESS)
      signal Read.readDone(err, 0);
    else
      signal Read.readDone(SUCCESS, (value-((uint32_t)calib.coefficient[4]<<8)));
  }
  
  command uint16_t Get.get[uint8_t index](){
    return calib.coefficient[index];
  }
}

