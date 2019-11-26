/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package oneway

import Chisel._

/**
 * Test a 2x2 Network.
 */
 /*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class OneWayMemTester(dut: OneWayMem) extends Tester(dut) {

  def read(n: Int) = {
    for (i <- 0 until 4*4) {
      poke(dut.io.memPorts(n).rdAddr, i)
      step(3)
      peek(dut.io.memPorts(n).rdData)
    }
  }

  for (i <- 0 until 32) {
    for (j <- 0 until 4) {
      poke(dut.io.memPorts(j).wrAddr, i)
      poke(dut.io.memPorts(j).wrData, 0x100 * (j + 1) + i)
      poke(dut.io.memPorts(j).wrEna, 1)
    }
    step(1)
  }
  for (j <- 0 until 4) {
    poke(dut.io.memPorts(j).wrEna, 0)
  }
  // read(0)
  step(300)
  read(0)
}

object OneWayMemTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new OneWayMem(2, 16))) {
        c => new OneWayMemTester(c)
      }
  }
}
*/