
/*
 * Arbiter for OCP burst slaves.
 * TDM arbitration. Each turn for a non-requesting master costs 16+4+2 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com) David Chong (davidchong99@gmail.com)
 *
 */

package ocp

import Chisel._
import chisel3.VecInit

class NodeTdmArbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int, ctrlDelay: Int) extends Module {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = IO(new Bundle {
    val master = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen) 
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
    val node = Input(UInt(6.W))
  })
  //debug(io.master) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
  //debug(io.slave)
  //debug(io.node)
  
  // MS: have all generated constants at one place

  val cntReg = Reg(init = UInt(0, log2Up(cnt*(burstLen + ctrlDelay + 1))))
  // slot length = burst size + 1 
  val burstCntReg = Reg(init = UInt(0, log2Up(burstLen)))
  val period = cnt * (burstLen + ctrlDelay + 1)
  val slotLen = burstLen + ctrlDelay + 1
  val numPipe = 3 
  
  val wrPipeDelay = burstLen + ctrlDelay + numPipe 
  val wrCntReg = Reg(init = UInt(0, log2Up(wrPipeDelay)))

  val rdPipeDelay = burstLen + ctrlDelay + numPipe 
  val rdCntReg = Reg(init = UInt(0, log2Up(rdPipeDelay)))
  
  // MS: merge rdCntReg and wrCntReg and let it count till slot length
 
  val cpuSlot = RegInit(VecInit(Seq.fill(cnt)(0.U(1.W))))

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = sIdle)

  /*debug(cntReg) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
  for(i <- (0 until cnt))
    debug(cpuSlot(i))
    
  debug(stateReg)
  debug(wrCntReg)
  debug(rdCntReg)*/

  cntReg := Mux(cntReg === UInt(period - 1), UInt(0), cntReg + UInt(1))
  
  def slotTable(i: Int): UInt = {
    (cntReg === UInt(i*slotLen)).asUInt
  }
  
  for (i <- 0 until cnt){
    cpuSlot(i) := slotTable(i)
  }
  
  // Initialize master data to zero when cpuSlot is not enabled 
  io.slave.M.Addr       := UInt(0)
  io.slave.M.Cmd        := UInt(0)
  io.slave.M.DataByteEn := UInt(0)
  io.slave.M.DataValid  := UInt(0)
  io.slave.M.Data       := UInt(0)

  // Initialize slave data to zero
  io.master.S.Data       := UInt(0)
  io.master.S.Resp       := OcpResp.NULL
  io.master.S.CmdAccept  := UInt(0)
  io.master.S.DataAccept := UInt(0)
  
  // FSM for TDM Arbiter 
  when (stateReg === sIdle) {
    when (cpuSlot(io.node) === UInt(1)) {
      val master = io.master.M
      //io.slave.M := master
      
      when (master.Cmd =/= OcpCmd.IDLE){
        when (master.Cmd === OcpCmd.RD) {
          io.slave.M := master
          stateReg := sRead
          io.master.S.CmdAccept := UInt(1)
          rdCntReg := UInt(0)
        }
        when (master.Cmd === OcpCmd.WR) {
          io.slave.M := master
          stateReg := sWrite
          io.master.S.CmdAccept := UInt(1)
          io.master.S.DataAccept := UInt(1)
          wrCntReg := UInt(0)
        }
      }
    }
  }

  when (stateReg === sWrite){
    io.slave.M := io.master.M
    io.master.S.DataAccept := UInt(1)
    // MS: why not counting just up to the slot length
    // Then we can avoid >= and use =
    wrCntReg := Mux(wrCntReg === UInt(wrPipeDelay), UInt(0), wrCntReg + UInt(1))
   
    // Sends ZEROs after the burst is done 
    when (wrCntReg >= UInt(burstLen-1)) {
      io.slave.M.Cmd  := UInt(0)
      io.slave.M.Addr := UInt(0)
      io.slave.M.Data := UInt(0)
      io.slave.M.DataValid := UInt(0)
      io.slave.M.DataByteEn := UInt(0)
    }

    // Turn off the DataValid after a burst of 4
    when (wrCntReg >= UInt(burstLen-1)){
      io.master.S.DataAccept := UInt(0)
    }
    
    // Forward Rsp/DVA back to node 
    // Ms: not hard coded constants in the source
    when (wrCntReg === UInt(4)) {
      io.master.S.Resp := OcpResp.DVA
    }
    // Wait on DVA 
    when(io.master.S.Resp === OcpResp.DVA){
      stateReg := sIdle
    }
  }
     
  when (stateReg === sRead){
    io.slave.M := io.master.M
    rdCntReg := Mux(rdCntReg === UInt(rdPipeDelay + burstLen), UInt(0), rdCntReg + UInt(1))
    
    // Sends ZEROs after the burst is done 
    // MS: This should also (as in write) be just the slot length.
    when (rdCntReg >= UInt(burstLen-1)) {
      io.slave.M.Cmd  := UInt(0)
      io.slave.M.Addr := UInt(0)
      io.slave.M.Data := UInt(0)
      io.slave.M.DataValid := UInt(0)
      io.slave.M.DataByteEn := UInt(0) 
    }
    
    // rdCntReg starts 1 clock cycle after the arrival of the 1st data
    // MS: rdCntReg is used for two different purposes -- fix it
    // The following shall also include number of pipeline stages on the return path
    when (rdCntReg >= UInt(ctrlDelay + numPipe)) {
      io.master.S.Data := io.slave.S.Data
      io.master.S.Resp := io.slave.S.Resp
    }
  
    when (io.master.S.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + UInt(1)
        when (burstCntReg === UInt(burstLen) - UInt(1)) {
          stateReg := sIdle
        }
     }
  }      
  
  //debug(io.slave.M) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
  
  //io.master.S := io.slave.S

}

