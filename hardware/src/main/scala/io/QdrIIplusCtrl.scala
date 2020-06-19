/*
 * Memory controller for QDR-II+ memory with OCP burst interface.
 * The controller assumes burst-of-four mode for QDR-II+ interface.
 *
 * TODO: Could be _much_ faster with some pipelining
 * 
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *         Martin Schoeberl (martin@jopdesign.com)
 *         Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import scala.math._

import Chisel._

import ocp._

import patmos.Constants._

object QdrIIplusCtrl extends DeviceObject {
  var ramAddrWidth = 19
  var ramDataWidth = 16
  var ocpAddrWidth = 22

  def init(params: Map[String, String]) = {
    ramAddrWidth = getPosIntParam(params, "ramAddrWidth")
    ramDataWidth = getPosIntParam(params, "ramDataWidth")
    ocpAddrWidth = getPosIntParam(params, "ocpAddrWidth")
  }

  def create(params: Map[String, String]) : QdrIIplusCtrl = {
    Module(new QdrIIplusCtrl(ocpAddrWidth, 
                             ocpBurstLen = BURST_LENGTH,
                             ramAddrWidth = ramAddrWidth,
                             ramDataWidth = ramDataWidth))
  }
}

class QdrIIplusCtrl(ocpAddrWidth   : Int,
                    ocpBurstLen    : Int = BURST_LENGTH,
                    ramAddrWidth   : Int = 19,
                    ramDataWidth   : Int = 16,
                    readWaitCycles : Int = 5) extends BurstDevice(ocpAddrWidth) {

  override val io = new BurstDeviceIO(ocpAddrWidth) with patmos.HasPins {
    override val pins = new Bundle {
      val addr  = Bits(OUTPUT, width = ramAddrWidth)

      val nrps  = Bits(OUTPUT, width = 1)
      val nwps  = Bits(OUTPUT, width = 1)
      val nbws  = Vec.fill(2) { Bits(OUTPUT, width = ramDataWidth / 8) }

      val din   = Vec.fill(2) { Bits(INPUT, width = ramDataWidth) }
      val dout  = Vec.fill(2) { Bits(OUTPUT, width = ramDataWidth) }

      val ndoff = Bits(OUTPUT, width = 1)
    }
  }

  // Checks for input parameters
  assert(Bool(DATA_WIDTH % ramDataWidth == 0),"DATA_WIDTH is not a multiple of ramDataWidth ")
  assert(Bool(ramAddrWidth <= ocpAddrWidth),"ocpAddrWidth cannot access the full RAM")
  assert(Bool(isPow2(ramDataWidth/8)),"number of bytes per transaction to RAM is not a power of 2")

  // Helper functions
  // log2up is a standard function but log2up(1)=1 and it should be 0
  def log2upNew(in: Int) = ceil(log(in)/log(2)).toInt
  // Constants
  val TRANSPERWORD = DATA_WIDTH/ramDataWidth
  val TRANSPERCMD  = TRANSPERWORD*ocpBurstLen
  val TRANSPERSEL  = 4 // burst-of-four mode
  val BYTESPERTRAN = ramDataWidth / 8
  val BYTESPERSEL  = BYTESPERTRAN * TRANSPERSEL

  // State type and variable
  val sReady :: sReadSel :: sReadWait :: sReadExe :: sReadRet :: sWriteRec  :: sWriteSel :: sWriteExe:: sWriteRet :: Nil = Enum(UInt(), 9)
  val stateReg = Reg(init = sReady)

  // Internal Registers
  val mAddrReg         = Reg(Bits(width = ramAddrWidth))
  val rdBufferReg      = Reg(Vec(TRANSPERCMD, Bits(width = ramDataWidth)))
  val wrBufferReg      = Reg(Vec(TRANSPERCMD, new Trans(BYTESPERTRAN, ramDataWidth)))
  val transCountReg    = Reg(init = UInt(0, width = log2upNew(TRANSPERCMD)))
  val subTransCountReg = Reg(init = UInt(0, width = log2upNew(TRANSPERSEL)))
  val wordCountReg     = Reg(init = UInt(0, width = log2upNew(ocpBurstLen)))
  val waitCountReg     = Reg(init = UInt(0, width = log2upNew(readWaitCycles)))

  // Output Registers
  val addrReg = Reg(Bits(width = ramAddrWidth))
  val nrpsReg = Reg(Bits(width = 1))
  val nwpsReg = Reg(Bits(width = 1))
  val nbwsReg = Reg(Vec(2, Bits(width = BYTESPERTRAN)))
  val doutReg = Reg(Vec(2, Bits(width = ramDataWidth)))

  // Default values for ocp io.ocp.S port
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.CmdAccept := Bits(0)
  io.ocp.S.DataAccept := Bits(0)
  val data = for(i <- 0 until TRANSPERWORD)
             yield rdBufferReg(wordCountReg ## UInt(i))
  io.ocp.S.Data := data.reduceLeft((x,y) => y ## x)

  // Default values for output ports
  addrReg := mAddrReg
  doutReg(0) := wrBufferReg(0).data
  doutReg(1) := wrBufferReg(1).data
  nrpsReg := Bits(1)
  nwpsReg := Bits(1)
  nbwsReg(0) := ~wrBufferReg(0).byteEna
  nbwsReg(1) := ~wrBufferReg(1).byteEna

  when(stateReg === sReady) {
    for(i <- 0 until TRANSPERWORD) {
      wrBufferReg(i).byteEna := io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(i).data    := io.ocp.M.Data((i+1)*ramDataWidth-1,i*ramDataWidth)
    }
    when(io.ocp.M.Cmd =/= OcpCmd.IDLE) {
      mAddrReg := io.ocp.M.Addr(ramAddrWidth + log2upNew(BYTESPERSEL) - 1,
                                log2upNew(BYTESPERSEL))
      io.ocp.S.CmdAccept := Bits(1)
      when(io.ocp.M.Cmd === OcpCmd.RD) {
        stateReg := sReadSel
      }
      when(io.ocp.M.Cmd === OcpCmd.WR) {
        io.ocp.S.DataAccept := Bits(1)
        wordCountReg := UInt(1) // The first ocp data word is already in wrBufferReg
        stateReg := sWriteRec
      }
    }
  }
  when(stateReg === sReadSel) {
    nrpsReg := Bits(0)
    if (readWaitCycles == 0) {
      stateReg := sReadExe
    } else {
      stateReg := sReadWait
    }
  }
  when(stateReg === sReadWait) {
    waitCountReg := waitCountReg + UInt(1)
    when(waitCountReg === UInt(readWaitCycles-1)) {
      stateReg := sReadExe
      waitCountReg := UInt(0)
    }
  }
  when(stateReg === sReadExe) {
    transCountReg := transCountReg + UInt(1)
    subTransCountReg := subTransCountReg + UInt(1)
    rdBufferReg(transCountReg ## UInt(0)) := io.pins.din(0)
    rdBufferReg(transCountReg ## UInt(1)) := io.pins.din(1)
    when(subTransCountReg === UInt((TRANSPERSEL+1)/2-1)) {
      stateReg := sReadSel
      mAddrReg := mAddrReg + UInt(1)
      subTransCountReg := UInt(0)
    }
    when(transCountReg === UInt((TRANSPERCMD+1)/2-1)) {
      stateReg := sReadRet
      transCountReg := UInt(0)
    }
  }
  when(stateReg === sReadRet) {
    io.ocp.S.Resp := OcpResp.DVA
    wordCountReg := wordCountReg + UInt(1)
    when(wordCountReg === UInt(ocpBurstLen-1)) {
      stateReg := sReady
      wordCountReg := UInt(0)
    }
  }
  when(stateReg === sWriteRec) {
    for(i <- 0 until TRANSPERWORD) {
      wrBufferReg(wordCountReg ## UInt(i)).byteEna :=
        io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(wordCountReg ## UInt(i)).data :=
        io.ocp.M.Data((i+1)*ramDataWidth-1,i*ramDataWidth)
    }
    when(io.ocp.M.DataValid === Bits(1)){
      io.ocp.S.DataAccept := Bits(1)
      wordCountReg := wordCountReg + UInt(1)
      when(wordCountReg === UInt(ocpBurstLen-1)){
        stateReg := sWriteExe
        wordCountReg := UInt(0)
        nwpsReg := Bits(0)
      }
    }
  }
  when(stateReg === sWriteSel) {
    stateReg := sWriteExe
    nwpsReg := Bits(0)
  }
  when(stateReg === sWriteExe) {
    doutReg(0) := wrBufferReg(transCountReg ## UInt(0)).data
    doutReg(1) := wrBufferReg(transCountReg ## UInt(1)).data
    nbwsReg(0) := ~wrBufferReg(transCountReg ## UInt(0)).byteEna
    nbwsReg(1) := ~wrBufferReg(transCountReg ## UInt(1)).byteEna
    transCountReg := transCountReg + UInt(1)
    subTransCountReg := subTransCountReg + UInt(1)
    when(subTransCountReg === UInt((TRANSPERSEL+1)/2-1)) {
      stateReg := sWriteSel
      mAddrReg := mAddrReg + UInt(1)
      subTransCountReg := UInt(0)
    }
    when(transCountReg === UInt((TRANSPERCMD+1)/2-1)) {
      io.ocp.S.Resp := OcpResp.DVA
      stateReg := sReady
      transCountReg := UInt(0)
    }
  }

  // Assign register values to pins
  io.pins.addr := addrReg
  io.pins.dout := doutReg
  io.pins.nrps := nrpsReg
  io.pins.nwps := nwpsReg
  io.pins.nbws := nbwsReg

  // Hard-wired pins
  io.pins.ndoff := Bits(1)

  class Trans(bytesEnaWidth: Int, dataWidth: Int) extends Bundle {
    val byteEna = Bits(width = bytesEnaWidth)
    val data = Bits(width = dataWidth)

    // This does not really clone, but Data.clone doesn't either
    override def cloneType() = {
      val res = new Trans(bytesEnaWidth, dataWidth)
      res.asInstanceOf[this.type]
    }
  }
}

/*
 Used to instantiate a single controller component
 */
object QdrIIplusCtrlMain {
  def main(args: Array[String]): Unit = {
    val chiselArgs = args.slice(1,args.length)
    val ocpAddrWidth = args(0).toInt
    chiselMain(chiselArgs, () => Module(new QdrIIplusCtrl(ocpAddrWidth)))
  }
}
