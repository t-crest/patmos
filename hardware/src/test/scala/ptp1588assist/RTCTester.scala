package ptp1588assist

import Chisel._
import ocp.{OcpCmd, OcpResp}
import patmos.Constants._
import sys.process._

class RTCTester(dut: RTC, testCycles: Int) extends Tester(dut){
  var asked: Boolean = false
  for(i <- 0 until testCycles) {
    step(1)
    peek(dut.io.ptpTimestamp)
    peek(dut.io.pps)
    if(!asked && i > 20) {
        asked = true
        poke(dut.io.ocp.M.Cmd, 1)//Write
        poke(dut.io.ocp.M.Data, 0x3B9A0000)
        poke(dut.io.ocp.M.Addr, 0xF00D0000)
    } else if(peek(dut.io.ocp.S.Resp)==1) {
        poke(dut.io.ocp.M.Cmd, 0)
    }
  }
  expect(dut.correctionStepReg, 0)
}

object RTCTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile","--vcd", "--targetDir", "generated/"+this.getClass.getSimpleName.dropRight(1)),
      () => Module(new RTC(clockFreq = 80000000, secondsWidth = 32, nanoWidth = 32, initialTime = 0x5ac385dcL, timeStep = 100))) {
      dut => new RTCTester(dut, testCycles = 10000)
    }
    "gtkwave generated/"++this.getClass.getSimpleName.dropRight(1)+"/"+"RTC.vcd" !
  }
}