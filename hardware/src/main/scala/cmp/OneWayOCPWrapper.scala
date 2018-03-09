/*
 * Copyright: 2018, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * OCP wrapper for the one-way shared memory
 */
package cmp

import Chisel._
import Node._

import patmos._
import patmos.Constants._
import ocp._
import io.CoreDeviceIO
import oneway._

class OneWayOCPWrapper(nrCores: Int) extends Module {

  val dim = math.sqrt(nrCores).toInt
  if (dim * dim != nrCores) throw new Error("Number of cores must be quadratic")
  
  // 256 words per channel (cores - 1), is 4 KB for a 2x2
  // Must be a power of 2
  val size = scala.math.pow(2, log2Up(256 * (nrCores - 1))).toInt
  println("OneWayMem memory size: " + size * 4 + " Bytes")

  val onewaymem = Module(new oneway.OneWayMem(dim, size))

  val io = Vec(nrCores, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  // Connection between OneWay memories and OCPcore ports
  for (i <- 0 until nrCores) {

    val resp = Mux(io(i).M.Cmd === OcpCmd.RD || io(i).M.Cmd === OcpCmd.WR,
      OcpResp.DVA, OcpResp.NULL)

    // addresses are in words
    onewaymem.io.memPorts(i).rdAddr := io(i).M.Addr >> 2
    onewaymem.io.memPorts(i).wrAddr := io(i).M.Addr >> 2
    onewaymem.io.memPorts(i).wrData := io(i).M.Data
    onewaymem.io.memPorts(i).wrEna := io(i).M.Cmd === OcpCmd.WR
    // Memory has one cycle latency (read address is in register)
    io(i).S.Data := onewaymem.io.memPorts(i).rdData
    io(i).S.Resp := Reg(init = OcpResp.NULL, next = resp)
  }
}
