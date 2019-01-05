

/*
 * Start for OCP tests.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp.test

import Chisel._

import ocp._

class AModule() extends Module {
  val io = IO(new Bundle {
    val fromMaster = new OcpBurstSlavePort(32, 32, 4)
    val toSlave = new OcpBurstMasterPort(32, 32, 4)
  })

  io.fromMaster <> io.toSlave
}

/*
class OcpTester(dut: AModule) extends Tester(dut) {
  val testVec = Array( OcpCmd.IDLE, OcpCmd.WR, OcpCmd.IDLE )

  for (i <- 0 until testVec.length) {
    poke(dut.io.fromMaster.M.Cmd,testVec(i))

    step(1)
  }
}*/

object OcpTester {
  def main(args: Array[String]): Unit = {
    chiselMain(args, () => Module(new AModule()))

  }
}
