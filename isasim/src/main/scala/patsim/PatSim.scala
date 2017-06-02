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
 * Issues: Dual issue executes sequential, that means register write from
 * first instruction is seen for the second, but not the other way round.
 * What is the behavouir in HW? Probably read old value for both.
 */

package patsim

import Constants._
import Opcode._
import OpcodeExt._
import Function._

class PatSim(instructions: Array[Int]) {

  var pc = 1 // We start on second word as method length is at address 0
  var reg = new Array[Int](32)
  reg(0) = 0

  var halt = false

  def tick() = {
    val instrA = instructions(pc)
    val dualFetch = (instrA & 0x80000000) != 0

    val instrB = if (dualFetch) instructions(pc + 1) else 0

    val longImmInstr = (((instrA >> 22) & 0x1f) == AluLongImm)

    val dualIssue = dualFetch && !longImmInstr

    if (dualIssue) {
      execute(instrA, 0)
      execute(instrB, 0)
    } else {
      execute(instrA, instrB)
    }
  }

  def execute(instr: Int, longImm: Int) = {

    val pred = (instr >> 27) & 0x0f
    val opcode = (instr >> 22) & 0x1f
    val rd = (instr >> 17) & 0x1f
    val rs1 = (instr >> 12) & 0x1f
    val rs2 = (instr >> 7) & 0x1f
    val opc = (instr >> 4) & 0x07
    val imm22 = instr & 0x3ffff
    val aluImm = (opcode >> 3) == 0
    val func = if (aluImm) {
      opcode & 0x7
    } else {
      instr & 0xf
    }

    val op1 = reg(rs1)
    val op2 = if (aluImm) {
      // always sign extend?
      // Actually docu says zero extended - see if any test case caches this
      (instr << 20) >> 20
    } else {
      reg(rs2)
    }
    val pcNext = pc + 1

    def alu(func: Int, op1: Int, op2: Int): Int = {

      val scale = if (func == SHADD) {
        1
      } else if (func == SHADD2) {
        2
      } else {
        0
      }
      val scaledOp1 = op1 << scale

      val sum = scaledOp1 + op2
      val shamt = op2 & 0x1f
      func match {
        case ADD => sum
        case SUB => op1 - op2
        case XOR => op1 ^ op2
        case SL => op1 << shamt
        case SR => op1 >>> shamt
        case SRA => op1 >> shamt
        case OR => op1 | op2
        case AND => op1 & op2
        case NOR => ~(op1 | op2)
        case SHADD => sum
        case SHADD2 => sum
        case _ => sum
      }
    }

    // Execute the instruction and return a tuple for the result:
    //   (ALU result, writeReg, writePred, next PC)
    val result = if (aluImm) {
      (alu(func, op1, op2), true, false, pcNext)
    } else {
      opcode match {
        //      case AluImm => (alu(funct3, sraSub, rs1Val, imm), true, pcNext)
        case Alu => opc match {
          case AluReg => (alu(func, op1, op2), true, false, pcNext)
          case _ => throw new Exception("OpcodeExt " + opc + " not (yet) implemented")
        }
        case AluLongImm => (alu(func, op1, longImm), true, false, pcNext+1)
        case Branch => throw new Exception("Branch")
        case BranchCf => (0, false, false, imm22)
        //      case Branch => (0, false, if (compare(funct3, rs1Val, rs2Val)) pc + imm else pcNext)
        //      case Load => (load(funct3, rs1Val, imm), true, pcNext)
        //      case Store =>
        //        store(funct3, rs1Val, imm, rs2Val); (0, false, pcNext)
        //      case Lui => (imm, true, pcNext)
        //      case AuiPc => (pc + imm, true, pcNext)
        //      case Jal => (pc + 4, true, pc + imm)
        //      case JalR => (pc + 4, true, (rs1Val + imm) & 0xfffffffe)
        //      case Fence => (0, false, pcNext)
        //      case SCall => (scall(), true, pcNext)
        case _ => throw new Exception("Opcode " + opcode + " not (yet) implemented")
      }
    }

    // write result back
    // distinguish between R0, register update, predicate update,
    // and the special cases...
    if (rd != 0 && result._2) {
      reg(rd) = result._1
    }

    // Quick hack for the halt instruction
    if (result._4 == 0) {
      halt = true
    }

    log

    // increment program counter
    pc = result._4
  }

  def error(s: String) {
    println("Error: " + s)
    System.exit(-1)
  }

  def log() = {
    print(pc * 4 + " - ")
    for (i <- 0 to 31) {
      print(reg(i) + " ")
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
    println("Patmos start")
    while (!simulator.halt && cnt < instr.length) {
      simulator.tick()
      cnt += 1
    }
    println("PASSED")
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
