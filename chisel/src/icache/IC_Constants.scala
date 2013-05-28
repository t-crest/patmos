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
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package icache

object Constants {
  // just now 32 to make it easy to use it as data dummy
  val PC_SIZE = 32

  //def log2(x: Double) = scala.math.log(x)/scala.math.log(2) //logarithm in scala???

  //on chip 4KB icache
  val ICACHE_SIZE = 4096 //* 8 //4KB = 2^12*2^3 = 32*1024 = 32768Bit

  //some of these should be generic
  val WORD_COUNT = 4
  val WORD_SIZE = 32 //not needed in bits
  val BLOCK_SIZE = WORD_COUNT * WORD_SIZE //not needed in bits
  val BLOCK_COUNT = ICACHE_SIZE / WORD_COUNT //--BLOCK_SIZE
  val VALIDBIT_FIELD_SIZE = 1  //could be enlarged like in leon where blocks are devided in subblocks?!
  val TAG_FIELD_SIZE = (32 - (8+2)) //32 - ((log2(BLOCK_COUNT) + log2(WORD_COUNT)) //32bit address - index field (8=address bits for block, 2=address bits for words
  val TAG_FIELD_HIGH = 31 //using 32 bit field address
  val TAG_FIELD_LOW = TAG_FIELD_HIGH - TAG_FIELD_SIZE + 1
  val INDEX_FIELD_HIGH = TAG_FIELD_LOW - 1
  val INDEX_FIELD_LOW = OFFSET_SIZE
  val INDEX_FIELD_SIZE = INDEX_FIELD_HIGH - INDEX_FIELD_LOW + 1
  val OFFSET_SIZE = 0 //could be added in case to address some subbytes in the block

  val TAG_CACHE_SIZE =  BLOCK_COUNT  // * (TAG_FIELD_SIZE + VALIDBIT_FIELD_SIZE)

  //for test only onchip double size of icache
  val EXTMEM_SIZE = 2 * ICACHE_SIZE // =32*2048

  //DEBUG INFO
  println("BLOCK_COUNT=" + BLOCK_COUNT)
  println("WORD_COUNT" + WORD_COUNT)
  println("TAG_FIELD_HIGH=" + TAG_FIELD_HIGH)
  println("TAG_FIELD_LOW=" + TAG_FIELD_LOW)
  println("TAG_FIELD_SIZE=" + TAG_FIELD_SIZE)
  println("INDEX_FIELD_LOW" + INDEX_FIELD_HIGH)
  println("INDEX_FIELD_HIGH" + INDEX_FIELD_LOW)
  println("INDEX_FIELD_SIZE" + INDEX_FIELD_SIZE)
  println("OFFSET_SIZE" + OFFSET_SIZE)
  println("TAG_CACHE_SIZE=" + TAG_CACHE_SIZE)
  println("EXT_MEM_SIZE=" + EXTMEM_SIZE)

}
