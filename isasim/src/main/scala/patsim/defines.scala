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
 * A simple ISA simulator of Patmos. Constant definitions.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 * 
 */

package patsim

/**
 * Opcodes are 5 bit, Arithmetic short immediate use 3 of the 5 bits for
 * for the function.
 */
object Opcode {
  val AluImm = 0x00
  val Alu = 0x08
  val AluLongImm = 0x1f
  val Branch = 0x13
  val BranchCf = 0x15
}
/**
 * The Opc field. Not sure if I like two opcode fields
 */
object OpcodeExt {
  val AluReg = 0x00
  val AluMul = 0x02
}

/**
 * Function for an ALU operation
 */
object Function {
  val ADD = 0x0
  val SUB = 0x1
  val XOR = 0x2
  val SL = 0x3
  val SR = 0x4
  val SRA = 0x5
  val OR = 0x6
  val AND = 0x7
  val NOR = 0xb
  val SHADD = 0xc
  val SHADD2 = 0xd
}
/**
 * These constants could be shared between the hardware
 * definition and the simulation.
 */
object Constants {



  // only two bits for immediate
//  val OPCODE_ALUI = 0x0
//
//  val OPCODE_ALU = 0x08
  val OPCODE_SPC = 0x09
  val OPCODE_LDT = 0x0a
  val OPCODE_STT = 0x0b
  val OPCODE_STC = 0x0c

//  val OPCODE_ALUL = 0x1f

  val OPCODE_CFL_LOW = 0x10
  // opcode for control flow is 4 bits plus delayed bit
  val CFLOP_CALL = 0x8
  val CFLOP_BR = 0x9
  val CFLOP_BRCF = 0xa
  val CFLOP_TRAP = 0xb
  val CFLOP_CFLR = 0xc

  // additional field for ALU type instructions
  val OPC_ALUR = 0x0
  val OPC_ALUU = 0x1
  val OPC_ALUM = 0x2
  val OPC_ALUC = 0x3
  val OPC_ALUP = 0x4
  val OPC_ALUB = 0x5
  val OPC_ALUCI = 0x6

  // special register handling
  val OPC_MTS = 0x2
  val OPC_MFS = 0x3

}
