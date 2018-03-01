/*
 * A shared scratchpad memory.
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

/*
 * 
 * Local control:

   * Three state FSM: idle, read, write
   * loose one cycle by registering the command and switching state
   * local counter tells when slot is there, drives the and gate
   * DVA generated from FSM, not from memory, draw a quick timing diagram
     * read data comes form memory one cycle after slot, dva is simply a register delay
     * write dva also delayed to be back in idle for pipelined operation, check this out with Patmos or a test case
   * memory DVA is ignored as we know the timing, just check it in the waveform that it is ok
   * maybe test also byte and word access, should work as in any other spm
 */
class NodeSPM(id: Int, nrCores: Int) extends Module {

  val io = new Bundle() {
    val fromCore = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val toMem = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  }

  val idle :: rd :: wr :: Nil = Enum(UInt(), 3)
  val state = RegInit(idle)
  val cnt = RegInit(UInt(0, log2Up(nrCores)))

  // TODO: how to reset with a harmless IDLE command?
  val masterReg = Reg(io.fromCore.M)

  cnt := Mux(cnt === UInt(nrCores - 1), UInt(0), cnt + UInt(1))

  val enable = Bool()
  enable := cnt === UInt(id)
  
  // this is not super nice to define the type this way
  val dvaRepl = UInt(width = 2)
  dvaRepl := OcpResp.NULL

  when(state === idle) {
    when(io.fromCore.M.Cmd === OcpCmd.WR) {
      state := wr
      masterReg := io.fromCore.M
    }.elsewhen(io.fromCore.M.Cmd === OcpCmd.RD) {
      state := rd
      masterReg := io.fromCore.M
    }
  }.elsewhen(state === rd) {
    when(enable) {
      masterReg.Cmd := OcpCmd.IDLE
      dvaRepl := OcpResp.DVA
    }
  }.elsewhen(state === wr) {
      masterReg.Cmd := OcpCmd.IDLE
      dvaRepl := OcpResp.DVA
  }
  
  io.fromCore.S.Resp := RegNext(dvaRepl)

  when(enable) {
    io.toMem.M := masterReg
  }.otherwise {
    // a simple way to assign all 0?
    io.toMem.M.Addr := UInt(0)
    io.toMem.M.Data := UInt(0)
    io.toMem.M.Cmd := UInt(0)
    io.toMem.M.ByteEn := UInt(0)
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

  // Data comes from the SPM, response from the node FSM
  for (i <- 0 until nrCores) {
    masters(i).S.Data := spm.io.S.Data
    masters(i).S.Resp := nd(i).io.fromCore.S.Resp
  }
}

