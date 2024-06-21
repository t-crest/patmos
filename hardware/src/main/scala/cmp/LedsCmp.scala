
package cmp

import chisel3._
import io._
import ocp.OcpCoreSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}


class LedsCmp(nrCores: Int, nrLedPerCore: Int) extends CmpDevice(nrCores) {
  val io = IO(new CmpIO(nrCores) with patmos.HasPins {
    val pins = new Bundle() {
      val led = Output(UInt((nrCores * nrLedPerCore).W))
    }
  })
  // commented out below as chisel3 do not support setWidth, trait are parameterless which means 
  // there is no good way of setting with. All uses of this class has nrLedPerCore = 1 anyway 
  //io.ledsCmpPins.led.setWidth(nrCores * nrLedPerCore) //modify number of ledPins dynamically

  io.pins.led := 0.U

  val ledDevs = Seq.fill(nrCores)(Module(new Leds(nrLedPerCore)).io)

  //Wire one led IO device per core, each with a number of led
  for (i <- 0 until nrCores) {
    ledDevs(i).ocp.M := io.cores(i).M
    io.cores(i).S := ledDevs(i).ocp.S
    ledDevs(i).superMode := false.B
  }
  io.pins.led := ledDevs.map(_.pins.led).reduceLeft((l, h) => h ## l)

}

