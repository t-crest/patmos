/*
 * Test for usability of OCP definitions
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package ocp

import Chisel._

class OcpMaster() extends Module {
  val io = IO(new OcpCacheMasterPort(8, 32))

  val cnt = Reg(UInt(), init = UInt(0))
  cnt := cnt + UInt(1, 32)

  io.M.Cmd := OcpCmd.IDLE
  io.M.Addr := cnt(15, 8)
  io.M.Data := cnt
  io.M.ByteEn := cnt(7, 4)

  when(cnt(3, 0) === UInt("b1111")) {
    io.M.Cmd := OcpCmd.WR
  }
}

class OcpSlave() extends Module {
  val io = IO(new OcpBurstSlavePort(8, 32, 4))

  val M = Reg(next = io.M)

  val data = Reg(UInt(), init = UInt(0))
  data := data + UInt(1, 32)

  val cnt = Reg(UInt(), init = UInt(0))

  io.S.Resp := OcpResp.NULL
  io.S.Data := data
  when(M.Cmd =/= OcpCmd.IDLE) {
    cnt := UInt(4)
  }

  when(cnt =/= UInt(0)) {
    cnt := cnt - UInt(1)
    io.S.Resp := OcpResp.DVA
  }
}

class Ocp() extends Module {
  val io = IO(new OcpBurstSlavePort(8, 32, 4))

  val master = Module(new OcpMaster())
  val slave = Module(new OcpSlave())
  val bridge = new OcpBurstBridge(master.io, slave.io)

  io <> slave.io
}

object OcpTestMain {
  def main(args: Array[String]): Unit = {
    chiselMain(args, () => Module(new Ocp()))
  }
}
