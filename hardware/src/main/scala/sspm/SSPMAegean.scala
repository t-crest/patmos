/*
   Copyright 2017 Technical University of Denmark, DTU Compute.
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
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 *         Andreas Toftegaard Kristensen (s144026@student.dtu.dk)
 *
 * The core for a shared scratchpad memory.
 */

package sspm

import Chisel._
import Node._

import patmos.Constants._

import ocp._

import scala.util.Try

/**
 * A top level of SSPMAegean
 */
class SSPMAegean(val nCores: Int,
                           val extendedSlotSize: Int,
                           val singleExtendedSlot: Boolean) extends Module {

  //override val io = new CoreDeviceIO()

  val io = Vec.fill(nCores) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) }

  // Generate modules
  val mem = Module(new memSPM(16384))
  val connectors = Vec.fill(nCores) { Module(new SSPMConnector()).io }

  val firstCore = 0
  val nextCore = Reg(init = UInt(firstCore + 1, log2Up(nCores)))
  val currentCore = Reg(init = UInt(firstCore, log2Up(nCores)))
  val decoder = UIntToOH(currentCore, nCores)

  // Connect the SSPMConnector with the SSPMAegean
  for (j <- 0 until nCores) {
      connectors(j).ocp.M <> io(j).M
      connectors(j).ocp.S <> io(j).S
      connectors(j).connectorSignals.S.Data := mem.io.S.Data

    // Enable connectors based upon one-hot coding of scheduler
    connectors(j).connectorSignals.enable := Bits(0)
  }

  mem.io.M.Data := connectors(currentCore).connectorSignals.M.Data
  mem.io.M.Addr := connectors(currentCore).connectorSignals.M.Addr
  mem.io.M.ByteEn := connectors(currentCore).connectorSignals.M.ByteEn
  mem.io.M.We := connectors(currentCore).connectorSignals.M.We

  // Synchronization state machine

  val s_idle :: s_sync :: Nil = Enum(UInt(), 2)

  val state = Reg(init = s_idle)
  val syncCounter = Reg(init = UInt(0))
  syncCounter := syncCounter

  val syncUsed = Reg(init = Bool(false))
  val syncCore = Reg(init = UInt(0))

  when(state === s_idle) {
    state := s_idle
    nextCore := nextCore + UInt(1)
    currentCore := nextCore
    connectors(currentCore).connectorSignals.enable := Bits(1)

    when(connectors(currentCore).connectorSignals.syncReq === Bits(1)) {
      if(singleExtendedSlot) {
        when(!syncUsed) {
          syncCounter := UInt(extendedSlotSize - 1)
          nextCore := nextCore
          currentCore := currentCore
          state := s_sync
        }.otherwise {
          connectors(currentCore).connectorSignals.enable := Bits(0)
        }
      } else {
        syncCounter := UInt(extendedSlotSize - 1)
        nextCore := nextCore
        currentCore := currentCore
        state := s_sync
      }
    }

    when(nextCore > UInt(nCores - 1)) {
      nextCore := UInt(0)
    }

    if(singleExtendedSlot) {
      when(currentCore === syncCore) {
        syncUsed := Bool(false)
      }
    }
  }

  when(state === s_sync) {

    syncCounter := syncCounter - UInt(1)
    if(singleExtendedSlot) {
      syncUsed := Bool(true)
      syncCore := currentCore
    }
    connectors(currentCore).connectorSignals.enable := Bits(1)

    state := s_sync

    when(syncCounter === UInt(0)) {
      nextCore := nextCore + UInt(1)
      currentCore := nextCore
      state := s_idle
    }
  }

}

// Generate the Verilog code by invoking chiselMain() in our main()
object SSPMAegeanMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPMAegean hardware")

    val chiselArgs = args.slice(1, args.length)
    val nCores = args(0)
    val extendedSlotSize = 5 // args(1)
    val singleExtendedSlot = "true" // args(2)
    chiselArgs.foreach(println)

    chiselMain(chiselArgs, () => Module(new SSPMAegean(
        nCores.toInt,
        extendedSlotSize.toInt,
        Try(singleExtendedSlot.toBoolean).getOrElse(false))))
  }
}


/**
 * Test the SSPMAegean design
 */
class SSPMAegeanTester(dut: SSPMAegean, size: Int) extends Tester(dut) {

  // Set CPU core idle
  // It is important that this is done if
  // you want to work with the same address later on
  def idle(core: Int) = {
    poke(dut.io(core).M.Cmd, OcpCmd.IDLE.litValue())
    poke(dut.io(core).M.Addr, 0)
    poke(dut.io(core).M.Data, 0)
    poke(dut.io(core).M.ByteEn, Bits("b0000").litValue())
  }

