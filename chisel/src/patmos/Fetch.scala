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
 * Fetch stage of Patmos.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._

class Fetch(fileName: String) extends Component {
  val io = new FetchIO()

  // using a vector for a ROM
  val v = Vec(256) { Bits(width = 32) }
  // should check the program for the size

  // TODO: move ROM file reading to an utility class
  println("Reading " + fileName)
  // an encodig to read a binary file? Strange new world.
  val source = scala.io.Source.fromFile(fileName)(scala.io.Codec.ISO8859)
  val byteArray = source.map(_.toByte).toArray
  source.close()
  for (i <- 0 until byteArray.length/4) {
    var word = 0
    for (j <- 0 until 4) {
      word <<= 8
      word += byteArray(i*4 +j).toInt & 0xff
    }
    printf("%08x\n", word)
    v(i) = Bits(word)
  }
  
  // generate some dummy data to fill the table
  for (x <- byteArray.length/4 until 256)
    v(x) = Bits(x * x + 10 + ((x - 2) << 24))
  
  val rom = v

  val pc_next = UFix()
  val pc = Reg(resetVal = UFix(0, Constants.PC_SIZE))
  when(io.ena) {
    pc := pc_next
  }
  
  pc_next := pc + UFix(1)
  when(io.exfe.doBranch) {
    pc := io.exfe.branchPc
  } 

  io.fedec.pc := pc
  io.fedec.instr_a := rom(pc)
}