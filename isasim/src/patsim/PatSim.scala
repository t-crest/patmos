/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
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
 * A simple ISA simulator of Patmos.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 * TODO:
 *
 * 
 */

package patsim

import scala.io.Source
import scala.collection.mutable.Map

class PatSim(instructions: Array[Int]) {

  var pc = 0
  var reg = new Array[Int](32)
  reg(0) = 0

  def tick() = {
    val instr = instructions(pc)
    val op = instr & 0x1f
    op match {
      case 1 => println("one")
      case _ => {
        println("the rest")
      }
    }
    log
    pc += 1
  }

  def error(s: String) {
    println("Error: " + s)
    System.exit(-1)
  }

  def log() = {
    println(pc + ": " + "r1 = " + reg(1))
  }
}

object PatSim {

  def main(args: Array[String]) = {
    println("Simulating Patmos")
    if (args.length != 1)
      throw new Error("Wrong Arguments, usage: PatSim file")
    val instr = readBin(args(0))
    val simulator = new PatSim(instr)
    for (i <- 1 to 3) {
      simulator.tick()
    }
  }

  /**
   * Read a binary file into an Array
   */
  def readBin(fileName: String): Array[Int] = {

    println("Reading " + fileName)
    // an encoding to read a binary file? Strange new world.
    val source = scala.io.Source.fromFile(fileName)(scala.io.Codec.ISO8859)
    val byteArray = source.map(_.toByte).toArray
    source.close()

    // binary file is multiple of 4 bytes, so this length/4 is ok
    val arr = new Array[Int](math.max(1, byteArray.length / 4))

    if (byteArray.length == 0) {
      arr(0) = 0
    }

    for (i <- 0 until byteArray.length / 4) {
      var word = 0
      for (j <- 0 until 4) {
        word <<= 8
        word += byteArray(i * 4 + j).toInt & 0xff
      }
      printf("%08x\n", word)
      arr(i) = word
    }

    // return the array
    arr
  }
}
