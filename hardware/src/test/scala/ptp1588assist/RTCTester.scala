package ptp1588assist

import Chisel._
import ocp.{OcpCmd, OcpResp}
import patmos.Constants._

class RTCTester(dut: RTC, testCycles: Int) extends Tester(dut){
  for(i <- 0 to testCycles) {
    var asked: Boolean = false
    step(1)
    peek(dut.io.ptpTimestamp)
    if(!asked && (i % 33) == 0 ) {
      if (i % 72 == 0) {
        poke(dut.io.ocp.M.Cmd, 1)//Write
      } else if(i % 32 == 0) {
        poke(dut.io.ocp.M.Cmd, 2)//Read
      }
      asked = true
    } else {
      if(asked && peek(dut.io.ocp.S.Resp)==0){
        poke(dut.io.ocp.M.Cmd, 1)
      } else {
        poke(dut.io.ocp.M.Cmd, 0)
      }
    }
  }
}

object RTCTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new RTC(80000000, 32, 32, 1522763228L))) {
      dut => new RTCTester(dut, testCycles = 10000)
    }
  }
}