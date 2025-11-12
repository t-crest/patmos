
/*
 * Arbiter for OCP burst slaves.
 * Pseudo round robin arbitration. Each turn for a non-requesting master costs 1 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp

import chisel3._
import chisel3.util._

abstract class ArbiterType(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int) extends Module {
  val io = IO(new Bundle {
    val master = Vec(cnt, new OcpBurstSlavePort(addrWidth, dataWidth, burstLen))
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  })
}
