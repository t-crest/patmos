/*
 * Start with a tester for an (async.) SRAM controller.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package io.test

import Chisel._
import ocp._
import io._

//class Master(nr: Int, burstLength: Int) extends Module {
//
//  val io = new Bundle {
//    val port = new OcpBurstMasterPort(32, 32, burstLength)
//  }
//
//  val cntReg = Reg(init = UInt(0, width=8))
//
//  io.port.M.Cmd := OcpCmd.IDLE
//  io.port.M.DataValid := Bits(0)
//  io.port.M.DataByteEn := Bits(15)
//
//  cntReg := cntReg + UInt(1)
//  switch(cntReg) {
//    is(UInt(1)) {
//      io.port.M.Cmd := OcpCmd.WR
//      io.port.M.DataValid := Bits(1)
//      when (io.port.S.CmdAccept === Bits(0)) {
//        cntReg := cntReg
//      }
//    }
//    is(UInt(2)) {
//      io.port.M.DataValid := Bits(1)
//    }
//    is(UInt(3)) {
//      io.port.M.DataValid := Bits(1)
//    }
//    // now we should be on our last word - wait for DVA
//    is(UInt(4)) {
//      io.port.M.DataValid := Bits(1)
//      when (io.port.S.Resp != OcpResp.DVA) {
//        cntReg := cntReg
//      }
//    }
//    is(UInt(5)) { io.port.M.Cmd := OcpCmd.IDLE }
//    is(UInt(6)) { io.port.M.Cmd := OcpCmd.RD }
//  }
//
//  io.port.M.Addr := (UInt(nr * 256) + cntReg).toBits()
//  io.port.M.Data := (UInt(nr * 256 * 16) + cntReg).toBits()
//}

/** A top level to test the arbiter */
class SRamCtrlTop() extends Module {

  val io = new Bundle {
    //val port = new OcpBurstMasterPort(32, 32, 4)
    val addr = Bits(OUTPUT, width=32)
  }

  val mem = Module(new SRamCtrl(21))
  val master = Module(new ocp.test.Master(0, 4))
  mem.io.ocp <> master.io.port
  io.addr := mem.io.sRamCtrlPins.ramOut.addr
  //io.port.M <> master.io.port.M
}


class SRamCtrlTester(dut: io.test.SRamCtrlTop) extends Tester(dut) {

  val testVec = Array( OcpCmd.IDLE, OcpCmd.WR, OcpCmd.IDLE )

  for (i <- 0 until 25) {
    step(1)
  }
}

object SRamCtrlTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => Module(new io.test.SRamCtrlTop)) {
      f => new SRamCtrlTester(f)
    }
  }
}
