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
 * Constants for Patmos.
 * 
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package patmos

import Chisel._
import Node._

object Constants {

  // TODO: this should come from some configuration file
  val CLOCK_FREQ = 100000000

  val UART_BAUD = 115200
  val LED_COUNT = 8
  val KEY_COUNT = 4

  // Exceptions/interrupts
  val EXC_SRC_BITS = 5
  val EXC_COUNT  = 1 << EXC_SRC_BITS
  val INTR_COUNT = 16

//  val SPM_MAX_BITS = 21
  // MS: maybe better use sizes in bytes and use log2Up() to
  // get the number of bits.
  // XX_BITS might confuse as the ISPM is origanized in words,
  // even in 2 times words, which results in other number of
  // bits.
  val ISPM_BITS = 15
  val DSPM_BITS = 14
  
  // we use now a very simple decode of ISPM ad address 0x00800000
  // this is the one bit in byte address counting
  val ISPM_ONE_BIT = 23

  // The PC counts in words. 30 bits are enough for the 4 GB address space.
  // We might cut that down to what we actually really support (16 MB)
  val PC_SIZE = 30

  val PIPE_COUNT = 2

  val REG_BITS = 5
  val REG_COUNT = 1 << REG_BITS

  val PRED_BITS = 3
  val PRED_COUNT = 1 << PRED_BITS

  val INSTR_WIDTH = 32
  val DATA_WIDTH = 32
  val ADDR_WIDTH = 32

  val BYTE_WIDTH = 8
  val BYTES_PER_WORD = DATA_WIDTH/BYTE_WIDTH

  val OPCODE_ALUI     = Bits("b00")
  val OPCODE_ALU      = Bits("b01000")
  val OPCODE_SPC      = Bits("b01001")
  val OPCODE_LDT      = Bits("b01010")
  val OPCODE_STT      = Bits("b01011")

  val OPCODE_STC      = Bits("b01100")

  val OPCODE_CFL_CALL = Bits("b11000")
  val OPCODE_CFL_BR   = Bits("b11001")
  val OPCODE_CFL_BRCF = Bits("b11010")
  val OPCODE_CFL_CFLI = Bits("b11100")
  val OPCODE_CFL_TRAP = Bits("b11101")
  val OPCODE_CFL_RET  = Bits("b11110")

  val OPCODE_ALUL     = Bits("b11111")

  val OPC_ALUR = Bits("b000")
  val OPC_ALUU = Bits("b001")
  val OPC_ALUM = Bits("b010")
  val OPC_ALUC = Bits("b011")
  val OPC_ALUP = Bits("b100")

  val OPC_MTS  = Bits("b010")
  val OPC_MFS  = Bits("b011")

  val MSIZE_W  = Bits("b000")
  val MSIZE_H  = Bits("b001")
  val MSIZE_B  = Bits("b010")
  val MSIZE_HU = Bits("b011")
  val MSIZE_BU = Bits("b100")

  val MTYPE_S  = Bits("b00")
  val MTYPE_L  = Bits("b01")
  val MTYPE_C  = Bits("b10")
  val MTYPE_M  = Bits("b11")

  val FUNC_ADD    = Bits("b0000")
  val FUNC_SUB    = Bits("b0001")
  val FUNC_XOR    = Bits("b0010")
  val FUNC_SL     = Bits("b0011")
  val FUNC_SR     = Bits("b0100")
  val FUNC_SRA    = Bits("b0101")
  val FUNC_OR     = Bits("b0110")
  val FUNC_AND    = Bits("b0111")
  val FUNC_NOR    = Bits("b1011")
  val FUNC_SHADD  = Bits("b1100")
  val FUNC_SHADD2 = Bits("b1101")

  val MFUNC_MUL   = Bits("b0000")
  val MFUNC_MULU  = Bits("b0001")

  val CFUNC_EQ    = Bits("b0000")
  val CFUNC_NEQ   = Bits("b0001")
  val CFUNC_LT    = Bits("b0010")
  val CFUNC_LE    = Bits("b0011")
  val CFUNC_ULT   = Bits("b0100")
  val CFUNC_ULE   = Bits("b0101")
  val CFUNC_BTEST = Bits("b0110")

  val PFUNC_OR    = Bits("b00")
  val PFUNC_AND   = Bits("b01")
  val PFUNC_XOR   = Bits("b10")
  val PFUNC_NOR   = Bits("b11")

  val JFUNC_CALL  = Bits("b0000")
  val JFUNC_BR    = Bits("b0001")
  val JFUNC_BRCF  = Bits("b0010")

  val RFUNC_RET   = Bits("b0000")
  val RFUNC_XRET  = Bits("b0001")

  val SPEC_FL  = Bits("b0000")
  val SPEC_SL  = Bits("b0010")
  val SPEC_SH  = Bits("b0011")
  val SPEC_SS  = Bits("b0101")
  val SPEC_ST  = Bits("b0110")

  val SPEC_SXB = Bits("b1010")
  val SPEC_SXO = Bits("b1011")

  val STC_SRES  = Bits("b0000")
  val STC_SFREE = Bits("b1000")
}
