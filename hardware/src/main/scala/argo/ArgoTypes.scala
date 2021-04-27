/*
 * Definition of Argo network types
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import Chisel._

class MemIFMaster(val HEADER_FIELD_WIDTH: Int, val HEADER_CTRL_WIDTH: Int) extends Bundle() {
  val Addr = UInt(width = HEADER_FIELD_WIDTH - HEADER_CTRL_WIDTH)
  val En = Bits(width = 2)
  val Wr = Bool()
  val Data = Bits(width = 64)
}

class MemIFSlave extends Bundle {
  val Data = Bits(width = 64)
  val Error = Bool()
}

class ConfIFMaster(val HEADER_FIELD_WIDTH: Int, val HEADER_CTRL_WIDTH: Int) extends Bundle() {
  val Addr = UInt(width = HEADER_FIELD_WIDTH - HEADER_CTRL_WIDTH)
  val En = Bool()
  val Wr = Bool()
  val Data = Bits(width = 64)
}

class ConfIFSlave extends Bundle {
  val Data = Bits(width = 32)
  val Error = Bool()
}

class SPMMasterPort(val headerFieldWidth: Int, val headerCtrlWidth: Int) extends Bundle {
  val M = Output(new MemIFMaster(headerFieldWidth, headerCtrlWidth))
  val S = Input(new MemIFSlave())
}

class SPMSlavePort(headerFieldWidth: Int, headerCtrlWidth: Int) extends Bundle {
  val M = Input(new MemIFMaster(headerFieldWidth, headerCtrlWidth))
  val S = Output(new MemIFSlave())
}

class ChannelForward(val argoConf : ArgoConfig) extends Bundle {
  val req = Bool()
  val data = Bits(width = argoConf.LINK_WIDTH)
}

class ChannelBackward(val argoConf: ArgoConfig) extends Bundle {
  val ack = Bool()
}

class RouterPort(argoConf: ArgoConfig) extends Bundle {
  val f = Input(new ChannelForward(argoConf))
  val b = Output(new ChannelBackward(argoConf))
}

class OutputPort(argoConf: ArgoConfig) extends Bundle {
  val f = Output(new ChannelForward(argoConf))
  val b = Input(new ChannelBackward(argoConf))
}

class NodeInterconnection(val argoConf: ArgoConfig) extends Bundle {
  val north_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val east_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val south_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val west_wire_in = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val north_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val east_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val south_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
  val west_wire_out = Wire(init = UInt(0, width = argoConf.LINK_WIDTH))
}
