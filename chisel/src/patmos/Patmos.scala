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
 * Patmos top level component and test driver.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._

import scala.collection.mutable.HashMap

class FetchIO(addrBits: Int) extends Bundle()
{
  val instr_a = Bits(OUTPUT, 32)
  val instr_b = Bits(OUTPUT, 32)
  val b_valid = Bool(OUTPUT)
  val pc = UFix(OUTPUT, addrBits)
}

class Fetch(addrBits: Int) extends Component {
  val io = new FetchIO(addrBits)
  
  val pc = Reg(resetVal = UFix(0, addrBits))
  
  pc := pc+UFix(1)
  
  io.pc := pc
}

/**
 * The main (top-level) component of Patmos.
 */
class Patmos() extends Component {
  val io = new Bundle {
    val led = Bits(OUTPUT, 8)
  }

  val fetch = new Fetch(10)
  // maybe instantiate the FSM here to get some output when
  // compiling for the FPGA
  
  val led = Reg(resetVal = Bits(1, 8))
  val led_next = Cat(led(6, 0), led(7))

  when(Bool(true)) {
    led := led_next
  }
  io.led := ~led | fetch.io.pc(7, 0)
}

// this testing and main file should go into it's own folder

class PatmosTest(pat: Patmos) extends Tester(pat, Array(pat.io, pat.fetch.io)) {
  defTests {
    val ret = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()

    for (i <- 0 until 10) {
      vars.clear
      step(vars, ovars)
      //      println("iter: " + i)
      //      println("ovars: " + ovars)
      println("led/litVal " + ovars(pat.io.led).litValue())
      println("pc" + ovars(pat.fetch.io.pc).litValue())
    }
    ret
  }
}

object PatmosMain {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => new Patmos()) { f => new PatmosTest(f) }
  }
}
