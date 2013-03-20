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
 * 
 * Size is in bytes.
 */
class Spm(size: Int) extends Component {
  val io = new SpmIO()

  // Unconditional registers for the on-chip memory
  // All stall/enable handling has been done in the input with a MUX
  val memInReg = Reg(io.in)

  // Big endian, where MSB is at the lowest address
  // default is word store
  val bw = Vec(4) { Bits() }
  bw(0) := io.in.data(31, 24)
  bw(1) := io.in.data(23, 16)
  bw(2) := io.in.data(15, 8)
  bw(3) := io.in.data(7, 0)
  val stmsk = Bits()
  stmsk := Bits("b1111")

  // Input multiplexing and write enables
  when(io.in.hword) {
    switch(io.in.addr(1)) {
      is(Bits("b0")) {
        bw(0) := io.in.data(15, 8)
        bw(1) := io.in.data(7, 0)
        stmsk := Bits("b0011")
      }
      is(Bits("b1")) {
        bw(2) := io.in.data(15, 8)
        bw(3) := io.in.data(7, 0)
        stmsk := Bits("b1100")
      }
    }
  }

  when(io.in.byte) {
    switch(io.in.addr(1, 0)) {
      is(Bits("b00")) {
        bw(0) := io.in.data(7, 0)
        stmsk := Bits("b0001")
      }
      is(Bits("b01")) {
        bw(1) := io.in.data(7, 0)
        stmsk := Bits("b0010")
      }
      is(Bits("b10")) {
        bw(2) := io.in.data(7, 0)
        stmsk := Bits("b0100")
      }
      is(Bits("b11")) {
        bw(3) := io.in.data(7, 0)
        stmsk := Bits("b1000")
      }
    }
  }

  when(!io.in.store) {
    stmsk := Bits(0)
  }
  // now unconditional registers for write data and enable
  val bw0Reg = Reg(bw(0))
  val bw1Reg = Reg(bw(1))
  val bw2Reg = Reg(bw(2))
  val bw3Reg = Reg(bw(3))
  val stmskReg = Reg(stmsk)

  // SPM
  // I would like to have a vector of memories.
  // val mem = Vec(4) { Mem(size, seqRead = true) { Bits(width = 32) } }

  val addrBits = log2Up(size / 4)

  // ok, the dumb way
  val mem0 = { Mem(size / 4, seqRead = true) { Bits(width = 8) } }
  val mem1 = { Mem(size / 4, seqRead = true) { Bits(width = 8) } }
  val mem2 = { Mem(size / 4, seqRead = true) { Bits(width = 8) } }
  val mem3 = { Mem(size / 4, seqRead = true) { Bits(width = 8) } }

  // store
  when(stmskReg(0)) { mem0(memInReg.addr(addrBits + 1, 2)) := bw0Reg }
  when(stmskReg(1)) { mem1(memInReg.addr(addrBits + 1, 2)) := bw1Reg }
  when(stmskReg(2)) { mem2(memInReg.addr(addrBits + 1, 2)) := bw2Reg }
  when(stmskReg(3)) { mem3(memInReg.addr(addrBits + 1, 2)) := bw3Reg }

  // load
  val br0 = mem0(memInReg.addr(addrBits + 1, 2))
  val br1 = mem1(memInReg.addr(addrBits + 1, 2))
  val br2 = mem2(memInReg.addr(addrBits + 1, 2))
  val br3 = mem3(memInReg.addr(addrBits + 1, 2))

  val dout = Bits()
  // default word read
  dout := Cat(br0, br1, br2, br3)

  // Output multiplexing and sign extensions if needed
  val bval = MuxLookup(memInReg.addr(1, 0), br0, Array(
    (Bits("b00"), br0),
    (Bits("b01"), br1),
    (Bits("b10"), br2),
    (Bits("b11"), br3)))

  val hval = MuxLookup(memInReg.addr(1), Cat(br0, br1), Array(
    (Bits("b00"), Cat(br0, br1)),
    (Bits("b01"), Cat(br2, br3))))

  when(memInReg.byte) {
    dout := Cat(Fill(24, bval(7)), bval)
    when(memInReg.zext) {
      dout := Cat(Bits(0, 24), bval)
    }
  }
  when(memInReg.hword) {
    dout := Cat(Fill(16, hval(15)), hval)
    when(memInReg.zext) {
      dout := Cat(Bits(0, 16), hval)
    }
  }

  io.data := dout
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
  val extMem = memIn.addr(31, 28) === Bits("b1111")
  // TODO: this should also be in the stall logic
  // mmh, or is the memIn already fine?
  // Issue on replaying load/store on IO devices with side effects
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