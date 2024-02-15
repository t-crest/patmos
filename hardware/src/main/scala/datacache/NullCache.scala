/*
 * A no-op cache
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._

import patmos.Constants._
import patmos.DataCachePerf

import ocp._

class NullCache() extends DCacheType(BURST_LENGTH) {

  io.perf.hit := false.B
  io.perf.miss := false.B

  val burstAddrBits = log2Up(BURST_LENGTH)
  val byteAddrBits = log2Up(DATA_WIDTH/8)

  // State machine for read bursts
  val idle :: read :: readResp :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = idle)
  val burstCntReg = Reg(init = 0.U(burstAddrBits.W))
  val posReg = Reg(Bits(width = burstAddrBits))

  // Register for master signals
  val masterReg = Reg(io.master.M)

  // Register to delay response
  val slaveReg = Reg(io.master.S)

  when(masterReg.Cmd =/= OcpCmd.RD || io.slave.S.CmdAccept === Bits(1)) {
    masterReg := io.master.M
  }
  when(reset) {
    masterReg.Cmd := OcpCmd.IDLE;
  }

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(ADDR_WIDTH-1, burstAddrBits+byteAddrBits),
                         Fill(burstAddrBits+byteAddrBits, Bits(0)))
  io.slave.M.Data := Bits(0)
  io.slave.M.DataValid := Bits(0)
  io.slave.M.DataByteEn := Bits(0)

  io.master.S.Resp := OcpResp.NULL
  io.master.S.Data := Bits(0)

  // Wait for response
  when(stateReg === read) {
    when(burstCntReg === posReg) {
      slaveReg := io.slave.S
    }
    when(io.slave.S.Resp =/= OcpResp.NULL) {
      when(burstCntReg === (BURST_LENGTH-1).U) {
        stateReg := readResp
      }
      burstCntReg := burstCntReg + 1.U
    }
  }
  // Pass data to master
  when(stateReg === readResp) {
    io.master.S := slaveReg
    stateReg := idle
  }

  // Start a read burst
  when(masterReg.Cmd === OcpCmd.RD) {
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === Bits(1)) {
      stateReg := read
      posReg := masterReg.Addr(burstAddrBits+byteAddrBits-1, byteAddrBits)
      io.perf.miss := true.B
    }
  }
}
