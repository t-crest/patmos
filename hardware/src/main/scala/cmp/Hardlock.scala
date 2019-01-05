/*
 * Speed Locking Unit
 * 
 * This component manages the synchronization locks for the processor cores. 
 * A core requests a lock and is halted until the lock becomes available.
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
package cmp
 
import Chisel._

import patmos.Constants._
import ocp._

class HardlockIO(lckCnt : Int) extends Bundle {
  val sel = UInt(INPUT, log2Up(lckCnt))
  val op = Bool(INPUT)
  val en = Bool(INPUT)
  val blck = Bool(OUTPUT)
}

abstract class AbstractHardlock(coreCnt : Int,lckCnt : Int) extends Module {

  val CoreCount = coreCnt
  val LockCount = lckCnt
  
  override val io = Vec.fill(coreCnt){new HardlockIO(lckCnt)}
  
  
  val queueReg = Vec.fill(lckCnt){RegInit(UInt(0, coreCnt))}
  for (i <- 0 until lckCnt) {
    for (j <- 0 until coreCnt) {
      when(io(j).sel === UInt(i) && io(j).en === Bool(true)) {
        queueReg(i)(j) := io(j).op
      }
    }
  }
  
  val curReg = Vec.fill(lckCnt){RegInit(UInt(0, log2Up(coreCnt)))}
  
  val blocks = Vec.fill(coreCnt)(Bits(width = lckCnt))
  
  for (i <- 0 until coreCnt) {
    blocks(i) := UInt(0)
    for (j <- 0 until lckCnt) {
      blocks(i)(j) := queueReg(j)(i) && (curReg(j) =/= UInt(i)) 
    }
    io(i).blck := orR(blocks(i))
  }
}

class Hardlock(coreCnt : Int,lckCnt : Int) extends AbstractHardlock(coreCnt, lckCnt) {  
  // Circular priority encoder
  val hi = Vec.fill(lckCnt){Bits(width = coreCnt)}
  val lo = Vec.fill(lckCnt){Bits(width = coreCnt)}
  
  for (i <- 0 until lckCnt) {
    lo(i) := UInt(0)
    hi(i) := UInt(0)
    for (j <- 0 until coreCnt) {
      lo(i)(j) := queueReg(i)(j) && (curReg(i) > UInt(j))
      hi(i)(j) := queueReg(i)(j) && (curReg(i) <= UInt(j))
    }
    
    when(orR(hi(i))) {
      curReg(i) := PriorityEncoder(hi(i))
    }
    .otherwise {
      curReg(i) := PriorityEncoder(lo(i))
    }
  }
}

class HardlockOCPWrapper(hardlockgen: () => AbstractHardlock) extends Module {
  
  val hardlock = Module(hardlockgen())
  
  override val io = Vec.fill(hardlock.CoreCount){new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)}
  
  // Mapping between internal io and OCP here
  
  val reqReg = Reg(init = Bits(0,hardlock.CoreCount))

  for (i <- 0 until hardlock.CoreCount) {
    hardlock.io(i).op := io(i).M.Data(0);
    hardlock.io(i).sel := io(i).M.Data >> 1;
    hardlock.io(i).en := Bool(false)
    when(io(i).M.Cmd === OcpCmd.WR) {
      hardlock.io(i).en := Bool(true)
    }

    when(io(i).M.Cmd =/= OcpCmd.IDLE) {
      reqReg(i) := Bool(true)
    }
    .elsewhen(reqReg(i) === Bool(true) && hardlock.io(i).blck === Bool(false)) {
      reqReg(i) := Bool(false)
    }
    
    io(i).S.Resp := OcpResp.NULL
    when(reqReg(i) === Bool(true) && hardlock.io(i).blck === Bool(false)) {
      io(i).S.Resp := OcpResp.DVA
    }
      
    io(i).S.Data := UInt(0)
  }
}
