/*
 * Audio Codec module in chisel.
 *
 * Author: Daniel Sanz Ausin s142290 & Fabian Goerge s150957
 *
 */

package io

import chisel3._
import chisel3.util._

import patmos.Constants._

import ocp._

object AudioInterface extends DeviceObject {
  //Audio Register length: can be 16 or 24
  var AUDIOLENGTH = -1
  //Audio Frequency divider
  var AUDIOFSDIVIDER = -1
  //Clock divider for the audio interface
  var AUDIOCLKDIVIDER = -1
  //Maximum DAC buffer power
  var MAXDACBUFFERPOWER = -1
  //Maximum ADC buffer power
  var MAXADCBUFFERPOWER = -1


  def init(params: Map[String, String]) = {
    AUDIOLENGTH = getPosIntParam(params, "audioLength")
    AUDIOFSDIVIDER = getPosIntParam(params, "audioFsDivider")
    AUDIOCLKDIVIDER = getPosIntParam(params, "audioClkDivider")
    MAXDACBUFFERPOWER = getPosIntParam(params, "maxDacBufferPower")
    MAXADCBUFFERPOWER = getPosIntParam(params, "maxAdcBufferPower")
  }

  def create(params: Map[String, String]): AudioInterface = {
    Module(new AudioInterface(AUDIOLENGTH, AUDIOFSDIVIDER, AUDIOCLKDIVIDER, MAXDACBUFFERPOWER, MAXADCBUFFERPOWER))
  }


}

class AudioInterface(AUDIOLENGTH: Int, AUDIOFSDIVIDER: Int, AUDIOCLKDIVIDER: Int, MAXDACBUFFERPOWER: Int, MAXADCBUFFERPOWER: Int) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new Bundle() {
      // Digital Audio Interface
      val dacDat = Output(UInt(1.W))
      val dacLrc = Output(UInt(1.W))
      val adcDat = Input(UInt(1.W))
      val adcLrc = Output(UInt(1.W))

      // I2C - Control Interface
      val sdIn = Input(UInt(1.W))
      val sdOut = Output(UInt(1.W))
      val we = Output(UInt(1.W))
      val sclkOut = Output(UInt(1.W))

      val xclk = Output(UInt(1.W))
      val bclk = Output(UInt(1.W))
    }
  }

  val AUDIOFSDIVIDERReg = RegInit(init = AUDIOFSDIVIDER.U(9.W))
  val AUDIOCLKDIVIDERReg = RegInit(init = AUDIOCLKDIVIDER.U(5.W))
  val AUDIOLENGTHReg = RegInit(init = AUDIOLENGTH.U(5.W))

  val audioDacLReg = RegInit(init = 0.U(AUDIOLENGTH.W))
  val audioDacRReg = RegInit(init = 0.U(AUDIOLENGTH.W))
  val audioDacEnReg = RegInit(init = 0.U(1.W)) //enable

  val audioDacBufferSizeReg = RegInit(init = 0.U((MAXDACBUFFERPOWER + 1).W))
  val audioDacBufferWritePulseReg = RegInit(init = 0.U(1.W))
  val audioDacBufferFullReg = RegInit(init = 0.U(1.W))

  val audioAdcLReg = RegInit(init = 0.U(AUDIOLENGTH.W))
  val audioAdcRReg = RegInit(init = 0.U(AUDIOLENGTH.W))
  val audioAdcEnReg = RegInit(init = 0.U(1.W)) //enable

  val audioAdcBufferSizeReg = RegInit(init = 0.U((MAXADCBUFFERPOWER + 1).W))
  val audioAdcBufferReadPulseReg = RegInit(init = 0.U(1.W))
  val audioAdcBufferEmptyReg = RegInit(init = 0.U(1.W))

  val i2cDataReg = RegInit(init = 0.U(9.W)) //9 Bit I2C data
  val i2cAdrReg = RegInit(init = 0.U(7.W)) //7 Bit I2C address
  val i2cAckReg = RegInit(init = 0.U(1.W)) //1 Bit acknowledge signal
  val i2cReqReg = RegInit(init = 0.U(1.W)) //1 Bit request signal

  // Default response
  val respReg = RegInit(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Read Audio IN
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
  }

  // Read information
  val masterReg = RegNext(next = io.ocp.M)
  val data = Wire(Bits(DATA_WIDTH.W))
  data := 0.U //Default Data

  switch(masterReg.Addr(9, 4)) {
    is("b00000".U) {
      data := audioDacLReg
    }
    is("b00001".U) {
      data := audioDacRReg
    }
    is("b00010".U) {
      data := audioDacEnReg
    }

    is("b00100".U) {
      data := audioDacBufferSizeReg
    }
    is("b00101".U) {
      data := audioDacBufferWritePulseReg
    }
    is("b00110".U) {
      data := audioDacBufferFullReg
    }

    is("b01000".U) {
      data := audioAdcLReg
    }
    is("b01001".U) {
      data := audioAdcRReg
    }
    is("b01010".U) {
      data := audioAdcEnReg
    }

    is("b01011".U) {
      data := audioAdcBufferSizeReg
    }
    is("b01100".U) {
      data := audioAdcBufferReadPulseReg
    }
    is("b01101".U) {
      data := audioAdcBufferEmptyReg
    }

    is("b01110".U) {
      data := i2cDataReg
    }
    is("b01111".U) {
      data := i2cAdrReg
    }
    is("b10000".U) {
      data := i2cAckReg
    }
    is("b10001".U) {
      data := i2cReqReg
    }
  }

  // Write Information
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    switch(io.ocp.M.Addr(9, 4)) {
      is("b00000".U) {
        audioDacLReg := io.ocp.M.Data(AUDIOLENGTH - 1, 0)
      }
      is("b00001".U) {
        audioDacRReg := io.ocp.M.Data(AUDIOLENGTH - 1, 0)
      }
      is("b00010".U) {
        audioDacEnReg := io.ocp.M.Data(0)
      }

      is("b00100".U) {
        audioDacBufferSizeReg := io.ocp.M.Data(MAXDACBUFFERPOWER, 0)
      }
      is("b00101".U) {
        audioDacBufferWritePulseReg := io.ocp.M.Data(0)
      }

      is("b01010".U) {
        audioAdcEnReg := io.ocp.M.Data(0)
      }

      is("b01011".U) {
        audioAdcBufferSizeReg := io.ocp.M.Data(MAXADCBUFFERPOWER, 0)
      }
      is("b01100".U) {
        audioAdcBufferReadPulseReg := io.ocp.M.Data(0)
      }

      is("b01110".U) {
        i2cDataReg := io.ocp.M.Data(8, 0)
      }
      is("b01111".U) {
        i2cAdrReg := io.ocp.M.Data(6, 0)
      }
      is("b10001".U) {
        i2cReqReg := io.ocp.M.Data(0)
      }
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
  mAudioDacBuffer.io.enDacI := audioDacEnReg
  mAudioDacBuffer.io.bufferSizeI := audioDacBufferSizeReg
  mAudioDacBuffer.io.writePulseI := audioDacBufferWritePulseReg
  audioDacBufferFullReg := mAudioDacBuffer.io.fullO

  //DAC:
  val mAudioDac = Module(new AudioDAC(AUDIOLENGTH, AUDIOFSDIVIDER))

  //DAC Buffer To DAC:
  mAudioDac.io.audioLI := mAudioDacBuffer.io.audioLIDAC
  mAudioDac.io.audioRI := mAudioDacBuffer.io.audioRIDAC
  mAudioDac.io.enDacI := mAudioDacBuffer.io.enDacO
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
