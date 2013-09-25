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
 * Start for OCP arbiter tests.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package ocp.test

import Chisel._
import Node._

import ocp._

import scala.collection.mutable.HashMap

//class BComponent() extends Component {
//  val io = new Bundle {
//    val fromMaster = new OcpBurstSlavePort(32, 32, 4)
//    val toSlave = new OcpBurstMasterPort(32, 32, 4)
//  }
//
//  io.fromMaster <> io.toSlave
//}

class ArbiterTester(dut: ocp.Arbiter) extends Tester(dut, Array(dut.io)) {
  defTests {
    val ret = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()

    val testVec = Array( OcpCmd.IDLE, OcpCmd.WR, OcpCmd.IDLE )

    for (i <- 0 until 10) {
      vars.clear
//      vars(dut.io.fromMaster.M.Cmd) = testVec(i)

//      vars(dut.io.slave.S.CmdAccept) = Bits(1)
//      vars(dut.io.slave.S.DataAccept) = Bits(1)
      step(vars, ovars)
//      println("out data: " + ovars(dut.io.slave))
      //      println("iter: "+i)
      //      println("vars: "+vars)
      //      println("ovars: "+ovars)
    }
    ret
  }
}

object ArbiterTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => new ocp.Arbiter(3, 4)) {
      f => new ArbiterTester(f)
    }

  }
}
