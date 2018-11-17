
/*
 * Arbiter for OCP burst slaves.
 * Pseudo round robin arbitration. Each turn for a non-requesting master costs 1 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp

import Chisel._

class Arbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int) extends Module {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = IO(new Bundle {
    val master = Vec.fill(cnt) { new OcpBurstSlavePort(addrWidth, dataWidth, burstLen) }
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  })

  val turnReg = Reg(init = UInt(0, log2Up(cnt)))
  val burstCntReg = Reg(init = UInt(0, log2Up(burstLen)))

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = sIdle)

  // buffer signals from master to cut critical paths
  val masterBuffer = Vec(io.master.map { m =>
    val port = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)
    val bus = Module(new OcpBurstBus(addrWidth, dataWidth, burstLen))
    m <> bus.io.slave
    new OcpBurstBuffer(bus.io.master, port)
    port
  })

  val master = masterBuffer(turnReg).M

  when(stateReg === sIdle) {
    when(master.Cmd =/= OcpCmd.IDLE) {
      when(master.Cmd === OcpCmd.RD) {
        stateReg := sRead
      }
      when(master.Cmd === OcpCmd.WR) {
        stateReg := sWrite
        burstCntReg := UInt(0)
      }
    }
      .otherwise {
        turnReg := Mux(turnReg === UInt(cnt - 1), UInt(0), turnReg + UInt(1))
      }
  }
  when(stateReg === sWrite) {
    // Just wait on the DVA after the write
    when(io.slave.S.Resp === OcpResp.DVA) {
      turnReg := Mux(turnReg === UInt(cnt - 1), UInt(0), turnReg + UInt(1))
      stateReg := sIdle
    }
  }
  when(stateReg === sRead) {
    // For read we have to count the DVAs
    when(io.slave.S.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + UInt(1)
      when(burstCntReg === UInt(burstLen) - UInt(1)) {
        turnReg := Mux(turnReg === UInt(cnt - 1), UInt(0), turnReg + UInt(1))
        stateReg := sIdle
      }
    }
  }

  io.slave.M := master

  for (i <- 0 to cnt - 1) {
    masterBuffer(i).S.CmdAccept := Bits(0)
    masterBuffer(i).S.DataAccept := Bits(0)
    masterBuffer(i).S.Resp := OcpResp.NULL
    // we forward the data to all masters
    masterBuffer(i).S.Data := io.slave.S.Data
  }
  masterBuffer(turnReg).S := io.slave.S

  // The response of the SSRAM comes a little bit late
}

object ArbiterMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(4, args.length)
    val cnt = args(0)
    val addrWidth = args(1)
    val dataWidth = args(2)
    val burstLen = args(3)

    chiselMain(chiselArgs, () => Module(new Arbiter(cnt.toInt,addrWidth.toInt,dataWidth.toInt,burstLen.toInt)))
  }
}


