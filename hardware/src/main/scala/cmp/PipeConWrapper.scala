package cmp

import chisel3._
import ocp.OcpResp
import s4noc._

/**
 * A simple wrapper to adapt the signal names between OCP and PipeCon
 * @param nrCores
 */
class PipeConWrapper(nrCores: Int) extends CmpDevice(nrCores) {
  val io = IO(new CmpIO(nrCores))

  val s4noc = Module(new S4NoCTop(Config(nrCores, RegType(8), RegType(2), RegType(8), 32)))
  for (i <- 0 until nrCores) {
    val np = s4noc.io.cpuPorts(i)
    val cp = io.cores(i)
    np.address := cp.M.Addr
    np.rd := cp.M.Cmd === ocp.OcpCmd.RD
    np.wr := cp.M.Cmd === ocp.OcpCmd.WR
    cp.S.Data := np.rdData
    np.wrData := cp.M.Data
    np.wrMask := cp.M.ByteEn
    cp.S.Resp := Mux(np.ack, OcpResp.DVA, OcpResp.NULL)
  }
}

// This was code for the old S4NoC, is this still valid? Difference to above?
/*
class S4nocOCPWrapper(nrCores: Int, txFifo: Int, rxFifo: Int) extends CmpDevice(nrCores) {

  val s4noc = Module(new S4noc(nrCores, txFifo, rxFifo))

  for (i <- 0 until nrCores) {

    val resp = Mux(io.cores(i).M.Cmd === OcpCmd.RD || io.cores(i).M.Cmd === OcpCmd.WR,
      OcpResp.DVA, OcpResp.NULL)

    // addresses are in words
    s4noc.io.cpuPorts(i).addr := io.cores(i).M.Addr
    s4noc.io.cpuPorts(i).wrData := io.cores(i).M.Data
    s4noc.io.cpuPorts(i).wr := io.cores(i).M.Cmd === OcpCmd.WR
    s4noc.io.cpuPorts(i).rd := io.cores(i).M.Cmd === OcpCmd.RD
    io.cores(i).S.Data := RegNext(s4noc.io.cpuPorts(i).rdData)
    io.cores(i).S.Resp := Reg(init = OcpResp.NULL, next = resp)
  }
}
*/
