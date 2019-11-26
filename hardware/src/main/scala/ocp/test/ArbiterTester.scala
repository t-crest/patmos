
/*
 * Start for OCP arbiter tests.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp.test

import Chisel._
import ocp._
import io.SSRam32Ctrl


class Master(nr: Int, burstLength: Int) extends Module {

  val io = IO(new Bundle {
    val port = new OcpBurstMasterPort(32, 32, burstLength)
  })

  val cntReg = Reg(init = UInt(0, width=32))
  val dataReg = Reg(init = UInt(0, width=32))
  val cntRead = Reg(init = UInt(0, width=3))
  debug(cntRead)

  io.port.M.Cmd := OcpCmd.IDLE
  io.port.M.DataValid := Bits(0)
  io.port.M.DataByteEn := Bits(15)

  cntReg := cntReg + UInt(1)
  switch(cntReg) {
    is(UInt(1)) {
      io.port.M.Cmd := OcpCmd.WR
      io.port.M.DataValid := Bits(1)
      when (io.port.S.CmdAccept === Bits(0)) {
        cntReg := cntReg
      }
    }
    is(UInt(2)) {
      io.port.M.DataValid := Bits(1)
    }
    is(UInt(3)) {
      io.port.M.DataValid := Bits(1)
    }
    // now we should be on our last word - wait for DVA
    is(UInt(4)) {
      io.port.M.DataValid := Bits(1)
      when (io.port.S.Resp =/= OcpResp.DVA) {
        cntReg := cntReg
      }
      dataReg := io.port.S.Data
    }
    is(UInt(5)) { io.port.M.Cmd := OcpCmd.IDLE }
    is(UInt(6)) {
      io.port.M.Cmd := OcpCmd.RD
      when (io.port.S.CmdAccept === Bits(0)) {
        cntReg := cntReg
      }
    }
    is(UInt(7)) {
      when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + UInt(1)
      }
    }
    is(UInt(8)) {
      when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + UInt(1)
      }
    }
    is(UInt(9)) {
       when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + UInt(1)
      }
    }
    is(UInt(10)){
       when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + UInt(1)
      }
    }
  }

  io.port.M.Addr := (UInt(nr * 256) + cntReg)
//  io.port.M.Addr := (dataReg).toBits()
  io.port.M.Data := (UInt(nr * 256 * 16) + cntReg)
}

/** A top level to test the arbiter */
class ArbiterTop() extends Module {

  val io = IO(new Bundle {
    val port = new OcpBurstMasterPort(32, 32, 4)
  })
  val CNT = 3
  val arb = Module(new ocp.Arbiter(CNT, 32, 32, 4))
  val mem = Module(new SSRam32Ctrl(21))

  for (i <- 0 until CNT) {
    val m = Module(new Master(i, 4))
    arb.io.master(i) <> m.io.port
  }

  mem.io.ocp <> arb.io.slave

  io.port.M <> arb.io.slave.M

}

/*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class ArbiterTester(dut: ocp.test.ArbiterTop) extends Tester(dut) {
  val testVec = Array( OcpCmd.IDLE, OcpCmd.WR, OcpCmd.IDLE )

  for (i <- 0 until 30) {
    step(1)
  }
}

object ArbiterTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => Module(new ocp.test.ArbiterTop)) {
      f => new ArbiterTester(f)
    }

  }*/
