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

  // Register from execution stage
  val memReg = Reg(new ExMem(), resetVal = ExMemResetVal)
  when(io.ena) {
    memReg := io.exmem
  }

  // Write data multiplexing and write enables
  // Big endian, where MSB is at the lowest address

  // default is word store
  val wrData = Vec(BYTES_PER_WORD) { Bits(width = BYTE_WIDTH) }
  for (i <- 0 until BYTES_PER_WORD) {
	wrData(i) := io.exmem.mem.data(DATA_WIDTH-i*BYTE_WIDTH-1,
								   DATA_WIDTH-i*BYTE_WIDTH-BYTE_WIDTH)
  }
  val byteEna = Bits(width = BYTES_PER_WORD)
  byteEna := Bits("b1111")  
  // half-word stores
  when(io.exmem.mem.hword) {
    switch(io.exmem.mem.addr(1)) {
      is(Bits("b0")) {
        wrData(0) := io.exmem.mem.data(2*BYTE_WIDTH-1, BYTE_WIDTH)
        wrData(1) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEna := Bits("b0011")
      }
      is(Bits("b1")) {
        wrData(2) := io.exmem.mem.data(2*BYTE_WIDTH-1, BYTE_WIDTH)
        wrData(3) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEna := Bits("b1100")
      }
    }
  }
  // byte stores
  when(io.exmem.mem.byte) {
    switch(io.exmem.mem.addr(1, 0)) {
      is(Bits("b00")) {
        wrData(0) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEna := Bits("b0001")
      }
      is(Bits("b01")) {
        wrData(1) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEna := Bits("b0010")
      }
      is(Bits("b10")) {
        wrData(2) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEna := Bits("b0100")
      }
      is(Bits("b11")) {
        wrData(3) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEna := Bits("b1000")
      }
    }
  }
  
  // Path to memories and IO is combinatorial, registering happens in
  // the individual modules
  io.localInOut.rd := Mux(io.exmem.mem.typ === MTYPE_L, io.exmem.mem.load, Bits("b0"))
  io.localInOut.wr := Mux(io.exmem.mem.typ === MTYPE_L, io.exmem.mem.store, Bits("b0"))
  io.localInOut.address := io.exmem.mem.addr
  io.localInOut.wrData := wrData
  io.localInOut.byteEna := byteEna

  io.globalInOut.rd := Mux(io.exmem.mem.typ != MTYPE_L, io.exmem.mem.load, Bits("b0"))
  io.globalInOut.wr := Mux(io.exmem.mem.typ != MTYPE_L, io.exmem.mem.store, Bits("b0"))
  io.globalInOut.address := io.exmem.mem.addr
  io.globalInOut.wrData := wrData
  io.globalInOut.byteEna := byteEna

  // Read data multiplexing and sign extensions if needed
  val rdData = Mux(memReg.mem.typ === MTYPE_L,
				   io.localInOut.rdData, io.globalInOut.rdData)

  val dout = Bits(width = DATA_WIDTH)
  // default word read
  dout := Cat(rdData(0), rdData(1), rdData(2), rdData(3))
  // byte read
  val bval = MuxLookup(memReg.mem.addr(1, 0), rdData(0), Array(
    (Bits("b00"), rdData(0)),
    (Bits("b01"), rdData(1)),
    (Bits("b10"), rdData(2)),
    (Bits("b11"), rdData(3))))
  // half-word read
  val hval = MuxLookup(memReg.mem.addr(1), Cat(rdData(0), rdData(1)), Array(
    (Bits("b0"), Cat(rdData(0), rdData(1))),
    (Bits("b1"), Cat(rdData(2), rdData(3)))))
  // sign extensions
  when(memReg.mem.byte) {
    dout := Cat(Fill(DATA_WIDTH-BYTE_WIDTH, bval(BYTE_WIDTH-1)), bval)
    when(memReg.mem.zext) {
      dout := Cat(Bits(0, DATA_WIDTH-BYTE_WIDTH), bval)
    }
  }
  when(memReg.mem.hword) {
    dout := Cat(Fill(DATA_WIDTH-2*BYTE_WIDTH, hval(DATA_WIDTH/2-1)), hval)
    when(memReg.mem.zext) {
      dout := Cat(Bits(0, DATA_WIDTH-2*BYTE_WIDTH), hval)
    }
  }
  
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
  io.memfe.callRetBase := memReg.mem.callRetBase(DATA_WIDTH-1, 2)

  // TODO: remember base address for faking return offset
  when(io.ena && io.memfe.doCallRet) {
	baseReg := memReg.mem.callRetBase
  }

  // ISPM write
  io.memfe.store := io.localInOut.wr
  io.memfe.addr := io.localInOut.address
  io.memfe.data := Cat(io.localInOut.wrData(0),
					   io.localInOut.wrData(1),
					   io.localInOut.wrData(2),
					   io.localInOut.wrData(3))

  // extra port for forwarding
  io.exResult := io.exmem.rd
}
