/*
 * Shared SPM. Later with ownership.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

/*

Questions (to Wolfgang):
  How does OcpIOBridge 'emulate' not available CmdAccep? I think it should
  register unconditional when there is a command and set it back to IDLE
  when CmdAccept is '1'. Otherwise how would the master be forced to keep
  it's signal active?
  
  I don't think merging responses of OCP slaves is legal OCP.
  What are the rules for Resp? Can a slave unconditionally drive DVA?
  I assume yes.
  
 */

package cmp

import Chisel._

import patmos._
import patmos.Constants._
import ocp._

class NodeSPM(n: Int, cnt: Int) extends Module {

  val io = new Bundle() {
    val fromCore = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val toMem = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  }

  // dummy connection, need a counter to count TDM slots,
  // register the request, and gating the master signal,
  // getting the reply back (data and DVA) to the core.
  io.toMem <> io.fromCore
  // just core 0 allowed now
  if (n != 0) {
    io.toMem.M.Addr := UInt(0)
    io.toMem.M.Data := UInt(0)
    io.toMem.M.Cmd := UInt(0)
  }

}

class SharedSPM(nrCores: Int) extends Module {

  val io = Vec(nrCores, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  val spm = Module(new Spm(1024))

  val nd = new Array[NodeSPM](nrCores)
  //   val masters = new Array[OcpCoreMasterPort](nrCores)
  val masters = Vec(nrCores, new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH))
  for (i <- 0 until nrCores) {
    nd(i) = Module(new NodeSPM(i, nrCores))
    // masters(i) = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
    nd(i).io.fromCore.M := io(i).M
    io(i).S := nd(i).io.fromCore.S
    masters(i) <> nd(i).io.toMem
  }

  def orMaster(x: OcpCoreMasterPort, y: OcpCoreMasterPort) = {
    val ret = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
    ret.M.Addr := x.M.Addr | y.M.Addr
    ret.M.Cmd := x.M.Cmd | y.M.Cmd
    ret.M.Data := x.M.Data | y.M.Data
    ret.M.ByteEn := x.M.ByteEn | y.M.ByteEn
    ret
  }

  // this or thing does not work. what is the issue?
  // spm.io.M <> masters.reduceLeft((x, y) => orMaster(x, y))

  // Torur's proposal for or reduction:
  val x = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  x.M.Addr := UInt(0)
  x.M.Cmd := UInt(0)
  x.M.Data := UInt(0)
  x.M.ByteEn := UInt(0)

  for (j <- 0 until ADDR_WIDTH) {
    x.M.Addr(j) := orR(Vec(masters.map(c => (c.M.Addr(j)))).toBits)
  }
  for (j <- 0 until DATA_WIDTH) {
    x.M.Data(j) := orR(Vec(masters.map(c => (c.M.Data(j)))).toBits)
  }
  for (j <- 0 until 4) {

    x.M.ByteEn(j) := orR(Vec(masters.map(c => (c.M.ByteEn(j)))).toBits)
  }
  for (j <- 0 until 3) {
    x.M.Cmd(j) := orR(Vec(masters.map(c => (c.M.Cmd(j)))).toBits)
  }
  spm.io.M := x.M

  // For a try simply do the no-functional or reduction
  // spm.io.M := masters(0).M
  for (i <- 0 until nrCores) {
    masters(i).S := spm.io.S
  }

  // TODO: a simple arbiter - see class Arbiter

  //  spm.io.M.Addr := 
  //  spm.io.M.Cmd := Bits(1)
  //  val xxx = spm.io.S.Data
  //  val yyy = spm.io.S.Resp

  //  for (i <- 1 to cnt - 1) {
  //    io.comConf(i).S.Data := UInt(i + 'A')
  //    // Is it legal OCP to have the response flags hard wired?
  //    // Probably yes.
  //    io.comConf(i).S.CmdAccept := Bits(1)
  //    io.comConf(i).S.Resp := Mux(io.comConf(i).M.Cmd =/= OcpCmd.IDLE,
  //      OcpResp.DVA, OcpResp.NULL)
  //  }

  // Connection between OneWay memories and OCPcore ports
  //  for (i <- 0 until nrCores) {
  //
  //    val resp = Mux(io(i).M.Cmd === OcpCmd.RD || io(i).M.Cmd === OcpCmd.WR,
  //      OcpResp.DVA, OcpResp.NULL)
  //
  //    // addresses are in words
  //    onewaymem.io.memPorts(i).rdAddr := io(i).M.Addr >> 2
  //    onewaymem.io.memPorts(i).wrAddr := io(i).M.Addr >> 2
  //    onewaymem.io.memPorts(i).wrData := io(i).M.Data
  //    onewaymem.io.memPorts(i).wrEna := io(i).M.Cmd === OcpCmd.WR
  //    // Memory has one cycle latency (read address is in register)
  //    io(i).S.Data := onewaymem.io.memPorts(i).rdData
  //    io(i).S.Resp := Reg(init = OcpResp.NULL, next = resp)
  //  }
}

