package ptp1588assist

import Chisel._

class MIIChannel extends Bundle{
  val clk = Bool(INPUT)
  val dv = Bool()
  val data = Bits(width=4)
  val err = Bool()
}

class MIIBundle extends Bundle{
  //RX
  val rxChannel = new MIIChannel().asInput()
  //TX
  val txChannel = new MIIChannel().asOutput()
}

