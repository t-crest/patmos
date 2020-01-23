/*
 * An OCP device allowing for single core testing of the SSPM.
 *
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 * License: Simplified BSD License
 *
 * An instance of the SSPM core implemenation is used,
 * so the device behavior is identical to a multicore
 * usage.
 *
 * This implementation is suitable for emulation using
 * the C++ emulator.
 *
 */

package io

import Chisel._

import ocp._

import sspm._

object SSPM extends DeviceObject {
  var nCores = 1
  var extendedSlotSize = 5
  var singleExtendedSlot = false

  def init(params: Map[String, String]) = {
    nCores = getPosIntParam(params, "nCores")
    extendedSlotSize = getPosIntParam(params, "extendedSlotSize")
    singleExtendedSlot = getBoolParam(params, "singleExtendedSlot")
  }

  def create(params: Map[String, String]): SSPM = Module(new SSPM(nCores, extendedSlotSize, singleExtendedSlot))

  trait Pins {}
}

/**
 * A top level of SSPM
 */
class SSPM(val nCores: Int, val extendedSlotSize: Int, val singleExtendedSlot: Boolean) extends CoreDevice {

  override val io = new CoreDeviceIO()

  // Connect one OCP interface from SSPM Aegean to the Patmos OCP
  Module(new SSPMAegean(nCores, extendedSlotSize, singleExtendedSlot)).io.cores(0) <> io.ocp
}

/**
 * Test the SSPM design
 */
 /* commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class SSPMTester(dut: SSPM) extends Tester(dut) {
  def idle() = {
    poke(dut.io.ocp.M.Cmd, OcpCmd.IDLE.litValue())
    poke(dut.io.ocp.M.Addr, 0)
    poke(dut.io.ocp.M.Data, 0)
    poke(dut.io.ocp.M.ByteEn, Bits("b0000").litValue())
  }

  def wr(addr: BigInt, data: BigInt, byteEn: BigInt) = {
    poke(dut.io.ocp.M.Cmd, OcpCmd.WR.litValue())
    poke(dut.io.ocp.M.Addr, addr)
    poke(dut.io.ocp.M.Data, data)
    poke(dut.io.ocp.M.ByteEn, byteEn)
  }

  def rd(addr: BigInt, byteEn: BigInt) = {
    poke(dut.io.ocp.M.Cmd, OcpCmd.RD.litValue())
    poke(dut.io.ocp.M.Addr, addr)
    poke(dut.io.ocp.M.Data, 0)
    poke(dut.io.ocp.M.ByteEn, byteEn)
  }

  // Initial setup
  println("\nSetup initial state\n")

  idle()

  expect(dut.io.ocp.S.Resp, 0)

  // Write test
  println("\nTest write\n")
  step(1)

  wr(0xF00B0001L, 42, Bits("b1111").litValue())

  step(1)

  idle()

  println("\nStall until data valid\n")
  // Stall until data valid
  while(peek(dut.io.ocp.S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  step(1)
  expect(dut.io.ocp.S.Resp, 0)

  // Read  test
  println("\nRead test\n")
  step(1)

  rd(0xF00B0001L, Bits("b1111").litValue())

  step(1)

  idle()

  println("\nStall until data valid\n")
  // Stall until data valid
  while(peek(dut.io.ocp.S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  expect(dut.io.ocp.S.Data, 42)

  step(1)
  expect(dut.io.ocp.S.Resp, 0)

}

object SSPMTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new SSPM(3, 5, false))) {
        f => new SSPMTester(f)
      }
  }
}*/

