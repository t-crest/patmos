/*
 * A no-op cache
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import chisel3._
import chisel3.util._

import patmos.Constants._
import patmos.DataCachePerf

import ocp._

class NullCache() extends DCacheType(BURST_LENGTH) {

  io.perf.hit := false.B
  io.perf.miss := false.B

  val burstAddrBits = log2Up(BURST_LENGTH)
  val byteAddrBits = log2Up(DATA_WIDTH/8)

  // State machine for read bursts
  val idle :: read :: readResp :: Nil = Enum(3)
  val stateReg = RegInit(init = idle)
  val burstCntReg = RegInit(init = 0.U(burstAddrBits.W))
  val posReg = Reg(UInt(burstAddrBits.W))

  // Register for master signals
  val masterReg = Reg(chiselTypeOf(io.master.M))

  // Register to delay response
  val slaveReg = Reg(chiselTypeOf(io.master.S))

  when(masterReg.Cmd =/= OcpCmd.RD || io.slave.S.CmdAccept === 1.U) {
    masterReg := io.master.M
  }
  when(reset.asBool) {
    masterReg.Cmd := OcpCmd.IDLE;
  }

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(ADDR_WIDTH-1, burstAddrBits+byteAddrBits),
                         Fill(burstAddrBits+byteAddrBits, 0.B))
  io.slave.M.Data := 0.U
  io.slave.M.DataValid := 0.U
  io.slave.M.DataByteEn := 0.U

  io.master.S.Resp := OcpResp.NULL
  io.master.S.Data := 0.U

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
    when(io.slave.S.CmdAccept === 1.U) {
      stateReg := read
      posReg := masterReg.Addr(burstAddrBits+byteAddrBits-1, byteAddrBits)
      io.perf.miss := true.B
    }
  }
}
