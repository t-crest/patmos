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

class SharedSPM(nrCores: Int) extends Module {

  val io = Vec(nrCores, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

  // Just use our spm, or define one it here?
  val spm = Module(new Spm(1024))
  
  io(0) <> spm.io
  
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

