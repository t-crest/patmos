/*
 * Start for OCP arbiter tests.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp.test

import chisel3._
import ocp._
import io.SSRam32Ctrl


/*class Master(nr: Int, burstLength: Int) extends Module {

  val io = new Bundle {
    val port = new OcpBurstMasterPort(32, 32, burstLength)
  }

  val cntReg = Reg(init = 0.U(8.W))

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
      when (io.port.S.Resp != OcpResp.DVA) {
        cntReg := cntReg
      }
    }
    is(5.U) { io.port.M.Cmd := OcpCmd.IDLE }
    is(6.U) { io.port.M.Cmd := OcpCmd.RD }
  }

  io.port.M.Addr := ((nr * 256).U + cntReg).toBits()
  io.port.M.Data := ((nr * 256 * 16).U + cntReg).toBits()
} */

/** A top level to test the arbiter */
class NodeTdmArbiterTop() extends Module {

  val io = IO(new Bundle {
    val port = VecInit(Seq.fill(3)(new OcpBurstMasterPort(32, 32, 4))) //TODO: this should pronanly a plain Scala Seq, or maybe not
  })
  val CNT = 3
  //val arb = Module(new ocp.NodeTdmArbiter(CNT, 32, 32, 4))
  val mem = Module(new SSRam32Ctrl(21))
  val memMux = Module(new MemMuxIntf(3, 32, 32, 4))

  for (i <- 0 until CNT) {
    val m = Module(new Master(i, 4))
    val nodeID = i.U(6.W)
    val arb = Module(new ocp.NodeTdmArbiter(CNT, 32, 32, 4, 16))
    arb.io.master <> m.io.port
    arb.io.node := nodeID

    memMux.io.master(i) <> arb.io.slave
    io.port(i).M <> memMux.io.slave.M
  }

  mem.io.ocp <> memMux.io.slave

}

/*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class NodeTdmArbiterTester(dut: ocp.test.NodeTdmArbiterTop) extends Tester(dut) {
  val testVec = Array( OcpCmd.IDLE, OcpCmd.WR, OcpCmd.IDLE )

  for (i <- 0 until 35) {
    step(1)
  }
}

object NodeTdmArbiterTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => Module(new ocp.test.NodeTdmArbiterTop)) {
      f => new NodeTdmArbiterTester(f)
    }

  }
}*/
