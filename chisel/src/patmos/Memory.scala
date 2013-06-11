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

class SpmIO extends Bundle() {
  val in = new MemIn().asInput
  val data = Bits(OUTPUT, DATA_WIDTH)
}

/*
 * A on-chip memory.
 * 
 * Has input registers (without enable or reset).
 * Shall do byte enable.
 * Output multiplexing and bit filling at the moment also here.
 * That might move out again when more than one memory is involved.
 * 
 * Address decoding here. At the moment map to 0x00000000.
 * Only take care on a write.
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
  val bw = Vec(BYTES_PER_WORD) { Bits(width = BYTE_WIDTH) }
  for (i <- 0 until BYTES_PER_WORD) {
	bw(i) := io.in.data(DATA_WIDTH-i*BYTE_WIDTH-1, DATA_WIDTH-i*BYTE_WIDTH-BYTE_WIDTH)
  }

  val stmsk = Bits(width = BYTES_PER_WORD)
  stmsk := Bits("b1111")
  
//  val select = ((io.in.addr(DATA_WIDTH-1, SPM_MAX_BITS) === Bits(0x0))
//				& (io.in.addr(SPM_MAX_BITS-1, DSPM_BITS) === Bits(0x0)))
  // Everything that is not ISPM is DSMP write
  val select = (io.in.addr(ISPM_ONE_BIT) === Bits(0x0))

  // Input multiplexing and write enables
  when(io.in.hword) {
    switch(io.in.addr(1)) {
      is(Bits("b0")) {
        bw(0) := io.in.data(2*BYTE_WIDTH-1, BYTE_WIDTH)
        bw(1) := io.in.data(BYTE_WIDTH-1, 0)
        stmsk := Bits("b0011")
      }
      is(Bits("b1")) {
        bw(2) := io.in.data(2*BYTE_WIDTH-1, BYTE_WIDTH)
        bw(3) := io.in.data(BYTE_WIDTH-1, 0)
        stmsk := Bits("b1100")
      }
    }
  }

  when(io.in.byte) {
    switch(io.in.addr(1, 0)) {
      is(Bits("b00")) {
        bw(0) := io.in.data(BYTE_WIDTH-1, 0)
        stmsk := Bits("b0001")
      }
      is(Bits("b01")) {
        bw(1) := io.in.data(BYTE_WIDTH-1, 0)
        stmsk := Bits("b0010")
      }
      is(Bits("b10")) {
        bw(2) := io.in.data(BYTE_WIDTH-1, 0)
        stmsk := Bits("b0100")
      }
      is(Bits("b11")) {
        bw(3) := io.in.data(BYTE_WIDTH-1, 0)
        stmsk := Bits("b1000")
      }
    }
  }

  when(!(io.in.store & select)) {
    stmsk := Bits("b0000")
  }
  // now unconditional registers for write data and enable
  val bwReg = Reg(bw)
  val stmskReg = Reg(stmsk)

  // SPM
  // I would like to have a vector of memories.
  // val mem = Vec(4) { Mem(size, seqRead = true) { Bits(width = DATA_WIDTH) } }

  val addrBits = log2Up(size / BYTES_PER_WORD)

  // ok, the dumb way
  val mem0 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  val mem1 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  val mem2 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  val mem3 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }

  // store
  when(stmskReg(0)) { mem0(memInReg.addr(addrBits + 1, 2)) := bwReg(0) }
  when(stmskReg(1)) { mem1(memInReg.addr(addrBits + 1, 2)) := bwReg(1) }
  when(stmskReg(2)) { mem2(memInReg.addr(addrBits + 1, 2)) := bwReg(2) }
  when(stmskReg(3)) { mem3(memInReg.addr(addrBits + 1, 2)) := bwReg(3) }

  // load
  val br = Vec(BYTES_PER_WORD) { Bits(width = BYTE_WIDTH) }
  br(0) := mem0(memInReg.addr(addrBits + 1, 2))
  br(1) := mem1(memInReg.addr(addrBits + 1, 2))
  br(2) := mem2(memInReg.addr(addrBits + 1, 2))
  br(3) := mem3(memInReg.addr(addrBits + 1, 2))

  val dout = Bits(width = DATA_WIDTH)
  // default word read
  dout := Cat(br(0), br(1), br(2), br(3))

  // Output multiplexing and sign extensions if needed
  val bval = MuxLookup(memInReg.addr(1, 0), br(0), Array(
    (Bits("b00"), br(0)),
    (Bits("b01"), br(1)),
    (Bits("b10"), br(2)),
    (Bits("b11"), br(3))))

  val hval = MuxLookup(memInReg.addr(1), Cat(br(0), br(1)), Array(
    (Bits("b00"), Cat(br(0), br(1))),
    (Bits("b01"), Cat(br(2), br(3)))))

  when(memInReg.byte) {
    dout := Cat(Fill(DATA_WIDTH-BYTE_WIDTH, bval(BYTE_WIDTH-1)), bval)
    when(memInReg.zext) {
      dout := Cat(Bits(0, DATA_WIDTH-BYTE_WIDTH), bval)
    }
  }
  when(memInReg.hword) {
    dout := Cat(Fill(DATA_WIDTH-2*BYTE_WIDTH, hval(DATA_WIDTH/2-1)), hval)
    when(memInReg.zext) {
      dout := Cat(Bits(0, DATA_WIDTH-2*BYTE_WIDTH), hval)
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

  // extra port for forwarding the registered value
  io.exResult := memReg.rd
  // debugging
  io.dbgMem := io.memInOut.rdData
}
