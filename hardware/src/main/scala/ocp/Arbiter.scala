
/*
 * Arbiter for OCP burst slaves.
 * Pseudo round robin arbitration. Each turn for a non-requesting master costs 1 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp

import chisel3._
import chisel3.util._

class Arbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int) extends ArbiterType(cnt, dataWidth, dataWidth, burstLen) {

  val turnReg = RegInit(0.U(log2Up(cnt).W))
  val burstCntReg = RegInit(0.U(log2Up(burstLen).W))

  val sIdle :: sRead :: sWrite :: Nil = Enum(3)
  val stateReg = RegInit(sIdle)

  // buffer signals from master to cut critical paths
  val masterBuffer = Wire(Vec(cnt, new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)))
  
  for (i <- 0 to cnt - 1) {
    
    val port = masterBuffer(i)
    val bus = Module(new OcpBurstBus(addrWidth, dataWidth, burstLen))
    bus.io.slave.M := io.master(i).M
    io.master(i).S := bus.io.slave.S
    new OcpBurstBuffer(bus.io.master, port)
  }

  val master = masterBuffer(turnReg).M

  when(stateReg === sIdle) {
    when(master.Cmd =/= OcpCmd.IDLE) {
      when(master.Cmd === OcpCmd.RD) {
        stateReg := sRead
      }
      when(master.Cmd === OcpCmd.WR) {
        stateReg := sWrite
        burstCntReg := 0.U
      }
    }
      .otherwise {
        turnReg := Mux(turnReg === (cnt - 1).U, 0.U, turnReg + 1.U)
      }
  }
  when(stateReg === sWrite) {
    // Just wait on the DVA after the write
    when(io.slave.S.Resp === OcpResp.DVA) {
      turnReg := Mux(turnReg === (cnt - 1).U, 0.U, turnReg + 1.U)
      stateReg := sIdle
    }
  }
  when(stateReg === sRead) {
    // For read we have to count the DVAs
    when(io.slave.S.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + 1.U
      when(burstCntReg === burstLen.U - 1.U) {
        turnReg := Mux(turnReg === (cnt - 1).U, 0.U, turnReg + 1.U)
        stateReg := sIdle
      }
    }
  }

  io.slave.M := master

  for (i <- 0 to cnt - 1) {
    masterBuffer(i).S.CmdAccept := 0.U
    masterBuffer(i).S.DataAccept := 0.U
    masterBuffer(i).S.Resp := OcpResp.NULL
    // we forward the data to all masters
    masterBuffer(i).S.Data := io.slave.S.Data
  }
  masterBuffer(turnReg).S := io.slave.S

  // The response of the SSRAM comes a little bit late
}
