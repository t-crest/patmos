/*
 * A shared scratchpad memory.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 */

package cmp

import chisel3._
import chisel3.util._

import patmos._
import patmos.Constants._
import ocp._

/**

 Local arbitration with the TDM counter and a tiny FSM.
 
   * Tree to the memory is node local enabled AND and an OR for merge
   * Three state FSM: idle, read, write
   * loose one cycle by registering the command and switching state
   * local counter tells when slot is there, drives the and gate
   * DVA generated from FSM, not from memory
     * read data comes from memory one cycle after slot, dva is simply a register delay
     * write dva also delayed to be back in idle for pipelined operation, check this out with Patmos or a test case
   * memory DVA is ignored as we know the timing
   * maybe test also byte and word access, should work as in any other spm
 */
class NodeSPM(id: Int, nrCores: Int) extends Module {

  val io = IO(new Bundle() {
    val fromCore = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val toMem = new OcpCoreMasterPort(ADDR_WIDTH, DATA_WIDTH)
  })

  val idle :: rd :: wr :: Nil = Enum(3)
  val state = RegInit(idle)
  val cnt = RegInit(0.U(log2Up(nrCores).W))

  // TODO: how to reset with a harmless IDLE command?
  val masterReg = Reg(chiselTypeOf(io.fromCore.M))

  cnt := Mux(cnt === (nrCores - 1).U, 0.U, cnt + 1.U)
  val enable = cnt === id.U
  
  // this is not super nice to define the type this way
  val dvaRepl = Wire(UInt(2.W))
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
      state := idle
      masterReg.Cmd := OcpCmd.IDLE
      dvaRepl := OcpResp.DVA
    }
  }.elsewhen(state === wr) {
    when (enable) {
      state := idle
      masterReg.Cmd := OcpCmd.IDLE
      dvaRepl := OcpResp.DVA
    }
  }
  
  // Data comes from the SPM, response from the FSM
  io.fromCore.S.Resp := RegNext(dvaRepl)
  io.fromCore.S.Data := io.toMem.S.Data

  // And the master signals
  when(enable) {
    io.toMem.M := masterReg
  }.otherwise {
    // a simple way to assign all 0?
    io.toMem.M.Addr := 0.U
    io.toMem.M.Data := 0.U
    io.toMem.M.Cmd := 0.U
    io.toMem.M.ByteEn := 0.U
  }
}

class SharedSPM(nrCores: Int, size: Int) extends CmpDevice(nrCores) {

  val io = IO(new CmpIO(nrCores))

  val spm = Module(new Spm(size))

  val nd = new Array[NodeSPM](nrCores)
  for (i <- 0 until nrCores) {
    nd(i) = Module(new NodeSPM(i, nrCores))
    io.cores(i) <> nd(i).io.fromCore
    nd(i).io.toMem.S <> spm.io.S
  }

  // Or the master signals
  spm.io.M.Addr := nd.map(_.io.toMem.M.Addr).reduce((x, y) => x | y)
  spm.io.M.Data := nd.map(_.io.toMem.M.Data).reduce((x, y) => x | y)
  spm.io.M.Cmd := nd.map(_.io.toMem.M.Cmd).reduce((x, y) => x | y)
  spm.io.M.ByteEn := nd.map(_.io.toMem.M.ByteEn).reduce((x, y) => x | y)
}

