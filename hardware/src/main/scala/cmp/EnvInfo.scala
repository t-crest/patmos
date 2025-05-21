/*
 * "I/O" module to access information about the "environment"
 *
 * Authors: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */

package cmp

import chisel3._
import chisel3.util._

import patmos.Constants._
import ocp._

class EnvInfo(coreCnt: Int) extends CmpDevice(coreCnt) {

  val io = IO(new CmpIO(coreCnt))
  
  val masterRegs =  RegNext(VecInit(io.cores.map(e => e.M)))
  
  val platform = dontTouch(WireInit(0.U(DATA_WIDTH.W)))
  val entrypoint = dontTouch(WireInit(0.U(DATA_WIDTH.W))) // ELF entry point (for emulator)
  val exitReg = dontTouch(RegInit(false.B))
  val exitcodeReg = dontTouch(Reg(UInt(DATA_WIDTH.W)))
  
  for (coreNr <- (0 until coreCnt).reverse) {
    
    val S = io.cores(coreNr).S
    val MReg = masterRegs(coreNr)
    
    S.Resp := OcpResp.NULL
    // we always respond positively except if we exit on the emulator, where we never respond
    when(MReg.Cmd === OcpCmd.RD || MReg.Cmd === OcpCmd.WR) {
      S.Resp := OcpResp.DVA
    }
    
    when(MReg.Cmd === OcpCmd.WR) {
      switch(MReg.Addr(5,2)) {
        is("b0010".U) { exitReg := MReg.Data(0) }
        is("b0011".U) { exitcodeReg := MReg.Data }
      }
    }
    
    // platform type is returned by default
    S.Data := platform
    switch(MReg.Addr(5,2)) {
      is("b0001".U) { S.Data := entrypoint }
      is("b0010".U) { S.Data := 0.U((DATA_WIDTH - 1).W) ## exitReg }
      is("b0011".U) { S.Data := exitcodeReg }
    }
  }
}
