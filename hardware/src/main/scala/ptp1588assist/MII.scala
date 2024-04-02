package ptp1588assist

import chisel3._

class MIIChannel extends Bundle{
  val clk = Input(Bool())
  val dv = Bool()
  val data = UInt(4.W)
  val err = Bool()
}

class MIIBundle extends Bundle{
  //RX
  val rxChannel = Input(new MIIChannel())
  //TX
  val txChannel = Output(new MIIChannel())
}

