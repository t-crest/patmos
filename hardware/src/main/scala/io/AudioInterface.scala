/*
 * Audio Codec module in chisel.
 *
 * Author: Daniel Sanz Ausin s142290 & Fabian Goerge s150957
 *
 */

package io

import Chisel._

import patmos.Constants._

import ocp._

object AudioInterface extends DeviceObject
{
  //Audio Register length: can be 16 or 24
  var AUDIOLENGTH  = -1
  //Audio Frequency divider
  var AUDIOFSDIVIDER = -1
  //Clock divider for the audio interface
  var AUDIOCLKDIVIDER = -1
  //Maximum DAC buffer power
  var MAXDACBUFFERPOWER = -1
  //Maximum ADC buffer power
  var MAXADCBUFFERPOWER = -1


  def init(params: Map[String, String]) =
  {
    AUDIOLENGTH	      = getPosIntParam(params, "audioLength")
    AUDIOFSDIVIDER    = getPosIntParam(params, "audioFsDivider")
    AUDIOCLKDIVIDER   = getPosIntParam(params, "audioClkDivider")
    MAXDACBUFFERPOWER = getPosIntParam(params, "maxDacBufferPower")
    MAXADCBUFFERPOWER = getPosIntParam(params, "maxAdcBufferPower")
  }

  def create(params: Map[String, String]) : AudioInterface =
  {
    Module(new AudioInterface (AUDIOLENGTH, AUDIOFSDIVIDER, AUDIOCLKDIVIDER, MAXDACBUFFERPOWER, MAXADCBUFFERPOWER))
  }



}

