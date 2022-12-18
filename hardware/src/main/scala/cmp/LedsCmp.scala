/*

package cmp

import Chisel._
import io._
import ocp.OcpCoreSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}


MS: commented out for the moment until we find a good better solution for pins on CPM devices.

class LedsCmp(nrCores: Int, nrLedPerCore: Int) extends Module {
  val io = new CmpIO(nrCores) with patmos.HasPins {
    override val pins = new Bundle() {
      val led = Output(Bits(width = nrCores * nrLedPerCore))
    }
  }
  // commented out below as chisel3 do not support setWidth, trait are parameterless which means 
  // there is no good way of setting with. All uses of this class has nrLedPerCore = 1 anyway 
  //io.ledsCmpPins.led.setWidth(nrCores * nrLedPerCore) //modify number of ledPins dynamically

  io.pins.led := 0.U

  val ledDevs = Vec(nrCores, Module(new Leds(nrLedPerCore)).io)

  //Wire one led IO device per core, each with a number of led
  for (i <- 0 until nrCores) {
    ledDevs(i).ocp.M := io.cores(i).M
    io.cores(i).S := ledDevs(i).ocp.S
    io.pins.led(i) := ledDevs(i).pins.led(0)
  }

}

 */
