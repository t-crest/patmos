/*
 * Writeback stage of Patmos.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package patmos

import Chisel._
import Node._

import Constants._

class WriteBack() extends Module {
  val io = IO(new WriteBackIO())

  val wbReg = Reg(new MemWb())
  when (io.ena) {
    wbReg := io.memwb
  }

  // The register file has input registers
  io.rfWrite <> io.memwb.rd
  // extra port for forwarding
  io.memResult := io.memwb.rd
}
