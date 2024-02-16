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
import chisel3.VecInit
import patmos.Constants._
import ocp._

class HardlockIO(val lckCnt : Int) extends Bundle {
  val sel = Input(UInt(log2Up(lckCnt).W))
  val op = Input(Bool())
  val en = Input(Bool())
  val blck = Output(Bool())
}

class HardlockIOVec(val coreCnt : Int, val lckCnt : Int) extends Bundle {
  val cores = Vec(coreCnt, new HardlockIO(lckCnt))
}

abstract class AbstractHardlock(coreCnt : Int,lckCnt : Int) extends Module {

  val CoreCount = coreCnt
  val LockCount = lckCnt
  
  val io = IO(new HardlockIOVec(coreCnt, lckCnt))
  
  
  val queueReg = RegInit(VecInit(Seq.fill(lckCnt)(VecInit(Seq.fill(coreCnt)(false.B)))))
  for (i <- 0 until lckCnt) {
    for (j <- 0 until coreCnt) {
      when(io.cores(j).sel === i.U && io.cores(j).en === true.B) {
        queueReg(i)(j) := io.cores(j).op
      }
    }
  }
  
  val curReg = RegInit(VecInit(Seq.fill(lckCnt)(0.U(log2Up(coreCnt).W))))
  
  val blocks = Wire(Vec(coreCnt, Vec(lckCnt, Bool())))
  
  for (i <- 0 until coreCnt) {
    for (j <- 0 until lckCnt) {
      blocks(i)(j) := queueReg(j)(i) && (curReg(j) =/= i.U)
    }
    io.cores(i).blck := blocks(i).asUInt.orR
  }
}

class Hardlock(coreCnt : Int,lckCnt : Int) extends AbstractHardlock(coreCnt, lckCnt) {  
  // Circular priority encoder
  val hi = Wire(Vec(lckCnt, Vec(coreCnt, Bool())))
  val lo = Wire(Vec(lckCnt, Vec(coreCnt, Bool())))
  
  for (i <- 0 until lckCnt) {
    for (j <- 0 until coreCnt) {
      lo(i)(j) := queueReg(i)(j) && (curReg(i) > j.U)
      hi(i)(j) := queueReg(i)(j) && (curReg(i) <= j.U)
    }
    
    when(hi(i).asUInt.orR) {
      curReg(i) := PriorityEncoder(hi(i))
    }
    .otherwise {
      curReg(i) := PriorityEncoder(lo(i))
    }
  }
}

class HardlockOCPWrapper(nrCores: Int, hardlockgen: () => AbstractHardlock) extends CmpDevice(nrCores) {
  
  val hardlock = Module(hardlockgen())

  // TODO: workaround:
  io.pins.tx := 0.U

  // Mapping between internal io and OCP here
  
  val reqReg = Reg(init = Bits(0,hardlock.CoreCount))
  val reqBools = Wire(Vec(hardlock.CoreCount, Bool()))

  reqBools := reqReg.asBools

  for (i <- 0 until hardlock.CoreCount) {
    hardlock.io.cores(i).op := io.cores(i).M.Data(0);
    hardlock.io.cores(i).sel := io.cores(i).M.Data >> 1;
    hardlock.io.cores(i).en := false.B
    when(io.cores(i).M.Cmd === OcpCmd.WR) {
      hardlock.io.cores(i).en := true.B
    }

    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE) {
      reqBools(i) := true.B
      reqReg := reqBools.asUInt()
    }
    .elsewhen(reqReg(i) === true.B && hardlock.io.cores(i).blck === false.B) {
      reqBools(i) := false.B
      reqReg := reqBools.asUInt()
    }
    
    io.cores(i).S.Resp := OcpResp.NULL
    when(reqReg(i) === true.B && hardlock.io.cores(i).blck === false.B) {
      io.cores(i).S.Resp := OcpResp.DVA
    }
      
    io.cores(i).S.Data := 0.U
  }
}
