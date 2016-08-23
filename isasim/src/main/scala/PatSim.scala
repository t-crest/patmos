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

import Constants._

class PatSim(instructions: Array[Int]) {

  var pc = 1 // method length at address 0
  var reg = new Array[Int](32)
  reg(0) = 0

  var halt = false

  val NOP = 0 // Using R0 as destination is a noop

  def tick() = {
    val instrA = instructions(pc)
    val dualFetch = (instrA & 0x80000000) != 0
    val longImmInstr = (((instrA >> 22) & 0x1f) == 0x1f)
    val dualIssue = dualFetch && !longImmInstr

    val instrB = if (dualFetch) {
      instructions(pc + 2)
    } else {
      NOP
    }
    execute(instrA)
    if (dualIssue) execute(instrB)
    if (dualFetch) pc += 2 else pc += 1
  }

  def alu(func: Int, op1: Int, op2: Int): Int = {

    val scale = if (func == FUNC_SHADD) {
      1
    } else if (func == FUNC_SHADD2) {
      2
    } else {
      0
    }
    val scaledOp1 = op1 << scale

    val sum = scaledOp1 + op2
    var result = sum // some default
    val shamt = op2 & 0x1f
    // is there a more functional approach for this?
    func match {
      case FUNC_ADD => result = sum
      case FUNC_SUB => result = op1 - op2
      case FUNC_XOR => result = op1 ^ op2
      case FUNC_SL => result = op1 << shamt
      case FUNC_SR => result = op1 >>> shamt
      case FUNC_SRA => result = op1 >> shamt
      case FUNC_OR => result = op1 | op2
      case FUNC_AND => result = op1 & op2
      case FUNC_NOR => result = ~(op1 | op2)
      case FUNC_SHADD => result = sum
      case FUNC_SHADD2 => result = sum
      case _ => result = sum
    }
    result
  }

  def execute(instr: Int) = {

    val pred = (instr >> 27) & 0x0f
    val opcode = (instr >> 22) & 0x1f
    val rd = (instr >> 17) & 0x1f
    val rs1 = (instr >> 12) & 0x1f
    val rs2 = (instr >> 7) & 0x1f
    val opc = (instr >> 4) & 0x07
    val aluImm = (opcode >> 3) == 0
    val func = if (aluImm) {
      opcode & 0x7
    } else {
      instr & 0xf
    }

    val op1 = reg(rs1)
    val op2 = if (aluImm) {
      // always sign extend?
      (instr << 20) >> 20
    } else {
      reg(rs2)
    }
    val aluResult = alu(func, op1, op2)
    val doExecute = rd != 0 // add predicates, but R0 only on ops with rd

    // default, which is ok for ALU immediate as well
    var result = aluResult

    if (aluImm) {
      result = aluResult
    } else if (opcode < OPCODE_CFL_LOW) {
      opcode match {
        case OPCODE_ALU => {
          opc match {
            case OPC_ALUR => result = aluResult
            case _ => println(opc + " not implemented (opc)")
          }
        }
        case _ => println(opcode + " not implemented")
      }
    } else {
      if (((opcode >> 1) == CFLOP_BRCF) && ((instr & 0x3ffff) == 0)) {
        // 'halt' instruction
        halt = true
      } else {
        println("Unimplemented control flow " + opcode + " " + (opcode >> 1))
      }
    }
    // write result back
    // -- need to distinguish between doExecute, write back, R0...
    if (doExecute) {
      reg(rd) = result
    }
    log
  }

  def executeLong(instr: Int, imm: Int) {

  }

  def error(s: String) {
    println("Error: " + s)
    System.exit(-1)
  }

  def log() = {
    print(pc + ":")
    for (i <- 0 to 8) {
      print(" r" + i + " = " + reg(i))
    }
    println
  }
}

object PatSim {

  def main(args: Array[String]) = {
    println("Simulating Patmos")
    if (args.length != 1)
      throw new Error("Wrong Arguments, usage: PatSim file")
    val instr = readBin(args(0))
    val simulator = new PatSim(instr)
    var cnt = 1;
    while (!simulator.halt && cnt < instr.length) {
      simulator.tick()
      cnt += 1
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
