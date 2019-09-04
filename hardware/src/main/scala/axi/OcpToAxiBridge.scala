package axi

import Chisel._
import ocp.{OcpCmd, OcpCoreMasterPort, OcpResp}

class OcpToAxiBridge(addrWidth: Int, dataWidth: Int) {

  /**
    * Converts an OCP master to an AXI4-Lite master interface
    * @param mOcpPort
    * @return an AXI4-Lite Master interface
    */
  def MOcpToMAxiWrapper(mOcpPort: OcpCoreMasterPort): AxiLiteMasterPort = {
    val mAxiPort = new AxiLiteMasterPort(addrWidth, dataWidth)

    val sCmdAccept = RegNext(mAxiPort.ar.ready || (mAxiPort.aw.ready && mAxiPort.w.ready))

    val mOcpReg = Reg(mOcpPort.M) // data channel

    when(sCmdAccept) {
      mOcpReg := mOcpPort.M
    }

    // Write channel
    mAxiPort.aw.valid := mOcpReg.Cmd === OcpCmd.WR
    mAxiPort.aw.bits.addr := mOcpReg.Addr
    mAxiPort.w.valid := mOcpReg.Cmd === OcpCmd.WR
    mAxiPort.w.bits.data := mOcpReg.Data
    mAxiPort.w.bits.strb := mOcpReg.ByteEn

    // Read channel
    mAxiPort.ar.valid := mOcpReg.Cmd === OcpCmd.RD
    mAxiPort.r.ready := true.B // the ocp bus is always ready to accept data

    // Drive OCP slave
    mOcpPort.S.Data := mAxiPort.r.bits.data
    mOcpPort.S.Resp := Mux(mAxiPort.b.bits.resp === Bits("00") || mAxiPort.r.bits.resp === Bits("00"), OcpResp.DVA, OcpResp.NULL)

    // Return an AXI wrapped interface
    mAxiPort
  }

}
