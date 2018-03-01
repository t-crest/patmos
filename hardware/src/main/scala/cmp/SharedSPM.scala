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
  val masters = new Array[OcpCoreMasterPort](nrCores)
  for (i <- 0 until nrCores) {
    nd(i) = Module(new NodeSPM(i, nrCores))
    masters(i) = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
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

  spm.io.M.Addr := masters.map(_.M.Addr).reduce(_ | _)
  spm.io.M.Data := masters.map(_.M.Data).reduce(_ | _)
  spm.io.M.Cmd := masters.map(_.M.Cmd).reduce(_ | _)
  spm.io.M.ByteEn := masters.map(_.M.ByteEn).reduce(_ | _)

  // For a simple try just return the slave signals (not correct)
  for (i <- 0 until nrCores) {
    masters(i).S := spm.io.S
  }
}

