/*
 * Test the shared SPM, should become some generic tester for OCP devices.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 */

package cmp

import Chisel._

class SharedSPMTester(dut: SharedSPM) extends Tester(dut) {

  println("Tester")
//  def read(n: Int) = {
//    for (i <- 0 until 4*4) {
//      poke(dut.io.memPorts(n).rdAddr, i)
//      step(3)
//      peek(dut.io.memPorts(n).rdData)
//    }
//  }
//
//  for (i <- 0 until 32) {
//    for (j <- 0 until 4) {
//      poke(dut.io.memPorts(j).wrAddr, i)
//      poke(dut.io.memPorts(j).wrData, 0x100 * (j + 1) + i)
//      poke(dut.io.memPorts(j).wrEna, 1)
//    }
//    step(1)
//  }
//  for (j <- 0 until 4) {
//    poke(dut.io.memPorts(j).wrEna, 0)
//  }
//  // read(0)
//  step(300)
//  read(0)
}

object SharedSPMTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new SharedSPM(3))) {
        c => new SharedSPMTester(c)
      }
  }
}
