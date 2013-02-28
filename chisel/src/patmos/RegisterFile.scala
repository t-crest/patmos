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

  // val rf = Vec(32){ Reg() { Bits(width = 32) } }
  // the reset version generates more logic and a slower fmax
  val rf = Vec(32) { Reg(resetVal = Bits(0, width = 32)) }

  // Shouldn't we also register the write signals to be
  // compatible with an on-chip memory?
  when(io.rfWrite.wrEn) {
    rf(io.rfWrite.wrAddr.toUFix) := io.rfWrite.wrData
  }

  // we are registering the addresses here, similar as it would
  // be with an on-chip memory for the register file
  val addr0 = Reg(io.rfRead.rsAddr(0).toUFix)
  val addr1 = Reg(io.rfRead.rsAddr(1).toUFix)
  io.rfRead.rsData(0) := rf(addr0)
  io.rfRead.rsData(1) := rf(addr1)
  // maybe do register 0 here
  // R0 handling could be done here, in decode, or as part of forwarding
  // Or we are just happy with relying on the fact that the registers are reset
  // and just disable writing to register 0

}