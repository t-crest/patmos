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
        poke(dut.io.ocp.M.Data, 0xFFFF)
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
      () => Module(new RTC(clockFreq = 80000000, secondsWidth = 32, nanoWidth = 32, initialTime = 0x5ac385dcffffff00L))) {
      dut => new RTCTester(dut, testCycles = 10000)
    }
  }
}