  // Simulate a write instruction from Patmos
  def wr(addr: BigInt, data: BigInt, byteEn: BigInt, core: Int) = {
    poke(dut.io(core).M.Cmd, OcpCmd.WR.litValue())
    poke(dut.io(core).M.Addr, 0xF00B0000L + addr)
    poke(dut.io(core).M.Data, data)
    poke(dut.io(core).M.ByteEn, byteEn)
  }

  // Simulate a read instruction from Patmos
  def rd(addr: BigInt, byteEn: BigInt, core: Int) = {
    poke(dut.io(core).M.Cmd, OcpCmd.RD.litValue())
    poke(dut.io(core).M.Addr, 0xF00B0000L + addr)
    poke(dut.io(core).M.Data, 0)
    poke(dut.io(core).M.ByteEn, byteEn)
  }

  // Check wires to shared scratch-pad memory
  def mem() = {
    peek(dut.mem.io.M.Data)
    peek(dut.mem.io.M.Addr)
    peek(dut.mem.io.M.We)
    peek(dut.mem.io.S.Data)
  }

  // Simulate a synchronization request from Patmos
  def sync(core: Int) = {
    rd(0xFFFF, 1, core)
  }

  // Initial setup, all cores set to idle

  println("\nSetup initial state\n")

  for(i <- 0 until size){
  	idle(i)
  }

  step(1)

  for(i <- 0 until size){
  	expect(dut.io(i).S.Resp, 0)
  }

  // Write test, write from core i to memory location,
  // each core only writes once the previous core has read
  // its value back

  println("\nTest write\n")

  for(i <- 0 until size){

    // Write

	  wr((i+1)*4, i+1, Bits("b1111").litValue(), i)

    step(1)

	  while(peek(dut.io(i).S.Resp) != OcpResp.DVA.litValue()) {
	    step(1)
	  }

    // Request to read back the data to determine if correct

    rd((i+1)*4, Bits("b1111").litValue(), i)

    step(1)

    while(peek(dut.io(i).S.Resp) != OcpResp.DVA.litValue()) {
      step(1)
    }

    expect(dut.io(i).S.Data, i+1)

    idle(i)
  }

  // Read test

  println("\nRead test\n")

  for(i <- 0 until size){

	  rd((i+1)*4, Bits("b1111").litValue(), i)

    step(1)

  	// Stall until data valid

	  while(peek(dut.io(i).S.Resp) != OcpResp.DVA.litValue()) {
	    step(1)
	  }

    expect(dut.io(i).S.Data, i+1)

    idle(i)
  }

  // Test for expected fails
  // byte writes uses byte enable and not address
  // so writing to address 5 should overwrite address 4 data

  println("\nTest for expected overwrite\n")

  wr(4, 1, Bits("b1111").litValue(), 0)

  step(1)

  idle(0)

