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

object AudioInterface extends DeviceObject
{
  //Audio Register length: can be 16 or 24
  var AUDIOLENGTH  = -1
  //Audio Frequency divider
  var AUDIOFSDIVIDER = -1
  //Clock divider for the audio interface
  var AUDIOCLKDIVIDER = -1


  def init(params: Map[String, String]) =
  {
    AUDIOLENGTH	= getPosIntParam(params, "audioLength")
    AUDIOFSDIVIDER = getPosIntParam(params, "audioFsDivider")
    AUDIOCLKDIVIDER = getPosIntParam(params, "audioClkDivider")
  }

  def create(params: Map[String, String]) : AudioInterface =
  {
    Module(new AudioInterface (AUDIOLENGTH, AUDIOFSDIVIDER, AUDIOCLKDIVIDER))
  }

  trait Pins
  {
    val audioInterfacePins = new Bundle()
    {
      // Digital Audio Interface
      val dacDat = Bits (OUTPUT, 1)
      val dacLrc = Bits (OUTPUT, 1)
      val adcDat = Bits (INPUT, 1)
      val adcLrc = Bits (OUTPUT, 1)

      // I2C - Control Interface
      val sdIn = Bits (INPUT, 1)
      val sdOut = Bits (OUTPUT, 1)
      val we = Bits (OUTPUT, 1)
      val sclkOut = Bits (OUTPUT, 1)

      val xclk = Bits (OUTPUT,1)
      val bclk = Bits (OUTPUT,1)
    }
  }



}

class AudioInterface(AUDIOLENGTH: Int, AUDIOFSDIVIDER: Int, AUDIOCLKDIVIDER: Int) extends CoreDevice() {
  override val io = new CoreDeviceIO() with AudioInterface.Pins

  val AUDIOFSDIVIDERReg	= Reg(init = Bits(AUDIOFSDIVIDER,9))
  val AUDIOCLKDIVIDERReg = Reg(init = Bits(AUDIOCLKDIVIDER,5))
  val AUDIOLENGTHReg = Reg(init = Bits(AUDIOLENGTH,5))

  val audioDacLReg = Reg(init = Bits(0, AUDIOLENGTH))
  val audioDacRReg = Reg(init = Bits(0, AUDIOLENGTH))
  val audioDacEnReg = Reg(init = Bits(0, 1)) //enable
  val audioDacBusyReg = Reg(init = Bits(0, 1))
  val audioDacReqReg = Reg(init = Bits(0, 1))
  val audioDacLrcReg = Reg(init = Bits(0, 1))

  val audioAdcLOReg = Reg(init = Bits(0, AUDIOLENGTH))
  val audioAdcROReg = Reg(init = Bits(0, AUDIOLENGTH))
  val audioAdcENReg = Reg(init = Bits(0, 1)) //enable
  val audioAdcBusyReg = Reg(init = Bits(0, 1))
  val audioAdcReqReg = Reg(init = Bits(0, 1))
  val audioAdcLrcReg = Reg(init = Bits(0, 1))

  val i2cDataReg = Reg(init = Bits(0,9)) //9 Bit I2C data
  val i2cAdrReg	= Reg(init = Bits(0, 7)) //7 Bit I2C address
  val i2cAckReg	= Reg(init = Bits(0, 1)) //1 Bit acknowledge signal
  val i2cReqReg = Reg(init = Bits(0, 1)) //1 Bit request signal

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
  data := Bits(0) //Default Data

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
      is(Bits("b0000")) { audioDacLReg := io.ocp.M.Data(AUDIOLENGTH-1,0) }
      is(Bits("b0001")) { audioDacRReg := io.ocp.M.Data(AUDIOLENGTH-1,0) }
      is(Bits("b0010")) { audioDacEnReg	:= io.ocp.M.Data(0)}
      is(Bits("b0100")) { audioDacReqReg := io.ocp.M.Data(0)}
      is(Bits("b1000")) { audioAdcENReg	:= io.ocp.M.Data(0)}
      is(Bits("b1010")) { audioAdcReqReg := io.ocp.M.Data(0)}
      is(Bits("b1100")) { i2cDataReg := io.ocp.M.Data(8,0)}
      is(Bits("b1101")) { i2cAdrReg := io.ocp.M.Data(6,0)}
      is(Bits("b1111")) { i2cReqReg := io.ocp.M.Data(0)}
    }
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := data


  //Audio Clock Unit:
  val mAudioClk = Module(new AudioClkGen(AUDIOCLKDIVIDER))
  mAudioClk.io.enAdcI := audioAdcENReg
  mAudioClk.io.enDacI := audioDacEnReg
  io.audioInterfacePins.bclk := mAudioClk.io.bclkO
  io.audioInterfacePins.xclk := mAudioClk.io.xclkO


  //Dac:
  val mAudioDac = Module(new AudioDAC(AUDIOLENGTH, AUDIOFSDIVIDER))

  mAudioDac.io.audioLI := audioDacLReg
  mAudioDac.io.audioRI := audioDacRReg
  mAudioDac.io.enDacI := audioDacEnReg
  mAudioDac.io.bclkI := mAudioClk.io.bclkO
  audioDacBusyReg := mAudioDac.io.busyO

  audioDacLrcReg := mAudioDac.io.dacLrcO
  io.audioInterfacePins.dacLrc := mAudioDac.io.dacLrcO
  io.audioInterfacePins.dacDat := mAudioDac.io.dacDatO

  //Adc:
  val mAudioAdc = Module(new AudioADC(AUDIOLENGTH, AUDIOFSDIVIDER))
  audioAdcLOReg := mAudioAdc.io.audioLO
  audioAdcROReg := mAudioAdc.io.audioRO
  audioAdcBusyReg := mAudioAdc.io.busyO
  mAudioAdc.io.enAdcI := audioAdcENReg
  mAudioAdc.io.bclkI := mAudioClk.io.bclkO

  audioAdcLrcReg := mAudioAdc.io.adcLrcO
  io.audioInterfacePins.adcLrc := mAudioAdc.io.adcLrcO
  mAudioAdc.io.adcDatI := io.audioInterfacePins.adcDat

  //IC2 Control Interface
  val mAudioCtrl = Module(new AudioI2C())
  mAudioCtrl.io.dataI := i2cDataReg
  mAudioCtrl.io.addrI := i2cAdrReg
  mAudioCtrl.io.reqI := i2cReqReg
  i2cAckReg := mAudioCtrl.io.ackO

  mAudioCtrl.io.sdinI := io.audioInterfacePins.sdIn
  io.audioInterfacePins.we := mAudioCtrl.io.weO
  io.audioInterfacePins.sdOut := mAudioCtrl.io.sdinO
  io.audioInterfacePins.sclkOut := mAudioCtrl.io.sclkO
}
