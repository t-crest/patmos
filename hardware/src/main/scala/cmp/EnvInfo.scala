/*
 * "I/O" module to access information about the "environment"
 *
 * Authors: Torur Biskopsto Strom (torur.strom@gmail.com)
 */

package cmp

import chisel3._
import chisel3.util._

import patmos.Constants._
import ocp._

class EnvInfoIO(nrCores: Int) extends CmpIO(nrCores) {
  val exit = Output(Bool())
  val exitcode = Output(UInt(32.W))
}

class EnvInfo(nrCores: Int) extends CmpDevice(nrCores) {

  override val io = IO(new EnvInfoIO(nrCores))

  // Entry point and exit registers
  val entrypointReg = Reg(UInt(32.W))
  // ELF entry point (for emulator)
  val exitReg = RegInit(false.B)
  val exitcodeReg = Reg(UInt(32.W))

  // Expose the signals
  io.exit := exitReg
  io.exitcode := exitcodeReg

  for (coreNr <- (0 until nrCores).reverse) {

    val M = io.cores(coreNr).M
    val S = io.cores(coreNr).S

    // Default response is NULL
    S.Resp := OcpResp.NULL
    S.Data := 0.U

    // we always respond positively except if we exit on the emulator, where we never respond
    when(M.Cmd === OcpCmd.RD || M.Cmd === OcpCmd.WR) {
      S.Resp := OcpResp.DVA
    }

    when(M.Cmd === OcpCmd.WR) {
      switch(M.Addr(5,2)) {
        is("b0001".U) {
          entrypointReg := M.Data
        }
        is("b0010".U) {
          exitReg := true.B
          exitcodeReg := M.Data
        }
      }
    }

    when(M.Cmd === OcpCmd.RD) {
      switch(M.Addr(5,2)) {
        is("b0000".U) {
          S.Data := entrypointReg
        }
        is("b0001".U) {
          S.Data := entrypointReg
        }
      }
    }

    when(exitReg) {
      S.Resp := OcpResp.NULL
    }
  }
}


