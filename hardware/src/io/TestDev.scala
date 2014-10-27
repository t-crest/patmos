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
 * I/O module with re-use of class for multiple instances of a device
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import Chisel._
import Node._

import ocp._

// The pins of the device
class TestDevPins(width : Int) extends Bundle() {
  val dataIn = Bits(INPUT, width)
  val dataOut = Bits(OUTPUT, width)
}

// A common pin trait for all instances
trait TestDevPinTrait {
  val testDevPins : TestDevPins
}

// A common class for the companion objects
abstract class TestDevObject extends DeviceObject {

  // The usual "init" and "create" functions
  def init(params: Map[String, String]) = {
  }
  def create(params: Map[String, String]) : TestDev = {
    Module(new TestDev(this))
  }

  // Get instance of actual pin type
  def getPins() : CoreDeviceIO with TestDevPinTrait
}

// First instance of TestDev
object TestDevA extends TestDevObject {

  trait Pins extends TestDevPinTrait {
    // The actual pins for the device, following naming convention
    val testDevAPins = new TestDevPins(3)
    // Alias generic and actual pins
    override val testDevPins = testDevAPins
  }

  // Get instance of actual pin type
  def getPins() = new CoreDeviceIO() with Pins
}

// Second instance of TestDev
object TestDevB extends TestDevObject {

  trait Pins extends TestDevPinTrait {
    // The actual pins for the device, following naming convention
    val testDevBPins = new TestDevPins(4)
    // Alias generic and actual pins
    override val testDevPins = testDevBPins
  }

  // Get instance of actual pin type
  def getPins() = new CoreDeviceIO() with Pins
}

class TestDev(obj : TestDevObject) extends CoreDevice() {

  // Get the actual pins, depending on who is doing the instantiation
  override val io = obj.getPins()

  val dataOutReg = Reg(init = Bits(0))
  val dataInReg = Reg(init = Bits(0))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Write to device
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    dataOutReg := io.ocp.M.Data(io.testDevPins.dataOut.width-1, 0)
  }

  // Read current state of device
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataInReg

  // Connections to pins
  dataInReg := io.testDevPins.dataIn
  io.testDevPins.dataOut := dataOutReg
}
