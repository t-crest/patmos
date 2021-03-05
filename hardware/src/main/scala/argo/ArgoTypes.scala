/*
 * Definition of Argo network types
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import chisel3._
import chisel3.util._

class MemIFMaster(val HEADER_FIELD_WIDTH: Int, val HEADER_CTRL_WIDTH: Int) extends Bundle() {
  val Addr = UInt((HEADER_FIELD_WIDTH - HEADER_CTRL_WIDTH).W)
  val En = UInt(2.W)
  val Wr = Bool()
  val Data = UInt(64.W)
}

class MemIFSlave extends Bundle {
  val Data = UInt(64.W)
  val Error = Bool()
}

class SPMMasterPort(val headerFieldWdith: Int, val headerCtrlWidth: Int) extends Bundle {
  val M = Output(new MemIFMaster(headerFieldWdith, headerCtrlWidth))
  val S = Input(new MemIFSlave())
}

class SPMSlavePort(headerFieldWdith: Int, headerCtrlWidth: Int) extends Bundle {
  val M = Input(new MemIFMaster(headerFieldWdith, headerCtrlWidth))
  val S = Output(new MemIFSlave())
}

class ChannelForward(val argoConf : ArgoConfig) extends Bundle {
  val req = Bool()
  val data = UInt(argoConf.LINK_WIDTH.W)
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
  val north_wire_in = UInt(argoConf.LINK_WIDTH.W)
  val east_wire_in = UInt(argoConf.LINK_WIDTH.W)
  val south_wire_in = UInt(argoConf.LINK_WIDTH.W)
  val west_wire_in = UInt(argoConf.LINK_WIDTH.W)
  val north_wire_out = UInt(argoConf.LINK_WIDTH.W)
  val east_wire_out = UInt(argoConf.LINK_WIDTH.W)
  val south_wire_out = UInt(argoConf.LINK_WIDTH.W)
  val west_wire_out = UInt(argoConf.LINK_WIDTH.W)
}
