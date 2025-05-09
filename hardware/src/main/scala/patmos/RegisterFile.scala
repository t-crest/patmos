/*
 * Register file for Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import chisel3._
import chisel3.util._

import Constants._

class RegisterFile(debug: Boolean = false) extends Module {
  val io = IO(new RegFileIO())

  // Using Mem (instead of Vec) leads to smaller HW for single-issue config
  val rf = Mem(REG_COUNT, UInt(DATA_WIDTH.W))

  // We are registering the inputs here, similar as it would
  // be with an on-chip memory for the register file
  val addrReg = Reg(Vec(2*PIPE_COUNT, UInt(REG_BITS.W)))
  val wrReg   = Reg(Vec(PIPE_COUNT, new Result()))
  val fwReg   = Reg(Vec(2*PIPE_COUNT, Vec(PIPE_COUNT, Bool())))

  when (io.ena) {
    addrReg := io.rfRead.rsAddr
    wrReg := io.rfWrite
    for (i <- 0 until 2*PIPE_COUNT) {
      for (k <- 0 until PIPE_COUNT) {
        fwReg(i)(k) := io.rfRead.rsAddr(i) === io.rfWrite(k).addr && io.rfWrite(k).valid
      }
    }
  }

  // RF internal forwarding
  for (i <- 0 until 2*PIPE_COUNT) {
    io.rfRead.rsData(i) := rf(addrReg(i))
    for (k <- 0 until PIPE_COUNT) {
      when (fwReg(i)(k)) {
        io.rfRead.rsData(i) := wrReg(k).data
      }
    }
    when(addrReg(i) === 0.U) {
      io.rfRead.rsData(i) := 0.U
    }
  }

  // Don't care about R0 here: reads return zero and writes to
  // register R0 are disabled in decode stage anyway
  for (k <- (0 until PIPE_COUNT).reverse) {
    when(io.rfWrite(k).valid) {
      rf(io.rfWrite(k).addr.asUInt) := io.rfWrite(k).data
    }
  }

  
  // Signal for debugging register values
  val rfDebug =
    if(debug) {
      val _rfDebug = Wire(Vec(REG_COUNT, UInt(DATA_WIDTH.W)))
      for(i <- 0 until REG_COUNT) {
        _rfDebug(i) := rf(i.U)
      }
      Some(_rfDebug)
    } else None
}
