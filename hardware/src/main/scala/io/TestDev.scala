/*
 * I/O module with re-use of class for multiple instances of a device
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import Chisel._

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

  val dataOutReg = Reg(init = 0.U)
  val dataInReg = Reg(init = 0.U)

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Write to device
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    dataOutReg := io.ocp.M.Data(io.testDevPins.dataOut.getWidth-1, 0)
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
