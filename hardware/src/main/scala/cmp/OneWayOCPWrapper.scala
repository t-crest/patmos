/*
 * Copyright: 2018, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * OCP wrapper
 */
package cmp

import Chisel._
import Node._

import patmos._
import patmos.Constants._
import ocp._
import io.CoreDeviceIO
import oneway._

class XXXIO(lckCnt: Int) extends Bundle {
  val sel = UInt(INPUT, log2Up(lckCnt))
  val op = Bool(INPUT)
  val en = Bool(INPUT)
  val blck = Bool(OUTPUT)
}

// Maybe I should also use this functional approach, but not for a quick test now
// class OneWayOCPWrapper(hardlockgen: () => AbstractHardlock) extends Module {

class OneWayOCPWrapper(nrCores: Int) extends Module {

  val dim = math.sqrt(nrCores).toInt
  if (dim * dim != nrCores) throw new Error("Number of cores must be quadratic")
  // just start with four words
  val size = 4 * nrCores
  val onewaymem = Module(new oneway.OneWayMem(dim, size))

  println("OneWayMem")

  val io = Vec.fill(nrCores) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) }

  // Mapping between OneWay memories and OCP

  for (i <- 0 until nrCores) {
    val respReg = Reg(init = OcpResp.NULL)
    respReg := OcpResp.NULL
    when(io(i).M.Cmd === OcpCmd.RD || io(i).M.Cmd === OcpCmd.WR) {
      respReg := OcpResp.DVA
    }
    
    onewaymem.io.memPorts(i).rdAddr := io(i).M.Addr
    onewaymem.io.memPorts(i).wrAddr := io(i).M.Addr
    onewaymem.io.memPorts(i).wrData := io(i).M.Data
    onewaymem.io.memPorts(i).wrEna := io(i).M.Cmd === OcpCmd.WR
    io(i).S.Data := onewaymem.io.memPorts(i).rdData
    
    io(i).S.Resp := respReg
  }
}
