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
 * Memory stage of Patmos.
 * 
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package patmos

import Chisel._
import Node._

import Constants._

class Memory() extends Component {
  val io = new MemoryIO()

  val memReg = Reg(new ExMem())
  when(io.ena) {
    memReg := io.exmem
  }

  // Use combinational input in regular case.
  // Replay old value on a stall.
  // This is for the on-chip memory without an enable.
  val memIn = Mux(io.ena, io.exmem.mem, memReg.mem)

  // SPM is straight forward
  val spm = new Spm(1 << DSPM_BITS)
  spm.io.in := memIn

  // IO address decode form the registered values.
  // Might be an optimization from doing it in EX.
  val selIO = memReg.mem.addr(DATA_WIDTH-1, DATA_WIDTH-4) === Bits("b1111")
  io.memInOut.rd := selIO  & memReg.mem.load & io.ena
  io.memInOut.wr := selIO  & memReg.mem.store & io.ena
  io.memInOut.address := memReg.mem.addr(11, 0)
  io.memInOut.wrData := memReg.mem.data  

  // ISPM write is handled in write
  // val selIspm = memReg.mem.addr(DATA_WIDTH-1, DATA_WIDTH-4) === Bits("b0001")

  // Read data select. For IO it is a single cycle read. No wait at the moment.
  val dout = Mux(selIO, io.memInOut.rdData, spm.io.data)

  // TODO: PC is absolute in ISPM, but we fake the return offset to
  // be relative to the base address.
  val baseReg = Reg(resetVal = UFix(0, DATA_WIDTH))

  io.memwb.pc := memReg.pc
  for (i <- 0 until PIPE_COUNT) {
	io.memwb.rd(i).addr := memReg.rd(i).addr
	io.memwb.rd(i).valid := memReg.rd(i).valid
	io.memwb.rd(i).data := memReg.rd(i).data 
  }
  // Fill in data from loads or calls
  io.memwb.rd(0).data := Mux(memReg.mem.load, dout,
							 Mux(memReg.mem.call,
								 Cat(io.femem.pc, Bits("b00")) - baseReg,
								 memReg.rd(0).data))  

  // call to fetch
  io.memfe.doCallRet := memReg.mem.call || memReg.mem.ret
  io.memfe.callRetPc := memReg.mem.callRetAddr(DATA_WIDTH-1, 2)

  // TODO: remember base address for faking return offset
  when(io.ena && io.memfe.doCallRet) {
	baseReg := memReg.mem.callRetBase
  }

  // ISPM write
  io.memfe.store := memIn.store
  io.memfe.addr := memIn.addr
  io.memfe.data := memIn.data

  // extra port for forwarding
  io.exResult := io.exmem.rd
  // debugging
  io.dbgMem := io.memInOut.rdData
}
