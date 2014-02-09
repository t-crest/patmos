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
 * TODO:
 *   don't forget assembler directive
 *   Predicates
 *   Long immediate needs some special handling
 *
 * 
 */

package tools

import scala.io.Source
import scala.collection.mutable.Map

object InstrType extends Enumeration {
  //  type InstrType = Value
  val ALUr, ALUi, ASM = Value
}

class Instruction {

  var mne: String = ""
  var code: Int = 0
  var isA: InstrType.Value = InstrType.ALUr

  val funcMap = Map("add" -> 0,
    "sub" -> 1,
    "xor" -> 2,
    "sl" -> 3,
    "sr" -> 4,
    "sra" -> 5,
    "or" -> 6,
    "and" -> 7,
    "not" -> 11,
    "shadd" -> 12,
    "shadd2" -> 13)

  def regVal(s: String) = {
    if (s(0) != 'r') error("Register expected")
    val nr = s.substring(1).toInt
    if (nr > 31) error("Impossible register number " + nr)
    nr
  }

  def getCode = { code }
  def getType = { isA }

  def getCode(p1: String, p2: String, p3: String): Int = {
    -1
  }
  def getCode(p1: String): Int = {
    -1
  }
}

class AluRInstruction(name: String) extends Instruction {

  mne = name
  code = 0x02000000 + funcMap(name)
  isA = InstrType.ALUr

  override def getCode(p1: String, p2: String, p3: String): Int = {
    code + (regVal(p1) << 17) + (regVal(p2) << 12) + (regVal(p3) << 7)
  }

}

class AluIInstruction(name: String) extends Instruction {

  mne = name
  val l = name.length
  val func = name.substring(0, l - 1)
  code = 0x00000000 + (funcMap(func) << 22)
  isA = InstrType.ALUi

}

class AsmInstruction(name: String, n: Int) extends Instruction {

  mne = name
  code = n
  isA = InstrType.ASM
  
  override def getCode(p1: String): Int = {
    code + p1.toInt
  }
}

object Instruction {

  def getMap = {
    import InstrType._
    val map = Map.empty[String, Instruction]
    map += ("add" -> new AluRInstruction("add"))
    map += ("addi" -> new AluIInstruction("addi"))
    map += (".word" -> new AsmInstruction(".word", 0))
    map
  }
}

class ConcreteInstruction(typeOf: Instruction) {

  var code = typeOf.getCode
//  def getType = typeOf
}

class Bundle {
  var instrA: ConcreteInstruction = null
  var instrB: ConcreteInstruction = null
  var address = 0
}

class PaAsm(file: String) {

  val instrMap = Instruction.getMap

  val symbolMap = Map.empty[String, Int]
  var pc = 0

  for (line <- Source.fromFile(file).getLines()) {
    parseLine(line, false)
  }
  val mem = new Array[Int](pc / 4)
  pc = 0
  for (line <- Source.fromFile(file).getLines()) {
    val bundle = parseLine(line, true)
    if (bundle != null) {
      mem(bundle.address / 4) = bundle.instrA.code
    }
  }
  for (cnt <- 0 to mem.length - 1) {
    println(mem(cnt))
  }

  def regVal(s: String) = {
    if (s(0) != 'r') error("Register expected")
    val nr = s.substring(1).toInt
    if (nr > 31) error("Impossible register number " + nr)
    nr
  }

  def parseInstr(instrStr: String): ConcreteInstruction = {

    if (instrStr == null) null

    val split = instrStr.split(Array(' ', '\t', '=', ',')).toList
    // remove this strange zero size string
    val tokens = split.filter(s => s.length != 0)
    if (tokens.length == 0) {
      println("Empty instruction")
      return null
    }

    if (!instrMap.contains(tokens(0))) {
      println("unknown instr: " + tokens(0))
      return null
      //      error("Unknown instruction "+tokens(0))      
    }

    val instruction = instrMap(tokens(0))
    val instr = new ConcreteInstruction(instruction)

    import InstrType._
    println(instruction.getType)

    instruction.getType match {
      case ASM => {
        instr.code = instruction.getCode(tokens(1))
      }
      case ALUr => {
        instr.code = instruction.getCode(tokens(1), tokens(2), tokens(3))
      }
      case _ => {}
    }

    for (s <- tokens) {

      print(":" + s + ":")
      if (s.length() == 0) {
        // looks like a zero sized String is at the end
        printf("empty line")
      } else {
        printf("?")
      }
      println
    }

    instr
  }

  def parseLine(line: String, secondPath: Boolean) = {

    val bundle = new Bundle()

    println(line)

    // do dumb for case classes, but assembler syntax is easy to parse
    var clean = line
    var second: String = null
    var pos = -1
    // cut off comment
    pos = line.indexOf('#')
    if (pos != -1) {
      clean = line.substring(0, pos)
    }
    // remove unused ';' - assembler syntax uses end of line as delimiter
    clean = clean.replace(";", "")
    clean = clean.replace("\n", "")
    clean = clean.replace("\r", "")
    // consume label
    println("Cleand up " + clean)
    pos = clean.indexOf(":")
    if (pos != -1) {
      if (!secondPath) {
        val s = clean.substring(0, pos)
        if (symbolMap.contains(s)) {
          error("Symbol " + s + " already defined")
        }
        symbolMap += (s -> pc)
        println("Label: " + s)
      }
      clean = clean.substring(pos + 1)
    }
    // bundle?
    var first = clean
    pos = clean.indexOf("||")
    if (pos != -1) {
      first = clean.substring(0, pos)
      second = clean.substring(pos + 2)
    }

    bundle.address = pc
    bundle.instrA = parseInstr(first)
    if (second != null) {
      bundle.instrB = parseInstr(second)
    }

    if (bundle.instrA == null) {
      println("Empty bundle")
      null
    } else {
      pc = pc + 4
      if (bundle.instrB != null) pc = pc + 4
      bundle
    }
  }

  def error(s: String) {
    println("Error: " + s)
    System.exit(-1)
  }
}

object PaAsm {

  def main(args: Array[String]) = {
    println("Hello, world!")
    val assembler = new PaAsm("/Users/martin/t-crest/patmos/asm/test_asm.s")
  }
}