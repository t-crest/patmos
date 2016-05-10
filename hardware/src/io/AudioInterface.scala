/*
 * Audio Codec module in chisel.
 *
 * Author: Daniel Sanz Ausin s142290 & Fabian Goerge s150957
 *
 */

package io

import Chisel._
import Node._

import patmos.Constants._
import util.Config
import util.Utility

import ocp._

object audio_interface extends DeviceObject 
{
	//Audio Register length: can be 16 or 24
	var audioLength  = -1
	//Audio Frequency divider
	var audioFsDivider = -1
	//Clock divider for the audio interface
	var audioClkDivider = -1


	def init(params: Map[String, String]) = 
	{
		audioLength		= getPosIntParam(params, "audioLength")
		audioFsDivider		= getPosIntParam(params, "audioFsDivider")
		audioClkDivider		= getPosIntParam(params, "audioClkDivider")
	}
	
	def create(params: Map[String, String]) : audio_interface = 
	{
	    Module	(new audio_interface (audioLength, audioFsDivider, audioClkDivider) )
	}

	trait Pins 
	{
		val audio_interfacePins = new Bundle() 
		{
			// Digital Audio Interface 
			val dacDat	= Bits (OUTPUT, 1)		
			val dacLrc 	= Bits (OUTPUT, 1) 
			val adcDat	= Bits (INPUT, 1)	
			val adcLrc	= Bits (OUTPUT, 1)	

			// I2C - Control Interface
			val sdIn	= Bits (INPUT, 1)
			val sdOut 	= Bits (OUTPUT, 1)
			val we		= Bits (OUTPUT, 1)
			val sclkOut	= Bits (OUTPUT, 1)		
						
			val xclk	= Bits (OUTPUT,1)		
			val bclk 	= Bits (OUTPUT,1)		
		}
	}



}

