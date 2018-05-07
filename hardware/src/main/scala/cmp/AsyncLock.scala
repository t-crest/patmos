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

class AsyncLock(corecnt: Int, lckcnt: Int) extends Module {

  val arbiters = (0 until lckcnt).map(i =>
  {
    val arb = Module(new AsyncArbiter(corecnt))
    arb.io.ack := arb.io.req
    arb
  })

  val arbiterio = Vec(arbiters.map(e => e.io))

  override val io = Vec(corecnt,new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  for (i <- 0 until corecnt) {

    val addr = io(i).M.Addr(log2Up(lckcnt)-1+2, 2)
    val acks = Bits(width = lckcnt)
    acks := 0.U
    val blck = orR(acks)

    for (j <- 0 until lckcnt) {
      val reqReg = Reg(init = Bool(false))
      arbiterio(j).cores(i).req := reqReg
      val ackReg = Reg(next = Reg(next = arbiterio(j).cores(i).ack))
      acks(j) := ackReg =/= reqReg

      when(addr === j.U) {
        when(io(i).M.Cmd === OcpCmd.RD) {
          reqReg := Bool(true)
        }.elsewhen(io(i).M.Cmd === OcpCmd.WR) {
          reqReg := Bool(false)
        }
      }
    }

    val dvaReg = Reg(init = Bool(false))

    when(io(i).M.Cmd =/= OcpCmd.IDLE) {
      dvaReg := Bool(true)
    }.elsewhen(dvaReg === Bool(true) && !blck) {
      dvaReg := Bool(false)
    }

    io(i).S.Resp := OcpResp.NULL
    when(dvaReg === Bool(true) && !blck) {
      io(i).S.Resp := OcpResp.DVA
    }

    // Perhaps remove this
    io(i).S.Data := UInt(0)
  }
}
