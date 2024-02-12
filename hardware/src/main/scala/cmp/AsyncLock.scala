/*
 * Asynchronous lock
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
package cmp

import Chisel._
import ocp._
import patmos.Constants._

import scala.collection.mutable

class AsyncMutexIO extends Bundle
{
  val req1 = Input(Bool())
  val req2 = Input(Bool())
  val gnt1 = Output(Bool())
  val gnt2 = Output(Bool())

  override def clone = new AsyncMutexIO().asInstanceOf[this.type]
}

class AsyncMutexBB() extends BlackBox {
  val io = new AsyncMutexIO()
  //throw new Error("BlackBox wrapper for AsyncMuteX in AsyncLock.scala needs update for Chisel 3")
  // rename component
  // should be Commented out to compile for chisel3
  /*setModuleName("AsyncMutex")

  renameClock(clock, "clk")
  renameReset("rst")

  io.req1.setName("req1")
  io.req2.setName("req2")
  io.gnt1.setName("gnt1")
  io.gnt2.setName("gnt2")*/
}

class AsyncArbiterMesh(corecnt: Int) extends AsyncArbiterBase(corecnt) {

  var avail = (0 until corecnt).flatMap(i => (i+1 until corecnt).map(j => (i,j)))

  val width = if(corecnt%2 == 1) corecnt else corecnt-1
  val depth = (if(corecnt%2 == 1) (corecnt-1)/2 else corecnt/2).toInt


  val _ins = io.cores.map(e => e.req)
  val ins = Array(_ins: _*)

  val genset = new ((IndexedSeq[Tuple2[Int,Int]], Int) => IndexedSeq[Tuple2[Int,Int]]){
    def apply(done: IndexedSeq[Tuple2[Int,Int]], _depth: Int):IndexedSeq[Tuple2[Int,Int]] =
    {

      for(i <- 0 until avail.length) {
        val tup = avail(i)
        if(!done.exists(e => e._1 == tup._1 || e._1 == tup._2 || e._2 == tup._1 || e._2 == tup._2 )) {
          if(_depth < depth) {
            val set = apply(done :+ tup, _depth+1)
            if(set.nonEmpty)
              return set :+ tup
          } else return IndexedSeq(tup)
        }
      }
      return IndexedSeq()
    }
  }

  while(avail.nonEmpty) {

    val seq = genset(IndexedSeq(), 1)

    if(seq.isEmpty)
      throwException("Should not happen!")
    for(tup <- seq) {
      avail = avail.filter(e => e != tup)

      val mutex = Module(new AsyncMutexBB())

      mutex.io.req1 := ins(tup._1)
      ins(tup._1) = mutex.io.gnt1

      mutex.io.req2 := ins(tup._2)
      ins(tup._2) = mutex.io.gnt2


    }

  }

  for(i <- 0 until corecnt) {
    io.cores(i).ack := ins(i)
  }
}

class AsyncLock(corecnt: Int, lckcnt: Int, fair: Boolean = false) extends Module {

  val arbiters =
    if(!fair)
      (0 until lckcnt).map(i =>
    {
      val arb = Module(new AsyncArbiterTree(corecnt))
      arb.io.ack := arb.io.req
      arb
    })
    else
      (0 until lckcnt).map(i => Module(new AsyncArbiterMesh(corecnt)))

  val arbiterio = Vec(arbiters.map(e => e.io))

  val io = IO(new CmpIO(corecnt)) //Vec(corecnt,new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  for (i <- 0 until corecnt) {

    val addr = io.cores(i).M.Addr(log2Up(lckcnt)-1+2, 2)
    val acks = Bits(width = lckcnt)
    acks := 0.U
    val blck = acks.orR

    for (j <- 0 until lckcnt) {
      val reqReg = Reg(init = Bool(false))
      arbiterio(j).cores(i).req := reqReg
      val ackReg = Reg(next = Reg(next = arbiterio(j).cores(i).ack))
      acks(j) := ackReg =/= reqReg

      when(addr === j.U) {
        when(io.cores(i).M.Cmd === OcpCmd.RD) {
          reqReg := Bool(true)
        }.elsewhen(io.cores(i).M.Cmd === OcpCmd.WR) {
          reqReg := Bool(false)
        }
      }
    }

    val dvaReg = Reg(init = Bool(false))

    when(io.cores(i).M.Cmd =/= OcpCmd.IDLE) {
      dvaReg := Bool(true)
    }.elsewhen(dvaReg === Bool(true) && !blck) {
      dvaReg := Bool(false)
    }

    io.cores(i).S.Resp := OcpResp.NULL
    when(dvaReg === Bool(true) && !blck) {
      io.cores(i).S.Resp := OcpResp.DVA
    }

    // Perhaps remove this
    io.cores(i).S.Data := UInt(0)
  }
}
