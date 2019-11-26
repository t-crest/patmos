/*
  Tester for the S4NOC.

  Author: Martin Schoeberl (martin@jopdesign.com)
  license see LICENSE
 */

package s4noc

import Chisel._
/*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class S4nocTester (dut: S4noc) extends Tester(dut) {

  def read(): Int = {
    poke(dut.io.cpuPorts(3).rd, 1)
    poke(dut.io.cpuPorts(3).addr, 3)
    val status = peek(dut.io.cpuPorts(3).rdData)
    var ret = 0
    step(1)
    if (status == 1) {
      poke(dut.io.cpuPorts(3).rd, 1)
      poke(dut.io.cpuPorts(3).addr, 0)
      ret = peek(dut.io.cpuPorts(3).rdData).toInt
      step(1)
      poke(dut.io.cpuPorts(3).rd, 1)
      poke(dut.io.cpuPorts(3).addr, 1)
      peek(dut.io.cpuPorts(3).rdData)
      step(1)
    }
    ret
  }

  poke(dut.io.cpuPorts(0).wrData, 0xcafebabe)
  poke(dut.io.cpuPorts(0).addr, 0)
  poke(dut.io.cpuPorts(0).wr, 1)
  step(1)
  poke(dut.io.cpuPorts(0).wrData, -1)
  poke(dut.io.cpuPorts(0).addr, -1)
  poke(dut.io.cpuPorts(0).wr, 0)
  var done = false
  for (i <- 0 until 14) {
    val ret = read()
    if (!done) done = ret == 0xcafebabe
  }
  if (!done) throw new Exception("Should have read in core 3 what core 0 wrote")
}

object S4nocTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new S4noc(4, 2, 2))) {
      c => new S4nocTester(c)
    }
  }
}*/
