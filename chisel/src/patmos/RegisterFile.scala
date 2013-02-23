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
 * Register file for Patmos.
 * 
 * Needs to be extended to support two ALUs
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._



class RegisterFile() extends Component {
  val io = new RegFileIO()
  
//  val mem1 = Mem(32, seqRead=true) { Bits(width=32) }
//  val mem2 = Mem(32, seqRead=true) { Bits(width=32) }
//  
//  // is it really modeled as output register?
//  // I would prefer address registers
//  val d1 = Reg() { Bits(width=32) }
//  val d2 = Reg() { Bits(width=32) }
//
//  // Where is the register for write?
//  when(io.rfWrite.wrEn) {
//    mem1(io.rfWrite.wrAddr.toUFix) := io.rfWrite.wrData
//    mem2(io.rfWrite.wrAddr.toUFix) := io.rfWrite.wrData
//  }
//  // Who is looking for read during write hazard?
//  // R0 handling could be done here, in decode, or as part of forwarding
//  d1 := mem1(io.rfRead.rs1Addr.toUFix)
//  d2 := mem2(io.rfRead.rs2Addr.toUFix)

//  val rf = Vec(32){ Reg(resetVal = Bits(0, width = 32)) }
//  val rf = Vec(32) { Bits(width=32) }
//  for (i <- 0 until 32) {
//    rf(i) = Reg() { Bits(width=32) }
//  }

  val rf = Vec(32){ Reg() { Bits(width = 32) } }
  
  when(io.rfWrite.wrEn) {
    rf(io.rfWrite.wrAddr.toUFix) := io.rfWrite.wrData
  }

  val data = Vec(2) { Bits(width=32) }
  data(0) := rf(io.rfRead.rs1Addr.toUFix)
  data(1) := rf(io.rfRead.rs2Addr.toUFix)
  // maybe do register 0 here
  // R0 handling could be done here, in decode, or as part of forwarding
  // Or we are just happy with relying on the fact that the registers are reset
  // and just disable writing to register 0
  
  
//  class RFile extends Bundle() {
//    val regs = Vec(32) { Bits(width=32) }
//  }
//  val rf = Reg(new RFile())
//  
//  val d1 = rf.regs(io.rfRead.rs1Addr.toUFix)
//  val d2 = rf.regs(io.rfRead.rs2Addr.toUFix)
  
//  val rf = new Array[{ Bits(width=32) }](32)
//  for (i <= 0 until 32)
//    rf(i) = new Reg() { Bits(width=32)}

//  val rf = Vec(Reg { Bits(width=32)}, Reg { Bits(width=32)})
//  val d1 = rf(io.rfRead.rs1Addr(0,0).toUFix)
//  // Where is the register for write?
//  when(io.rfWrite.wrEn) {
//    mem1(io.rfWrite.wrAddr.toUFix) := io.rfWrite.wrData
//    mem2(io.rfWrite.wrAddr.toUFix) := io.rfWrite.wrData
//  }
//  // Who is looking for read during write hazard?
//  // R0 handling could be done here, in decode, or as part of forwarding
//
//  d1 := mem1(io.rfRead.rs1Addr.toUFix)
//  d2 := mem2(io.rfRead.rs2Addr.toUFix)

  io.rfRead.rs1Data := data(0)
  io.rfRead.rs2Data := data(1)
  
}