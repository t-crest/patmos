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

  
  val reqReg = Reg(init = Bits(0,corecnt))
  val dvaReg = Reg(init = Bits(0,corecnt))


  for (i <- 0 until corecnt) {

    val reqReg = Reg(init = Bool(false))
    val addr = io(i).M.Addr(log2Up(lckcnt)-1+2, 2)
    val addrReg = Reg(addr)
    val ack1Reg = Reg(next = arbiterio(addrReg).cores(i).ack)
    val ack2Reg = Reg(next = ack1Reg)
    val dvaReg = Reg(init = Bool(false))

    for (j <- 0 until lckcnt) {
      when(addrReg === j.U) {
        arbiterio(j).cores(i).req := reqReg
      }.otherwise {
        arbiterio(j).cores(i).req := Bool(false)
      }
    }

    //arbiterio(addrReg).cores(i).req := reqReg

    when(io(i).M.Cmd === OcpCmd.RD) {
      reqReg := Bool(true)
      ack1Reg := Bool(false)
      ack2Reg := Bool(false)
      addrReg := addr
      dvaReg := Bool(true)
    }.elsewhen(io(i).M.Cmd === OcpCmd.WR) {
      reqReg := Bool(false)
      ack1Reg := Bool(true)
      ack2Reg := Bool(true)
      addrReg := addr
      dvaReg := Bool(true)
    }.elsewhen(dvaReg === Bool(true) && reqReg === ack2Reg) {
      dvaReg := Bool(false)
    }

    io(i).S.Resp := OcpResp.NULL
    when(dvaReg === Bool(true) && reqReg === ack2Reg) {
      io(i).S.Resp := OcpResp.DVA
    }


    //io(i).S.Data := UInt(0)
  }
}
