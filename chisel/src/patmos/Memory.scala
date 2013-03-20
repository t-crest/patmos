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
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._

class SpmIO extends Bundle() {
  val in = new MemIn().asInput
  val data = Bits(OUTPUT, 32)
}

/*
 * A on-chip memory.
 * 
 * Has input registers (without enable or reset).
 * Shall do byte enable.
 * Output mulitplexing and bit filling at the moment also here.
 * That might move out again when more than one memory is involved.
 */
class Spm(size : Int) extends Component {
  val io = new SpmIO()
  
    // now the unconditional registers for the on-chip memory
  val memInReg = Reg(io.in)

  // Manual would like an output register here???
  // val dout = Reg() { Bits() }
  // However, with registers at the input it looks ok
  // With the additional output register we get an additional wait cycle

  // SPM
  val mem = Mem(1024, seqRead = true) { Bits(width = 32) }
  io.data := mem(memInReg.addr)
  when (memInReg.store) { mem(memInReg.addr) := memInReg.data }

}

/**
 * The memory stage.
 */
class Memory() extends Component {
  val io = new MemoryIO()

  val memReg = Reg(new ExMem())
  when(io.ena) {
    memReg := io.exmem
  }
  
  // Use combinational input in regular case.
  // Replay old value on a stall.
  val memIn = Mux(io.ena, io.exmem.mem, memReg.mem)
  
  // Use a Bundle for the memory signals
  // Some primary decoding here - it is done from the
  // unregistered values. Therefore, practically in EX
  // to have write enable a real register
  // breaks the current blinking LED
  // TODO: check (and write into TR) our address map
  val extMem = memIn.addr(31, 28) != Bits("b0000")
  val extWrReg = Reg(extMem & memIn.store)
  val extWrDataReg = Reg(memIn.data)

  
  // TODO: address decoding for a store between SPM and IO
  // add a write enable
  
  val spm = new Spm(1024)
  spm.io.in := memIn
  val dout = spm.io.data


  // connection of external IO, memory, NoC,...
  io.memBus.wr := extWrReg
  io.memBus.dataOut := extWrDataReg

  io.memwb.pc := memReg.pc
  io.memwb.rd.addr := memReg.rd.addr
  io.memwb.rd.valid := memReg.rd.valid // || memReg.mem.load
  io.memwb.rd.data := Mux(memReg.mem.load, dout, memReg.rd.data)
  // extra port for forwarding the registered value
  io.exResult := memReg.rd
  // debugging
  io.dbgMem := dout
}