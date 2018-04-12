/*
  Tester for the S4NOC.

  Author: Martin Schoeberl (martin@jopdesign.com)
  license see LICENSE
 */

package s4noc

import Chisel._

class S4nocTester (dut: S4noc) extends Tester(dut) {

  poke(dut.io.cpuPorts(0).wrData, 0xcafebabe)
  poke(dut.io.cpuPorts(0).addr, 0)
  poke(dut.io.cpuPorts(0).wr, 1)
  step(1)
  poke(dut.io.cpuPorts(0).wrData, -1)
  poke(dut.io.cpuPorts(0).addr, -1)
  poke(dut.io.cpuPorts(0).wr, 0)
  for (i <- 0 until 8) {
    peek(dut.io.cpuPorts(3).rdData)
    step(1)
  }
}

object S4nocTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new S4noc(4))) {
      c => new S4nocTester(c)
    }
  }
}
