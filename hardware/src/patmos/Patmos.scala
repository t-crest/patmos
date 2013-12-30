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
 * Patmos top level component and test driver.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

import scala.collection.mutable.HashMap
import java.io.File

import Constants._

import util._
import datacache._
import ocp._

/**
 * Module for one Patmos core.
 */
class PatmosCore(binFile: String, datFile: String) extends Module {

  val io = Config.getPatmosCoreIO()

  val mcache = Module(new MCache())

  val fetch = Module(new Fetch(binFile))
  val decode = Module(new Decode())
  val execute = Module(new Execute())
  val memory = Module(new Memory())
  val writeback = Module(new WriteBack())
  val iocomp = Module(new InOut())
  val dcache = Module(new DataCache())

  //connect mcache
  mcache.io.femcache <> fetch.io.femcache
  mcache.io.mcachefe <> fetch.io.mcachefe
  mcache.io.exmcache <> execute.io.exmcache
  mcache.io.ena_out <> memory.io.ena_in
  mcache.io.ena_in <> memory.io.ena_out

  decode.io.fedec <> fetch.io.fedec
  execute.io.decex <> decode.io.decex
  decode.io.exdec <> execute.io.exdec
  memory.io.exmem <> execute.io.exmem
  writeback.io.memwb <> memory.io.memwb
  // RF write connection
  decode.io.rfWrite <> writeback.io.rfWrite

  // This is forwarding of registered result
  // Take care that it is the plain register
  execute.io.exResult <> memory.io.exResult
  execute.io.memResult <> writeback.io.memResult

  // We branch in EX
  fetch.io.exfe <> execute.io.exfe
  // We call in MEM
  fetch.io.memfe <> memory.io.memfe
  fetch.io.femem <> memory.io.femem

  memory.io.localInOut <> iocomp.io.memInOut

  // The boot memories intercept accesses before they are translated to bursts
  val bootMem = Module(new BootMem(datFile))
  memory.io.globalInOut <> bootMem.io.memInOut

  dcache.io.master <> bootMem.io.extMem

  // Merge OCP ports from data caches and method cache
  val burstBus = Module(new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
  val burstJoin = new OcpBurstJoin(mcache.io.ocp_port, dcache.io.slave,
                                   burstBus.io.slave)


  //join class for I-Cache buffering the d-cache request
  //use this burstJoin when I-Cache is used
  // val burstJoin = new OcpBurstPriorityJoin(mcache.io.ocp_port, dcache.io.slave,
  //                                  burstBus.io.slave, mcache.io.ena_out)

  // Enable signal
  val enable = memory.io.ena_out & mcache.io.ena_out
  fetch.io.ena := enable
  decode.io.ena := enable
  execute.io.ena := enable
  writeback.io.ena := enable
  val enableReg = Reg(next = enable)

  // The inputs and outputs
  io.comConf <> iocomp.io.comConf
  io.comSpm <> iocomp.io.comSpm
  io.memPort <> burstBus.io.master
  Config.connectAllIOPins(io, iocomp.io)

  // Keep signal alive for debugging
  debug(enableReg)
}

object PatmosCoreMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(3, args.length)
    val configFile = args(0)
    val binFile = args(1)
    val datFile = args(2)

	Config.conf = Config.load(configFile)
    Config.minPcWidth = (new File(binFile)).length.toInt / 4
    chiselMain(chiselArgs, () => Module(new PatmosCore(binFile, datFile)))
	// Print out the configuration
	Utility.printConfig(configFile)
  }
}

/**
 * The main (top-level) component of Patmos.
 */
class Patmos(configFile: String, binFile: String, datFile: String) extends Module {
  Config.conf = Config.load(configFile)
  Config.minPcWidth = log2Up((new File(binFile)).length.toInt / 4)

  val io = Config.getPatmosIO()

  // Instantiate core
  val core = Module(new PatmosCore(binFile, datFile))

  // Forward ports to/from core
  io.comConf <> core.io.comConf
  io.comSpm <> core.io.comSpm
  Config.connectAllIOPins(io, core.io)

  // Connect memory controller
  val ssram = Module(new SsramBurstRW(EXTMEM_ADDR_WIDTH))
  ssram.io.ocp_port <> core.io.memPort
  io.sramPins.ram_out <> ssram.io.ram_out
  io.sramPins.ram_in <> ssram.io.ram_in

  // Print out the configuration
  Utility.printConfig(configFile)
}

// this testing and main file should go into it's own folder

class PatmosTest(pat: Patmos) extends Tester(pat,
  Array(pat.io, pat.core.decode.io, pat.core.decode.rf.io, pat.core.memory.io, pat.core.execute.io)
  ) {

  defTests {
    val ret = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()

	println("Patmos start")

    for (i <- 0 until 100) {
      vars.clear
      step(vars, ovars, false) // false as third argument disables printout
      // The PC printout is a little off on a branch
      val pc = ovars(pat.core.memory.io.memwb.pc).litValue() - 2
      // println(ovars(pat.io.led).litValue())
      print(pc + " - ")
      for (j <- 0 until 32)
        print(ovars(pat.core.decode.rf.rf(UInt(j))).litValue() + " ")
      println()
      //      println("iter: " + i)
      //      println("ovars: " + ovars)
      //      println("led/litVal " + ovars(pat.io.led).litValue())
      //      println("pc: " + ovars(pat.core.fetch.io.fedec.pc).litValue())
      //      println("instr: " + ovars(pat.core.fetch.io.fedec.instr_a).litValue())
      //      println("pc decode: " + ovars(pat.core.decode.io.decex.pc).litValue())
    }
    ret
  }
}

object PatmosMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(3, args.length)
    val configFile = args(0)
    val binFile = args(1)
    val datFile = args(2)

    chiselMainTest(chiselArgs, () => Module(new Patmos(configFile, binFile, datFile))) { f => new PatmosTest(f) }
  }
}
