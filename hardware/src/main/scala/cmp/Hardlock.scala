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
import chisel3.core.VecInit
import patmos.Constants._
import ocp._

class HardlockIO(val lckCnt : Int) extends Bundle {
  val sel = UInt(INPUT, log2Up(lckCnt))
  val op = Bool(INPUT)
  val en = Bool(INPUT)
  val blck = Bool(OUTPUT)
}

class HardlockIOVec(val coreCnt : Int, val lckCnt : Int) extends Bundle {
  val cores = Vec(coreCnt, new HardlockIO(lckCnt))
}

abstract class AbstractHardlock(coreCnt : Int,lckCnt : Int) extends Module {

  val CoreCount = coreCnt
  val LockCount = lckCnt
  
  override val io = IO(new HardlockIOVec(coreCnt, lckCnt)) // Vec.fill(coreCnt){new HardlockIO(lckCnt)}
  
  
  val queueReg = RegInit(Vec.fill(lckCnt){UInt(0, coreCnt)})
  val queueBools = Wire(Vec(lckCnt, Vec(coreCnt, Bool()))) //Chisel3 - Subword assignment unsupported
  for (i <- 0 until lckCnt) {
    queueBools(i) := queueReg(i).toBools
    for (j <- 0 until coreCnt) {
      when(io.cores(j).sel === UInt(i) && io.cores(j).en === Bool(true)) {
        queueBools(i)(j) := io.cores(j).op
        queueReg(i) := queueBools(i).asUInt()
      }
    }
  }
  
  val curReg = RegInit(Vec.fill(lckCnt){UInt(0, log2Up(coreCnt))})
  
  val blocks = Wire(Vec(coreCnt, Bits(width = lckCnt)))
  val blockBools = Wire(Vec(coreCnt, Vec(lckCnt, Bool()))) //subword assignment workaround
  
  for (i <- 0 until coreCnt) {
    blockBools(i) := UInt(0).toBools
    blocks(i) := UInt(0)
    for (j <- 0 until lckCnt) {
      blockBools(i)(j) := queueReg(j)(i) && (curReg(j) =/= UInt(i))
      blocks(i) := blockBools(i).asUInt()
    }

    io.cores(i).blck := blocks(i).orR
  }
}

class Hardlock(coreCnt : Int,lckCnt : Int) extends AbstractHardlock(coreCnt, lckCnt) {  
  // Circular priority encoder
  val hi = Wire(Vec(lckCnt, Bits(width = coreCnt)))
  val hiBools = Wire(Vec(lckCnt, Vec(coreCnt, Bool())))//subword assignment workaround
  val lo = Wire(Vec(lckCnt, Bits(width = coreCnt)))
  val loBools = Wire(Vec(lckCnt, Vec(coreCnt, Bool())))//subword assignment workaround
  
  for (i <- 0 until lckCnt) {
    lo(i) := UInt(0)
    hiBools(i) := UInt(0).toBools
    hi(i) := UInt(0)
    loBools(i) := UInt(0).toBools
    for (j <- 0 until coreCnt) {
      loBools(i)(j) := queueReg(i)(j) && (curReg(i) > UInt(j))
      hiBools(i)(j) := queueReg(i)(j) && (curReg(i) <= UInt(j))
      lo(i) := loBools(i).asUInt()
      hi(i) := hiBools(i).asUInt()
    }
    
    when(hi(i).orR) {
      curReg(i) := PriorityEncoder(hi(i))
    }
    .otherwise {
      curReg(i) := PriorityEncoder(lo(i))
    }
  }
}

class HardlockOCPWrapper(hardlockgen: () => AbstractHardlock) extends Module {
  
  val hardlock = Module(hardlockgen())
  
  override val io = IO(new CmpIO(hardlock.CoreCount)) //Vec.fill(hardlock.CoreCount){new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)}
  
  // Mapping between internal io and OCP here
  
  val reqReg = Reg(init = Bits(0,hardlock.CoreCount))
  val reqBools = Wire(Vec(hardlock.CoreCount, Bool()))

  reqBools := reqReg.toBools

  for (i <- 0 until hardlock.CoreCount) {
    hardlock.io.cores(i).op := io.cores(i).M.Data(0);
    hardlock.io.cores(i).sel := io.cores(i).M.Data >> 1;
    hardlock.io.cores(i).en := Bool(false)
    when(io.cores(i).M.Cmd === OcpCmd.WR) {
      hardlock.io.cores(i).en := Bool(true)
    }

    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE) {
      reqBools(i) := Bool(true)
      reqReg := reqBools.asUInt()
    }
    .elsewhen(reqReg(i) === Bool(true) && hardlock.io.cores(i).blck === Bool(false)) {
      reqBools(i) := Bool(false)
      reqReg := reqBools.asUInt()
    }
    
    io.cores(i).S.Resp := OcpResp.NULL
    when(reqReg(i) === Bool(true) && hardlock.io.cores(i).blck === Bool(false)) {
      io.cores(i).S.Resp := OcpResp.DVA
    }
      
    io.cores(i).S.Data := UInt(0)
  }
}
