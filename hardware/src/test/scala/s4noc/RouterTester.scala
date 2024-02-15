/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package s4noc

import Chisel._

/**
 * Test the router by printing out the value at each clock cycle
 * and checking some known end values.
 */
 /*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class RouterTester(c: S4Router[UInt]) extends Tester(c) {

  for (i <- 0 until 5) {
    poke(c.io.ports(0).in.data, 0x10 + i)
    poke(c.io.ports(1).in.data, 0x20 + i)
    poke(c.io.ports(2).in.data, 0x30 + i)
    poke(c.io.ports(3).in.data, 0x40 + i)
    poke(c.io.ports(4).in.data, 0x50 + i)
    poke(c.io.ports(0).in.valid, 1)    
    poke(c.io.ports(1).in.valid, 1)    
    poke(c.io.ports(2).in.valid, 1)    
    poke(c.io.ports(3).in.valid, 1)    
    poke(c.io.ports(4).in.valid, 1)    
    step(1)
    println(peek(c.io.ports))
  }
  expect(c.io.ports(0).out.data, 0x14)
  expect(c.io.ports(4).out.data, 0x34)

}

object RouterTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new S4Router(Schedule.getSchedule(2)._1, UInt(16.W)))) {
        c => new RouterTester(c)
      }
  }
}*/