class AudioInterface(AUDIOLENGTH: Int, AUDIOFSDIVIDER: Int, AUDIOCLKDIVIDER: Int, MAXDACBUFFERPOWER: Int, MAXADCBUFFERPOWER: Int) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new Bundle()
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

  val AUDIOFSDIVIDERReg	 = Reg(init = Bits(AUDIOFSDIVIDER,9))
  val AUDIOCLKDIVIDERReg = Reg(init = Bits(AUDIOCLKDIVIDER,5))
  val AUDIOLENGTHReg     = Reg(init = Bits(AUDIOLENGTH,5))

  val audioDacLReg   = Reg(init = Bits(0, AUDIOLENGTH))
  val audioDacRReg   = Reg(init = Bits(0, AUDIOLENGTH))
  val audioDacEnReg  = Reg(init = Bits(0, 1)) //enable

  val audioDacBufferSizeReg        = Reg(init = Bits(0, (MAXDACBUFFERPOWER+1)))
  val audioDacBufferWritePulseReg  = Reg(init = Bits(0, 1))
  val audioDacBufferFullReg        = Reg(init = Bits(0, 1))

  val audioAdcLReg   = Reg(init = Bits(0, AUDIOLENGTH))
  val audioAdcRReg   = Reg(init = Bits(0, AUDIOLENGTH))
  val audioAdcEnReg  = Reg(init = Bits(0, 1)) //enable

  val audioAdcBufferSizeReg      = Reg(init = Bits(0, (MAXADCBUFFERPOWER+1)))
  val audioAdcBufferReadPulseReg = Reg(init = Bits(0, 1))
  val audioAdcBufferEmptyReg     = Reg(init = Bits(0, 1))

  val i2cDataReg = Reg(init = Bits(0,9)) //9 Bit I2C data
  val i2cAdrReg	 = Reg(init = Bits(0, 7)) //7 Bit I2C address
  val i2cAckReg	 = Reg(init = Bits(0, 1)) //1 Bit acknowledge signal
  val i2cReqReg  = Reg(init = Bits(0, 1)) //1 Bit request signal

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
  val data = Wire(Bits(DATA_WIDTH.W))
  data := 0.U //Default Data

  switch(masterReg.Addr(9,4))
  {
    is(Bits("b00000")) { data := audioDacLReg }
    is(Bits("b00001")) { data := audioDacRReg }
    is(Bits("b00010")) { data := audioDacEnReg }

    is(Bits("b00100")) { data := audioDacBufferSizeReg }
    is(Bits("b00101")) { data := audioDacBufferWritePulseReg }
    is(Bits("b00110")) { data := audioDacBufferFullReg }

    is(Bits("b01000")) { data := audioAdcLReg }
    is(Bits("b01001")) { data := audioAdcRReg }
    is(Bits("b01010")) { data := audioAdcEnReg }

    is(Bits("b01011")) { data := audioAdcBufferSizeReg }
    is(Bits("b01100")) { data := audioAdcBufferReadPulseReg }
    is(Bits("b01101")) { data := audioAdcBufferEmptyReg }

    is(Bits("b01110")) { data := i2cDataReg }
    is(Bits("b01111")) { data := i2cAdrReg }
    is(Bits("b10000")) { data := i2cAckReg }
    is(Bits("b10001")) { data := i2cReqReg }
  }

  // Write Information
  when(io.ocp.M.Cmd === OcpCmd.WR)
  {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(9,4))
    {
      is(Bits("b00000")) { audioDacLReg := io.ocp.M.Data(AUDIOLENGTH-1,0) }
      is(Bits("b00001")) { audioDacRReg := io.ocp.M.Data(AUDIOLENGTH-1,0) }
      is(Bits("b00010")) { audioDacEnReg := io.ocp.M.Data(0) }

      is(Bits("b00100")) { audioDacBufferSizeReg := io.ocp.M.Data(MAXDACBUFFERPOWER,0) }
      is(Bits("b00101")) { audioDacBufferWritePulseReg := io.ocp.M.Data(0) }

      is(Bits("b01010")) { audioAdcEnReg := io.ocp.M.Data(0) }

      is(Bits("b01011")) { audioAdcBufferSizeReg := io.ocp.M.Data(MAXADCBUFFERPOWER, 0) }
      is(Bits("b01100")) { audioAdcBufferReadPulseReg := io.ocp.M.Data(0) }

      is(Bits("b01110")) { i2cDataReg := io.ocp.M.Data(8,0) }
      is(Bits("b01111")) { i2cAdrReg := io.ocp.M.Data(6,0) }
      is(Bits("b10001")) { i2cReqReg := io.ocp.M.Data(0) }
    }
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := data


  //Audio Clock Unit:
  val mAudioClk = Module(new AudioClkGen(AUDIOCLKDIVIDER))
  mAudioClk.io.enAdcI := audioAdcEnReg
  mAudioClk.io.enDacI := audioDacEnReg
  io.pins.bclk := mAudioClk.io.bclkO
  io.pins.xclk := mAudioClk.io.xclkO


  //DAC Buffer:
  val mAudioDacBuffer = Module(new AudioDACBuffer(AUDIOLENGTH, MAXDACBUFFERPOWER))

  //Patmos to DAC Buffer:
  mAudioDacBuffer.io.audioLIPatmos := audioDacLReg
  mAudioDacBuffer.io.audioRIPatmos := audioDacRReg
  mAudioDacBuffer.io.enDacI        := audioDacEnReg
  mAudioDacBuffer.io.bufferSizeI   := audioDacBufferSizeReg
  mAudioDacBuffer.io.writePulseI   := audioDacBufferWritePulseReg
  audioDacBufferFullReg            := mAudioDacBuffer.io.fullO

  //DAC:
  val mAudioDac = Module(new AudioDAC(AUDIOLENGTH, AUDIOFSDIVIDER))

  //DAC Buffer To DAC:
  mAudioDac.io.audioLI       := mAudioDacBuffer.io.audioLIDAC
  mAudioDac.io.audioRI       := mAudioDacBuffer.io.audioRIDAC
  mAudioDac.io.enDacI        := mAudioDacBuffer.io.enDacO
  mAudioDacBuffer.io.writeEnDacI := mAudioDac.io.writeEnDacO
  mAudioDacBuffer.io.convEndI := mAudioDac.io.convEndO

  // DAC to others:
  mAudioDac.io.bclkI := mAudioClk.io.bclkO
  io.pins.dacLrc := mAudioDac.io.dacLrcO
  io.pins.dacDat := mAudioDac.io.dacDatO

  //ADC Buffer:
  val mAudioAdcBuffer = Module(new AudioADCBuffer(AUDIOLENGTH, MAXADCBUFFERPOWER))

  //Patmos to ADC Buffer:
  mAudioAdcBuffer.io.enAdcI := audioAdcEnReg
  audioAdcLReg := mAudioAdcBuffer.io.audioLPatmosO
  audioAdcRReg := mAudioAdcBuffer.io.audioRPatmosO
  mAudioAdcBuffer.io.readPulseI := audioAdcBufferReadPulseReg
  audioAdcBufferEmptyReg := mAudioAdcBuffer.io.emptyO
  mAudioAdcBuffer.io.bufferSizeI := audioAdcBufferSizeReg

  //ADC:
  val mAudioAdc = Module(new AudioADC(AUDIOLENGTH, AUDIOFSDIVIDER))

  // ADC Buffer to ADC:
  mAudioAdcBuffer.io.audioLAdcI := mAudioAdc.io.audioLO
  mAudioAdcBuffer.io.audioRAdcI := mAudioAdc.io.audioRO
  mAudioAdc.io.enAdcI := mAudioAdcBuffer.io.enAdcO
  mAudioAdcBuffer.io.readEnAdcI := mAudioAdc.io.readEnAdcO

  // model of the ADC of WM8731
  //val mAudioWM8731AdcModel = Module(new AudioWM8731ADCModel(AUDIOLENGTH)) // comment for FPGA
  //mAudioWM8731AdcModel.io.bClk := mAudioClk.io.bclkO // comment for FPGA

  // ADC to others:
  mAudioAdc.io.bclkI := mAudioClk.io.bclkO
  io.pins.adcLrc := mAudioAdc.io.adcLrcO //comment for simulation
  //mAudioWM8731AdcModel.io.adcLrc := mAudioAdc.io.adcLrcO //comment for FPGA
  mAudioAdc.io.adcDatI := io.pins.adcDat //comment for simulation
  //mAudioAdc.io.adcDatI := mAudioWM8731AdcModel.io.adcDat //comment for FPGA

  //IC2 Control Interface
  val mAudioCtrl = Module(new AudioI2C())
  mAudioCtrl.io.dataI := i2cDataReg
  mAudioCtrl.io.addrI := i2cAdrReg
  mAudioCtrl.io.reqI := i2cReqReg
  i2cAckReg := mAudioCtrl.io.ackO

  mAudioCtrl.io.sdinI := io.pins.sdIn
  io.pins.we := mAudioCtrl.io.weO
  io.pins.sdOut := mAudioCtrl.io.sdinO
  io.pins.sclkOut := mAudioCtrl.io.sclkO
}
