/*
 * Definitions for Patmos' OCP ports for general I/O
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package ocp

import chisel3._

// Masters include a RespAccept signal
class OcpIOMasterSignals(addrWidth: Int, dataWidth: Int)
    extends OcpCoreMasterSignals(addrWidth, dataWidth) {
  val RespAccept = UInt(1.W)

}

// Slaves include a CmdAccept signal
class OcpIOSlaveSignals(dataWidth: Int)
    extends OcpSlaveSignals(dataWidth) {
  val CmdAccept = UInt(1.W)

}

// Master port
class OcpIOMasterPort(val addrWidth: Int, val dataWidth: Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = Output(new OcpIOMasterSignals(addrWidth, dataWidth))
  val S = Input(new OcpIOSlaveSignals(dataWidth))
}

// Slave port is reverse of master port
class OcpIOSlavePort(val addrWidth: Int, val dataWidth: Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = Input(new OcpIOMasterSignals(addrWidth, dataWidth))
  val S = Output(new OcpIOSlaveSignals(dataWidth))
}

// Bridge between ports that do/do not support CmdAccept
class OcpIOBridge(master: OcpCoreMasterPort, slave: OcpIOSlavePort) {
  // Register signals that come from master
  val masterReg = RegInit(init = master.M)
  when(masterReg.Cmd === OcpCmd.IDLE || slave.S.CmdAccept === 1.U) {
    masterReg := master.M
  }
  // Forward master signals to slave, always accept responses
  slave.M := masterReg
  slave.M.RespAccept := "b1".U

  // Forward slave signals to master
  master.S <> slave.S
}

// Bridge between ports that do/do not support CmdAccept
// An alternative version, being on the safer side for reset.
// Inserts one cycle delay for the command register (could be improved)
//   including the earlier reaction on CmdAccep
//   adds than combinational paths
class OcpIOBridgeAlt(master: OcpCoreMasterPort, slave: OcpIOSlavePort) {
  
  val masterReg = RegInit(init = master.M) // What is the reset value of this bundle?
  val busyReg = RegInit(init = false.B)

  when(!busyReg) {
    masterReg := master.M
  }
  when(master.M.Cmd === OcpCmd.RD || master.M.Cmd === OcpCmd.WR) {
    busyReg := true.B
  }
  when(busyReg && slave.S.CmdAccept === 1.U) {
    busyReg := false.B
    masterReg.Cmd := OcpCmd.IDLE
  }

  // Forward master signals to slave, always accept responses
  slave.M := masterReg
  slave.M.RespAccept := "b1".U

  // Forward slave signals to master
  master.S <> slave.S
}
// Provide a "bus" with a master port and a slave port to simplify plumbing
class OcpIOBus(addrWidth: Int, dataWidth: Int) extends Module {
  val io = new Bundle {
    val slave = new OcpIOSlavePort(addrWidth, dataWidth)
    val master = new OcpIOMasterPort(addrWidth, dataWidth)
  }
  io.master <> io.slave
}
