/*
 * A shared scratchpad memory supporting LL/SC, with
 * time allocated arbitration.
 *
 * Author: Davide Laezza - Roberts Fanning - Wenhao Li
 */

package conc

import Chisel._

import conc._
import conc.Util._
import ocp._
import patmos.Constants._

/*
    The SharedLLSCSpm module connects a LLSCSpm to the cores via arbiters
    that use a time-slotted policy to access the underlying scratchpad memory.
*/
class SharedLLSCSpm(
    granularity :Int,
    nrCores :Int,
    size :Int,
    resultOnData :Boolean = false
) extends Module {

    // ncCores OCP slaves to connect to the cores
    val io = Vec.fill(nrCores)(new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))

    // The LL/SC scratchpad memory
    val mem = LLSCSpm(granularity, nrCores, size, resultOnData)

    /*
        The arbiters, connected each to a different core, but all to the
        output of the underlying scratchpad memory.
    */
    val arbiters = new Array[Arbiter](nrCores)
    for (i <- 0 until nrCores) {
        arbiters(i) = Arbiter(i, nrCores, 0)
        val arbiter = arbiters(i)

        arbiter.io.slave <> io(i)
        arbiter.io.master.S <> mem.io.slave.S
    }

    /*
        OR-ing the outputs of the arbiters to the scratchpad memory, since
        time-slot arbitration ensures that only one of them is using the
        outputs at a given time
    */
    mem.io.slave.M <> orAllOcpMaster(arbiters.map(_.io.master.M))
    mem.io.core := orAll(arbiters.map(_.io.core))
}

object SharedLLSCSpm {
    def apply(granularity :Int, nrCores: Int, size: Int, resultOnData :Boolean = false) = {
        Module(new SharedLLSCSpm(granularity, nrCores, size, resultOnData))
    }
}

class SharedLLSCSpmTester(dut: SharedLLSCSpm, resultOnData :Boolean = false) extends Tester(dut) {
    import SharedLLSCSpmTester._

  println("Shared LL/SC SPM Tester")

  def read(n: Int, addr: Int) = {
    poke(dut.io(n).M.Addr, addr)
    poke(dut.io(n).M.Cmd, 2) // OcpCmd.RD
    step(1)
    poke(dut.io(n).M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io(n).S.Resp) != 1) {
      step(1)
    }
    peek(dut.io(n).S.Data)
    dut.io(n).S.Data
  }

  // This is the write for when the results have to be read from
  // the result register
  def writeNoResult(n: Int, addr: Int, data: Int) = {
    poke(dut.io(n).M.Addr, addr)
    poke(dut.io(n).M.Data, data)
    poke(dut.io(n).M.Cmd, 1) // OcpCmd.WR
    poke(dut.io(n).M.ByteEn, 0x0f)
    step(1)
    poke(dut.io(n).M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io(n).S.Resp) != 1) {
      step(1)
    }
    read(n, SIZE)
  }

  // This is the write for when the results are returned on the data wires
  def writeResult(n: Int, addr: Int, data: Int) = {
    poke(dut.io(n).M.Addr, addr)
    poke(dut.io(n).M.Data, data)
    poke(dut.io(n).M.Cmd, 1) // OcpCmd.WR
    poke(dut.io(n).M.ByteEn, 0x0f)
    step(1)
    poke(dut.io(n).M.Cmd, 0) // OcpCmd.IDLE
    while (peek(dut.io(n).S.Resp) != 1) {
      step(1)
    }
    peek(dut.io(n).S.Data)
    dut.io(n).S.Data
  }

  // Choosing the right write based on the parameter
  val write = if (resultOnData) writeResult _ else writeNoResult _

  // Basic read-write test

  for (i <- 0.until(SIZE, GRANULARITY)) {
    expect(write(0, i, i * 0x100 + 0xa), 0)
  }
  step(1)
  for (i <- 0.until(SIZE, GRANULARITY)) {
    expect(read(0, i), i * 0x100 + 0xa)
  }

  // LL/SC tests

  // Write to the same memory location without reading in between should fail
  expect(write(0, 0, 0xFF), 0)
  expect(write(0, 0, 0xF0), 1)

  // Failed writes should not affect memory
  expect(read(0, 0), 0xFF)

  // Write to the same memory location with a read from the same core in
  expect(write(0, 0, 0xFF), 0)
  expect(read(0, 0), 0xFF)
  expect(write(0, 0, 0xF0), 0)

  // Write to memory in the same granularity block should fail
  if (GRANULARITY != 1) {
      expect(write(0, GRANULARITY * 9, 0xFF), 0)
      expect(write(0, GRANULARITY * 9 + 1, 0xF0), 1)
  }

  // Write to memory in different granularity blocks should succeed
  expect(write(0, GRANULARITY * 5, 0xFF), 0)
  expect(write(0, GRANULARITY * 3, 0xF0), 0)

  // Write frrom a different core should invalidate subsequent writes
  // from other cores
  read(0, GRANULARITY * 8)
  read(1, GRANULARITY * 8)
  expect(write(1, GRANULARITY * 8, 0xF0), 0)
  expect(write(0, GRANULARITY * 8, 0xFF), 1)

  // The first write after a read shoudl always succeed
  read(0, GRANULARITY * 8)
  read(1, GRANULARITY * 8)
  expect(write(1, GRANULARITY * 8, 0xF0), 0)
  expect(read(0, GRANULARITY * 8), 0xF0)
  expect(write(0, GRANULARITY * 8, 0xFF), 0)

  // Unlike CAS, LL/SC doesn't suffer the ABA problem
  val aValue = 0x19
  val bValue = 0x84

  read(0, GRANULARITY * 8)                     // So that next write doesn't fail
  expect(write(0, GRANULARITY * 8, aValue), 0) // a value is written by core 0
  expect(read(0, GRANULARITY * 8), aValue)     // a value is read

  expect(read(1, GRANULARITY * 8), aValue)     // So that next write doesn't fail
  expect(write(1, GRANULARITY * 8, bValue), 0) // value b is written by core 1
  expect(read(1, GRANULARITY * 8), bValue)     // So that next write doesn't fail
  expect(write(1, GRANULARITY * 8, aValue), 0) // vaue a is written again by core 1

  expect(write(0, GRANULARITY * 8, 0xFF), 1)   // Store Conditional fails
}

object SharedLLSCSpmTester {
    val GRANULARITY = 32
    val SIZE = 1024

    def main(args: Array[String]): Unit = {

        // Write result has to be read from the result register
        chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
                "--compile", "--vcd", "--targetDir", "generated"),
            () => SharedLLSCSpm(GRANULARITY, 4, SIZE)) {
                c => new SharedLLSCSpmTester(c)
        }

        // Write result is returned on data wires
        chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
                "--compile", "--vcd", "--targetDir", "generated"),
            () => SharedLLSCSpm(GRANULARITY, 4, SIZE, true)) {
                c => new SharedLLSCSpmTester(c, true)
        }
    }
}