class audio_interface(	audioLength: Int,
			audioFsDivider: Int,
			audioClkDivider: Int ) extends CoreDevice() 
{
	override val io = new CoreDeviceIO() with audio_interface.Pins

	val audioFsDividerReg		= Reg(init = Bits(audioFsDivider,9))
	val audioClkDividerReg		= Reg(init = Bits(audioClkDivider,5))
	val audioLengthReg		= Reg(init = Bits(audioLength,5))

	val audioDacLReg		= Reg(init = Bits(0, audioLength))	//16 or 24 Bit of Audio input Data
	val audioDacRReg		= Reg(init = Bits(0, audioLength))	//16 or 24 Bit of Audio input Data
	val audioDacEnReg		= Reg(init = Bits(0, 1))		//1  Bit to turn on the Audio device
	val audioDacBusyReg		= Reg(init = Bits(0, 1))		//1  Bit acknowledge signal
	val audioDacReqReg		= Reg(init = Bits(0, 1))		//1  Bit request signal
	val audioDacLrcReg		= Reg(init = Bits(0, 1))

	val audioAdcLOReg		= Reg(init = Bits(0, audioLength))	//16 or 24 Bit of Audio output Data
	val audioAdcROReg		= Reg(init = Bits(0, audioLength))	//16 or 24 Bit of Audio output Data
	val audioAdcENReg		= Reg(init = Bits(0, 1))		//1  Bit to enable on the Audio device
	val audioAdcBusyReg		= Reg(init = Bits(0, 1))		//1  Bit acknowledge signal
	val audioAdcReqReg		= Reg(init = Bits(0, 1))		//1  Bit request signal
	val audioAdcLrcReg		= Reg(init = Bits(0, 1))

	val i2cDataReg 			= Reg(init = Bits(0,9))			//9 Bit I2C data	
	val i2cAdrReg			= Reg(init = Bits(0, 7))		//7 Bit I2C address
	val i2cAckReg			= Reg(init = Bits(0, 1))		//1  Bit acknowledge signal
	val i2cReqReg 			= Reg(init = Bits(0, 1))			//1  Bit request signal		

	// Default response
	val respReg = Reg(init = OcpResp.NULL)
	respReg := OcpResp.NULL

	// Read Audio IN
	when(io.ocp.M.Cmd === OcpCmd.RD) 
	{
		respReg := OcpResp.DVA
	}
	

	// Read information
	val masterReg = Reg(next = io.ocp.M)
  	val data = Bits(width = DATA_WIDTH)
  	data := Bits(0)		//Default Data

	switch(masterReg.Addr(8,4)) 
	{
		is(Bits("b0000")) { data := audioDacLReg }	
		is(Bits("b0001")) { data := audioDacRReg }	
		is(Bits("b0010")) { data := audioDacEnReg }	 
		is(Bits("b0011")) { data := audioDacBusyReg }	
		is(Bits("b0100")) { data := audioDacReqReg }	
		is(Bits("b0101")) { data := audioDacLrcReg }	
		is(Bits("b0110")) { data := audioAdcLOReg }
		is(Bits("b0111")) { data := audioAdcROReg }
		is(Bits("b1000")) { data := audioAdcENReg }		
		is(Bits("b1001")) { data := audioAdcBusyReg }	
		is(Bits("b1010")) { data := audioAdcReqReg }	
		is(Bits("b1011")) { data := audioAdcLrcReg }	
		is(Bits("b1100")) { data := i2cDataReg } 
		is(Bits("b1101")) { data := i2cAdrReg }	
		is(Bits("b1110")) { data := i2cAckReg }
		is(Bits("b1111")) { data := i2cReqReg }
	}

	// Write Audio In
	when(io.ocp.M.Cmd === OcpCmd.WR) 
	{
		respReg := OcpResp.DVA
		switch(io.ocp.M.Addr(8,4)) 
		{
			is(Bits("b0000")) { audioDacLReg		:= io.ocp.M.Data(audioLength-1,0) }
			is(Bits("b0001")) { audioDacRReg		:= io.ocp.M.Data(audioLength-1,0) }
			is(Bits("b0010")) { audioDacEnReg	 	:= io.ocp.M.Data(0)} 
			is(Bits("b0100")) { audioDacReqReg		:= io.ocp.M.Data(0)} 
			is(Bits("b1000")) { audioAdcENReg		:= io.ocp.M.Data(0)}
			is(Bits("b1010")) { audioAdcReqReg		:= io.ocp.M.Data(0)}	
			is(Bits("b1100")) { i2cDataReg 			:= io.ocp.M.Data(8,0)}	
			is(Bits("b1101")) { i2cAdrReg 			:= io.ocp.M.Data(6,0)}	
			is(Bits("b1111")) { i2cReqReg 			:= io.ocp.M.Data(0)}	
		}
	} 
	
	// Connections to master
	io.ocp.S.Resp := respReg
	io.ocp.S.Data := data


	//Audio Clock Unit:
	val mAudioClk = Module(new audio_clk_gen(audioClkDivider))
		mAudioClk.io.en_adc_i 		:= audioAdcENReg
		mAudioClk.io.en_dac_i		:= audioDacEnReg
	    	io.audio_interfacePins.bclk	:= mAudioClk.io.BCLK_o 
		io.audio_interfacePins.xclk	:= mAudioClk.io.XCLK_o


	//Dac:
	val mAudioDac = Module(new audio_dac(audioLength, audioFsDivider))

		mAudioDac.io.audio_l_i		:= audioDacLReg
		mAudioDac.io.audio_r_i		:= audioDacRReg
    		mAudioDac.io.en_dac_i		:= audioDacEnReg
		mAudioDac.io.BCLK_i		:= mAudioClk.io.BCLK_o
		audioDacBusyReg			:= mAudioDac.io.busy_o 

		audioDacLrcReg			:= mAudioDac.io.DACLRC_o
	    	io.audio_interfacePins.dacLrc	:= mAudioDac.io.DACLRC_o 
		io.audio_interfacePins.dacDat	:= mAudioDac.io.DACDAT_o

	//Adc:
	val mAudioAdc = Module(new audio_adc(audioLength, audioFsDivider))
		audioAdcLOReg 			:= mAudioAdc.io.audio_l_o 
		audioAdcROReg 			:= mAudioAdc.io.audio_r_o
		audioAdcBusyReg 		:= mAudioAdc.io.busy_o 		
		mAudioAdc.io.en_adc_i 		:= audioAdcENReg
		mAudioAdc.io.BCLK_i 		:= mAudioClk.io.BCLK_o

		audioAdcLrcReg			:= mAudioAdc.io.ADCLRC_o
		io.audio_interfacePins.adcLrc 	:= mAudioAdc.io.ADCLRC_o 
		mAudioAdc.io.ADCDAT_i 		:= io.audio_interfacePins.adcDat

	//IC2 Control Interface
	val mAudioCtrl = Module(new audio_i2c())
		mAudioCtrl.io.data_i 		:= i2cDataReg
		mAudioCtrl.io.addr_i 		:= i2cAdrReg
    		mAudioCtrl.io.req_i 		:= i2cReqReg
		i2cAckReg 			:= mAudioCtrl.io.ack_o 
	    
		mAudioCtrl.io.sdin_i 		:= io.audio_interfacePins.sdIn 		
		io.audio_interfacePins.we 	:= mAudioCtrl.io.we_o
		io.audio_interfacePins.sdOut 	:= mAudioCtrl.io.sdin_o
		io.audio_interfacePins.sclkOut 	:= mAudioCtrl.io.sclk_o
}


