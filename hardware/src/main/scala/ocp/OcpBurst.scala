
/*
 * Definitions for OCP ports that support Bursts
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package ocp

import Chisel._
import chisel3.VecInit

// Burst masters provide handshake signals
class OcpBurstMasterSignals(addrWidth : Int, dataWidth : Int)
  extends OcpMasterSignals(addrWidth, dataWidth) {
  val DataValid = Output(UInt(width = 1))
  val DataByteEn = Output(UInt(width = dataWidth/8))

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpBurstMasterSignals(addrWidth, dataWidth)
    res.asInstanceOf[this.type]
  }
}

// Burst slaves provide handshake signal
class OcpBurstSlaveSignals(dataWidth : Int)
  extends OcpSlaveSignals(dataWidth) {
  val CmdAccept = Input(UInt(width = 1))
  val DataAccept = Input(UInt(width = 1))

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpBurstSlaveSignals(dataWidth)
    res.asInstanceOf[this.type]
  }
}

// Master port
class OcpBurstMasterPort(addrWidth : Int, dataWidth : Int, burstLen : Int) extends Bundle() {
  val burstLength = burstLen
  // Clk is implicit in Chisel
  val M = Output(new OcpBurstMasterSignals(addrWidth, dataWidth))
  val S = Input(new OcpBurstSlaveSignals(dataWidth))
}

// Slave port is reverse of master port
class OcpBurstSlavePort(val addrWidth : Int, val dataWidth : Int, val burstLen : Int) extends Bundle() {
  val burstLength = burstLen
  // Clk is implicit in Chisel
  val M = Input(new OcpBurstMasterSignals(addrWidth, dataWidth))
  val S = Output(new OcpBurstSlaveSignals(dataWidth))

  // This does not really clone, but Data.clone doesn't either
  override def cloneType() = {
    val res = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)
    res.asInstanceOf[this.type]
  }

}

// Bridge between word-oriented port and burst port
class OcpBurstBridge(master : OcpCacheMasterPort, slave : OcpBurstSlavePort) {
  val addrWidth = master.M.Addr.getWidth
  val dataWidth = master.M.Data.getWidth
  val burstLength = slave.burstLength
  val burstAddrUInt = log2Up(burstLength)

  // State of transmission
  val idle :: read :: readResp :: write :: Nil = Enum(UInt(), 4)
  val state = Reg(init = idle)
  val burstCnt = Reg(init = UInt(0, burstAddrUInt))
  val cmdPos = Reg(UInt(width = burstAddrUInt))

  // Register signals that come from master
  val masterReg = Reg(init = master.M)

  // Register to delay response
  val slaveReg = Reg(master.S)

  when(state =/= write && (masterReg.Cmd === OcpCmd.IDLE || slave.S.CmdAccept === UInt(1))) {
    masterReg := master.M
  }

  // Default values
  slave.M.Cmd := masterReg.Cmd
  slave.M.Addr := Cat(masterReg.Addr(addrWidth-1, burstAddrUInt+log2Up(dataWidth/8)),
                      Fill(burstAddrUInt+log2Up(dataWidth/8), UInt(0)))
  slave.M.Data := UInt(0)
  slave.M.DataByteEn := UInt(0)
  slave.M.DataValid := UInt(0)
  master.S := slave.S

  // Read burst
  when(state === read) {
    when(slave.S.Resp =/= OcpResp.NULL) {
      when(burstCnt === cmdPos) {
        slaveReg := slave.S
      }
      when(burstCnt === UInt(burstLength - 1)) {
        state := readResp
      }
      burstCnt := burstCnt + UInt(1)
    }
    master.S.Resp := OcpResp.NULL
    master.S.Data := UInt(0)
  }
  when(state === readResp) {
    state := idle
    master.S := slaveReg
  }

  // Write burst
  when(state === write) {
    masterReg.Cmd := OcpCmd.IDLE
    slave.M.DataValid := UInt(1)
    when(burstCnt === cmdPos) {
      slave.M.Data := masterReg.Data
      slave.M.DataByteEn := masterReg.ByteEn
    }
    when(burstCnt === UInt(burstLength - 1)) {
      state := idle
    }
    when(slave.S.DataAccept === UInt(1)) {
      burstCnt := burstCnt + UInt(1)
    }
  }

  // Start new transaction
  when(master.M.Cmd === OcpCmd.RD) {
    state := read
    cmdPos := master.M.Addr(burstAddrUInt+log2Up(dataWidth/8)-1, log2Up(dataWidth/8))
  }
  when(master.M.Cmd === OcpCmd.WR) {
    state := write
    cmdPos := master.M.Addr(burstAddrUInt+log2Up(dataWidth/8)-1, log2Up(dataWidth/8))
  }
}

// Join two OcpBurst ports, assume no collisions between requests
class OcpBurstJoin(left : OcpBurstMasterPort, right : OcpBurstMasterPort,
                   joined : OcpBurstSlavePort, selectLeft : Bool = Wire(Bool())) {

  val selRightReg = Reg(Bool())
  val selRight = Wire(Bool())
  selRight := Mux(left.M.Cmd =/= OcpCmd.IDLE, false.B,
                     Mux(right.M.Cmd =/= OcpCmd.IDLE, true.B,
                         selRightReg))

  joined.M := left.M
  when (selRight) {
    joined.M := right.M
  }
  joined.M.Cmd := right.M.Cmd | left.M.Cmd

  right.S := joined.S
  left.S := joined.S

  when(selRightReg) {
    left.S.Resp := OcpResp.NULL
  }
  .otherwise {
    right.S.Resp := OcpResp.NULL
  }

  selRightReg := selRight

  selectLeft := !selRight
}

// Join two OcpBurst ports, left port has priority in case of colliding requests
class OcpBurstPriorityJoin(left : OcpBurstMasterPort, right : OcpBurstMasterPort,
                           joined : OcpBurstSlavePort, selectLeft : Bool = Bool()) {

  val selLeft = left.M.Cmd =/= OcpCmd.IDLE
  val selRight = right.M.Cmd =/= OcpCmd.IDLE

  val leftPendingReg = Reg(init = false.B)
  val rightPendingReg = Reg(init = false.B)

  val pendingRespReg = Reg(init = UInt(0))

  // default port forwarding
  joined.M := Mux(rightPendingReg, right.M, left.M)

  // pass back data to masters
  right.S := joined.S
  left.S := joined.S
  // suppress responses when not serving a request
  when (!rightPendingReg) {
    right.S.Resp := OcpResp.NULL
  }
  when (!leftPendingReg) {
    left.S.Resp := OcpResp.NULL
  }

  // do not accept commands while another request is being served
  left.S.CmdAccept   := Mux(rightPendingReg, UInt(0), joined.S.CmdAccept)
  left.S.DataAccept  := Mux(rightPendingReg, UInt(0), joined.S.DataAccept)
  right.S.CmdAccept  := Mux(leftPendingReg,  UInt(0), joined.S.CmdAccept)
  right.S.DataAccept := Mux(leftPendingReg,  UInt(0), joined.S.DataAccept)

  // forward requests from left port
  when (selLeft) {
    when (!rightPendingReg) {
      joined.M := left.M
      pendingRespReg := Mux(left.M.Cmd === OcpCmd.WR, UInt(1), UInt(left.burstLength))
      leftPendingReg := true.B
      right.S.CmdAccept  := UInt(0)
      right.S.DataAccept := UInt(0)
    }
  }
  // forward requests from right port
  when (selRight) {
    when (!selLeft && !leftPendingReg) {
      joined.M := right.M
      pendingRespReg := Mux(right.M.Cmd === OcpCmd.WR, UInt(1), UInt(right.burstLength))
      rightPendingReg := true.B
    }
  }

  // count responses, clear pending flags at end of requests
  when (joined.S.Resp =/= OcpResp.NULL) {
    pendingRespReg := pendingRespReg - UInt(1)
    when (pendingRespReg === UInt(1)) {
      when (leftPendingReg) {
        leftPendingReg := false.B
      }
      when (rightPendingReg) {
        rightPendingReg := false.B
      }
    }
  }

  selectLeft := selLeft
}

// Provide a "bus" with a master port and a slave port to simplify plumbing
class OcpBurstBus(addrWidth : Int, dataWidth : Int, burstLen : Int) extends Module {
  val io = IO(new Bundle {
    val master = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
    val slave = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)
  })
  io.master.M <> io.slave.M
  io.slave.S <> io.master.S
}

// Buffer a burst for pipelining
class OcpBurstBuffer(master : OcpBurstMasterPort, slave : OcpBurstSlavePort) {

  val MBuffer = RegInit(VecInit(Seq.fill(master.burstLength)(master.M)))

  val free = MBuffer(0).Cmd === OcpCmd.IDLE
  when (free || slave.S.CmdAccept === UInt(1)) {
    for (i <- 0 until master.burstLength-1) {
      MBuffer(i) := MBuffer(i+1)
    }
    MBuffer(master.burstLength-1) := master.M
  }
  slave.M := MBuffer(0)

  val SBuffer = Reg(next = slave.S)
  master.S := SBuffer

  master.S.CmdAccept := free
  master.S.DataAccept := free
}
