/*
 * Definition of Argo network types
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import Chisel._

class MemIFMaster(HEADER_FIELD_WIDTH: Int, HEADER_CTRL_WIDTH: Int) extends Bundle() {
  val Addr = UInt(width = HEADER_FIELD_WIDTH - HEADER_CTRL_WIDTH)
  val En = Bits(width = 2)
  val Wr = Bool()
  val Data = Bits(width = 64)
}

class MemIFSlave extends Bundle {
  val Data = Bits(width = 64)
  val Error = Bool()
}

class SPMMasterPort(headerFieldWdith: Int, headerCtrlWidth: Int) extends Bundle {
  val M = new MemIFMaster(headerFieldWdith, headerCtrlWidth).asOutput()
  val S = new MemIFSlave().asInput()
}

class SPMSlavePort(headerFieldWdith: Int, headerCtrlWidth: Int) extends Bundle {
  val M = new MemIFMaster(headerFieldWdith, headerCtrlWidth).asInput()
  val S = new MemIFSlave().asOutput()
}

class ChannelForward(argoConf : ArgoConfig) extends Bundle {
  val req = Bool()
  val data = Bits(width = argoConf.LINK_WIDTH)
}

class ChannelBackward(argoConf: ArgoConfig) extends Bundle {
  val ack = Bool()
}

class RouterPort(argoConf: ArgoConfig) extends Bundle {
  val f = new ChannelForward(argoConf).asInput()
  val b = new ChannelBackward(argoConf).asOutput()
}

class OutputPort(argoConf: ArgoConfig) extends Bundle {
  val f = new ChannelForward(argoConf).asOutput()
  val b = new ChannelBackward(argoConf).asInput()
}

class NodeInterconnection(argoConf: ArgoConfig) extends Bundle {
  val north_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val east_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val south_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val west_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val north_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val east_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val south_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val west_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
}