/* Mux for all arbiters' outputs */
class MemMuxIntf(nr: Int, addrWidth : Int, dataWidth : Int, burstLen: Int) extends Module {
  val io = IO(new Bundle {
    val master = Vec(nr, new OcpBurstSlavePort(addrWidth, dataWidth, burstLen))
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  })
    //debug(io.master) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
    //debug(io.slave)
    
    // MS: would like pipeline number configurable
    
    // 1st stage pipeline registers for inputs 
    val mCmd_p1_Reg         = RegInit(VecInit(Seq.fill(nr)(0.U(3.W))))
    val mAddr_p1_Reg        = Reg(Vec(nr, UInt(width=addrWidth)))
    val mData_p1_Reg        = Reg(Vec(nr, UInt(width=dataWidth)))
    val mDataByteEn_p1_Reg  = Reg(Vec(nr, UInt(width=dataWidth/8)))
    val mDataValid_p1_Reg   = Reg(Vec(nr, UInt(width=1)))

    // 2st stage pipeline registers for inputs
    // MS: what about using the whole bundle as a single signal?
    // val mMasterReg = Reg(init=OcpBurstMasterSignals(...))
    val mCmd_p2_Reg         = Reg(init=UInt(0, width=3))
    val mAddr_p2_Reg        = Reg(UInt(width=addrWidth))
    val mData_p2_Reg        = Reg(UInt(width=dataWidth))
    val mDataByteEn_p2_Reg  = Reg(UInt(width=dataWidth/8))
    val mDataValid_p2_Reg   = Reg(UInt(width=1))
    
    // Pipeline registers default to 0
    for(i <- 0 until nr){
      mCmd_p1_Reg(i)         := 0.U
      mAddr_p1_Reg(i)        := 0.U
      mData_p1_Reg(i)        := 0.U
      mDataByteEn_p1_Reg(i)  := 0.U
      mDataValid_p1_Reg(i)   := 0.U
    }
    
    // 1st stage pipeline of the input
    for (i <- 0 until nr){
      mCmd_p1_Reg(i) := io.master(i).M.Cmd
    }
    
    for (i <- 0 until nr){
      mAddr_p1_Reg(i) := io.master(i).M.Addr
    }
    
    for (i <- 0 until nr){
      mData_p1_Reg(i) := io.master(i).M.Data
    }
    
    for (i <- 0 until nr){
      mDataByteEn_p1_Reg(i) := io.master(i).M.DataByteEn
    }
    
    for (i <- 0 until nr){
      mDataValid_p1_Reg(i) := io.master(i).M.DataValid
    }
     
    // OR gate of all inputs (2nd stage pipeline)
    
    mCmd_p2_Reg := mCmd_p1_Reg.reduce(_|_)
    mAddr_p2_Reg := mAddr_p1_Reg.reduce(_|_)
    mData_p2_Reg := mData_p1_Reg.reduce(_|_)
    mDataByteEn_p2_Reg := mDataByteEn_p1_Reg.reduce(_|_)
    mDataValid_p2_Reg := mDataValid_p1_Reg.reduce(_|_)
    
    // Transfer data from input pipeline registers to output
    io.slave.M.Addr       := mAddr_p2_Reg
    io.slave.M.Cmd        := mCmd_p2_Reg
    io.slave.M.DataByteEn := mDataByteEn_p2_Reg
    io.slave.M.DataValid  := mDataValid_p2_Reg
    io.slave.M.Data       := mData_p2_Reg
   
    // 1st stage pipeline registers for output
    val sResp_p1_Reg        = Reg(next=io.slave.S.Resp)
    val sData_p1_Reg        = Reg(next=io.slave.S.Data)

    
    // Forward response to all arbiters  
    for (i <- 0 until nr) {
      io.master(i).S.Data := sData_p1_Reg
      io.master(i).S.Resp := sResp_p1_Reg 
    }
    
}

object NodeTdmArbiterMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(5, args.length)
    val cnt = args(0)
    val addrWidth = args(1)
    val dataWidth = args(2)
    val burstLen = args(3)
    val ctrlDelay = args(4)

    chiselMain(chiselArgs, () => Module(new NodeTdmArbiter(cnt.toInt,addrWidth.toInt,dataWidth.toInt,burstLen.toInt, ctrlDelay.toInt)))
  }
}