  while(peek(dut.io(0).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  wr(5, 3, Bits("b1111").litValue(), 1)

  step(1)

  idle(1)

  while(peek(dut.io(1).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  rd(4, Bits("b1111").litValue(), 0)

  step(1)

  idle(0)

  // Stall until data valid
  println("\nStall until data valid\n")

  while(peek(dut.io(0).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  expect(dut.io(0).S.Data, 3)

  step(1)

  expect(dut.io(0).S.Resp, 0)

  step(1)

  for(i <- 0 until size){
    idle(i)
  }

  // We just wait long enough such that core 1 gets its response

  step(1)

  // Have multiple cores write at the same time and then reading
  // They should then be allowed to read once they have a response

  println("\nTest with multiple cores\n")

  var rdResp = 0
  var currentCore = 0
  var prevCore = 0
  var wrRespCores: Array[Int] =  Array[Int](0, 0, 0, 0)

  wr(4, 2, Bits("b1111").litValue(), 0)
  wr(8, 3, Bits("b1111").litValue(), 1)
  wr(12, 4, Bits("b1111").litValue(), 2)
  wr(16, 5, Bits("b1111").litValue(), 3)

  currentCore = peek(dut.currentCore).toInt

  step(1)

  prevCore = currentCore

  for(i <- 0 until size){
    if(i != prevCore){
      idle(i)
    }
  }

  currentCore = peek(dut.currentCore).toInt

  while(rdResp != size) {

    if(peek(dut.io(prevCore).S.Resp) == OcpResp.DVA.litValue() && wrRespCores(prevCore) == 0){

      // Receive response for write, now read
      rd((prevCore+1)*4, Bits("b1111").litValue(), prevCore)
      wrRespCores(prevCore) = 1

    } else if (peek(dut.io(prevCore).S.Resp) == OcpResp.DVA.litValue() && wrRespCores(prevCore) == 1) {

      // check read
      rdResp = rdResp + 1
      expect(dut.io(prevCore).S.Data, prevCore + 2)
      idle(prevCore)
    }

    step(1)

    prevCore = currentCore
    currentCore = peek(dut.currentCore).toInt

    if(peek(dut.io(prevCore).S.Resp) == OcpResp.NULL.litValue()) {
      idle(prevCore)
    }


  }

  step(1)

  // Synchronization

  println("\nSynchronization\n")

  step(1)
  sync(0)

  step(1)
  idle(0)

  while(peek(dut.io(0).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  expect(dut.currentCore, 0)

  rd(4, Bits("b1111").litValue(), 0)

  step(1)
  idle(0)

  while(peek(dut.io(0).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  expect(dut.currentCore, 0)
  expect(dut.io(0).S.Resp, OcpResp.DVA.litValue())

  wr(4, 1, Bits("b1111").litValue(), 0)

  step(1)

  idle(0)

  while(peek(dut.io(0).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  expect(dut.currentCore, 0)
  expect(dut.io(0).S.Resp, OcpResp.DVA.litValue())

  step(1)

  // Request synchronization aligned exactly with the scheduler
  println("\nRequest synchronization aligned exactly with the scheduler\n")

  while(peek(dut.currentCore) == 0) {
    step(1)
  }

  while(peek(dut.nextCore) != 0) {
    step(1)
  }

  sync(0)
  peek(dut.connectors(peek(dut.nextCore).toInt).connectorSignals.syncReq)

  step(1)

  expect(dut.connectors(0).connectorSignals.syncReq, 1)

  idle(0)

  while(peek(dut.io(0).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  rd(4, Bits("b1111").litValue(), 0)

  step(1)
  idle(0)

  step(1)
  expect(dut.io(0).S.Resp, OcpResp.DVA.litValue())

  step(1)

  // Prevent double synchronization
  println("\nPrevent double synchronization\n")

  while(peek(dut.currentCore) == 0) {
    step(1)
  }
  step(1)

  sync(0)

  step(1)
  idle(0)

  while(peek(dut.io(0).S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  sync(0)

  step(1)
  idle(0)

  while(peek(dut.currentCore) == 0) {
    step(1)
    expect(dut.io(0).S.Resp, OcpResp.NULL.litValue())
  }

  while(peek(dut.currentCore) != 0) {
    step(1)
    expect(dut.io(0).S.Resp, OcpResp.NULL.litValue())
  }

  step(1)

  while(peek(dut.currentCore) != 0) {
    step(1)
    expect(dut.io(0).S.Resp, OcpResp.NULL.litValue())
  }

  while(peek(dut.io(0).S.Resp) == 0) { step(1) }

  expect(dut.io(0).S.Resp, OcpResp.DVA.litValue())

  // Request synchronization during another reserved period
  println("\nRequest synchronization during another reserved period\n")

  while(peek(dut.currentCore) == 0) {
    step(1)
  }

  step(1)

  sync(0)
  sync(1)
  sync(2)
  sync(3)

  step(1)
  idle(0)
  idle(1)
  idle(2)
  idle(3)

  var outstanding = size
  while(outstanding > 0) {
    for(i <- 0 until size){
      if(peek(dut.io(i).S.Resp).toInt == OcpResp.DVA.litValue()) {
        outstanding = outstanding - 1
      }
    }

    step(1)
  }

  var curCore = peek(dut.currentCore)
  while(peek(dut.currentCore) == curCore) {
    step(1)
  }

  step(1)

  {
    for(i <- 0 until size){
      sync(i)
    }
    step(1)
    for(i <- 0 until size){
      idle(i)
    }

    var outstanding = Array.fill(size) { false }
    while(!outstanding.forall((T) => T)) {
      for(i <- 0 until size){
        if(!outstanding(i) && peek(dut.io(i).S.Resp).toInt == OcpResp.DVA.litValue()) {
          outstanding(i) = true
          wr(4, i, Bits("b1111").litValue(), i)
        }
      }

      step(1)
      for(i <- 0 until size){
        idle(i)
      }
    }

    val curCore = peek(dut.currentCore)
    while(peek(dut.currentCore) == curCore) {
      step(1)
    }

    step(1)
  }
}

object SSPMAegeanTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPMAegean")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated", "--vcd"),
      () => Module(new SSPMAegean(4, 5, true))) {
        f => new SSPMAegeanTester(f, 4)
      }
  }
}

