/*
 * Memory stage of Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._

import Constants._

import ocp._

class Memory() extends Module {
  val io = IO(new MemoryIO())

  // Register from execution stage
  val memReg = Reg(new ExMem())

  // React on error responses
  val illMem = (io.localInOut.S.Resp === OcpResp.ERR ||
                io.globalInOut.S.Resp === OcpResp.ERR ||
                io.icacheIllMem || io.scacheIllMem)
  val illMemReg = Reg(next = illMem)

  // Flush logic
  val flush = (memReg.mem.xcall || memReg.mem.trap ||
               ((memReg.mem.call || memReg.mem.ret ||
                 memReg.mem.brcf || memReg.mem.xret) && memReg.mem.nonDelayed) ||
               memReg.mem.illOp || illMemReg)
  io.flush := flush

  // Stall logic
  val mayStallReg = RegInit(Bool(false))
  val enable = (io.localInOut.S.Resp === OcpResp.DVA
                || io.globalInOut.S.Resp === OcpResp.DVA
                || !mayStallReg)
  io.ena_out := enable

  // Register from execution stage
  when(enable && io.ena_in) {
    memReg := io.exmem
    mayStallReg := io.exmem.mem.load || io.exmem.mem.store
    when(flush) {
      memReg.flush()
      mayStallReg := Bool(false)
    }
  }
  when(illMem) {
      mayStallReg := Bool(false)
  }

  // Buffer incoming data while being stalled from I-cache
  val rdDataEnaReg = RegInit(Bool(false))
  val rdDataReg = RegInit(Bits(0, width = 32))
  // Save incoming data if available during I-cache stall
  when (!io.ena_in) {
    when (io.localInOut.S.Resp =/= OcpResp.NULL || io.globalInOut.S.Resp =/= OcpResp.NULL) {
      mayStallReg := Bool(false)
      rdDataEnaReg := Bool(true)
    }
    when (io.localInOut.S.Resp === OcpResp.DVA) {
      rdDataReg := io.localInOut.S.Data
    }
    when (io.globalInOut.S.Resp === OcpResp.DVA) {
      rdDataReg := io.globalInOut.S.Data
    }
  }
  .otherwise {
    rdDataEnaReg := Bool(false)
  }

  // Write data multiplexing and write enables
  // Big endian, where MSB is at the lowest address

  // default is word store
  val wrData = Wire(Vec(BYTES_PER_WORD, Bits(width = BYTE_WIDTH)))
  for (i <- 0 until BYTES_PER_WORD) {
    wrData(i) := io.exmem.mem.data((i+1)*BYTE_WIDTH-1, i*BYTE_WIDTH)
  }
  val byteEn = Wire(Bits(width = BYTES_PER_WORD))
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
  val cmd = Mux(enable && io.ena_in && !flush,
                Bits("b0") ## io.exmem.mem.load ## io.exmem.mem.store,
                OcpCmd.IDLE)

  io.localInOut.M.Cmd := Mux(io.exmem.mem.typ === MTYPE_L, cmd, OcpCmd.IDLE)
  io.localInOut.M.Addr := Cat(io.exmem.mem.addr(ADDR_WIDTH-1, 2), Bits("b00"))
  io.localInOut.M.Data := Cat(wrData(3), wrData(2), wrData(1), wrData(0))
  io.localInOut.M.ByteEn := byteEn

  io.globalInOut.M.Cmd := Mux(io.exmem.mem.typ =/= MTYPE_L, cmd, OcpCmd.IDLE)
  io.globalInOut.M.Addr := Cat(io.exmem.mem.addr(ADDR_WIDTH-1, 2), Bits("b00"))
  io.globalInOut.M.Data := Cat(wrData(3), wrData(2), wrData(1), wrData(0))
  io.globalInOut.M.ByteEn := byteEn
  io.globalInOut.M.AddrSpace := Mux(io.exmem.mem.typ === MTYPE_S, OcpCache.STACK_CACHE,
                                    Mux(io.exmem.mem.typ === MTYPE_C, OcpCache.DATA_CACHE,
                                        OcpCache.UNCACHED))

  def splitData(word: Bits) = {
    val retval = Wire(Vec(BYTES_PER_WORD, Bits(width = BYTE_WIDTH)))
    for (i <- 0 until BYTES_PER_WORD) {
      retval(i) := word((i+1)*BYTE_WIDTH-1, i*BYTE_WIDTH)
    }
    retval
  }

  // Read data multiplexing and sign extensions if needed
  val rdData = splitData(Mux(Bool(ICACHE_TYPE == ICACHE_TYPE_LINE) && rdDataEnaReg,
                             rdDataReg,
                             Mux(memReg.mem.typ === MTYPE_L,
                                 io.localInOut.S.Data, io.globalInOut.S.Data)))

  val dout = Wire(Bits(width = DATA_WIDTH))
  // default word read
  dout := Cat(rdData(3), rdData(2), rdData(1), rdData(0))

  // byte read
  val bval = MuxLookup(memReg.mem.addr(1, 0), rdData(0), Array(
    (Bits("b00"), rdData(3)),
    (Bits("b01"), rdData(2)),
    (Bits("b10"), rdData(1)),
    (Bits("b11"), rdData(0))))
  // half-word read
  val hval = Mux(memReg.mem.addr(1) === Bits(0),
                 Cat(rdData(3), rdData(2)),
                 Cat(rdData(1), rdData(0)))

  // sign extensions
  when(memReg.mem.byte) {
    dout := Mux(memReg.mem.zext,
                Bits(0, DATA_WIDTH-BYTE_WIDTH),
                Fill(DATA_WIDTH-BYTE_WIDTH, bval(BYTE_WIDTH-1))) ## bval
  }
  when(memReg.mem.hword) {
    dout := Mux(memReg.mem.zext,
                Bits(0, DATA_WIDTH-2*BYTE_WIDTH),
                Fill(DATA_WIDTH-2*BYTE_WIDTH, hval(DATA_WIDTH/2-1))) ## hval
  }

  io.memwb.pc := memReg.pc
  io.memwb.rd := memReg.rd
  // Fill in data from loads
  io.memwb.rd(0).data := Mux(memReg.mem.load, dout, memReg.rd(0).data)

  // call to fetch
  io.memfe.doCallRet := (memReg.mem.call || memReg.mem.ret || memReg.mem.brcf ||
                         memReg.mem.xcall || memReg.mem.xret)
  io.memfe.callRetPc := memReg.mem.callRetAddr(DATA_WIDTH-1, 2)
  io.memfe.callRetBase := memReg.mem.callRetBase(DATA_WIDTH-1, 2)

  // ISPM write
  io.memfe.store := io.localInOut.M.Cmd === OcpCmd.WR
  io.memfe.addr := io.exmem.mem.addr
  io.memfe.data := Cat(wrData(3), wrData(2), wrData(1), wrData(0))

  // extra port for forwarding
  io.exResult := io.exmem.rd

  // acknowledge exception
  io.exc.call := memReg.mem.xcall
  io.exc.ret := memReg.mem.xret
  // trigger exception
  io.exc.exc := memReg.mem.trap || memReg.mem.illOp || illMemReg

  io.exc.src := Mux(memReg.mem.illOp, Bits(0),
                    Mux(illMemReg, Bits(1),
                        memReg.mem.xsrc))
  io.exc.excBase := memReg.base
  io.exc.excAddr := Mux(memReg.mem.trap, memReg.relPc + UInt(1), memReg.relPc)

  // Keep signal alive for debugging
  debug(io.memwb.pc)

  // reset at end to override any computations
  when(reset) { memReg.flush() }
}
