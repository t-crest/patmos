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

import ocp._

class Memory() extends Component {
  val io = new MemoryIO()

  // Register from execution stage
  val memReg = Reg(resetVal = ExMem.resetVal)

  // React on error responses
  val illMem = (io.localInOut.S.Resp === OcpResp.ERR ||
                io.globalInOut.S.Resp === OcpResp.ERR)

  // Flush logic
  val flush = (memReg.mem.xcall || memReg.mem.trap || memReg.mem.illOp || illMem)
  io.flush := flush

  // Stall logic
  val stall = Reg(Bool(), resetVal = Bool(false))
  val enable = Mux(stall, (io.localInOut.S.Resp != OcpResp.NULL
						   || io.globalInOut.S.Resp != OcpResp.NULL),
				   Bool(true))
  stall := (io.exmem.mem.load || io.exmem.mem.store) && !flush
  io.ena := enable

  // Latch register
  when(enable) {
    memReg := io.exmem
    when(flush) {
      memReg.reset()
    }
  }

  // Write data multiplexing and write enables
  // Big endian, where MSB is at the lowest address

  // default is word store
  val wrData = Vec(BYTES_PER_WORD) { Bits(width = BYTE_WIDTH) }
  for (i <- 0 until BYTES_PER_WORD) {
	wrData(i) := io.exmem.mem.data((i+1)*BYTE_WIDTH-1, i*BYTE_WIDTH)
  }
  val byteEn = Bits(width = BYTES_PER_WORD)
  byteEn := Bits("b1111")  
  // half-word stores
  when(io.exmem.mem.hword) {
    switch(io.exmem.mem.addr(1)) {
      is(Bits("b0")) {
        wrData(2) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        wrData(3) := io.exmem.mem.data(2*BYTE_WIDTH-1, BYTE_WIDTH)
        byteEn := Bits("b1100")
      }
      is(Bits("b1")) {
        wrData(0) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        wrData(1) := io.exmem.mem.data(2*BYTE_WIDTH-1, BYTE_WIDTH)
        byteEn := Bits("b0011")
      }
    }
  }
  // byte stores
  when(io.exmem.mem.byte) {
    switch(io.exmem.mem.addr(1, 0)) {
      is(Bits("b00")) {
        wrData(3) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEn := Bits("b1000")
      }
      is(Bits("b01")) {
        wrData(2) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEn := Bits("b0100")
      }
      is(Bits("b10")) {
        wrData(1) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEn := Bits("b0010")
      }
      is(Bits("b11")) {
        wrData(0) := io.exmem.mem.data(BYTE_WIDTH-1, 0)
        byteEn := Bits("b0001")
      }
    }
  }
  
  // Path to memories and IO is combinatorial, registering happens in
  // the individual modules
  val cmd = Mux(enable && !flush,
				Mux(io.exmem.mem.load, OcpCmd.RD,
					Mux(io.exmem.mem.store, OcpCmd.WRNP,
						OcpCmd.IDLE)),
				OcpCmd.IDLE)

  io.localInOut.M.Cmd := Mux(io.exmem.mem.typ === MTYPE_L, cmd, OcpCmd.IDLE)
  io.localInOut.M.Addr := Cat(io.exmem.mem.addr(ADDR_WIDTH-1, 2), Bits("b00"))
  io.localInOut.M.Data := Cat(wrData(3), wrData(2), wrData(1), wrData(0))
  io.localInOut.M.ByteEn := byteEn

  io.globalInOut.M.Cmd := Mux(io.exmem.mem.typ != MTYPE_L, cmd, OcpCmd.IDLE)
  io.globalInOut.M.Addr := Cat(io.exmem.mem.addr(ADDR_WIDTH-1, 2), Bits("b00"))
  io.globalInOut.M.Data := Cat(wrData(3), wrData(2), wrData(1), wrData(0))
  io.globalInOut.M.ByteEn := byteEn

  def splitData(word: Bits) = {
	val retval = Vec(BYTES_PER_WORD) { Bits(width = BYTE_WIDTH) }
	for (i <- 0 until BYTES_PER_WORD) {
	  retval(i) := word((i+1)*BYTE_WIDTH-1, i*BYTE_WIDTH)
	}
	retval
  }

  // Read data multiplexing and sign extensions if needed
  val rdData = splitData(Mux(memReg.mem.typ === MTYPE_L,
							 io.localInOut.S.Data, io.globalInOut.S.Data))

  val dout = Bits(width = DATA_WIDTH)
  // default word read
  dout := Cat(rdData(3), rdData(2), rdData(1), rdData(0))
  // byte read
  val bval = MuxLookup(memReg.mem.addr(1, 0), rdData(0), Array(
    (Bits("b00"), rdData(3)),
    (Bits("b01"), rdData(2)),
    (Bits("b10"), rdData(1)),
    (Bits("b11"), rdData(0))))
  // half-word read
  val hval = MuxLookup(memReg.mem.addr(1), Cat(rdData(2), rdData(3)), Array(
    (Bits("b0"), Cat(rdData(3), rdData(2))),
    (Bits("b1"), Cat(rdData(1), rdData(0)))))
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
  val baseReg = Reg(resetVal = UFix(4, DATA_WIDTH))

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
  io.memfe.doCallRet := (memReg.mem.call || memReg.mem.ret || memReg.mem.brcf ||
                         memReg.mem.xcall || memReg.mem.xret)
  io.memfe.callRetPc := memReg.mem.callRetAddr(DATA_WIDTH-1, 2)
  io.memfe.callRetBase := memReg.mem.callRetBase(DATA_WIDTH-1, 2)

  // TODO: remember base address for faking return offset
  when(enable && io.memfe.doCallRet) {
	baseReg := memReg.mem.callRetBase
  }

  // ISPM write
  io.memfe.store := io.localInOut.M.Cmd === OcpCmd.WRNP
  io.memfe.addr := io.exmem.mem.addr
  io.memfe.data := Cat(wrData(3), wrData(2), wrData(1), wrData(0))

  // extra port for forwarding
  io.exResult := io.exmem.rd

  // acknowledge exception
  io.exc.call := memReg.mem.xcall
  io.exc.ret := memReg.mem.xret
  // trigger exception
  io.exc.exc := memReg.mem.trap || memReg.mem.illOp || illMem

  io.exc.src := Mux(memReg.mem.illOp, Bits(0),
					Mux(illMem, Bits(1),
						memReg.mem.xsrc))
  io.exc.excAddr := Mux(memReg.mem.trap, memReg.pc + UFix(1), memReg.pc)
}
