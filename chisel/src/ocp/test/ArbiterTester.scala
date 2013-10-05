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
import patmos._

import scala.collection.mutable.HashMap


class Master(nr: Int, burstLength: Int) extends Component {
  
  val io = new Bundle {
    val port = new OcpBurstMasterPort(32, 32, burstLength)
  }
  
  val cntReg = Reg(resetVal = UFix(0, width=8))

  io.port.M.Cmd := OcpCmd.IDLE
  io.port.M.DataValid := Bits(0)
  io.port.M.DataByteEn := Bits(15)

  cntReg := cntReg + UFix(1)
  switch(cntReg) {
    is(UFix(1)) {
      io.port.M.Cmd := OcpCmd.WR
      io.port.M.DataValid := Bits(1)
      when (io.port.S.CmdAccept === Bits(0)) {
        cntReg := cntReg
      }
    }
    is(UFix(2)) {
      io.port.M.DataValid := Bits(1)      
    }
    is(UFix(3)) {
      io.port.M.DataValid := Bits(1)
    }
    // now we should be on our last word - wait for DVA
    is(UFix(4)) {
      io.port.M.DataValid := Bits(1)
      when (io.port.S.Resp != OcpResp.DVA) {
        cntReg := cntReg
      }
    }
    is(UFix(5)) { io.port.M.Cmd := OcpCmd.IDLE }
    is(UFix(6)) { io.port.M.Cmd := OcpCmd.RD }
  }
  
  io.port.M.Addr := (UFix(nr * 256) + cntReg).toBits()
  io.port.M.Data := (UFix(nr * 256 * 16) + cntReg).toBits()
}

/** A top level to test the arbiter */
class ArbiterTop() extends Component {
  
  val io = new Bundle {
    val port = new OcpBurstMasterPort(32, 32, 4)
  }
  val CNT = 3
  val arb = new ocp.Arbiter(CNT, 4)
  val mem = new SsramBurstRW()
  
  for (i <- 0 until CNT) {
    val m = new Master(i, 4)
    arb.io.master(i) <> m.io.port
  }
  
  mem.io.ocp_port <> arb.io.slave
  
  io.port.M <> arb.io.slave.M

}


class ArbiterTester(dut: ocp.test.ArbiterTop) extends Tester(dut, Array(dut.io)) {
  defTests {
    val ret = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()

    val testVec = Array( OcpCmd.IDLE, OcpCmd.WR, OcpCmd.IDLE )

    for (i <- 0 until 25) {
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
    chiselMainTest(args, () => new ocp.test.ArbiterTop) {
      f => new ArbiterTester(f)
    }

  }
}
