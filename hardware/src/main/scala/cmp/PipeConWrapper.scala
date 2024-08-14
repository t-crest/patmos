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

  val s4noc = Module(new S4NoCTop(Config(nrCores, BubbleType(8), BubbleType(2), BubbleType(8), 32)))
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