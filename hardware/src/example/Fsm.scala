/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Just a minimal 'hello world' Chisel example.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package example

import Chisel._
import Node._

import scala.collection.mutable.HashMap

class FsmContainer() extends Module {
  val io = new Bundle {
    val led = Bits(OUTPUT, 8)
  }

  var tck = Module(new Tick());

  val led = Reg(init = Bits(1, 8))
  val led_next = Cat(led(6, 0), led(7))

  when (tck.io.tick === Bits(1)) {
    led := led_next
  }
  io.led := ~led
}

/**
 * Generate a 2 Hz tick to drive the FSM input test bench.
 */
class Tick() extends Module {
  val io = new Bundle {
    val tick = Bits(OUTPUT, 1)
  }
  // BeMicro has a 16 MHz clock
  val CNT_MAX = UInt(16000000/2-1)

  val r1 = Reg(init = UInt(0, 25))

  val limit = r1 === CNT_MAX
  val tick = limit

  r1 := r1 + UInt(1)
  when (limit) {
    r1 := UInt(0)
  }

  io.tick := tick

  // **** uninteresting stuff below ****

// the following does not work - example in package error
//  val ticka = when(limit) { Bits(0) } .otherwise { Bits(1) }
//  val tickb = when(limit) { Bits(1) } .otherwise { Bits(0) }

// this is a very verbose version
//  val ticka = Bits(width=1)
//  ticka := Bits(0)
//  when (limit) { ticka := Bits(0, 1) } .otherwise{ ticka := Bits(1, 1) }
//  val tickb = Bits(width=1)
//  tickb := Bits(0)
//  when (limit) { tickb := Bits(1, 1) } .otherwise{ tickb := Bits(0, 1) }

  // That's the MUX version. Do we know which is which? True first? 0 first?
  // val ticka = Mux(limit, Bits(1), Bits(0))

  // no if in Chisel
  // val ticka = if (limit) then Bits(1) else Bits(0)

}

class FsmTest(fsm: FsmContainer) extends Tester(fsm) {
  //val ret = true
  //val vars = new HashMap[Node, Node]()
  //val ovars = new HashMap[Node, Node]()
  var test = 1;
  step(16000000/2-1)
  for (i <- 0 until 7) {
    expect(c.io.led, (~test)&0x00FF)
    step(16000000/2-1)
    test = test << 1
//    vars.clear
//    step(vars, ovars)
//    println("iter: "+i)
//    println("ovars: "+ovars)
//    // where does the 'c' come from? Why does this work?
//    println("led/litVal "+ovars(c.io.led).litValue())
////      println("led/litVal "+ovars(io.led).litValue())
//    println("led/litVal "+ovars(fsm.io.led).litValue())
//    // the following does not compile
////      println("led/litVal "+ovars(xyz.io.led).litValue())
  }
  expect(c.io.led, (~test)&0x00FF)
}

object FsmMain {
  def main(args: Array[String]): Unit = {
    // chiselMain(args, () => Module(new FsmContainer()));
    // Looks like the MainTest is also fine for Verilog code generation
    chiselMainTest(args, () => Module(new FsmContainer())) {
      f => new FsmTest(f)
    }

  }
}
