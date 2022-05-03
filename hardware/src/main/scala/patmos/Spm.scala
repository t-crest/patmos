/*
 * An on-chip memory.
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
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import Chisel._

import Constants._

import ocp._

class Spm(size: Int) extends Module {
  val io = IO(new OcpCoreSlavePort(log2Up(size), DATA_WIDTH))

  val addrUInt = log2Up(size / BYTES_PER_WORD)

  // respond and return (dummy) data
  val cmdReg = Reg(next = io.M.Cmd)
  io.S.Resp := Mux(cmdReg === OcpCmd.WR || cmdReg === OcpCmd.RD,
                   OcpResp.DVA, OcpResp.NULL)
  io.S.Data := UInt(0)

  if (size > 0) {
    // generate byte memories
    val mem = new Array[MemBlockIO](BYTES_PER_WORD)
    for (i <- 0 until BYTES_PER_WORD) {
      mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH).io
    }

    // store
    val stmsk = Mux(io.M.Cmd === OcpCmd.WR, io.M.ByteEn,  UInt(0))
    for (i <- 0 until BYTES_PER_WORD) {
      mem(i) <= (stmsk(i), io.M.Addr(addrUInt + 1, 2),
                 io.M.Data(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
    }

    // load
    val rdData = mem.map(_(io.M.Addr(addrUInt + 1, 2))).reduceLeft((x,y) => y ## x)

    // return actual data
    io.S.Data := rdData
  }
}
