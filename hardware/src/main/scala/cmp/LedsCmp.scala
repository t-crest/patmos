package cmp

import Chisel._
import io._
import ocp.OcpCoreSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

object LedsCmp {
  var ledPinSize = -1

  def initLedSize(size: Int) = {
    ledPinSize = size
  }
  trait Pins {
    val ledsCmpPins = new Bundle() {
      val led = Output(Bits(width = ledPinSize))
    }
  }

}

class LedsCmp(nrCores: Int, nrLedPerCore: Int) extends Module {
  LedsCmp.initLedSize(nrCores * nrLedPerCore)
  val io = new CmpIO(nrCores) with LedsCmp.Pins
  // commented out below as chisel3 do not support setWidth, trait are parameterless which means 
  // there is no good way of setting with. All uses of this class has nrLedPerCore = 1 anyway 
  //io.ledsCmpPins.led.setWidth(nrCores * nrLedPerCore) //modify number of ledPins dynamically

  io.ledsCmpPins.led := 0.U

  val ledDevs = Vec(nrCores, Module(new Leds(nrLedPerCore)).io)

  //Wire one led IO device per core, each with a number of led
  for (i <- 0 until nrCores) {
    ledDevs(i).ocp.M := io.cores(i).M
    io.cores(i).S := ledDevs(i).ocp.S
    io.ledsCmpPins.led(i) := ledDevs(i).ledsPins.led(0)
  }

}
