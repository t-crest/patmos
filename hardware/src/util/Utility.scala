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
 * Utility functions for Patmos.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package patmos

import Chisel._
import Node._
import scala.math._

import Constants._

// Borrowed from the Chisel source tree.
// Hacked to now support longs
object log2Up
{
  def apply(in: Long): Int = if(in == 1) 1 else ceil(log(in)/log(2)).toInt
}

object Utility {

  /**
   * Read a binary file into the ROM vector
   */
  def readBin(fileName: String, width: Int): Vec[Bits] = {

    val bytesPerWord = (width+7) / 8

    println("Reading " + fileName)
    // an encoding to read a binary file? Strange new world.
    val source = scala.io.Source.fromFile(fileName)(scala.io.Codec.ISO8859)
    val byteArray = source.map(_.toByte).toArray
    source.close()

    // use an array to convert input
    val arr = new Array[Bits](math.max(1, byteArray.length / bytesPerWord))

    if (byteArray.length == 0) {
      arr(0) = Bits(0, width = width)
    }

    for (i <- 0 until byteArray.length / bytesPerWord) {
      var word = BigInt(0)
      for (j <- 0 until bytesPerWord) {
        word <<= 8
        word += byteArray(i * bytesPerWord + j).toInt & 0xff
      }
      // printf("%08x\n", Bits(word))
      arr(i) = Bits(word, width = width)
    }

    // use vector to represent ROM
    Vec[Bits](arr)
  }

  def sizeToStr(size: Long): String = {
    if (size < (1 << 10)) {
      size + " B"
    } else if (size < (1 << 20)) {
      (size / (1 << 10)) + " KB"
    } else if (size < (1 << 30)) {
      (size / (1 << 20)) + " MB"
    } else {
      (size / (1 << 30)) + " GB"
    }
  }

  def printConfig(configFile: String): Unit = {
    printf("\nPatmos configuration \"%s\"\n", util.Config.getConfig.description)
    printf("\tFrequency: %d MHz\n", CLOCK_FREQ/1000000)
    printf("\tPipelines: %d\n", PIPE_COUNT)
    printf("\tMethod cache: %s, %d methods\n", sizeToStr(MCACHE_SIZE), METHOD_COUNT)
    printf("\tData cache: %s", sizeToStr(DCACHE_SIZE))
    if (DCACHE_ASSOC == 1) {
      printf(", direct-mapped")
    } else {
      printf(", %d-way set associative with %s replacement", DCACHE_ASSOC, DCACHE_REPL)
    }
    printf(", write %s\n", if (DCACHE_WRITETHROUGH) "through" else "back")
    printf("\tStack cache: %s\n",  sizeToStr(SCACHE_SIZE))
    printf("\tInstruction SPM: %s\n", sizeToStr(ISPM_SIZE))
    printf("\tData SPM: %s\n", sizeToStr(DSPM_SIZE))
    printf("\tBoot SPM: %s\n", sizeToStr(BOOTSPM_SIZE))
    printf("\tAddressable external memory: %s\n",
           sizeToStr(util.Config.getConfig.ExtMem.size))
    printf("\n")
  }
}
