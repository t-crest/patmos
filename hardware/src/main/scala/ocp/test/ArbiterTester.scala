
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

  val cntReg = Reg(init = 0.U(32.W))
  val dataReg = Reg(init = 0.U(32.W))
  val cntRead = Reg(init = 0.U(3.W))
  
  //debug(cntRead) does nothing in chisel3 (no proning in frontend of chisel3 anyway)

  io.port.M.Cmd := OcpCmd.IDLE
  io.port.M.DataValid := 0.U
  io.port.M.DataByteEn := 15.U

  cntReg := cntReg + 1.U
  switch(cntReg) {
    is(1.U) {
      io.port.M.Cmd := OcpCmd.WR
      io.port.M.DataValid := 1.U
      when (io.port.S.CmdAccept === 0.U) {
        cntReg := cntReg
      }
    }
    is(2.U) {
      io.port.M.DataValid := 1.U
    }
    is(3.U) {
      io.port.M.DataValid := 1.U
    }
    // now we should be on our last word - wait for DVA
    is(4.U) {
      io.port.M.DataValid := 1.U
      when (io.port.S.Resp =/= OcpResp.DVA) {
        cntReg := cntReg
      }
      dataReg := io.port.S.Data
    }
    is(5.U) { io.port.M.Cmd := OcpCmd.IDLE }
    is(6.U) {
      io.port.M.Cmd := OcpCmd.RD
      when (io.port.S.CmdAccept === 0.U) {
        cntReg := cntReg
      }
    }
    is(7.U) {
      when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + 1.U
      }
    }
    is(8.U) {
      when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + 1.U
      }
    }
    is(9.U) {
       when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + 1.U
      }
    }
    is(10.U){
       when (io.port.S.Resp === OcpResp.DVA) {
        cntRead := cntRead + 1.U
      }
    }
  }

  io.port.M.Addr := ((nr * 256).U + cntReg)
//  io.port.M.Addr := (dataReg).toBits()
  io.port.M.Data := ((nr * 256 * 16).U + cntReg)
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
