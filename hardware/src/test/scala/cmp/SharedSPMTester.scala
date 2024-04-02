/*
 * Test the shared SPM, should become some generic tester for OCP devices.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 */

package cmp

import chisel3._

import patmos.Constants._
import ocp._

/*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class SharedSPMTester(dut: SharedSPM) extends Tester(dut) {

  println("Shared SPM Tester")

  def read(n: Int, addr: Int) = {
    poke(dut.io(n).M.Addr, addr << 2)
    poke(dut.io(n).M.Cmd, 2) // OcpCmd.RD
    step(1)
    poke(dut.io(n).M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io(n).S.Resp) != 1) {
      step(1)
    }
    peek(dut.io(n).S.Data)
    dut.io(n).S.Data
  }

  def write(n: Int, addr: Int, data: Int) = {
    poke(dut.io(n).M.Addr, addr << 2)
    poke(dut.io(n).M.Data, data)
    poke(dut.io(n).M.Cmd, 1) // OcpCmd.WR
    poke(dut.io(n).M.ByteEn, 0x0f)
    step(1)
    poke(dut.io(n).M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io(n).S.Resp) != 1) {
      step(1)
    }
  }
  

  for (i <- 0 until 32) {
    write(0, i, i * 0x100 + 0xa)
    write(1, i+32, i * 0x10000 + 0xb)
  }
  step(1)
  for (i <- 0 until 32) {
    expect(read(0, i), i * 0x100 + 0xa)
    expect(read(1, i+32), i * 0x10000 + 0xb) 
  }
}

object SharedSPMTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new SharedSPM(4, 1024))) {
        c => new SharedSPMTester(c)
      }
  }
}*/
