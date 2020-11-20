/** Copyright (c) 2009, University of Szeged
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
* Author: Zoltan Kincses
*/

#include"Taos2550.h"

generic module Taos2550LogicP()
{
	provides interface Read<uint8_t> as VLight;
	provides interface Read<uint8_t> as IRLight;
	uses interface I2CPacket<TI2CBasicAddr>;
	uses interface Resource as I2CResource;
}
implementation
{
	task void read();
	task void readDone();
	task void failTask();
	error_t performCommand(uint8_t);
	
	uint8_t cmd,readData;

	command error_t VLight.read() {
		return performCommand(TAOS_I2C_ADC0);
	}
  
	command error_t IRLight.read() {
		return performCommand(TAOS_I2C_ADC1);
	}

	error_t performCommand(uint8_t Command) {
		cmd=Command;
		return call I2CResource.request(); 
	}
    
	event void I2CResource.granted(){
		if(call I2CPacket.write((I2C_START|I2C_STOP|I2C_ACK_END),TAOS_I2C_ADDR,1,&cmd)!=SUCCESS){
			call I2CResource.release();
			post failTask();
		}
	}
	
	async event void I2CPacket.writeDone(error_t error, uint16_t addr,uint8_t length, uint8_t* data){
		if(error!=SUCCESS){	
			call I2CResource.release();
			post failTask();
		}else{
			post read();
		}
	}
		
	task void read(){
		if(call I2CPacket.read((I2C_START|I2C_STOP|I2C_ACK_END),TAOS_I2C_ADDR,1,&readData)!=SUCCESS){
			call I2CResource.release();
			post failTask();
		}
	}	
	
	async event void I2CPacket.readDone(error_t error, uint16_t addr,uint8_t length, uint8_t* data){
		call I2CResource.release();
		if(error!=SUCCESS){
			post failTask();
		}else{
			post readDone();
		}
	}
	
	task void readDone(){
		error_t res=(readData>>7==1)?SUCCESS:FAIL;
		if(cmd==TAOS_I2C_ADC0){
			signal VLight.readDone( res, (readData & 0x7F));
		} else {
			signal IRLight.readDone( res, (readData & 0x7F) );
		}
	}
	
	task void failTask(){
		if(cmd==TAOS_I2C_ADC0){
			signal VLight.readDone( FAIL, 0 );
		} else {
			signal IRLight.readDone( FAIL, 0 );
		}
	}
}
