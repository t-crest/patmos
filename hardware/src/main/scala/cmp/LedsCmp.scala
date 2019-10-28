package cmp

import Chisel._
import io._
import ocp.OcpCoreSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

object LedsCmp {

  trait Pins {
    val ledsCmpPins = new Bundle() {
      val led = Bits(OUTPUT)
    }
  }

}

class LedsCmp(nrCores: Int, nrLedPerCore: Int) extends Module {
  val io = new CmpIO(nrCores) with LedsCmp.Pins
  io.ledsCmpPins.led.setWidth(nrCores * nrLedPerCore) //modify number of ledPins dynamically
  io.ledsCmpPins.led := 0.U

  val ledDevs = Vec(nrCores, Module(new Leds(nrLedPerCore)).io)

  //Wire one led IO device per core, each with a number of led
  for (i <- 0 until nrCores) {
    ledDevs(i).ocp.M := io.cores(i).M
    io.cores(i).S := ledDevs(i).ocp.S
    io.ledsCmpPins.led(i) := ledDevs(i).ledsPins.led(0)
  }

}
