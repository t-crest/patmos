/*
 * Utility functions for Patmos.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * Edited: Bosse Bandowski (bosse.bandowski@outlook.com)
 *
 */

package util

import java.nio.file.{ Files, Paths }

import util._

import chisel3._
import scala.math._

import patmos.Constants._

// Borrowed from the Chisel source tree.
// Hacked to now support longs
object log2Up
{
  def apply(in: Long): Int = if(in == 1) 1 else ceil(log(in.toDouble)/log(2)).toInt
}

object Utility {

  /*
    Replacing Utility.readBin in Chisel3. Instead of reading the BOOTAPP binary into a vector,
    it is read into two Array[BigInt] objects containing the ROM instructions that are parsed to Verilog code in utils.BlackBoxRom
    and used in Fetch.scala. The two arrays represent the even and odd sections of the dual issue RAM.
  */
  def binToDualRom(fileName: String, width: Int): (Array[BigInt], Array[BigInt]) = {
    val bytesPerWord = (width+7) / 8

    println("Reading " + fileName)
    val byteArray = Files.readAllBytes(Paths.get(fileName))

    // compute ROM length (length of binary rounded up to power of two, then divided by half to match dual issue ROM)
    val numInstructions = math.max(1, byteArray.length / bytesPerWord)
    val romLen = math.pow(2, log2Up(numInstructions) - 1).toInt

    // init even and odd ROM
    val romEven = new Array[BigInt](romLen)
    val romOdd = new Array[BigInt](romLen)


    // if no BOOTAPP, then init ROM with 0
    if (byteArray.length == 0) {
      romEven(0) = BigInt(0)
      romOdd(0) = BigInt(0)
    }

    // split instructions into even and odd rom. Pad unused PC entries with 0s
    for (i <- 0 until romLen * 2) {
      var word = BigInt(0)

      if (i < numInstructions) {
        for (j <- 0 until bytesPerWord) {
          word <<= 8
          word += byteArray(i * bytesPerWord + j).toInt & 0xff
        }
      }

      if (i % 2 == 0) {
        romEven(i / 2) = word
      }
      else {
        romOdd(i / 2) = word
      }
    }

    return (romEven, romOdd)
  }

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
      arr(0) = 0.U(width.W)
    }

    for (i <- 0 until byteArray.length / bytesPerWord) {
      var word = BigInt(0)
      for (j <- 0 until bytesPerWord) {
        word <<= 8
        word += byteArray(i * bytesPerWord + j).toInt & 0xff
      }
      // printf("%08x\n", Bits(word))
      arr(i) = word.U(width.W)


    }

    // use vector to represent ROM
    VecInit[Bits](arr.toIndexedSeq)
  }

  def sizeToStr(size: Long): String = {
    if (size < (1 << 10)) {
      s"$size B"
    } else if (size < (1 << 20)) {
      s"${(size / (1 << 10))} KB"
    } else if (size < (1 << 30)) {
      s"${(size / (1 << 20))} MB"
    } else {
      s"${(size / (1 << 30))} GB"
    }
  }

  def printConfig(configFile: String): Unit = {
    var tempStr = ""
    println("\nPatmos configuration \"" + Config.getConfig.description + "\"")
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
    println("\tAddressable external memory: "+ sizeToStr(Config.getConfig.ExtMem.size))
    println("\tMMU: "+HAS_MMU)
    println("\tBurst length: "+ Config.getConfig.burstLength)
    println("")
  }
}
