/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package twoway

import Chisel._

/**
 * Test a 2x2 Network.
 */
class TwoWayMemTester(dut: TwoWayMem) extends Tester(dut) {

  for (j <- 0 until 4) {
      poke(dut.nodesx(j).io.rwp, 1)  
      poke(dut.nodesx(j).io.datap, j)
      poke(dut.nodesx(j).io.addressp, 0x300 + j)
      poke(dut.nodesx(j).io.validp, true)
      
  }

  for (i <- 0 until 1){
    peek(dut.NIs(3).io.writeChannel.in.address)
    peek(dut.NIs(3).io.writeChannel.in.data)
    peek(dut.NIs(3).io.writeChannel.in.valid)
    
    step(1)
  }

  // read(0)
  step(300)
}

object TwoWayMemTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new TwoWayMem(2, 32))) {
        c => new TwoWayMemTester(c)
      }
  }
}