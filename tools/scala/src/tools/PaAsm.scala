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
 * Assembler functions for Patmos.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package tools

import scala.io.Source
import scala.collection.mutable.Map

class PaAsm {

}

object InstrType extends Enumeration {
  //  type InstrType = Value
  val ALU, ALUi = Value
}

class Instruction(mne: String, code: Int, isA: InstrType.Value) {

  def getCode = { code }
}

object Instruction {

  def getMap = {
    import InstrType._
    val map = Map.empty[String, Instruction]
    map + ("add" -> new Instruction("add", 123, ALU))
    map + ("addi" -> new Instruction("addi", 456, ALUi))
    map
  }
}

class Bundle {
  var label = ""
  //  var instrA 
}

object PaAsm {

  val instrMap = Instruction.getMap

  def doit(line: String) = {

    val bundle = new Bundle()

    println(line)

    // do dumb for case classes, but assembler syntax is easy to parse
    var clean = line
    var second = ""
    var pos = -1
    // cut off comment
    pos = line.indexOf('#')
    if (pos != -1) {
      clean = line.substring(0, pos)
    }
    // remove unused ';' - assembler syntax uses end of line as delimiter
    clean = clean.replace(";","")
    // consume label
    println("Cleand up "+clean)
    pos = clean.indexOf(":")
    if (pos != -1) {
      bundle.label = clean.substring(0, pos)
      println("Label " + bundle.label)
      clean = clean.substring(pos + 1)
    }
    // bundle?
    pos = clean.indexOf("||")
    if (pos != -1) {
      second = clean.substring(pos + 2)
      println("Second: "+second)
    }
    val tokens = clean.split(Array(' ', '\t'))
    var stop = false
    for (s <- tokens) {

      if (!stop) {
        print(":" + s + ":")
        val pos = s.indexOf(":");
        if (pos != -1) {
          bundle.label = s.substring(0, pos)
          printf("Label again" + bundle.label)
        } else if (s.length() == 0) {
          // looks like a zero sized String is at the end
          // printf("empty line")
        } else if (s(0) == '#') {
          printf("Comment")
          stop = true
        } else {
          printf("?")
        }
        println
      }
    }

    bundle
  }

  def main(args: Array[String]) = {
    println("Hello, world!")
    for (line <- Source.fromFile("/Users/martin/t-crest/patmos/asm/test_asm.s").getLines()) {
      doit(line)

    }
  }
}