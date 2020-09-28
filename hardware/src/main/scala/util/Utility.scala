/*
 * Utility functions for Patmos.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package util

import java.nio.file.{ Files, Paths }

import Chisel._
import scala.math._

import patmos.Constants._

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
    val byteArray = Files.readAllBytes(Paths.get(fileName))

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
    var tempStr = ""
    println("\nPatmos configuration \"" + util.Config.getConfig.description + "\"")
    println("\tFrequency: "+ (CLOCK_FREQ/1000000).toString +" MHz")
    println("\tPipelines: "+ PIPE_COUNT.toString)
    println("\tCores: "+ Config.getConfig.coreCount.toString)
    if (ICACHE_TYPE == ICACHE_TYPE_METHOD) {
      println("\tMethod cache: "+ sizeToStr(ICACHE_SIZE) +", "+ ICACHE_ASSOC.toString +" methods")
    } else {
      tempStr = "\tInstruction cache: "+ sizeToStr(ICACHE_SIZE)
      
      if (ICACHE_ASSOC == 1) {
        println(tempStr + ", direct-mapped")
      } else {
        println(tempStr + ", "+ ICACHE_ASSOC.toString +"-way set associative with %s replacement")
      }
    }
    tempStr = "\tData cache: "+ sizeToStr(DCACHE_SIZE)
    if (DCACHE_ASSOC == 1) {
      tempStr = tempStr + ", direct-mapped"
    } else {
      tempStr = tempStr + ", "+ DCACHE_ASSOC.toString +"-way set associative with "+ DCACHE_REPL +" replacement"
    }
    if (DCACHE_WRITETHROUGH){
      println(tempStr + ", write through")
    }else{
      println(tempStr + ", write back")
    }
    println("\tStack cache: "+ sizeToStr(SCACHE_SIZE))
    println("\tInstruction SPM: "+ sizeToStr(ISPM_SIZE))
    println("\tData SPM: "+ sizeToStr(DSPM_SIZE))
    println("\tAddressable external memory: "+ sizeToStr(util.Config.getConfig.ExtMem.size))
    println("\tMMU: "+HAS_MMU)
    println("\tBurst length: "+ util.Config.getConfig.burstLength)
    println("")
  }
}
