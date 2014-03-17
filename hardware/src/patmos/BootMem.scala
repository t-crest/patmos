/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Boot data memory (ROM and SPM) for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

import Constants._

import ocp._

class BootMem(fileName : String) extends Module {
  val io = new BootMemIO()

  // Compute selects
  val selExt = io.memInOut.M.Addr(ADDR_WIDTH-1, ADDR_WIDTH-1) === Bits("b0")
  val selRom = !selExt & io.memInOut.M.Addr(BOOTMEM_ONE_BIT) === Bits(0x0)
  val selSpm = !selExt & io.memInOut.M.Addr(BOOTMEM_ONE_BIT) === Bits(0x1)

  // Register selects
  val selRomReg = Reg(init = Bool(false))
  val selSpmReg = Reg(init = Bool(false))
  when(io.memInOut.M.Cmd != OcpCmd.IDLE) {
      selRomReg := selRom
      selSpmReg := selSpm
  }

  // The data ROM for read only initialized data
  val rom = Utility.readBin(fileName, DATA_WIDTH)
  val romCmdReg = Reg(next = Mux(selRom, io.memInOut.M.Cmd, OcpCmd.IDLE))
  val romAddr = Reg(next = io.memInOut.M.Addr)
  val romResp = Mux(romCmdReg === OcpCmd.IDLE, OcpResp.NULL, OcpResp.DVA)
  val romData = rom(romAddr(log2Up(rom.length)+1, 2))

  // The SPM - used for stack of bootables, can be used for initialized read/write data
  val spm = Module(new Spm(BOOTSPM_SIZE))
  spm.io.M := io.memInOut.M
  spm.io.M.Cmd := Mux(selSpm, io.memInOut.M.Cmd, OcpCmd.IDLE)
  val spmS = spm.io.S

  // Connect to external memory
  io.extMem.M := io.memInOut.M
  io.extMem.M.Cmd := Mux(selExt, io.memInOut.M.Cmd, OcpCmd.IDLE)

  // Return data to pipeline
  io.memInOut.S.Data := Mux(selRomReg, romData,
                            Mux(selSpmReg, spmS.Data,
                                io.extMem.S.Data))
  io.memInOut.S.Resp := romResp | spmS.Resp | io.extMem.S.Resp
}
