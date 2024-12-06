/*
 * SRAM controller with OCP burst interface.
 *
 * Is running on DE1-115, MS shall test it on the BeMicro
 *
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *         Martin Schoeberl (martin@jopdesign.com)
 *         Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import scala.math._

import chisel3._
import chisel3.util._

import ocp._

import patmos.Constants._

object SRamCtrl extends DeviceObject {
  var sramAddrWidth = 20
  var sramDataWidth = 16
  var ocpAddrWidth = 21

  def init(params: Map[String, String]) = {
    sramAddrWidth = getPosIntParam(params, "sramAddrWidth")
    sramDataWidth = getPosIntParam(params, "sramDataWidth")
    ocpAddrWidth = getPosIntParam(params, "ocpAddrWidth")
  }

  def create(params: Map[String, String]) : SRamCtrl = {
    Module(new SRamCtrl(ocpAddrWidth,ocpBurstLen=BURST_LENGTH,sramAddrWidth=sramAddrWidth,sramDataWidth=sramDataWidth))
  }

  trait Pins extends patmos.HasPins {
    val pins: Bundle {
      val ramOut: Bundle {
        val addr: UInt
        val doutEna: UInt
        val dout: UInt
        val nce: UInt
        val noe: UInt
        val nwe: UInt
        val nlb: UInt
        val nub: UInt
      }
      val ramIn: Bundle {
        val din: UInt
      }
    } = new Bundle {
      val ramOut = Output(new Bundle {
        val addr = UInt(sramAddrWidth.W)
        val doutEna = UInt(1.W)
        val dout = UInt(sramDataWidth.W)
        val nce = UInt(1.W)
        val noe = UInt(1.W)
        val nwe = UInt(1.W)
        val nlb = UInt(1.W)
        val nub = UInt(1.W)
      })
      val ramIn = Input(new Bundle {
        val din = UInt(sramDataWidth.W)
      })
    }
  }
}

class SRamCtrl( ocpAddrWidth    : Int,
                ocpBurstLen     : Int=4,
                sramAddrWidth   : Int=20,
                sramDataWidth   : Int=16,
                singleCycleRead : Boolean=false,
                writeWaitCycles : Int=1) extends BurstDevice(ocpAddrWidth) {

  override val io = IO(new BurstDeviceIO(ocpAddrWidth) with SRamCtrl.Pins)

  // Checks for input parameters
  require((DATA_WIDTH % sramDataWidth == 0),"DATA_WIDTH is not a multiple of sramDataWidth")
  require((sramAddrWidth <= ocpAddrWidth),"ocpAddrWidth cannot access the full sram")
  require(isPow2(sramDataWidth/8),"number of bytes per transaction to sram is not a power of 2")

  // Helper functions
  // log2up is a standard function but log2up(1)=1 and it should be 0
  def log2upNew(in: Int) = ceil(log(in)/log(2)).toInt
  // Constants
  val TRANSPERWORD = DATA_WIDTH/sramDataWidth
  val TRANSPERCMD = TRANSPERWORD*ocpBurstLen
  val BYTESPERTRAN = sramDataWidth/8

  // State type and variable
  val sReady :: sReadExe :: sReadExe2 :: sReadRet :: sWriteRec :: sWriteExe :: sWriteExe2 :: sWriteRet :: Nil = Enum(8)
  val stateReg = RegInit(init = sReady)

  // Internal Registers
  val mAddrReg = Reg(UInt(sramAddrWidth.W))
  val rdBufferReg = Reg(Vec(TRANSPERCMD, UInt(sramDataWidth.W)))
  val wrBufferReg = Reg(Vec(TRANSPERCMD, new Trans(BYTESPERTRAN,sramDataWidth)))
  val transCountReg = RegInit(init = 0.U(log2upNew(TRANSPERCMD).W))
  val wordCountReg = RegInit(init = 0.U(log2upNew(ocpBurstLen).W))
  val waitCountReg = RegInit(init = 0.U(log2upNew(writeWaitCycles+1).W))
  // Output Registers
  val addrReg = Reg(UInt(sramAddrWidth.W))
  val doutEnaReg = Reg(Bits())
  val doutReg = Reg(UInt(DATA_WIDTH.W))
  val nceReg = Reg(Bits())
  val noeReg = Reg(Bits())
  val nweReg = Reg(Bits())
  val nlbReg = Reg(Bits())
  val nubReg = Reg(Bits())

  // Default values for ocp io.ocp.S port
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.CmdAccept := 1.U
  io.ocp.S.DataAccept := 1.U
  val data = for(i <- 0 until TRANSPERWORD)
             yield rdBufferReg(wordCountReg ## i.U)
  io.ocp.S.Data := data.reduceLeft((x,y) => y ## x)

  // Default values for sRamCtrlPins.ramOut port
  addrReg := mAddrReg
  doutEnaReg := 0.U
  doutReg := wrBufferReg(0).data
  nceReg := 0.U
  noeReg := 1.U
  nweReg := 1.U
  nlbReg := 1.U
  nubReg := 1.U

  waitCountReg := 0.U

  when(stateReg === sReady) {
    for( i <- 0 until TRANSPERWORD) {
      wrBufferReg(i).byteEna := io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(i).data := io.ocp.M.Data((i+1)*sramDataWidth-1,i*sramDataWidth)
    }
    when(io.ocp.M.Cmd =/= OcpCmd.IDLE) {
      mAddrReg := io.ocp.M.Addr(sramAddrWidth+log2upNew(BYTESPERTRAN) - 1,
                                log2upNew(BYTESPERTRAN))
      when(io.ocp.M.Cmd === OcpCmd.RD) {
        stateReg := sReadExe
      }
      when(io.ocp.M.Cmd === OcpCmd.WR) {
        wordCountReg := 1.U // The first ocp data word is already in wrBufferReg
        stateReg := sWriteRec
      }
    }
  }
  when(stateReg === sReadExe) {
    noeReg := 0.U
    nceReg := 0.U
    nubReg := 0.U
    nlbReg := 0.U
    if (singleCycleRead){
      addrReg := mAddrReg + 1.U
      mAddrReg := mAddrReg + 1.U
      for (i <- 0 until TRANSPERCMD-1) { rdBufferReg(i) := rdBufferReg(i+1) }
      rdBufferReg(TRANSPERCMD-1) := io.pins.ramIn.din
      transCountReg := transCountReg + 1.U
      stateReg := sReadExe
      when(transCountReg === (TRANSPERCMD-1).U){
        stateReg := sReadRet
        transCountReg := 0.U
      }
    } else {
      stateReg := sReadExe2
    }
  }
  when(stateReg === sReadExe2) {
    noeReg := 0.U
    nceReg := 0.U
    nubReg := 0.U
    nlbReg := 0.U
    addrReg := mAddrReg + 1.U
    mAddrReg := mAddrReg + 1.U
    for (i <- 0 until TRANSPERCMD-1) { rdBufferReg(i) := rdBufferReg(i+1) }
    rdBufferReg(TRANSPERCMD-1) := io.pins.ramIn.din
    transCountReg := transCountReg + 1.U
    stateReg := sReadExe
    when(transCountReg === (TRANSPERCMD-1).U){
      stateReg := sReadRet
      transCountReg := 0.U
    }
  }
  when(stateReg === sReadRet) {
    io.ocp.S.Resp := OcpResp.DVA
    wordCountReg := wordCountReg + 1.U
    when(wordCountReg === (ocpBurstLen-1).U){
      stateReg := sReady
      wordCountReg := 0.U
    }
  }
  when(stateReg === sWriteRec) {
    for(i <- 0 until TRANSPERWORD) {
      wrBufferReg(wordCountReg ## i.U).byteEna :=
        io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(wordCountReg ## i.U).data :=
        io.ocp.M.Data((i+1)*sramDataWidth-1,i*sramDataWidth)
    }
    doutReg := wrBufferReg(0).data
    doutEnaReg := 1.U
    when(io.ocp.M.DataValid === 1.U){
      wordCountReg := wordCountReg + 1.U
      when(wordCountReg === (ocpBurstLen-1).U){
        stateReg := sWriteExe
        wordCountReg := 0.U
        waitCountReg := 1.U
      }
    } otherwise {
      stateReg := sWriteRec
    }
    when(wordCountReg === (ocpBurstLen-1).U){
      nceReg := 0.U
      nweReg := 0.U
      nubReg := !wrBufferReg(0).byteEna(1)
      nlbReg := !wrBufferReg(0).byteEna(0)
    }
  }
  when(stateReg === sWriteExe) {
    nceReg := 0.U
    nweReg := 0.U
    doutReg := wrBufferReg(transCountReg).data
    doutEnaReg := 1.U
    when(waitCountReg < writeWaitCycles.U){
      waitCountReg := waitCountReg + 1.U
      nubReg := !wrBufferReg(transCountReg).byteEna(1)
      nlbReg := !wrBufferReg(transCountReg).byteEna(0)
      stateReg := sWriteExe
    } otherwise {
      waitCountReg := 0.U
      nubReg := 1.U
      nlbReg := 1.U
      stateReg := sWriteExe2
    }
  }
  when(stateReg === sWriteExe2) {
    when(transCountReg < (TRANSPERCMD-1).U){
      nceReg := 0.U
      nweReg := 0.U
      nubReg := !wrBufferReg(transCountReg+1.U).byteEna(1)
      nlbReg := !wrBufferReg(transCountReg+1.U).byteEna(0)
      doutReg := wrBufferReg(transCountReg+1.U).data
      doutEnaReg := 1.U
      addrReg := mAddrReg + 1.U
      mAddrReg := mAddrReg + 1.U
      transCountReg := transCountReg + 1.U
      waitCountReg := 1.U
      stateReg := sWriteExe
    }
    when(transCountReg === (TRANSPERCMD-1).U){
      stateReg := sWriteRet
      transCountReg := 0.U
      waitCountReg := 0.U
    }
  }
  when(stateReg === sWriteRet) {
    io.ocp.S.Resp := OcpResp.DVA
    stateReg := sReady
  }

  io.pins.ramOut.addr := addrReg
  io.pins.ramOut.doutEna := doutEnaReg
  io.pins.ramOut.dout := doutReg
  io.pins.ramOut.nce := nceReg
  io.pins.ramOut.noe := noeReg
  io.pins.ramOut.nwe := nweReg
  io.pins.ramOut.nlb := nlbReg
  io.pins.ramOut.nub := nubReg


  class Trans(bytesEnaWidth: Int, dataWidth: Int) extends Bundle {
    val byteEna = UInt(bytesEnaWidth.W)
    val data = UInt(dataWidth.W)

  }
}

/*
 Used to instantiate a single SRAM control component
 */
object SRamMain {
  def main(args: Array[String]): Unit = {
    val chiselArgs = args.slice(1,args.length)
    val ocpAddrWidth = args(0).toInt
    emitVerilog(new SRamCtrl(ocpAddrWidth), chiselArgs)
  }
}
