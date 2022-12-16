/*
  OCP wrapper for the S4NOC.

  Author: Martin Schoeberl (martin@jopdesign.com)
  license see LICENSE
 */
package cmp

import Chisel._

import patmos.Constants._
import ocp._
import s4noc._

class S4nocOCPWrapper(nrCores: Int, txFifo: Int, rxFifo: Int) extends CmpDevice(nrCores) {

  val s4noc = Module(new S4noc(nrCores, txFifo, rxFifo))

  for (i <- 0 until nrCores) {

    val resp = Mux(io.cores(i).M.Cmd === OcpCmd.RD || io.cores(i).M.Cmd === OcpCmd.WR,
      OcpResp.DVA, OcpResp.NULL)

    // addresses are in words
    s4noc.io.cpuPorts(i).addr := io.cores(i).M.Addr >> 2
    s4noc.io.cpuPorts(i).wrData := io.cores(i).M.Data
    s4noc.io.cpuPorts(i).wr := io.cores(i).M.Cmd === OcpCmd.WR
    s4noc.io.cpuPorts(i).rd := io.cores(i).M.Cmd === OcpCmd.RD
    io.cores(i).S.Data := RegNext(s4noc.io.cpuPorts(i).rdData)
    io.cores(i).S.Resp := Reg(init = OcpResp.NULL, next = resp)
  }
}
