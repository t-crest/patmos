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
import chisel3.VecInit

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
      val nbws  = Output(VecInit(Seq.fill(2)(Bits(width = ramDataWidth / 8))))

      val din   = VecInit(Seq.fill(2)(Bits(INPUT, width = ramDataWidth)))
      val dout  = VecInit(Seq.fill(2)(Bits(OUTPUT, width = ramDataWidth)))

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
  val transCountReg    = Reg(init = 0.U(log2upNew(TRANSPERCMD).W))
  val subTransCountReg = Reg(init = 0.U(log2upNew(TRANSPERSEL).W))
  val wordCountReg     = Reg(init = 0.U(log2upNew(ocpBurstLen).W))
  val waitCountReg     = Reg(init = 0.U(log2upNew(readWaitCycles).W))

  // Output Registers
  val addrReg = Reg(Bits(width = ramAddrWidth))
  val nrpsReg = Reg(Bits(width = 1))
  val nwpsReg = Reg(Bits(width = 1))
  val nbwsReg = Reg(Vec(2, Bits(width = BYTESPERTRAN)))
  val doutReg = Reg(Vec(2, Bits(width = ramDataWidth)))

  // Default values for ocp io.ocp.S port
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.CmdAccept := 0.U
  io.ocp.S.DataAccept := 0.U
  val data = for(i <- 0 until TRANSPERWORD)
             yield rdBufferReg(wordCountReg ## i.U)
  io.ocp.S.Data := data.reduceLeft((x,y) => y ## x)

  // Default values for output ports
  addrReg := mAddrReg
  doutReg(0) := wrBufferReg(0).data
  doutReg(1) := wrBufferReg(1).data
  nrpsReg := 1.U
  nwpsReg := 1.U
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
      io.ocp.S.CmdAccept := 1.U
      when(io.ocp.M.Cmd === OcpCmd.RD) {
        stateReg := sReadSel
      }
      when(io.ocp.M.Cmd === OcpCmd.WR) {
        io.ocp.S.DataAccept := 1.U
        wordCountReg := 1.U // The first ocp data word is already in wrBufferReg
        stateReg := sWriteRec
      }
    }
  }
  when(stateReg === sReadSel) {
    nrpsReg := 0.U
    if (readWaitCycles == 0) {
      stateReg := sReadExe
    } else {
      stateReg := sReadWait
    }
  }
  when(stateReg === sReadWait) {
    waitCountReg := waitCountReg + 1.U
    when(waitCountReg === (readWaitCycles-1).U) {
      stateReg := sReadExe
      waitCountReg := 0.U
    }
  }
  when(stateReg === sReadExe) {
    transCountReg := transCountReg + 1.U
    subTransCountReg := subTransCountReg + 1.U
    rdBufferReg(transCountReg ## 0.U) := io.pins.din(0)
    rdBufferReg(transCountReg ## 1.U) := io.pins.din(1)
    when(subTransCountReg === ((TRANSPERSEL+1)/2-1).U) {
      stateReg := sReadSel
      mAddrReg := mAddrReg + 1.U
      subTransCountReg := 0.U
    }
    when(transCountReg === ((TRANSPERCMD+1)/2-1).U) {
      stateReg := sReadRet
      transCountReg := 0.U
    }
  }
  when(stateReg === sReadRet) {
    io.ocp.S.Resp := OcpResp.DVA
    wordCountReg := wordCountReg + 1.U
    when(wordCountReg === (ocpBurstLen-1).U) {
      stateReg := sReady
      wordCountReg := 0.U
    }
  }
  when(stateReg === sWriteRec) {
    for(i <- 0 until TRANSPERWORD) {
      wrBufferReg(wordCountReg ## i.U).byteEna :=
        io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(wordCountReg ## i.U).data :=
        io.ocp.M.Data((i+1)*ramDataWidth-1,i*ramDataWidth)
    }
    when(io.ocp.M.DataValid === 1.U){
      io.ocp.S.DataAccept := 1.U
      wordCountReg := wordCountReg + 1.U
      when(wordCountReg === (ocpBurstLen-1).U){
        stateReg := sWriteExe
        wordCountReg := 0.U
        nwpsReg := 0.U
      }
    }
  }
  when(stateReg === sWriteSel) {
    stateReg := sWriteExe
    nwpsReg := 0.U
  }
  when(stateReg === sWriteExe) {
    doutReg(0) := wrBufferReg(transCountReg ## 0.U).data
    doutReg(1) := wrBufferReg(transCountReg ## 1.U).data
    nbwsReg(0) := ~wrBufferReg(transCountReg ## 0.U).byteEna
    nbwsReg(1) := ~wrBufferReg(transCountReg ## 1.U).byteEna
    transCountReg := transCountReg + 1.U
    subTransCountReg := subTransCountReg + 1.U
    when(subTransCountReg === ((TRANSPERSEL+1)/2-1).U) {
      stateReg := sWriteSel
      mAddrReg := mAddrReg + 1.U
      subTransCountReg := 0.U
    }
    when(transCountReg === ((TRANSPERCMD+1)/2-1).U) {
      io.ocp.S.Resp := OcpResp.DVA
      stateReg := sReady
      transCountReg := 0.U
    }
  }

  // Assign register values to pins
  io.pins.addr := addrReg
  io.pins.dout := doutReg
  io.pins.nrps := nrpsReg
  io.pins.nwps := nwpsReg
  io.pins.nbws := nbwsReg

  // Hard-wired pins
  io.pins.ndoff := 1.U

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
